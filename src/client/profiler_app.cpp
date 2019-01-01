/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include <psapi.h>

#include "cstdmf/memory_tracker.hpp"
#include "app.hpp"
#include "profiler_app.hpp"
#include "pyscript/personality.hpp"
#include "filter.hpp" 
#include "player.hpp" 

static const float DAMPING_RATIO = 0.97f;

Matrix * g_profilerAppMatrix;

// -----------------------------------------------------------------------------
// Section ProfilerApp
// -----------------------------------------------------------------------------

ProfilerApp ProfilerApp::instance;
int ProfilerApp_token;


ProfilerApp::ProfilerApp() :
	cpuStall_( 0.f ),
	dTime_( 0.f ),
	filteredDTime_( 0.f ),
	fps_( 0.f ),
	filteredFps_( 0.f )
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Profiler/App", NULL );
}


ProfilerApp::~ProfilerApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Profiler/App" );*/
}


bool ProfilerApp::init()
{
	BW_GUARD;

	// Set up watches
	MF_WATCH( "Render/Performance/FPS(filtered)",
		filteredFps_,
		Watcher::WT_READ_ONLY,
		"Filtered FPS." );
	MF_WATCH( "Render/Performance/Frame Time (filtered)",
		filteredDTime_,
		Watcher::WT_READ_ONLY,
		"Filtered frame time." );
	MF_WATCH( "Render/Performance/CPU Stall",
		cpuStall_,
		Watcher::WT_READ_WRITE,
		"Stall CPU by this much each frame to see if we are GPU bound." );

#ifdef ENABLE_MEMTRACKER
#if ENABLE_WATCHERS
	MemTracker::addWatchers();
#endif
#endif

	return true;
}


void ProfilerApp::fini()
{
	BW_GUARD;	
}


void ProfilerApp::tick( float dTime )
{
	BW_GUARD;
	// Gather stats
	dTime_ = dTime;
	filteredDTime_ = ( DAMPING_RATIO * filteredDTime_ ) + ( ( 1.f - DAMPING_RATIO ) * dTime );

	double cpuFreq = stampsPerSecondD() / 1000.0;
	if (cpuStall_ > 0.f )
	{
		uint64 cur = timestamp();
		uint64 end = cur + ( uint64 )( cpuStall_ * cpuFreq );
		do
		{
			cur = timestamp();
		} while ( cur < end );
	}

	if ( dTime > 0.f )
	{
		fps_ = 1.f / dTime;
		filteredFps_ = ( DAMPING_RATIO * filteredFps_ ) + ( ( 1.f - DAMPING_RATIO ) * 1.f / dTime );
	}

	// Update profiler and camera
	updateProfiler( dTime );
	updateCamera( dTime );
}


void ProfilerApp::draw()
{
	BW_GUARD;	
}


// -----------------------------------------------------------------------------
// Section Profile
// -----------------------------------------------------------------------------

static PyObject* s_firstCameraNode = NULL;
static bool s_resetFlyThrough = false;
static double s_soakTime;
static char s_csvFilename[256];
static FILE * s_csvFile = NULL;

static std::vector<float> s_frameTimes;
static float s_frameTimesTotal = 0.0f;

static char s_firstCameraNodeName[80];
static bool s_fixedFrameTime = false;
static bool s_soakTest = false;
static bool s_loop = false;
static float s_speed = 20.f;
static bool s_queueProfile = false;
static int s_state = -1;

static const float FRAME_STEP = 0.1f;
static const float CAMERA_HEIGHT = 0.0f;
static const double SOAK_SETTLE_TIME = 60.0 * 20.0;
static const double VALIDATE_TIME = 5.0;

static int  s_profilerNumRuns = 2;
static char s_profilerOutputRoot[MAX_PATH];
static bool s_profilerExitOnComplete = true;

//The original location and fall value for the entity are reverted when
//fly through ends
static bool s_setValue = false;
static Position3D s_origPos;
static Position3D s_origDir;
static bool s_origFall;

/**
 *
 */
