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

#include "avatar_filter.hpp"


#include "entity.hpp"
#include "filter_utility_functions.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


PY_TYPEOBJECT( AvatarFilter )

PY_BEGIN_METHODS( AvatarFilter )
	PY_METHOD( callback )
	/*~ function AvatarFilter.debugMatrixes
	 *
	 *	This function returns a tuple of matrix providers that when applied to
	 *	a unit cube visualises the received position data being used by the
	 *	filter.
	 *
	 *	Note: These matrix providers hold references to the filter that
	 *	issued them.
	 *
	 *	@return tuple([MatrixProvider,])	A tuple of newly created matrix
	 *										providers
	 */
	PY_METHOD( debugMatrixes )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( AvatarFilter )
	/*~ attribute AvatarFilter.latency
	 *
	 *	This attribute is the current latency applied to the entity.  That is,
	 *	how far in the past that entity is, relative to the current game time.
	 *
	 *	Latency always moves towards the "ideal" latency with a velocity of
	 *	velLatency, measured in seconds per second.
	 *
	 *	If an update has just come in from the server, then the ideal latency is
	 *	the time between the two most recent updates, otherwise it is
	 *	two times minLatency.
	 *
	 *	The position and yaw of the entity are linearly interpolated between
	 *	the positions and yaws in the last 8 updates using (time - latency).
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( latency )
	/*~ attribute AvatarFilter.velLatency
	 *
	 *	This attribute is the speed, in seconds per second, that the latency
	 *	attribute can change at, as it moves towards its ideal latency.
	 *	If an update has just come in from the server, then the ideal latency is
	 *	the time between the two most recent updates, otherwise it is
	 *	two times minLatency.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( velLatency )
	/*~ attribute AvatarFilter.minLatency
	 *
	 *	This attribute is the minimum bound for latency.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( minLatency )
	/*~ attribute AvatarFilter.latencyFrames
	 *
	 *	
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( latencyFrames )
	/*~ attribute AvatarFilter.latencyCurvePower
	 *
	 *	The power used to scale the latency velocity |latency - idealLatency|^power
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( latencyCurvePower )
PY_END_ATTRIBUTES()


/*~ function BigWorld.AvatarFilter
 *
 *	This function creates a new AvatarFilter, which is used to
 *	interpolate between the position and yaw updates from the server
 *	for its owning entity.
 *
 *  @param	an optional AvatarFilter to initialise the filter with
 *
 *	@return a new AvatarFilter
 */
PY_FACTORY( AvatarFilter, BigWorld )


/**
 *	The speed in seconds per second that the avatar filters output method
 *	should adjust its latency offset.\n
 *	@note	This setting is exposed through the watcher
 *			"Comms/Latency Velocity"
 *
 *	@note	This value is scaled by \f$(latency-desiredLatency)^2\f$ before its
 *			application.
 */
float AvatarFilter::s_latencyVelocity_ = 1.00f;


/**
 *	The minimum ideal latency in seconds\n
 *	@note	This setting is exposed through the watcher
 *			"Comms/Minimum Latency"
 *
 */
float AvatarFilter::s_latencyMinimum_ = 0.10f;


/**
 *	The ideal latency measured in intervals of input packets.\n
 *	@note	This setting is exposed through the watcher
 *			"Comms/Ideal Latency Frames"
 */
float AvatarFilter::s_latencyFrames_ = 2.0f;


/**
 *	The power used to scale the latency velocity
 *	\f$\left|latency - idealLatency\right|^power\f$.\n
 *
 *	@note	This setting is exposed through the watcher
 *			"Comms/Latency Speed Curve Power"
 */
float AvatarFilter::s_latencyCurvePower_ = 2.0f;


/**
 *	This type creates a number of watchers during its construction. A single 
 *	static instance (s_avf_initer) is then used to then initiate their creation
 *	during program start-up.
 */
class AvatarFilterWatcherCreator
{
public:
	AvatarFilterWatcherCreator()
	{
		BW_GUARD;
		MF_WATCH( "Client Settings/Filters/Latency Velocity",	AvatarFilter::s_latencyVelocity_, Watcher::WT_READ_WRITE, "The speed at which the latency changes in seconds per second" );
		MF_WATCH( "Client Settings/Filters/Minimum Latency",	AvatarFilter::s_latencyMinimum_, Watcher::WT_READ_WRITE, "The minimum ideal latency in seconds" );
		MF_WATCH( "Client Settings/Filters/Ideal Latency Frames", AvatarFilter::s_latencyFrames_, Watcher::WT_READ_WRITE, "The ideal latency in update frequency (0.0-4.0)" );
		MF_WATCH( "Client Settings/Filters/Latency Speed Curve Power", AvatarFilter::s_latencyCurvePower_, Watcher::WT_READ_WRITE, "The power used to scale the latency velocity |latency - idealLatency|^power" );

		MF_WATCH( "Client Settings/Filters/Enabled", AvatarFilter::isActive, AvatarFilter::isActive, "Enable or disable entity position historical filtering" );
	}
} s_avf_initer;



