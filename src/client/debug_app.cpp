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
#include "debug_app.hpp"

#include "app.hpp"
#include "app_config.hpp"
#include "avatar_drop_filter.hpp"
#include "client_camera.hpp"
#include "connection_control.hpp"
#include "device_app.hpp"
#include "entity_picker.hpp"
#include "frame_rate_graph.hpp"
#include "python_server.hpp"
#include "version_info.hpp"
#include "world.hpp"

#include "chunk/chunk_manager.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/bwversion.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/memory_trace.hpp"

#include "network/watcher_glue.hpp"

#include "resmgr/datasection.hpp"

#include "romp/console_manager.hpp"
#include "romp/diary_display.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/frame_logger.hpp"
#include "romp/progress.hpp"
#include "romp/resource_manager_stats.hpp"
#include "romp/resource_statistics.hpp"

namespace // anonymous
{
const float DAMPING_RATIO = 0.97f;
} // namespace (anonymous)

extern void registerAccountBudget( const std::string & account, uint32 budget );
extern void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 & c );
extern void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 (*c)() );

/**
 *	Helper class to save out diaries
 */
class MyDiarySaver : public Diary::Saver
{
public:
	MyDiarySaver( const std::string& path, int i )
	{
		BW_GUARD;
		char fname[42];
		bw_snprintf( fname, sizeof(fname), "%s\\thread%d.diary", path.c_str(), i );
		pFILE_ = fopen( fname, "wb" );
		if (!pFILE_)
			ERROR_MSG( "Could not open diary file %s for writing.\n", fname );
	}

	~MyDiarySaver()
	{
		BW_GUARD;
		if (pFILE_)
			fclose( pFILE_ );
	}

	virtual void save( DiaryEntries::const_iterator beg,
		DiaryEntries::const_iterator end )
	{
		BW_GUARD;
		if (pFILE_)
		{
			char * bigbuf = new char[1024*1024];
			char * bbcur = bigbuf;
			while (beg != end)
			{
				DiaryEntry & de = **beg;

				*((uint64*&)bbcur)++ = de.start_;
				*((uint64*&)bbcur)++ = de.stop_;
				strcpy( bbcur, de.desc_.c_str() );
				bbcur += de.desc_.length()+1;
				*bbcur++ = (de.level_ << 4) | (de.colour_ & 7);

				++beg;
			}
			fwrite( bigbuf, 1, bbcur-bigbuf, pFILE_ );
			delete [] bigbuf;
		}
	}

	static void openAll( const std::string path )
	{		
		BW_GUARD;
		std::vector<Diary*> diaries;
		Diary::look( diaries );
		for (uint i = 0; i < diaries.size(); i++)
		{
			diaries[i]->saver( new MyDiarySaver( path, i ) );
		}	
	}

	static void closeAll()
	{
		BW_GUARD;
		std::vector<Diary*> diaries;
		Diary::look( diaries );
		for (uint i = 0; i < diaries.size(); i++)
		{
			Diary::Saver * pSV = diaries[i]->saver();
			if (pSV != NULL)
			{				
				diaries[i]->saver( NULL );
				delete pSV;
			}
		}
	}

private:
	FILE * pFILE_;
};




DebugApp DebugApp::instance;

static uint32 _totalMemUsedBytes;
static uint32 _totalMemUsed;

int DebugApp_token;


DebugApp::DebugApp() :
	pVersionInfo_( NULL ),
	pFrameRateGraph_( NULL ),
	pPyServer_( NULL ),
	dTime_( 0.f ),
	fps_( 0.f ),
	maxFps_( 0.f ),
	minFps_( 0.f ),
	fpsIndex_( 0 ),
	timeSinceFPSUpdate_( 1.f ),
	slowTime_( 1.f ),
	drawSpecialConsole_( false ),
	shouldBreakOnCritical_( true ),
	shouldAddTimePrefix_( false )
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Debug/App", NULL );
	ZeroMemory( fpsCache_, sizeof( fpsCache_ ) );
	ZeroMemory( dTimeCache_, sizeof( dTimeCache_ ) );
}