static PyObject* findCameraNode( const char* nodeName )
{
	BW_GUARD;
	PyObject* pBigWorld = PyImport_AddModule( "BigWorld" );

	// Find the first camera node
	PyObject* pDict = PyObject_GetAttrString( pBigWorld, "userDataObjects" );
	PyObject* pUdoList = PyMapping_Values( pDict );

	PyObject* pIter = PyObject_GetIter( pUdoList );
	PyObject* pCameraNode = NULL;

	char namedNode[256] = "";

	while ( PyObject* pUdo = PyIter_Next( pIter ) )
	{
		//If we don't have a name move on to the next
		if (!PyObject_HasAttrString( pUdo, "name" )) continue;
		
		PyObject* pName = PyObject_GetAttrString( pUdo, "name" );
		
		if (pName == NULL) continue;
		
		if ( strcmp( PyString_AsString( pName ), "" ) != 0 )
		{
			strncpy( namedNode, PyString_AsString( pName ), 255 );
			namedNode[255] = 0;
		}

		if ( strcmp( nodeName, PyString_AsString( pName ) ) == 0 )
		{
			pCameraNode = pUdo;		// This will DECREF later
			Py_DECREF( pName );
			break;
		}
		
		Py_DECREF( pName );
		Py_DECREF( pUdo );
	}
	
	Py_DECREF( pIter );
	Py_DECREF( pUdoList );
	Py_DECREF( pDict );

	if (pCameraNode == NULL)
	{
		char msg[256];
		sprintf( msg, "Unable to locate camera node [%s], did you mean [%s]?", nodeName, namedNode );
		PyErr_SetString( PyExc_StandardError, msg );
	}
	
	return pCameraNode;
}


static void revertPlayerLocation()
{
	//revert the player filter
	if (  Player::entity() && Player::entity()->pPhysics() && 
		s_setValue) 
	{
		Player::entity()->pPhysics()->teleport(s_origPos,s_origDir);
		Player::entity()->pPhysics()->fall(s_origFall);
	}	
	else 
	{
		WARNING_MSG("profiler_app.cpp Can't revert player location\n");
	}
}


/**
 *	Sets up a fly-through. Returns false if a Python error occurred while 
 *	finding the camera node name.
 *
 *	@param cameraNode
 *	@param fixedFrameTime
 *	@param soakTest
 *	@param loop
 */
static bool setupFlyThrough( const char * cameraNodeName, bool fixedFrameTime, 
		bool soakTest, bool loop )
{
	BW_GUARD;
	PyObject* cameraNode = findCameraNode( cameraNodeName );
	if (cameraNode)
	{
		s_firstCameraNode = cameraNode;
		strncpy(s_firstCameraNodeName,
			cameraNodeName,
			sizeof( s_firstCameraNodeName ) );
		s_firstCameraNodeName[sizeof( s_firstCameraNodeName ) - 1] = 0;

		s_fixedFrameTime = fixedFrameTime;
		s_soakTest = soakTest;
		s_loop = loop;
		//Store the original position
		s_setValue = false;
		if ( Player::entity() && Player::entity()->pPhysics()) 
		{
			s_setValue = true;
			s_origPos = Player::entity()->position();
			s_origDir[0] = Player::entity()->auxVolatile()[0];
			s_origDir[1] = Player::entity()->auxVolatile()[1];
			s_origDir[2] = Player::entity()->auxVolatile()[2];
			s_origFall = Player::entity()->pPhysics()->fall();
			//set fall to false so player will not fall during fly through
			Player::entity()->pPhysics()->fall(false);
		}	
	}
	return (cameraNode != NULL);
}

/**
 *	Run the fly through. 
 *	This method returns false if a Python error occurred.
 *
 *	@param cameraNodeName
 *	@param loop
 */
static bool runFlyThrough( const char * cameraNodeName, bool loop )
{
	BW_GUARD;
	if (setupFlyThrough( cameraNodeName, false, false, loop ))
	{
		s_resetFlyThrough = true;
		return true;
	}
	return false;
}


/**
 *
 */
static void queueProfile( char * cameraNodeName, bool fixedFrameTime, bool soakTest )
{
	BW_GUARD;
	if (setupFlyThrough( cameraNodeName, fixedFrameTime, soakTest, false ))
	{
		s_queueProfile = true;
	}
}


/**
 *	Return the averge time per frame and the frame standard deviation
 */