///////////////////////////////////////////////////////////////////////////////
///  Member Documentation for AvatarFilter
///////////////////////////////////////////////////////////////////////////////


/**
 *	@property	Frame AvatarFilter::frames_[8]
 *
 *	This member is a circular buffer storing a history of eight received
 *	inputs.\n
 *	These are not necessarily the eight most recent inputs. In the case that
 *	the oldest input is still being used by the output method, the oldest input
 *	that is not currently being used will be discarded instead.
 *
 *	@see AvatarFilter::input
 */


/**
 *	@property	uint AvatarFilter::topFrame_
 *
 *	This member holds the index to the most recent input in the frames_
 *	circular buffer.
 */
	

/**
 *	@property	float AvatarFilter::latency_
 *
 *	This is the offset applied to client time when updating the entity.\n
 *	This latency is applied prevent the filter hitting the front of the input
 *	history. Over time the latency_ is moved closer to the idealLatency_
 *	derived from the frequency of received inputs.
 *
 *	@see	AvatarFilter::output()
 *	@see	AvatarFilter::idealLatency_
 *	@see	AvatarFilter::s_latencyVelocity_;
 *	@see	AvatarFilter::s_latencyCurvePower_;
 */


/**
 *	@property	float AvatarFilter::idealLatency_
 *
 *	This member holds the estimated ideal latency based on the frequency of
 *	inputs.\n
 *	It is recalculated after the arrival of new input using the period of time
 *	represented by the last four inputs.
 *
 *	@see	AvatarFilter::output()
 *	@see	AvatarFilter::s_latencyMinimum_
 *	@see	AvatarFilter::s_latencyFrames_
 */


/**
 *	@property	double AvatarFilter::outputTime_
 *
 *	This member holds the last client time with which output was requested.\n
 *	It is used when deciding which stored input to discard when new input
 *	is received.
 */


/**
 *	@property	bool AvatarFilter::gotNewInput_
 *
 *	This member is used to notify the output method that new input was
 *	received.
 */


/**
 *	@property	bool AvatarFilter::reset_
 *
 *	This member is used to flag to the input method that the filter has been
 *	reset and that it should reinitialise the history upon arrival of next
 *	input.
 *
 *	@see	AvatarFilter::reset()
 *	@see	AvatarFilter::input()
 */


/**
 *	@property	CallbackQueue AvatarFilter::callbacks_
 *
 *	This member holds a queue of callbacks to be triggered once the filter's
 *	output time (game time + latency_) reaches that specified in the callback.
 *	The queue is sorted by the time stamps of the callbacks referenced.
 *
 *	@see	AvatarFilter::callback()
 *	@see	AvatarFilter::output()
 */


///////////////////////////////////////////////////////////////////////////////
/// End Member Documentation for AvatarFilter
///////////////////////////////////////////////////////////////////////////////


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 *					The default is 'AvatarFilter' but it may be overridden by
 *					derived types like AvatarDropFilter.
 */
AvatarFilter::AvatarFilter( PyTypePlus * pType ) :
	Filter( pType ),
	latency_( 0 ),
	idealLatency_( 0 ),
	timeOfLastOutput_( 0 ),
	gotNewInput_( false ),
	reset_( true )
{
	BW_GUARD;
	this->resetStoredInputs( -2000, 0, 0, Position3D( 0, 0, 0 ), Vector3(0,0,0), NULL );
	this->reset( 0 );
}


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 *					The default is 'AvatarFilter' but it may be overridden by
 *					derived types like AvatarDropFilter.
 *	@param	filter	AvatarFilter to copy.
 */
AvatarFilter::AvatarFilter( const AvatarFilter & filter, PyTypePlus * pType ) :
	Filter( pType ),
	nextWaypoint_( filter.nextWaypoint_ ),
	previousWaypoint_( filter.previousWaypoint_ ),
	currentInputIndex_( filter.currentInputIndex_ ),
	latency_( filter.latency_ ),
	idealLatency_( filter.idealLatency_ ),
	timeOfLastOutput_( filter.timeOfLastOutput_ ),
	gotNewInput_( filter.gotNewInput_ ),
	reset_( filter.reset_ )
{
	BW_GUARD;
	for (uint i=0; i<NUM_STORED_INPUTS; i++)
	{
		storedInputs_[i] = filter.storedInputs_[i];
	}
}