DebugApp::~DebugApp()
{
	BW_GUARD;
	// Check that fini was called.
	MF_ASSERT_DEV( !pPyServer_ );
	MF_ASSERT_DEV( !pVersionInfo_ );
	MF_ASSERT_DEV( !pFrameRateGraph_ );
}

namespace
{
	std::string getVersionString() { return BWVersion::versionString(); }
}

bool DebugApp::init()
{
	BW_GUARD;
	MEM_TRACE_BEGIN( "DebugApp::init" )

	DataSectionPtr configSection = AppConfig::instance().pRoot();

	// 1. Version Info

	// query the version
	pVersionInfo_ = new VersionInfo();
	pVersionInfo_->queryAll();

	//retrieve host name
	char buf[256];
	if ( !gethostname( buf, 256 ) )
		hostName_ = buf;
	else
		hostName_ = "LocalHost";

	//retrieve device and driver info
	/*D3DEnum_DeviceInfo* device = NULL;
	D3DEnum_DriverInfo* driver = NULL;

	Moo::rc().getDeviceParams( &device, &driver );
	if ( driver != NULL )
	{
		driverDesc_ = driver->strDesc;
		driverName_ = driver->strName;
	}
	else
	{*/
		driverDesc_ = "unknown";
		driverName_ = "unknown";
	//}

	/*if ( device != NULL )
	{
		deviceName_ = device->strName;
	}
	else
	{*/
		deviceName_ = "unknown";
	//}

	MF_WATCH( "System/Network/Host", hostName_, Watcher::WT_READ_WRITE, "Server hostname" );

	MF_WATCH( "System/Operating System/Version", *pVersionInfo_, &VersionInfo::OSName, "Name of Operating System" );
	MF_WATCH( "System/Operating System/Major", *pVersionInfo_, &VersionInfo::OSMajor, "Major version number of Operating System" );
	MF_WATCH( "System/Operating System/Minor", *pVersionInfo_, &VersionInfo::OSMinor, "Minor version number of Operating System" );
	MF_WATCH( "System/Operating System/Pack", *pVersionInfo_, &VersionInfo::OSServicePack, "Installed service pack" );

	MF_WATCH( "System/DirectX/Version", *pVersionInfo_, &VersionInfo::DXName, "Version of installed DirectX" );
	MF_WATCH( "System/DirectX/Major", *pVersionInfo_, &VersionInfo::DXMajor, "Major version number of installed DirectX" );
	MF_WATCH( "System/DirectX/Minor", *pVersionInfo_, &VersionInfo::DXMinor, "Minor version number of installed DirectX" );
	MF_WATCH( "System/DirectX/Device", deviceName_, Watcher::WT_READ_WRITE, "DirectX device name" );
	MF_WATCH( "System/DirectX/Driver Name", driverName_, Watcher::WT_READ_WRITE, "DirectX driver name" );
	MF_WATCH( "System/DirectX/Driver Desc", driverDesc_, Watcher::WT_READ_WRITE, "DirectX driver description" );
	MF_WATCH( "System/Video Adapter/Driver", *pVersionInfo_, &VersionInfo::adapterDriver, "Name of video adapter driver" );
	MF_WATCH( "System/Video Adapter/Desc", *pVersionInfo_, &VersionInfo::adapterDesc, "Description of video adapter driver" );
	MF_WATCH( "System/Video Adapter/Major Version", *pVersionInfo_, &VersionInfo::adapterDriverMajorVer, "Major version number of video adapter" );
	MF_WATCH( "System/Video Adapter/Minor Version", *pVersionInfo_, &VersionInfo::adapterDriverMinorVer, "Minor version number of video adapter" );

	MF_WATCH( "System/Global Memory/Total Physical(K)", *pVersionInfo_, &VersionInfo::totalPhysicalMemory, "Total physical memory" );
	MF_WATCH( "System/Global Memory/Avail Physical(K)", *pVersionInfo_, &VersionInfo::availablePhysicalMemory, "Available physical memory" );
	MF_WATCH( "System/Global Memory/Total Virtual(K)", *pVersionInfo_, &VersionInfo::totalVirtualMemory, "Total virtual memory" );
	MF_WATCH( "System/Global Memory/Avail Virtual(K)", *pVersionInfo_, &VersionInfo::availableVirtualMemory, "Available virtual memory" );
	MF_WATCH( "System/Global Memory/Total Page File(K)", *pVersionInfo_, &VersionInfo::totalPagingFile, "Total page file memory" );
	MF_WATCH( "System/Global Memory/Avail Page File(K)", *pVersionInfo_, &VersionInfo::availablePagingFile, "Available page file memory" );

	MF_WATCH( "System/Process Memory/Page Faults", *pVersionInfo_, &VersionInfo::pageFaults, "Memory page faults" );
	MF_WATCH( "System/Process Memory/Peak Working Set", *pVersionInfo_, &VersionInfo::peakWorkingSet, "Peak working set	size" );
	MF_WATCH( "System/Process Memory/Working Set", *pVersionInfo_, &VersionInfo::workingSet, "Current working set size" );
	MF_WATCH( "System/Process Memory/Quota Peaked Page Pool Usage", *pVersionInfo_, &VersionInfo::quotaPeakedPagePoolUsage, "Quota peak amount of paged pool usage" );
	MF_WATCH( "System/Process Memory/Quota Page Pool Usage", *pVersionInfo_, &VersionInfo::quotaPagePoolUsage, "Current quota amount of paged pool usage" );
	MF_WATCH( "System/Process Memory/Quota Peaked Nonpage Pool Usage", *pVersionInfo_, &VersionInfo::quotaPeakedNonPagePoolUsage, "Quota peak amount of non-paged pool usage" );
	MF_WATCH( "System/Process Memory/Quota Nonpage Pool Usage", *pVersionInfo_, &VersionInfo::quotaNonPagePoolUsage, "Current quota amount of non-paged pool usage" );
	MF_WATCH( "System/Process Memory/Peak Page File Usage", *pVersionInfo_, &VersionInfo::peakPageFileUsage, "Peak amount of page file space used" );
	MF_WATCH( "System/Process Memory/Page File Usage", *pVersionInfo_, &VersionInfo::pageFileUsage, "Current amount of page file space used" );

	static float cpuSpeed_ = float( stampsPerSecondD() / 1000000.0 );
	//
	MF_WATCH( "System/CPU/Speed", cpuSpeed_, Watcher::WT_READ_ONLY, "CPU Speed" );

#if ENABLE_WATCHERS
	//
	MF_WATCH<int32>( "Memory/UnclaimedSize", &memoryUnclaimed, NULL, "Amount of unclaimed memory" );
	registerAccountContributor( "Base", "UnclaimedSize", (uint32(*)())memoryUnclaimed );

//	MF_WATCH( "Memory/Tale KB", &memoryAccountedFor );
	MF_WATCH<uint32>( "Memory/Used KB", &memUsed, NULL, "Amount of memory in use" );
#endif

	MF_WATCH( "Memory/TextureManagerReckoning", *Moo::TextureManager::instance(),
		&Moo::TextureManager::textureMemoryUsed, "Amount of texture being used by textures (total)" );
	MF_WATCH( "Memory/TextureManagerReckoningFrame", Moo::ManagedTexture::totalFrameTexmem_,
		Watcher::WT_READ_ONLY, "Amount of texture being used by textures (frame)" );

	const uint32 MB = 1024*1024;
	registerAccountBudget( "Texture", 16*MB );
	// Texture memory and associated data
	registerAccountBudget( "Geometry", 8*MB );		// +4 from BSP:3
	// Object-based memory related to geoemtry
	registerAccountBudget( "Animation", 4*MB );
	// Animation
	registerAccountBudget( "Sound", 4*MB );
	// Sound

	registerAccountBudget( "Video", 6*MB );
	// Video memory for push buffers and render targets
	registerAccountBudget( "Environment", 4*MB );	// +0 from Terrain:3
	// Mostly constant sized memory for environmental effects

	registerAccountBudget( "Scene", 1*MB );			// +1
	// Render scene definition and management structures (instance-based)
	registerAccountBudget( "Substance", 1*MB );		// +1
	// Collision scene definition and management structurs

	registerAccountBudget( "Base", 8*MB );
	// Base code and data and miscellaneous data structures
	registerAccountBudget( "Entity", 8*MB );		// +2 from GUI:1
	// Entity code and data (network and script-created or related objects)

	// Total: 60MB (But don't tell anyone!)


	// 2. Misc watchers

	// Top Level Info.
	MF_WATCH( "Render/FPS",
		fps_,
		Watcher::WT_READ_ONLY,
		"Frames-per-second of the previous frame." );
	MF_WATCH( "Render/FPSAverage",
		fpsAverage_,
		Watcher::WT_READ_ONLY,
		"Frames-per-second running average for the last 50 frames." );
	MF_WATCH( "Render/FPS(max)",
		maxFps_,
		Watcher::WT_READ_ONLY,
		"Recorded maxmimum FPS" );
	MF_WATCH( "Render/FPS(min)",
		minFps_,
		Watcher::WT_READ_ONLY,
		"Recorded minimum FPS" );
	MF_WATCH( "System/Network/SendReportThreshold", 
		*ConnectionControl::serverConnection(), 
		MF_ACCESSORS( double, ServerConnection, sendTimeReportThreshold ) ); 

#ifdef _DEBUG
	MF_WATCH( "Memory/Mem allocs (bytes)",
		_totalMemUsedBytes,
		Watcher::WT_READ_ONLY,
		"Current number of bytes used." );

	MF_WATCH( "Memory/Memory allocs (k)",
		_totalMemUsed,
		Watcher::WT_READ_ONLY,
		"Current number of kilobytes used." );
#endif


	MF_WATCH( "debug/filterThreshold",
				DebugFilter::instance(),
				MF_ACCESSORS( int, DebugFilter, filterThreshold ) );

	MF_WATCH( "Build/Version", &getVersionString );

	static const char * compileTime	= App::instance().compileTime();

	MF_WATCH( "Build/Date", compileTime,
		Watcher::WT_READ_ONLY,
		"Build date of this executable." );

	MF_WATCH( "Build/Config", const_cast<char*&>(configString),
		Watcher::WT_READ_ONLY,
		"Build configuration of this executable." );

	MF_WATCH( "Client Settings/Slow time",
		slowTime_,
		Watcher::WT_READ_WRITE,
		"Time divisor.  Increase this value to slow down time." );

	MF_WATCH( "Client Settings/Consoles/Special",
		drawSpecialConsole_,
		Watcher::WT_READ_WRITE,
		"Toggle drawing of the special (debug) console." );


	// 3. Diary Savers
#if ENABLE_DIARIES
	std::string diaryPath = configSection->readString("diaryPath", "");
	if (!diaryPath.empty())
	{
		Sleep( 50 );
		MyDiarySaver::openAll(diaryPath);
		atexit( MyDiarySaver::closeAll );
	}
#endif


	// 4. Python server
#if ENABLE_PYTHON_TELNET_SERVICE
	pPyServer_ = new PythonServer();
	pPyServer_->startup( ConnectionControl::serverConnection()->dispatcher(), 50001 );
#endif

	// 5. Critical message callback (move much earlier?)
	shouldBreakOnCritical_ = configSection->readBool(
		"breakOnCritical", shouldBreakOnCritical_ );
	shouldAddTimePrefix_ = configSection->readBool(
		"addTimePrefix", shouldAddTimePrefix_ );
	DebugFilter::instance().hasDevelopmentAssertions(
		configSection->readBool( "hasDevelopmentAssertions",
		DebugFilter::instance().hasDevelopmentAssertions() ) );

	MF_WATCH( "Debug/Break on critical",
		shouldBreakOnCritical_,
		Watcher::WT_READ_WRITE,
		"If enabled, will display an error dialog on critical errors. "
		"Otherwise the application will attempt to continue running." );

	MF_WATCH( "Debug/Add time prefix",
		shouldAddTimePrefix_,
		Watcher::WT_READ_WRITE,
		"" );

	MF_WATCH( "Debug/Log slow frames",
		EngineStatistics::logSlowFrames_,
		Watcher::WT_READ_WRITE,
		"If enabled, slow frames are logged to a file so you can cross-check "
		"them with other debug output." );


	// 6. Consoles
	ConsoleManager & mgr = ConsoleManager::instance();
	XConsole * pStatisticsConsole =
		new StatisticsConsole( &EngineStatistics::instance() );
	mgr.add( pStatisticsConsole,	"Statistics" );
	mgr.add( new ResourceUsageConsole( &ResourceStatistics::instance() ), "Special" );
#if ENABLE_WATCHERS
	mgr.add( new DebugConsole(),	"Watcher" );
#endif
	mgr.add( new HistogramConsole(),	"Histogram" );

	pFrameRateGraph_ = new FrameRateGraph();

	#if ENABLE_DOG_WATCHERS
		FrameLogger::init();
	#endif // ENABLE_DOG_WATCHERS

	// 7. Watcher value specific settings
	DataSectionPtr pWatcherSection = configSection->openSection( "watcherValues" );
	if (pWatcherSection)
	{
		pWatcherSection->setWatcherValues();
	}

	bool ret = DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP);

	MEM_TRACE_END()

	return ret;
}