static void calculatePerformance( float & frameTimesAvg, 
								 float & frameTimesStdDev, 
								 float& frameTimeDifferences,
								 float& frameTimePow2Differences )
{
	frameTimesAvg = s_frameTimesTotal / float( s_frameTimes.size() );
	frameTimesStdDev = 0.0f;
	frameTimeDifferences = 0.0;
	frameTimePow2Differences = 0.0;

	for ( uint i = 0; i < s_frameTimes.size(); i++ )
	{
		float dev = s_frameTimes[i] - frameTimesAvg;
		frameTimesStdDev += dev * dev;
		if (i > 0)
		{
			float diff = (s_frameTimes[i] - s_frameTimes[i-1]);
			frameTimeDifferences += abs(diff);
			frameTimePow2Differences += (diff * diff * 
				/*just to make the numbers more significant */ 100);
		}
	}

	frameTimesStdDev = sqrt( frameTimesStdDev / float( s_frameTimes.size() ) );
}

static void reportCSVStats(FILE * file, double elapsedTime )
{
	BW_GUARD;	
#ifdef ENABLE_MEMTRACKER
	// Get memory stats
	MemTracker::AllocStats curStats;
	MemTracker::instance().readStats( curStats );

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) );

	// Calculate performance stats
	float frameTimesAvg;
	float frameTimesStdDev;
	float frameTimeDifferences;
    float frameTimePow2Differences;
	calculatePerformance( frameTimesAvg, frameTimesStdDev, frameTimeDifferences, frameTimePow2Differences );

	// Print stats for that block
	fprintf( file, "%g, %d, %d, %d, %d, %f, %f, %f, %f\n", elapsedTime,
		curStats.curBytes_, curStats.curBlocks_,
		pmc.PagefileUsage, s_frameTimes.size(), s_frameTimesTotal * 1000.0f,
		frameTimesAvg * 1000.0f, frameTimesStdDev * 1000.0f, 1.0f / frameTimesAvg );

#endif
}


/**
 *
 */