/**
 *	Destructor
 */
AvatarFilter::~AvatarFilter()
{
	BW_GUARD;
	while( !callbacks_.empty() )
	{
		delete &*callbacks_.top();
		callbacks_.pop();
	}
}


/**
 *	This method invalidates all previously collected inputs. They will
 *	then be discarded by the next input that is received.
 *
 *	@param	time	This value is ignored.
 */
void AvatarFilter::reset( double time )
{
	reset_ = true;
}


/**
 *	This method gives the filter a new set of inputs will most likely have
 *	come from the server. If reset() has been called since the last input,
 *	the input history will be wiped and filled with this new value.
 *
 *	@param	time			The estimated client time when the input was sent
 *							from the server.
 *	@param	spaceID			The server space that the position resides in.
 *	@param	vehicleID		The ID of the vehicle in who's coordinate system
 *							the position is defined. A null vehicle ID means
 *							that the position is in world coordinates.
 *	@param	position		The new position in either local vehicle or
 *							world space/common coordinates. The player relative
 *							compression will have already been decoded at this
 *							point by the network layer.
 *	@param	positionError	The amount of uncertainty in the position.
 *	@param	auxFiltered		If not NULL, a pointer to an array of two floats
 *							representing yaw and pitch.
 */
void AvatarFilter::input(	double time,
							SpaceID spaceID,
							EntityID vehicleID,
							const Position3D & position,
							const Vector3 & positionError,
							float * auxFiltered )
{
	BW_GUARD;
	if ( reset_ )
	{
		this->resetStoredInputs( time, spaceID, vehicleID, position, positionError, auxFiltered );
		reset_ = false;
	}
	else
	{
		if (time > this->getStoredInput( 0 ).time_)
		{
			currentInputIndex_ = (currentInputIndex_ + NUM_STORED_INPUTS - 1) % NUM_STORED_INPUTS;

			StoredInput & storedInput = this->getStoredInput( 0 );

			storedInput.time_ = time;
			storedInput.spaceID_ = spaceID;
			storedInput.vehicleID_ = vehicleID;
			storedInput.position_ = position;
			storedInput.positionError_ = positionError;
			storedInput.direction_.set( auxFiltered ? auxFiltered[0] : 0.0f,
										auxFiltered ? auxFiltered[1] : 0.0f,
										0.0f );

			FilterUtilityFunctions::resolveOnGroundPosition( storedInput.position_, storedInput.onGround_ );

			gotNewInput_ = true;
		}
	}
}



/**
 *	This method updates the slave entity's position, velocity, yaw, pitch and
 *	roll to match the estimated values at the time specified.
 *	This function also moves the filter's latency towards its estimated ideal.
 *
 *	@param	time	The client game time in seconds that the entity's volatile
 *					members should be updated for.
 *
 *	@see	AvatarFilter::extract()
 */
void AvatarFilter::output( double time )
{
	BW_GUARD;
	static DogWatch dwAvatarFilterOutput("AvatarFilter");
	dwAvatarFilterOutput.start();

	// adjust ideal latency if we got something new
	if ( gotNewInput_ )
	{
		gotNewInput_ = false;

		const double newestTime = this->getStoredInput( 0 ).time_;
		const double olderTime = this->getStoredInput( NUM_STORED_INPUTS - 1 ).time_;

		AvatarFilter::s_latencyFrames_ = Math::clamp<float>( 0.0f, AvatarFilter::s_latencyFrames_, NUM_STORED_INPUTS - 1 );

		double ratio = ((NUM_STORED_INPUTS - 1) - AvatarFilter::s_latencyFrames_) / (NUM_STORED_INPUTS - 1);

		idealLatency_ = float( time - Math::lerp( ratio, olderTime, newestTime ) );

		idealLatency_ = std::max( idealLatency_, s_latencyMinimum_ );
	}

	// move latency towards the ideal...
	float dTime = float(time - timeOfLastOutput_);
	if ( idealLatency_ > latency_ )
	{
		latency_ += ( s_latencyVelocity_ * dTime ) * std::min( 1.0f, powf( fabsf( idealLatency_ - latency_ ), AvatarFilter::s_latencyCurvePower_ ) );
		latency_ = min( latency_, idealLatency_ );
	}
	else
	{
		latency_ -= ( s_latencyVelocity_ * dTime ) * std::min( 1.0f, powf( fabsf( idealLatency_ - latency_ ), AvatarFilter::s_latencyCurvePower_ ) );
		latency_ = max( latency_, idealLatency_ );
	}


	// record this so we can move latency at a velocity independent
	//  of the number of times we're called.
	timeOfLastOutput_ = time;

	// find the position at 'time - latency'
	double outputTime = time - latency_;

	SpaceID		resultSpaceID;
	EntityID	resultVehicleID;
	Position3D	resultPosition;
	Vector3		resultVelocity;
	Vector3		resultDirection;

	this->extract( outputTime, resultSpaceID, resultVehicleID, resultPosition, resultVelocity, resultDirection );

	// make sure it's in the right coordinate system
	FilterUtilityFunctions::coordinateSystemCheck( entity_, resultSpaceID, resultVehicleID );

	{
		entity_->pos( resultPosition, resultDirection, 3, resultVelocity );
	}

	dwAvatarFilterOutput.stop();

	// also call any timers that have gone off
	while( !callbacks_.empty() && outputTime >= callbacks_.top()->time_ )
	{
		Callback * cb = &*callbacks_.top();
		callbacks_.pop();

		Py_XINCREF( &*cb->function_ );
		PyObject * res = Script::ask(
			&*cb->function_,
			cb->passMissedBy_ ?
				Py_BuildValue( "(f)", float( outputTime - cb->time_ ) ) :
				PyTuple_New(0),
			"AvatarFilter::output callback: " );
		// see if it wants to snap to the position at its time
		if ( res != NULL )
		{
			if ( PyInt_Check( res ) && PyInt_AsLong( res ) == 1 )
			{
				this->extract( cb->time_, resultSpaceID, resultVehicleID, resultPosition, resultVelocity, resultDirection );
				FilterUtilityFunctions::coordinateSystemCheck( entity_, resultSpaceID, resultVehicleID );
				entity_->pos( resultPosition, resultDirection, ARRAY_SIZE( resultDirection), resultVelocity );
			}
			Py_DECREF( res );
		}

		delete cb;
	}
}