void DebugApp::fini()
{
	BW_GUARD;
	delete pFrameRateGraph_;
	pFrameRateGraph_ = NULL;

	if (pVersionInfo_)
	{
		delete pVersionInfo_;
		pVersionInfo_ = NULL;
	}

#if ENABLE_PYTHON_TELNET_SERVICE
		if (pPyServer_)
		{
			pPyServer_->shutdown();
			Py_DECREF( pPyServer_ );
			pPyServer_ = NULL;
		}
#endif

	// Close down the diaries:
	Diary::fini();

	// Cleanup the ResourceManagerStats
	ResourceManagerStats::fini();
}


void DebugApp::tick( float dTime )
{
	BW_GUARD;
	dTime_ = dTime;

	if (fpsIndex_ >= 50)
		fpsIndex_ = 0;

	fpsCache_[ fpsIndex_ ] = 1.f / dTime;
	dTimeCache_[ fpsIndex_ ] = dTime;

	minFps_ = fpsCache_[0];
	maxFps_ = fpsCache_[0];

	float totalTimeInCache = 0.f;
	for (uint i = 1; i < 50; i++)
	{
		minFps_ = min( minFps_, fpsCache_[i] );
		maxFps_ = max( maxFps_, fpsCache_[i] );
		totalTimeInCache += dTimeCache_[i];
	}

	fpsIndex_++;

	timeSinceFPSUpdate_ += dTime;
	if (timeSinceFPSUpdate_ >= 0.5f)
	{
		fps_ = 1.f / dTime;
		fpsAverage_ = 50.f / totalTimeInCache;
		timeSinceFPSUpdate_ = 0;
	}

#if ENABLE_PYTHON_TELNET_SERVICE
	pPyServer_->pollInput();
#endif
	// A combined metric of total memory load and page file usage relative
	// to total physical memory. Used to trigger the memoryCritical callback.
	static bool first = true;
	if ( first && 
		 ( (pVersionInfo_->memoryLoad() >= 99 &&
			pVersionInfo_->pageFileUsage() > (pVersionInfo_->totalPhysicalMemory()*0.7)) ||
			Moo::rc().memoryCritical() ) )
	{
		// TODO: want this to appear again later?? Only triggering once for now.
		first = false;
		WARNING_MSG("Memory load critical, adjust the detail settings.\n");
		App::instance().memoryCriticalCallback();
	}
}