void ProfilerApp::updateProfiler( float dTime )
{
	BW_GUARD;
	static time_t startTime;
	static double prevValidateTime;
#ifdef ENABLE_MEMTRACKER
	static MemTracker::AllocStats settledStats;
#endif
	static bool settled = false;
	static int numSoakFrames;

	//If we have a profiling job to do
	if ( s_queueProfile )
	{
		if ( s_firstCameraNode = findCameraNode( s_firstCameraNodeName ) )
		{
			s_state = 0;
			time( &startTime );
			numSoakFrames = 0;
			if(s_soakTest)
			{
				s_csvFile = fopen(s_csvFilename, "w");
				if(s_csvFile)
				{
					fprintf( s_csvFile,
						"Time Stamp, Bytes, Blocks, Proc Bytes, Num Frames, Time, Frame Time Avg, Frame Time Std Dev, FPS Avg\n" );
					DEBUG_MSG("created csv file: %s.\n", s_csvFilename);
				}
				else
					DEBUG_MSG("could not create csv file: %s.\n", s_csvFilename);
			}
		}
		else
		{
			DEBUG_MSG( "Unable to find camera node [%s]\n", s_firstCameraNodeName );
		}
		s_queueProfile = false;
	}

	if ( s_state >= 0 )
	{
		time_t curTime;
		time( &curTime );
		double elapsedTime = difftime( curTime, startTime );
		numSoakFrames++;

		// Store frame time
		s_frameTimes.push_back( dTime );
		s_frameTimesTotal += dTime;

		if ( s_soakTest )
		{
			// Process soak test update
			if ( ( elapsedTime - prevValidateTime ) > VALIDATE_TIME )
			{
#ifdef ENABLE_MEMTRACKER
				MemTracker::instance().reportStats();

				// report stats to csv file
				if(s_csvFile)
				{
					reportCSVStats( s_csvFile, elapsedTime );
				}

				// Reset frame times
				s_frameTimes.clear();
				s_frameTimesTotal = 0.0f;

				prevValidateTime = elapsedTime;

				if ( ( elapsedTime >= SOAK_SETTLE_TIME ) && !settled )
				{
					settled = true;
					MemTracker::instance().readStats( settledStats );
					DEBUG_MSG( "Soak settle period complete\n" );
				}
#endif

				if ( elapsedTime >= s_soakTime )
				{
					if (s_csvFile)
					{
						fclose(s_csvFile);
						s_csvFile = NULL;
					}

					float secondsPerFrame = ( float )elapsedTime / ( float )numSoakFrames;
					float framesPerSecond = 1.0f / secondsPerFrame;

					DEBUG_MSG( "Soak Test Done\n" );
					DEBUG_MSG( "%d frames, averaging %f ms per frame (%f frames per second)\n",
						numSoakFrames, 1000.0f * secondsPerFrame, framesPerSecond );

#ifdef ENABLE_MEMTRACKER
					// Print leak error if we have > 110% of settled
					MemTracker::AllocStats curStats;
					MemTracker::instance().readStats( curStats );

/* TODO: Get the approximate peak based on our sampling from the app and put this back

					if ( settled && ( 10 * curStats.peakBytes_ ) > ( 11 * settledStats.peakBytes_ ) )
					{
						WARNING_MSG( "Too much memory has been leaked\n" );
					}
*/					 
					ListStringFloat sentList;
					sentList.push_back(PairStringFloat("Total Frames", (float)numSoakFrames));
					sentList.push_back(PairStringFloat("Mili per frame", 1000.0f * secondsPerFrame));
					sentList.push_back(PairStringFloat("FPS", (float)framesPerSecond));
					profilerFinished(sentList);
					App::instance().quit();
#endif
					return;
				}
			}

			if ( !g_profilerAppMatrix )
			{
				s_resetFlyThrough = true;
				prevValidateTime = elapsedTime;
			}
		}
		else
		{
			// Process profiler update
			if ( !g_profilerAppMatrix )
			{
				// If we've done a run, report
				if ( s_state > 0 )
				{
					float frameTimesAvg;
					float frameTimesStdDev;
					float frameTimeDifferences;
					float frameTimePow2Differences;
					calculatePerformance( frameTimesAvg, frameTimesStdDev, frameTimeDifferences, frameTimePow2Differences );

					DEBUG_MSG( "Profile run %d done\n", s_state );
					DEBUG_MSG( "%d frames, averaging %f ms per frame (%f frames per second)\n",
						s_frameTimes.size(), 1000.0f * frameTimesAvg, 1.0f / frameTimesAvg );
					DEBUG_MSG( "Standard Deviation: %f\n", 1000.0f * frameTimesStdDev );
					DEBUG_MSG( "Frame differences: %f\n", frameTimeDifferences );
					DEBUG_MSG( "Frame differences Pow 2: %f\n", frameTimePow2Differences );

#ifdef ENABLE_MEMTRACKER
					MemTracker::instance().reportStats();
#endif
				}

				// If we've done the last run quit out
				if ( s_state == s_profilerNumRuns )
				{
					s_state = -1;
					Profiler::instance().flushHistory();
					profilerFinishedWithStats();

					if (s_profilerExitOnComplete)
					{
						App::instance().quit();
					}
					return;
				}

				// Advance to the next run
				char historyFileName[MAX_PATH];
				sprintf( historyFileName, "%s_%d.csv", s_profilerOutputRoot, s_state );
				Profiler::instance().setNewHistory( historyFileName );
				s_resetFlyThrough = true;
				time( &startTime );
				numSoakFrames = 0;
				s_frameTimes.clear();
				s_frameTimesTotal = 0.0f;
				s_state++;
			}
		}
	}
}

/**
 * A helper function to clamp rotation to the acute angles between 2 angles.
 */
void ProfilerApp::clampAngles( const Vector3& start, Vector3& end )
{
	Vector3 rot = end - start;
	if (rot[0] <= -MATH_PI) end[0] += MATH_2PI; else if (rot[0] > MATH_PI) end[0] -= MATH_2PI;
	if (rot[1] <= -MATH_PI) end[1] += MATH_2PI; else if (rot[1] > MATH_PI) end[1] -= MATH_2PI;
	if (rot[2] <= -MATH_PI) end[2] += MATH_2PI; else if (rot[2] > MATH_PI) end[2] -= MATH_2PI;
}

/**
 *
 */