/**
 *	This method changes the coordinate system of the waypoint by first
 *	transforming into common coordinates and then into the new coordinates.
 *	spaceID_ and vehicleID_ are also set to that of the new coordinate system.
 */
void AvatarFilter::Waypoint::changeCoordinateSystem( SpaceID spaceID, EntityID vehicleID )
{
	BW_GUARD;
	if (spaceID_ == spaceID && vehicleID_ == vehicleID)
		return;

	FilterUtilityFunctions::transformIntoCommon(	spaceID_,
													vehicleID_,
													position_,
													direction_ );

	spaceID_ = spaceID;
	vehicleID_ = vehicleID;

	FilterUtilityFunctions::transformFromCommon(	spaceID_,
													vehicleID_,
													position_,
													direction_ );
}


/**
 *	Construtor
 *
 *	@param	time		The time in the filter's playback that the python
 *						function should be called.
 *	@param	function	The python object that will be called when the
 *						specified time is reached.
 *	@param	passMB		True if the callback function wishes to receive the
 *						difference between the call time and the call time
 *						requested as a parameter.
 */
AvatarFilter::Callback::Callback(	double time,
									PyObjectPtr function,
									bool passMB ) :
	time_( time ),
	function_( function ),
	passMissedBy_( passMB )
{
}


/**
 *	This implements the less-than operator for the callback, comparing one time
 *	stamp against another.
 *
 *	@return		Returns true if timestamp of this callback is less than that of
 *				the callback given.
 */
bool AvatarFilter::Callback::operator<( const Callback & b ) const
{
	return b.time_ < this->time_;
}


/**
 *	This impliments the CallbackPtrLessThanFunctor. 
 *
 *	@param	a	This is a valid pointer to a Callback object.
 *	@param	b	This is a valid pointer to a Callback object.
 */
bool AvatarFilter::Callback::CallbackPtrLessThanFunctor::operator()( 
											const AvatarFilter::Callback * a,
											const AvatarFilter::Callback * b )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( a && "CallbackPtrLessThanFunctor, null ptr passed" )
	{
		// if a is null and b is valid, pretend a is less than b
		return a != b;
	}

	IF_NOT_MF_ASSERT_DEV( b && "CallbackPtrLessThanFunctor, null ptr passed" )
	{
		// if b is NULL pretend it is not less than a
		return false;
	}

	return *a < *b;
}