static DogWatch	g_watchDebugApp("DebugApp");

#ifdef CHUNKINTEST
/**
 * This class implements a collision callback that displays the triangles 
 * that the collision code intersects with.
 */
class DisplayCC : public CollisionCallback
{
private:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		Moo::rc().push();
		Moo::rc().world( obstacle.transform_ );

		Moo::Colour yellow( 0x00ffff00 );
		Geometrics::drawLine( triangle.v0(), triangle.v1(), yellow );
		Geometrics::drawLine( triangle.v1(), triangle.v2(), yellow );
		Geometrics::drawLine( triangle.v2(), triangle.v0(), yellow );

		Moo::rc().pop();
		return COLLIDE_ALL;
	}
};
#endif


void DebugApp::draw()
{
	BW_GUARD;
	g_watchDebugApp.start();

	AvatarDropFilter::drawDebugStuff();

	ClientCamera::instance().drawDebugStuff();

	EntityPicker::instance().drawDebugStuff();

	pFrameRateGraph_->graph( dTime_ );

	PyModel::debugStuff( dTime_ );

	DiaryDisplay::displayAll();

#if 0
	// Draw sound debugging info
	soundMgr().drawDebugStuff();
#endif

	ClientSpeedProvider::instance().drawDebugStuff();

	World::drawDebugTriangles();


#ifdef CHUNKINTEST
	static DogWatch dwHullCollide( "HullCollide" );
	// test the collision scene (testing only!)
	Vector3 camPos = Moo::rc().invView().applyToOrigin();
	Vector3 camDir = Moo::rc().invView().applyToUnitAxisVector( 2 );
	camPos += camDir * Moo::rc().camera().nearPlane();
	DisplayCC dcc;

	dwHullCollide.start();
	Vector3 camDirProp(camDir.z,0,-camDir.x);
	WorldTriangle camTri(
		camPos - Moo::rc().invView().applyToUnitAxisVector( 0 ),
		camPos + Moo::rc().invView().applyToUnitAxisVector( 0 ),
		camPos + Moo::rc().invView().applyToUnitAxisVector( 1 ) );

	float res = ChunkManager::instance().space( 0 )->collide(
		camTri, camPos + camDir * 15, dcc );
	dwHullCollide.stop();

	char resBuf[256];
	bw_snprintf( resBuf, sizeof(resBuf), "Collision result: %f\n\n", res );
	g_specialConsoleString += resBuf;
#endif

	// update our frame statistics
	EngineStatistics::instance().tick( dTime_ );

	g_watchDebugApp.stop();

	// draw consoles
	static DogWatch consoleWatch( "Console" );
	consoleWatch.start();

	ConsoleManager::instance().draw( dTime_ );

	// draw our special console too
	if (drawSpecialConsole_)
	{
		XConsole * specialConsole =
			ConsoleManager::instance().find( "Special" );
		specialConsole->clear();
		if (ChunkManager::s_specialConsoleString.empty())
		{
			specialConsole->setCursor( 0, 0 );
			specialConsole->print( "Special Console (empty)" );
		}
		else
		{
			specialConsole->setCursor( 0, 0 );
			specialConsole->print( ChunkManager::s_specialConsoleString );
		}
		specialConsole->draw( dTime_ );
	}
	ChunkManager::s_specialConsoleString = "";

	consoleWatch.stop();
}


// debug_app.cpp