void ProfilerApp::updateCamera( float dTime )
{
	BW_GUARD;
	static std::vector< Vector3 > cameraPos;
	static std::vector< Vector3 > cameraRot;
	static uint destId;

	static Matrix transform;
	static float t, dt;
	static Vector3 p0, p1, p2, p3;
	static Vector3 r0, r1, r2, r3;

	if ( s_resetFlyThrough )
	{
		s_resetFlyThrough = false;

		PyObject* pCameraNode = s_firstCameraNode;

		// Follow the links
		cameraPos.clear();
		cameraRot.clear();

		while ( pCameraNode )
		{
			// Get the position
			PyObject* pPosition = PyObject_GetAttrString( pCameraNode, "position" );

			if ( !pPosition )
			{
				// There's something wrong with the UDO, we'll try again next frame
				DEBUG_MSG( "Node %p is still loading\n", pCameraNode );
				s_state = -1;
				return;
			}

			PyObject* pX = PyObject_GetAttrString( pPosition, "x" ); 
			PyObject* pY = PyObject_GetAttrString( pPosition, "y" );
			PyObject* pZ = PyObject_GetAttrString( pPosition, "z" );

			Vector3 position(	( float )PyFloat_AsDouble( pX ),
								( float )PyFloat_AsDouble( pY ) + CAMERA_HEIGHT,
								( float )PyFloat_AsDouble( pZ ) );

			cameraPos.push_back( position );
			
			// Get the direction of the camera
			PyObject* pRoll = PyObject_GetAttrString( pCameraNode, "roll" );
			PyObject* pPitch = PyObject_GetAttrString( pCameraNode, "pitch" );
			PyObject* pYaw = PyObject_GetAttrString( pCameraNode, "yaw" );

			Vector3 rot(	(float)PyFloat_AsDouble( pYaw ),
							(float)PyFloat_AsDouble( pPitch ),
							(float)PyFloat_AsDouble( pRoll ) );

			cameraRot.push_back( rot );

			// Grab a link to next
			pCameraNode = PyObject_GetAttrString( pCameraNode, "next" );
			if ( pCameraNode == s_firstCameraNode )
				pCameraNode = NULL;

			// Clean up
			Py_DECREF( pZ );
			Py_DECREF( pY );
			Py_DECREF( pX );

			Py_DECREF( pPosition );

			Py_DECREF( pRoll );
			Py_DECREF( pPitch );
			Py_DECREF( pYaw );
		}

		if ( s_soakTest || s_loop )
		{
			//If we are looping then we need to copy the first 3 camera nodes to the end (required for the spline)
			for (unsigned i=0; i<3; i++)
			{
				if ( cameraPos.size() > i )
				{
					cameraPos.push_back( cameraPos[i] );
					cameraRot.push_back( cameraRot[i] );
				}
			}
		}

		// Set state to begin if we have four or more camera nodes (required for the spline)
		if ( cameraPos.size() >= 4 )
		{
			g_profilerAppMatrix = &transform;
			destId = 1;
			t = 1.f; // This will make sure the co-efficients will update on the first iteration
		}
	}

	if ( !g_profilerAppMatrix )
		return;

	float timeScale = s_fixedFrameTime ? FRAME_STEP : dTime;
	t += timeScale * dt;

	if (t >= 1.f)
	{
		
		destId++;

		if ( destId >= cameraPos.size() - 1 )
		{
			if ( s_soakTest || s_loop )
			{
				destId = 2;
			}
			else
			{
				g_profilerAppMatrix = NULL;
				profilerFinishedWithStats();
				return;
			}
		}

		//Get the interpolation samples
		p0 = cameraPos[ destId - 2 ];
		p1 = cameraPos[ destId - 1 ];
		p2 = cameraPos[ destId + 0 ];
		p3 = cameraPos[ destId + 1 ];

		r0 = cameraRot[ destId - 2 ];
		r1 = cameraRot[ destId - 1 ];
		r2 = cameraRot[ destId + 0 ];
		r3 = cameraRot[ destId + 1 ];

		//Make sure the rotations only result in acute turns
		clampAngles( r0, r1 );
		clampAngles( r1, r2 );
		clampAngles( r2, r3 );

		//We need to blend the time increment to avoid large speed changes
		dt = s_speed / ((p0 - p1).length() +
					2.f * (p1 - p2).length() +
					(p2 - p3).length());

		t -= 1.f;
	}

	//Interpolate the position and rotation using a Catmull-Rom spline
	float t2 = t * t;
	float t3 = t * t2;
	Vector3 pos =	0.5 *((2 * p1) +
					(-p0 + p2) * t +
					(2*p0 - 5*p1 + 4*p2 - p3) * t2 +
					(-p0 + 3*p1- 3*p2 + p3) * t3 );

	Vector3 rot =	0.5 *((2 * r1) +
					(-r0 + r2) * t +
					(2*r0 - 5*r1 + 4*r2 - r3) * t2 +
					(-r0 + 3*r1- 3*r2 + r3) * t3 );

	//Work out the direction and up vector
	Matrix m = Matrix::identity;
	m.preRotateY( rot[0] );
	m.preRotateX( rot[1] );
	m.preRotateZ( rot[2] );

	Vector3 dir = m.applyToUnitAxisVector( Z_AXIS );
	Vector3 up = m.applyToUnitAxisVector( Y_AXIS );

	//Update the transform
	transform.lookAt( pos, dir, up );

	//Update player transform too. 
	if ( Player::entity() ) 
 	{
 		double timeNow = App::instance().getTime();
 		SpaceID spaceID = Player::entity()->pSpace()->id();
 		float newAuxVolatile[3];
		Vector3 dir2D( dir.x, 0.f, dir.z );
		dir2D.normalise();
		Vector3 playerLocation = pos - dir2D * 3.f;
		playerLocation.y = pos.y;
 		Player::entity()->filter().input( timeNow, spaceID, 0, playerLocation, Vector3::zero(), newAuxVolatile );
 	}
}