void AvatarFilter::resetStoredInputs(	double time,
										SpaceID spaceID,
										EntityID vehicleID,
										const Position3D & position,
										const Vector3 & positionError,
										float * auxFiltered )
{
	BW_GUARD;
	const float NONZERO_TIME_DIFFERENCE = 0.01f;

	currentInputIndex_ = 0;
	gotNewInput_ = true;

	for (uint i=0; i< NUM_STORED_INPUTS; i++)
	{
		StoredInput & storedInput = this->getStoredInput(i);

		// set times of older inputs as to avoid zero time differences
		storedInput.time_ = time - (i * NONZERO_TIME_DIFFERENCE);

		storedInput.spaceID_ = spaceID;
		storedInput.vehicleID_ = vehicleID;
		storedInput.position_ = position;
		storedInput.positionError_ = positionError;
		storedInput.direction_.set( auxFiltered ? auxFiltered[0] : 0.0f,
									auxFiltered ? auxFiltered[1] : 0.0f,
									0.0f );

		FilterUtilityFunctions::resolveOnGroundPosition( storedInput.position_, storedInput.onGround_ );
	}

	this->latency_ = s_latencyFrames_ * NONZERO_TIME_DIFFERENCE;

	nextWaypoint_.time_ = time - NONZERO_TIME_DIFFERENCE;
	nextWaypoint_.spaceID_ = spaceID;
	nextWaypoint_.vehicleID_ = vehicleID;
	nextWaypoint_.position_ = position;
	nextWaypoint_.direction_.set(	auxFiltered ? auxFiltered[0] : 0.0f,
									auxFiltered ? auxFiltered[1] : 0.0f,
									0.0f );

	previousWaypoint_ = nextWaypoint_;
	previousWaypoint_.time_ -= NONZERO_TIME_DIFFERENCE;
}



/**
 *	This method 'extracts' a set of filtered values from the input history,
 *	clamping at the extremes of the time period stored. In the case that
 *	requested time falls between two inputs a weighed blend is performed
 *	taking into account vehicle transitions.
 *	A small amount of speculative movement supported when the most recent
 *	value in the history is older than the time requested.
 *
 *	@param	time		The client time stamp of the values required.
 *	@param	iSID		The resultant space ID
 *	@param	iVID		The resultant vehicle ID
 *	@param	iPos		The estimated position in the space of iVID
 *	@param	iVelocity	The estimated velocity of the entity at the time
 *						specified.
 *	@param	iDir		The estimated yaw and pitch of the entity.
 */
void AvatarFilter::extract(	double time,
							SpaceID & outputSpaceID,
							EntityID & outputVehicleID,
							Position3D & outputPosition,
							Vector3 & outputVelocity,
							Vector3 & outputDirection )
{
	BW_GUARD;
	if ( !Filter::isActive() )
	{
		const StoredInput & mostRecentInput = this->getStoredInput( 0 );

		outputSpaceID = mostRecentInput.spaceID_;
		outputVehicleID = mostRecentInput.vehicleID_;
		outputPosition = mostRecentInput.position_;
		outputDirection = mostRecentInput.direction_;
		outputVelocity.setZero();

		return;
	}
	else
	{
		if (time > nextWaypoint_.time_)
		{
			this->chooseNextWaypoint( time );
		}

		float proportionateDifferenceInTime =	float((time - previousWaypoint_.time_ ) /
												(nextWaypoint_.time_ - previousWaypoint_.time_));
		outputSpaceID		= nextWaypoint_.spaceID_;
		outputVehicleID		= nextWaypoint_.vehicleID_;
		outputPosition.lerp(	previousWaypoint_.position_,
								nextWaypoint_.position_,
								proportionateDifferenceInTime );
		outputVelocity		=	(nextWaypoint_.position_ - previousWaypoint_.position_) /
								float(nextWaypoint_.time_- previousWaypoint_.time_);

		Angle yaw;
		Angle pitch;
		yaw.lerp( previousWaypoint_.direction_[YAW], nextWaypoint_.direction_[YAW], proportionateDifferenceInTime );
		pitch.lerp( previousWaypoint_.direction_[PITCH], nextWaypoint_.direction_[PITCH], proportionateDifferenceInTime );

		outputDirection.set( yaw, pitch, 0 );
	}


}


/**
 *	This internal method choses a new set of waypoints to traverse based on the
 *	history of stored input and the requested time. A few approaches are used
 *	depending on the number of received inputs ahead of the requested time.
 *
 *	Two inputs ahead of time
 *	A vector is made from head of the previous waypoints to the centre of the
 *	input two ahead. The point on this vector that exists one input ahead in
 *	time is then found and its position clamped to the box of error tolerance
 *	of that same input. This point forms the new head waypoint and the previous
 *	becomes the new tail.
 *
 *	Only one input ahead of time
 *	The current pair of waypoints are projected into the future to the time of
 *	the next input ahead. The resultant position is then clamped to the box of
 *	error tolerance of that input.
 *
 *	No inputs ahead of time
 *	In the event no inputs exist ahead of the time both waypoints are set to
 *	the same position. The entity will stand still until an input is received
 *	that is ahead of game time minus latency.
 *
 *	Note: Both waypoints are always in the same coordinate system; that of the
 *	next input ahead.
 *
 *	@param time	The time which the new waypoints should enclose
 */