/**
* This method calls profilerFinished sending it the standard results list (fps statistics)
*/
void ProfilerApp::profilerFinishedWithStats()
{
	float frameTimesAvg;
	float frameTimesStdDev;
	float frameTimeDifferences;
    float frameTimePow2Differences;
	calculatePerformance( frameTimesAvg, frameTimesStdDev, frameTimeDifferences, frameTimePow2Differences );

	ListStringFloat sentList;
	sentList.push_back(PairStringFloat("Total Frames", (float)s_frameTimes.size()));
	sentList.push_back(PairStringFloat("Mili per frame", 1000.0f * frameTimesAvg));
	sentList.push_back(PairStringFloat("FPS", 1.0f / frameTimesAvg));
	sentList.push_back(PairStringFloat("Standard Deviation", 1000.0f * frameTimesStdDev));
	sentList.push_back(PairStringFloat("Frame differences", 1000.0f * frameTimeDifferences));
	sentList.push_back(PairStringFloat("Frame differences Pow 2", 1000.0f * frameTimePow2Differences));
	profilerFinished(sentList);
}

void ProfilerApp::profilerFinished(const ListStringFloat& sentList)
{
	PyObject * args = PyTuple_New( 1 );
	PyObject * pythonList = PyList_New(sentList.size());
	ListStringFloat::const_iterator iter = sentList.begin();
	for (; iter != sentList.end(); iter++)
	{
		PyObject * Pair = PyTuple_New(2);
		PyTuple_SetItem(Pair, 0, Script::getData(iter->first)); // Label
		PyTuple_SetItem(Pair, 1, Script::getData(iter->second)); // Enabled
		int settingIndex = std::distance(sentList.begin(), iter);
		PyList_SetItem(pythonList, settingIndex, Pair);
	}
	revertPlayerLocation();
	PyTuple_SetItem(args, 0, pythonList);
	Script::call(
			PyObject_GetAttrString( Personality::instance(),
				"onFlyThroughFinished" ),
			args,
			"Personality onFlyThroughFinished: ", true );
}

// -----------------------------------------------------------------------------
// Section Python module functions
// -----------------------------------------------------------------------------

/**
 *
 */
#ifdef ENABLE_MEMTRACKER
PyObject* py_outputMemoryStats( PyObject * args )
{
	BW_GUARD;
	MemTracker::instance().reportStats();
	MemTracker::instance().reportAllocations();
	Py_Return;
}
PY_MODULE_FUNCTION( outputMemoryStats, BigWorld )
#endif


/**
 *
 */