void AvatarFilter::chooseNextWaypoint( double time )
{
	BW_GUARD;

	Waypoint & previousWaypoint = previousWaypoint_;
	Waypoint & currentWaypoint = nextWaypoint_;

	Waypoint newWaypoint;

	if (this->getStoredInput( 0 ).time_ > time)
	{
		for (int i=NUM_STORED_INPUTS - 1; i >= 0; i--)
		{
			if (this->getStoredInput( i ).time_ > time)
			{
				const StoredInput & lookAheadInput = this->getStoredInput( 0 );
				const StoredInput & nextInput = this->getStoredInput( i );

				newWaypoint.time_ = nextInput.time_;
				newWaypoint.spaceID_ = nextInput.spaceID_;
				newWaypoint.vehicleID_ = nextInput.vehicleID_;
				newWaypoint.direction_ = nextInput.direction_;

				newWaypoint.storedInput_ = nextInput;

				previousWaypoint.changeCoordinateSystem(	newWaypoint.spaceID_,
															newWaypoint.vehicleID_ );

				currentWaypoint.changeCoordinateSystem(	newWaypoint.spaceID_,
														newWaypoint.vehicleID_ );

				float lookAheadRelativeDifferenceInTime = float((lookAheadInput.time_ - previousWaypoint.time_) /
																(currentWaypoint.time_ - previousWaypoint.time_) );

				Vector3 lookAheadPosition;
				lookAheadPosition.lerp(	previousWaypoint.position_,
										currentWaypoint.position_,
										lookAheadRelativeDifferenceInTime );

				FilterUtilityFunctions::transformIntoCommon(	newWaypoint.spaceID_,
																newWaypoint.vehicleID_,
																lookAheadPosition,
																Vector3(0,0,0) );
				FilterUtilityFunctions::transformFromCommon(	lookAheadInput.spaceID_,
																lookAheadInput.vehicleID_,
																lookAheadPosition,
																Vector3(0,0,0) );

				lookAheadPosition.clamp(	lookAheadInput.position_ - lookAheadInput.positionError_,
											lookAheadInput.position_ + lookAheadInput.positionError_ );

				FilterUtilityFunctions::transformIntoCommon(	lookAheadInput.spaceID_,
																lookAheadInput.vehicleID_,
																lookAheadPosition,
																Vector3(0,0,0) );
				FilterUtilityFunctions::transformFromCommon(	newWaypoint.spaceID_,
																newWaypoint.vehicleID_,
																lookAheadPosition,
																Vector3(0,0,0) );
				
				// Handel overlapping error rectangles
				{
					BoundingBox newWaypointBB(	newWaypoint.storedInput_.position_ - newWaypoint.storedInput_.positionError_,
												newWaypoint.storedInput_.position_ + newWaypoint.storedInput_.positionError_);
					BoundingBox currentWaypointBB(	currentWaypoint.storedInput_.position_ - currentWaypoint.storedInput_.positionError_,
													currentWaypoint.storedInput_.position_ + currentWaypoint.storedInput_.positionError_);

					if (newWaypoint.spaceID_ == currentWaypoint.storedInput_.spaceID_ &&
						newWaypoint.vehicleID_ == currentWaypoint.storedInput_.vehicleID_ &&
						!almostEqual(newWaypoint.storedInput_.positionError_, currentWaypoint.storedInput_.positionError_) &&
						newWaypointBB.intersects( currentWaypointBB ))
					{
						// Remain still if the previous move was only to adjust
						// for changes in position error (ie overlapping error regions).
						newWaypoint.position_ = currentWaypoint.position_;
					}
					else
					{
						float proportionateDifferenceInTime = float(	(nextInput.time_ - currentWaypoint.time_) /
							(lookAheadInput.time_ - currentWaypoint.time_) );

						newWaypoint.position_.lerp(	currentWaypoint.position_,
							lookAheadPosition,
							proportionateDifferenceInTime );
					}
				}

				// Constrain waypoint position to its input error rectangle
				{
					BoundingBox nextInputBB(nextInput.position_ - nextInput.positionError_,
											nextInput.position_ + nextInput.positionError_);

					if (!nextInputBB.intersects(newWaypoint.position_))
					{
						Position3D clampedPosition( newWaypoint.position_ );
						clampedPosition.clamp(	nextInput.position_ - nextInput.positionError_,
												nextInput.position_ + nextInput.positionError_ );

						Vector3 lookAheadVector = newWaypoint.position_ - currentWaypoint.position_;
						Vector3 clampedVector = clampedPosition - currentWaypoint.position_;

						if (lookAheadVector.lengthSquared() > 0.0f)
							newWaypoint.position_ = currentWaypoint.position_ + clampedVector.projectOnto( lookAheadVector );
						else
							newWaypoint.position_ = currentWaypoint.position_;

						newWaypoint.position_.clamp(nextInput.position_ - nextInput.positionError_,
													nextInput.position_ + nextInput.positionError_ );
					}
				}

				break;
			}
		}
	}
	else
	{
		// In the event there is no more input data, stand still for one frame.
		newWaypoint = nextWaypoint_;
		newWaypoint.time_ = time;
	}



	previousWaypoint_ = currentWaypoint;
	nextWaypoint_ = newWaypoint;
}

/**
 *	This function returns a reference to one of the stored input structures 
 *
 *	@param index	The index of the required input. Zero being the newest and
 *					NUM_STORED_INPUTS - 1 being the oldest
 */
AvatarFilter::StoredInput & AvatarFilter::getStoredInput( uint index )
{
	BW_GUARD;
	MF_ASSERT( index < NUM_STORED_INPUTS );

	return storedInputs_[(currentInputIndex_ + index) % NUM_STORED_INPUTS];
}


/**
 *	This function returns a reference to one of the stored input structures 
 *
 *	@param index	The index of the required input. Zero being the newest and
 *					NUM_STORED_INPUTS - 1 being the oldest
 */
const AvatarFilter::StoredInput & AvatarFilter::getStoredInput( uint index ) const
{
	BW_GUARD;
	return const_cast<AvatarFilter*>(this)->getStoredInput( index );
}


/**
 *	This function gets the last input received by the filter.
 *	Its arguments will be untouched if no input has yet been received.
 *
 *	@param	time		Will be set to the time stamp of the last input.
 *	@param	spaceID		Will be set to the space ID of the last input.
 *	@param	vehicleID	Will be set to the vehicle ID provided in the last
 *						input.
 *	@param	pos			The position of received in the last input, with its
 *						height fully resolved.
 *	@param	posError	The amount of uncertainty in the position.
 *	@param	auxFiltered	An array of two floats into which the yaw and pitch of
 *						the last input will be placed.
 *
 *	@return				Returns true if an input existed and the values in the parameters
 *						were set.
 */
bool AvatarFilter::getLastInput(	double & time,
									SpaceID & spaceID,
									EntityID & vehicleID,
									Position3D & pos,
									Vector3 & posError,
									float * auxFiltered )
{
	BW_GUARD;
	if ( !reset_ )
	{
		const StoredInput & storedInput = this->getStoredInput( 0 );
		time = storedInput.time_;
		spaceID = storedInput.spaceID_;
		vehicleID = storedInput.vehicleID_;
		pos = storedInput.position_;
		if ( auxFiltered != NULL )
		{
			auxFiltered[0] = storedInput.direction_[YAW];
			auxFiltered[1] = storedInput.direction_[PITCH];
		}
		return true;
	}
	return false;
}


/**
 *	Standard get attribute method.
 */
PyObject * AvatarFilter::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return Filter::pyGetAttribute( attr );
}


/**
 *	Standard set attribute method.
 */
int AvatarFilter::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return Filter::pySetAttribute( attr, value );
}


/*~ function AvatarFilter.callback
 *
 *	This method adds a callback function to the filter.  This function will
 *	be called at a time after the event specified by the whence argument.
 *	The amount of time after the event is specified, in seconds, by the
 *	extra argument.
 *
 *	If whence is -1, then the event is when the filter's
 *	timeline reaches the penultimate update (i.e. just before the current
 *	update position starts influencing the entity).  If whence is 0, then
 *	the event is when the current update is reached.  If whence is 1
 *	then the event is when the timeline reaches the time that
 *	the callback was specified.
 *
 *	The function will get one argument, which will be zero if the
 *	passMissedBy argumenty is zero, otherwise it will be the amount
 *	of time that passed between the time specified for the callback
 *	and the time it was actually called at.
 *
 *	<b>Note:</b> When the callback is made, the entity position will already
 *			have been set for the frame (so it'll be at some spot after the
 *			desired callback time), but the motors will not yet have been run
 *			on the model to move it. The positions of other entities may or
 *			may not yet have been updated for the frame.
 *
 *	<b>Note:</b>	If a callback function returns '1', the entity position
 *			will be snapped to the position that the entity would have had at
 *			the exact time the callback should have been called.

 *	@param	whence	an integer (-1, 0, 1).  Which event to base the callback
 *					on.
 *	@param	fn		a callable object.  The function to call.  It should take
 *					one integer argument.
 *	@param	extra	a float.  The time after the event specified by whence
 *					tha the function should be called at.
 *	@param	passMissedBy	whether or not to pass to the function the time
 *					it missed its ideal call-time by.
 *
 */