PyObject* py_setProfilerHistory( PyObject * args )
{
	BW_GUARD;
	const char * historyFileName = NULL;
	if (!PyArg_ParseTuple( args, "s:BigWorld.setProfilerHistory",
			&historyFileName ))
	{
		return NULL;
	}

	Profiler::instance().setNewHistory( historyFileName );

	Py_Return;
}
PY_MODULE_FUNCTION( setProfilerHistory, BigWorld )


/**
 *
 */
PyObject* py_closeProfilerHistory( PyObject * args )
{
	Profiler::instance().closeHistory();
	Py_Return;
}
PY_MODULE_FUNCTION( closeProfilerHistory, BigWorld )

/**
 *
 */
PyObject* py_runFlyThrough( PyObject * args )
{
	BW_GUARD;
	char * cameraNodeName = NULL;

	s_loop = false;
	s_speed = 20.f;
	if (!PyArg_ParseTuple( args, "s|if:BigWorld.runFlyThrough",
			&cameraNodeName, &s_loop, &s_speed ))
	{
		return NULL;
	}

	if (!runFlyThrough( cameraNodeName, s_loop ))
	{
		return NULL;
	}

	Py_Return;
}
PY_MODULE_FUNCTION( runFlyThrough, BigWorld )

/**
 *
 */
PyObject* py_cancelFlyThrough( PyObject* args )
{
	s_state = -1;
	g_profilerAppMatrix = NULL;
	revertPlayerLocation();
	Py_Return;
}
PY_MODULE_FUNCTION( cancelFlyThrough, BigWorld )

/**
 *
 */
PyObject * py_runProfiler( PyObject * args )
{
	BW_GUARD;

	char * cameraNodeName = NULL;
	s_profilerNumRuns = 2;
	strncpy( s_profilerOutputRoot, "profiler_run", MAX_PATH - 1 );
	char * profilerOutputRoot = NULL;
	s_profilerExitOnComplete = true;

	if (!PyArg_ParseTuple( args, "s|isi:BigWorld.runProfiler",
			&cameraNodeName, &s_profilerNumRuns,
			&profilerOutputRoot, &s_profilerExitOnComplete ))
	{
		return NULL;
	}

	if (profilerOutputRoot)
	{
		strncpy( s_profilerOutputRoot, profilerOutputRoot, MAX_PATH - 1 );
	}

	queueProfile( cameraNodeName, true, false );

	Py_Return;
}
PY_MODULE_FUNCTION( runProfiler, BigWorld )

/**
 *
 */
PyObject* py_runSoakTest( PyObject* args )
{
	BW_GUARD;
	char * cameraNodeName = NULL;
	char * filename = NULL;
	s_soakTime = 24.0 * 60.0;
	s_fixedFrameTime = false;

	if (!PyArg_ParseTuple( args, "s|dsi;"
				"BigWorld.runSoakTest() takes args (str)cameraNodeName "
				"[, (float)soakTime, (str)filename, (int)fixedFrameTime]",
			&cameraNodeName,
			&s_soakTime, &filename, &s_fixedFrameTime ))
	{
		return NULL;
	}

	// read filename
	memset(s_csvFilename, 0, sizeof(s_csvFilename));
	if(filename)
		strncpy(s_csvFilename, filename, sizeof(s_csvFilename));
	else
		sprintf(s_csvFilename, "soak_%d.csv", (int) time(NULL));

	s_soakTime *= 60.0; // make amount equal minutes

	queueProfile( cameraNodeName, s_fixedFrameTime, true );

	Py_Return;
}
PY_MODULE_FUNCTION( runSoakTest, BigWorld )


/*~ function BigWorld.flyThroughRunning
 *
 *	This function returns true if a fly through is currently running
 *
 *	@return True if a fly through is currently running
 *
 */
PyObject* py_flyThroughRunning( PyObject* args )
{
	return Script::getData((s_state != -1) || (g_profilerAppMatrix != NULL));
}
PY_MODULE_FUNCTION( flyThroughRunning, BigWorld )


/**
 *
 *	This function returns true if the background thread is loading
 *
 *	@return True if a fly through is currently running
 *
 */
PyObject* py_isBackgroundWorking( PyObject* args )
{
	bool isWorking = BgTaskManager::instance().isWorking();
	return Script::getData( isWorking );
}
PY_MODULE_FUNCTION( isBackgroundWorking, BigWorld )



// profiler_app.cpp