/**
 *	Call the given function when the filter's timeline reaches a given
 *	point in time, plus or minus any extra time. The point can be either
 *	-1 for the penultimate update time (i.e. just before current position
 *	update begins influencing the entity), 0 for the last update time
 *	(i.e. when at the current position update), or 1 for the game time
 *	(i.e. some time after the current position update is at its zenith).
 *
 *	@note	When the callback is made, the entity position will already have
 *			been set for the frame (so it'll be at some spot after the desired
 *			callback time), but the motors will not yet have been run on the
 *			model to move it. The positions of other entities may or may not
 *			yet have been updated for the frame.
 *	@note	If a callback function returns '1', the entity position will be
 *			snapped to the position that the entity would have had at the
 *			exact time the callback should have been called.
 */
void AvatarFilter::callback(	int whence,
								SmartPointer<PyObject> fn,
								float extra,
								bool passMissedBy )
{
	BW_GUARD;
	double	whTime;
	switch ( whence )
	{
		case -1:	whTime = this->getStoredInput( 7 ).time_;	break;
		case  0:	whTime = this->getStoredInput( 0 ).time_;	break;
		case  1:	whTime = Filter::getTimeNow();				break;
		default:	whTime = 0.0;								break;
	}
	callbacks_.push( new Callback( whTime + extra, fn, passMissedBy ) );
}

/**
 *	This class implements a matrix provider for the purpose of visualising the
 *	input data stored by the avatar filter.
 */
class AvatarFilterDebugMatrixProvider : public MatrixProvider
{
	Py_Header( AvatarFilterDebugMatrixProvider, MatrixProvider )

public:
	AvatarFilterDebugMatrixProvider(	AvatarFilter & avatarFilter,
										int inputIndex,
										PyTypePlus * pType = &s_type_ ) : 
			avatarFilter_( &avatarFilter ),
			inputIndex_( inputIndex ),
			MatrixProvider( false, pType ) {}

	virtual void matrix( Matrix & m ) const
	{
		BW_GUARD;
		const AvatarFilter::StoredInput & storedInput = avatarFilter_->getStoredInput( inputIndex_);

		m.setScale( storedInput.positionError_ * 2 );
		Matrix unitCubeOffset;
		unitCubeOffset.setTranslate( -storedInput.positionError_ );
		m.postMultiply( unitCubeOffset );

		Position3D position( storedInput.position_ );
		Vector3 vehicleDirection( 0, 0, 0 );
		FilterUtilityFunctions::transformIntoCommon(	storedInput.spaceID_,
														storedInput.vehicleID_,
														position,
														vehicleDirection );

		Matrix rotation;
		rotation.setRotate( vehicleDirection.x, vehicleDirection.y, vehicleDirection.z );
		m.postMultiply( rotation );

		Matrix translation;
		translation.setTranslate( position );
		m.postMultiply( translation );
	}

private:
	const SmartPointer< AvatarFilter >	avatarFilter_;
	const int							inputIndex_;
};

PY_TYPEOBJECT( AvatarFilterDebugMatrixProvider )


/**
 *	This is a python exposed function to provide debug information in the form
 *	of bounding boxes of received inputs for use in visualisation of the
 *	filter’s behaviour.
 */
PyObject * AvatarFilter::py_debugMatrixes( PyObject * params )
{
	BW_GUARD;
	PyObject * result = PyTuple_New( NUM_STORED_INPUTS );

	for (int i=0; i<NUM_STORED_INPUTS; i++)
	{
		AvatarFilterDebugMatrixProvider * m = new AvatarFilterDebugMatrixProvider( *this, i );
		PyTuple_SetItem( result, i, m );
	}

	return result;
}


/**
 *	Python factory method
 */
PyObject * AvatarFilter::pyNew( PyObject * args )
{
	BW_GUARD;
	int argc = PyTuple_Size( args );
	PyObject * pFilter = NULL;

	if (!PyArg_ParseTuple( args, "|O", &pFilter ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.AvatarFilter() "
			"expects an optional AvatarFilter as argument" );
		return NULL;
	}
	
	if (pFilter != NULL)
	{
		if (!AvatarFilter::Check( pFilter ))
		{
			PyErr_SetString( PyExc_TypeError, "BigWorld.AvatarFilter() "
				"expects an optional AvatarFilter as argument" );
			return NULL;
		}
		else
		{
			return new AvatarFilter( *static_cast<AvatarFilter*>( pFilter ) );
		}
	}

	return new AvatarFilter();
}


// avatar_filter.cpp
