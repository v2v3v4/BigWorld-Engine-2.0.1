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

#pragma warning(disable: 4786)	// browse info too long
#pragma warning(disable: 4503)	// class name too long

#include "physics.hpp"

#include "app.hpp"
#include "entity.hpp"
#include "entity_manager.hpp"
#include "entity_picker.hpp"
#include "filter.hpp"
#include "player.hpp"
#include "action_matcher.hpp"

#include "camera/annal.hpp"
#include "camera/collision_advance.hpp"
#include "camera/direction_cursor.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_overlapper.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"

#include "duplo/chunk_attachment.hpp"

#include "input/input.hpp"

#include "physics2/worldpoly.hpp"

#include "pyscript/script.hpp"

#include "terrain/base_terrain_block.hpp"

#include <limits>

template <class TYPE> inline TYPE sqr( TYPE t ) { return t*t; }
// TODO: Put the line above in a header file.

static bool g_standStill = false;
static bool g_reverseCollisionCheck = true;
static bool g_slowUpHill = true;

#define DEBUG_PHYSICS 1

#if DEBUG_PHYSICS
bool g_debugDrawCollisions = false;
extern void s_dtriadd( const WorldTriangle & wt, uint32 col );
extern void s_darrowadd( const Vector3 & start, const Vector3 & end, uint32 col );
#endif

/**
 * TODO: to be documented.
 */
class MyDebug
{
public:
	MyDebug()
	{
		BW_GUARD;
		MF_WATCH( "Client Settings/Physics/stand still", g_standStill );
		MF_WATCH( "Client Settings/Physics/reverse collision check", g_reverseCollisionCheck );
		MF_WATCH( "Client Settings/Physics/uphill slow down",
			g_slowUpHill,
			Watcher::WT_READ_WRITE,
			"Toggle whether or not player entities slow down as they move up hill." );		
#if DEBUG_PHYSICS
		MF_WATCH( "Client Settings/Physics/debug draw",
			g_debugDrawCollisions,
			Watcher::WT_READ_WRITE,
			"Toggle whether or not to draw physics debug triangles" );
#endif
	}
};
static MyDebug myDebug;

DECLARE_DEBUG_COMPONENT2( "App", 0 )


static const float MOVEMENT_DEAD_ZONE = 0.2f * 0.2f;

float Physics::movementThreshold_ = 0.25f;


/// Constructor
Physics::Physics( Entity * pSlave, int style, PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pSlave_( pSlave ),
	style_( style ),
	velocity_( Vector3::zero() ),
	smoothingTime_( 0.3f ),
	uncorrectedVelocity_( new Vector4Basic(Vector4::zero()), true ),
	nudge_( Vector3::zero() ),
	velocityMouse_( MA_None ),
	userDriven_( true ),
	joystickEnabled_( false ),
	angular_( 0 ),
	turn_( 0 ),
	angularMouse_( MA_None ),
	userDirected_( true ),
	dcLocked_( false ),
	collide_( false ),
	fall_( false ),
	fallingVelocity_( 0 ),
	gravity_( 9.8f ),
	thrust_( 0 ),
	brake_( 0 ),
	thrustAxis_( 0, 0, 1 ),
	seeking_( false ),
	seek_( Vector4::zero() ),
	seekDeadline_( -1.f ),
	seekHeightTolerance_( 0.10f ),
	seekCallback_( NULL ),
	// lastDir_( Vector3::zero() ),
	lastYaw_( 0 ),
	lastPitch_( 0 ),
	lastVelocity_( Vector3::zero() ),

	maximumSlope_( 45.f ),	

	runFwdSpeed_( 0.f ),
	runBackSpeed_( 0.f ),
	joystickPos_( Vector2::zero() ),
	joystickVelocity_( Vector3::zero() ),

	desiredPosition_( Vector3::zero() ),

	blendingFromSpeed_( 0.f ),
	blendingFromFinish_( -1.f ),
	blendingFromDirection_( Vector3::zero() ),
	lastBlendedSpeed_( 0.f ),
	teleportSpace_( NULL ),
	teleportVehicle_( -1 ),
	teleportPos_( FLT_MAX, 0, 0 ),
	teleportDir_( FLT_MAX, 0, 0 ),
	modelHeight_( 0.f ),
	modelWidth_( 0.f ),
	modelDepth_( 0.f ),
	scrambleHeight_( 0.f ),
	inWater_( false ),
	waterSurfaceHeight_( 0.f ),
	buoyancy_( 3.f ),
	viscosity_( 1.5f ),
	chaseTarget_( NULL ),
	chaseDistance_( 0.f ),
	chaseTolerance_( 0.f ),
	movedDistance_( 0.f ),
	moving_( false ),
	blendingReset_( false ),
	axisEventNotifierThreshold_( MOVEMENT_DEAD_ZONE )
{
	BW_GUARD;

/*	DEBUG_MSG( "Physics::Physics: momentum is (%f, %f, %f)\n",
		data_.momentum_.x,
		data_.momentum_.y,
		data_.momentum_.z );*/

	// add ourselves to the census
	s_physicians_.push_back( this );
}


/// Destructor
Physics::~Physics()
{
	BW_GUARD;
	if (seekCallback_ != NULL)
	{
		// TODO: Maybe call the callback with a 'failed' result
		Py_DECREF( seekCallback_ );
		seekCallback_ = NULL;
	}

	if (chaseTarget_ != NULL)
	{
		Py_XDECREF( chaseTarget_ );
		chaseTarget_ = NULL;
	}

	// remove ourselves from the census
	Physicians::iterator it = std::find(
		s_physicians_.begin(), s_physicians_.end(), this );
	if (it != s_physicians_.end())
		s_physicians_.erase( it );
}


/**
 *	Static method to tick all physics systems
 *	(that haven't been disowned by their entities)
 */
void Physics::tickAll( double timeNow, double timeLast )
{
	BW_GUARD;
	// with an index for safety (in case new entities are controlled)
	for (uint i = 0; i < s_physicians_.size(); i++)
	{
		Physics * pPhys = s_physicians_[i];
		if (pPhys->pSlave_ != NULL)
			pPhys->tick( timeNow, timeLast );
	}
}


// Macro to scale from 0 when NUMER is 0 to TOP when NUMER is DENOM.
// More for readability than anything else.
#define LINEAR_SCALE( TOP, NUMER, DENOM ) ((TOP) * (NUMER) / (DENOM))


// push up the entity up this amount before dropping it
// static const float FALL_HOP	= 0.01f;



/**
 *	This method is called frequently to actually perform the physics
 *	on the slave entity.
 */
void Physics::tick( double timeNow, double timeLast )
{
	BW_GUARD;
	if (timeNow <= timeLast)
		return;

	if (pSlave_->physicsCorrected() != 0.0)
	{
		ServerConnection * pServer = EntityManager::instance().pServer();

		if (pServer && pSlave_->physicsCorrected() == pServer->lastSendTime())
			return;

		// ok that correction is all over now 
		pSlave_->physicsCorrected( 0.0 );

		return; // skip this frame
	}

	if (!pSlave_->isPhysicsAllowed())
	{
		return;
	}

	if (pSlave_->isOnVehicle())
	{
		Entity * vehicle = pSlave_->pVehicle();
		if (!(vehicle->selfControlled() || vehicle->tickCalled()))
		{
			vehicle->tick( timeNow, timeLast );
			vehicle->setTickAdvanced();
		}
	}
	if (style_ == STANDARD_PHYSICS)
	{
		this->avatarStyleTick( timeNow, timeLast );
	}
	else if (style_ == HOVER_PHYSICS)
	{
		this->hoverStyleTick( timeNow, timeLast );
	}
	else if (style_ == CHASE_PHYSICS &&
		chaseTarget_ != NULL && chaseTarget_->isInWorld())
	{
		SpaceID spaceID = chaseTarget_->pSpace()->id();
		EntityID vehicleID = chaseTarget_->isOnVehicle() ?
			chaseTarget_->pVehicle()->id() : 0;
		Vector3 newPos = chaseTarget_->position();
		float newAuxVolatile[3];
		newAuxVolatile[0] = chaseTarget_->auxVolatile()[0];
		newAuxVolatile[1] = chaseTarget_->auxVolatile()[1];
		newAuxVolatile[2] = chaseTarget_->auxVolatile()[2];

		this->transformCommonToLocal(
			spaceID, vehicleID, newPos, *(Vector3*)newAuxVolatile );

		pSlave_->filter().input( timeNow,
			spaceID, vehicleID, newPos, Vector3::zero(), newAuxVolatile );
	}
	else if (style_ == TURRET_PHYSICS)
	{
		this->weaponEmplacementStyleTick( timeNow, timeLast );
	}
	else
	{
		double unused;
		SpaceID spaceID;
		EntityID vehicleID;
		Vector3 oldPos;
		Vector3 err( Vector3::zero() );
		float oldDir[3];

		pSlave_->filter().getLastInput( unused,
			spaceID, vehicleID, oldPos, err, oldDir );

		// __kyl__ (5/12/2006) The following two function calls should cancel
		// each other out. Not sure why they are in here. Commenting them out
		// for now.
//		this->transformLocalToCommon(
//			spaceID, vehicleID, oldPos, *(Vector3*)oldDir );

//		this->transformCommonToLocal(
//			spaceID, vehicleID, oldPos, *(Vector3*)oldDir );

		pSlave_->filter().input( timeNow,
			spaceID, vehicleID, oldPos, err, oldDir );
	}
}

/**
 *	This helper struct contains constant parameters related to motion.
 */
struct MotionConstants
{
	MotionConstants( const Physics & physics );

	Angle		modelYaw;	// not really a constant
	Angle		modelPitch;	// nor this...
	float		modelWidth;
	float		modelHeight;
	float		modelDepth;
	float		scrambleHeight;
};

/// MotionConstants constructor
MotionConstants::MotionConstants( const Physics & physics )
{
	BW_GUARD;
	modelYaw = 0.f;
	modelPitch = 0.f;
	modelWidth = physics.modelWidth_;
	modelHeight = physics.modelHeight_;
	modelDepth = physics.modelDepth_;
	scrambleHeight = physics.scrambleHeight_;

	ChunkEmbodiment * pEmb = physics.pSlave_->pPrimaryEmbodiment();
	if (pEmb != NULL)
	{
		const Matrix& modelTransform = pEmb->worldTransform();
		modelYaw = atan2f( modelTransform.m[2][0], modelTransform.m[2][2] );

		BoundingBox bb;
		pEmb->localBoundingBox( bb, true );
		Vector3 bbSize( bb.maxBounds() - bb.minBounds() );
		if (!modelWidth) modelWidth  = bbSize[0];
		if (!modelHeight) modelHeight = bbSize[1];
		if (!modelDepth) modelDepth	= bbSize[2];
	}

	if (!modelWidth) modelWidth = 0.4f;
	if (!modelHeight) modelHeight = 1.8f;
	if (!modelDepth) modelDepth = 1.8f;
	if (!scrambleHeight) scrambleHeight = modelHeight/3.f;
}


/**
 *	This class records the difference in a position from its construction
 *	to its destruction, and adds an arrow to the debug triangle routines
 *	that visualises this delta.
 */
class ScopedDeltaPositionDebug
{
public:
	ScopedDeltaPositionDebug( const Vector3& pos, uint32 col = 0x00ffffff ):
		colour_(col),
		pos_( pos ),
		spos_( pos )
	{		
	}

	~ScopedDeltaPositionDebug()
	{
		BW_GUARD;
		if (g_debugDrawCollisions)		
		{
			s_darrowadd(spos_, pos_, colour_);
		}
	}

private:
	Vector3 spos_;
	const Vector3& pos_;
	uint32 colour_;
};


/**
 *	Do avatar-style processing
 */
void Physics::avatarStyleTick( double timeNow, double timeLast )
{
	BW_GUARD;
	float dTime = float(timeNow - timeLast);

	//Vector3 newPos = pSlave_->position();
	MotionConstants mc( *this );
	ChunkEmbodiment * pEmb = pSlave_->pPrimaryEmbodiment();

	double unused;
	SpaceID spaceID;
	EntityID vehicleID;
	Vector3 oldPos;
	Vector3 err( Vector3::zero() );
	float oldAuxVolatile[3];
	Angle velocityYaw;
	Angle thisPitch;
	Vector3	useVelocity( 0.f, 0.f, 0.f );
	float useAngular = float(angular_);
	bool disableVelocitySmoothing = false;

	if (blendingReset_)
	{
		if ( Player::instance().entity() && 
			Player::instance().entity()->pPrimaryModel() && 
			Player::instance().entity()->pPrimaryModel()->motorZero() )
		{
			ActionMatcher* am = dynamic_cast<ActionMatcher*>(Player::instance().entity()->pPrimaryModel()->motorZero());
			if (am)
			{
				am->resetVelocitySmoothing(smoothingTime_);
			}
		}
	}

	pSlave_->filter().getLastInput( unused,
		spaceID, vehicleID, oldPos, err, oldAuxVolatile );
	this->transformLocalToCommon(
		spaceID, vehicleID, oldPos, *(Vector3*)oldAuxVolatile );

	// promote motion from impacting actions to entity pose here, since it
	// has 'already happened' underneath us, like a vehicle movement
	// TODO: adirection should also be included
	if (pSlave_->pPrimaryModel() != NULL)
		oldPos += pSlave_->pPrimaryModel()->actionQueue().lastPromotion();

	Vector3 newPos = oldPos;

	// Determine our DirectionCursor.
	DirectionCursor& dc = DirectionCursor::instance();
	if (userDirected_) // Get our angles from the DirectionCursor.
	{
		velocityYaw = dc.yaw();
		thisPitch	= dc.pitch();
	}
	else
	{
		velocityYaw = oldAuxVolatile[0];
		thisPitch	= oldAuxVolatile[1];
	}
	Angle newHeadYaw( velocityYaw - mc.modelYaw );
	Angle newHeadPitch( thisPitch - mc.modelPitch );

	// Constants that should be read in somewhere else.
	const float HEAD_YAW_RANGE   = DEG_TO_RAD( 89.0f );
	const float HEAD_YAW_SPEED   = DEG_TO_RAD( 360.0f );
	const float HEAD_PITCH_RANGE = DEG_TO_RAD( 60.0f );
	const float HEAD_PITCH_SPEED = DEG_TO_RAD( 180.0f );

	// First calculate the input velocity, taking into account
	// braking, seekingnees, and keybaord or joystick mode.	
	this->calculateInputVelocity(velocityYaw, thisPitch, useVelocity, dTime);	
	
	while (seeking_ && seekDeadline_ < timeNow)
	{
		this->seekFinished( false );
		useVelocity.setZero();
	}

	if (seeking_)
	{
		this->doSeek(mc, useVelocity, disableVelocitySmoothing,
			newHeadYaw, newPos, dTime);
	}
	else if (chaseTarget_ != NULL)
	{
		this->doChaseTarget(mc, disableVelocitySmoothing,
			newHeadYaw, newPos, dTime);
	}
	else
	{
		// normal operation!
		newPos += (useVelocity * dTime) + nudge_;
		newHeadYaw += (useAngular * dTime) + float(turn_);
	}

	// Determine the desired head yaw and pitch this frame.	
	Angle desiredHeadPitch( thisPitch - mc.modelPitch );
	Angle desiredHeadYaw( velocityYaw - mc.modelYaw );

	// Check if the player is targeting an Entity.
	this->doTargetDestination(mc, desiredHeadPitch, desiredHeadYaw);

	// Limit desired yaw and pitch angle values.
	if (!userDirected_)
	{
		desiredHeadYaw = newHeadYaw;
		desiredHeadPitch = newHeadPitch;
	}

	// Determine the actual head yaw and pitch this frame.
	newHeadYaw += Math::clamp( HEAD_YAW_SPEED * dTime,
		float(desiredHeadYaw) - float(newHeadYaw) );
	newHeadPitch += Math::clamp( HEAD_PITCH_SPEED * dTime,
		float(desiredHeadPitch) - float(newHeadPitch) );

	// Keep head yaw and pitch within range
	newHeadYaw = Math::clamp( float(HEAD_YAW_RANGE),
		float(newHeadYaw) );
	newHeadPitch = Math::clamp( float(HEAD_PITCH_RANGE),
		float(newHeadPitch) );

	// Now clear stuff and prepare for next time.
	nudge_.set( 0.f, 0.f, 0.f );
	turn_ = 0;
	lastYaw_ = velocityYaw;
	lastPitch_ = thisPitch;
	spaceID = pSlave_->pSpace()->id();
	vehicleID = pSlave_->isOnVehicle() ?
		pSlave_->pVehicle()->id() : 0;

	// OK, that's where the entity wanted to go...
	desiredPosition_ = newPos;	
	Vector3 thisDisplacement(newPos - oldPos);
	Vector3 origDesiredDir(thisDisplacement);
	origDesiredDir.normalise();
	
	this->applyVelocitySmoothing(disableVelocitySmoothing, oldPos, newPos,
		thisDisplacement, dTime, timeNow, timeLast);
	
	// Remember how far we're allowed to go
	float maxDistance = thisDisplacement.length();
	if (maxDistance < 0.01f) maxDistance = 0.01f;

	// Store the current uncorrected displacement
	// use local space
	Vector3 localNewPos(newPos);
	float localOldAuxVolatile[3] = {oldAuxVolatile[0], oldAuxVolatile[1], oldAuxVolatile[2]};
	this->transformCommonToLocal(
		spaceID, vehicleID, localNewPos, *(Vector3*)localOldAuxVolatile );
	
	Vector3 localOldPos(oldPos);
	for (uint i = 0; i < 3; i++)
		localOldAuxVolatile[i] = oldAuxVolatile[i]; 
	this->transformCommonToLocal(
		spaceID, vehicleID, localOldPos, *(Vector3*)localOldAuxVolatile );

	Vector3 uncorrectedDisplacement(localNewPos - localOldPos);

	// Now see if we're supposed to collide or fall
	if (collide_)
	{
		ScopedDeltaPositionDebug spd(newPos,0x00ffff80);
		this->preventCollision(oldPos, newPos, mc);
	}
	else if (g_standStill)
	{
		newPos = oldPos;
	}	

	if (fall_)
	{
		fallingVelocity_ = this->applyGravityAndBuoyancy(vehicleID, oldPos, newPos,
			mc, dTime);
	}
	
	bool posNotLoaded = false;

	this->ensureAboveTerrain(pEmb, mc, newPos, posNotLoaded);	

	if (collide_ && g_reverseCollisionCheck)
	{
		// Now do a reverse collision check to see if we can back out
		// to our old position
		Vector3 backoutPos = oldPos;
		if (this->preventCollision(newPos, backoutPos, mc, false, true)
			&& newPos.y > oldPos.y)
		{
			newPos = oldPos;
		}
	}

	{
		ScopedDeltaPositionDebug spd(newPos,0x0080ffff);
		this->preventSteepSlope(origDesiredDir, oldPos, newPos, mc, dTime);
	}

	{
		float amt = this->scaleUphillSpeed(maxDistance, oldPos, newPos);
		uncorrectedDisplacement *= amt;
	}

	// Cannot move if old pos in unloaded region (except by teleport below)
	// TODO: Perhaps cond should be 'disallow movement to unloaded dest'...
	if (posNotLoaded && fall_)
	{
		newPos = oldPos;
		fallingVelocity_ = 0.f;
	}	

	// Total override - teleport
	this->doTeleport( pEmb, mc, newPos, newHeadYaw, newHeadPitch, spaceID,
		vehicleID );

	// OK, we've finally got the new position, so add it to the filter.
	localNewPos = newPos;
	float localAuxVolatile[3] = {Angle(mc.modelYaw + newHeadYaw),
		Angle(mc.modelPitch + newHeadPitch), 0};

	this->transformCommonToLocal( spaceID, vehicleID,
		localNewPos, *(Vector3*)localAuxVolatile );

	pSlave_->filter().input( timeNow, spaceID, vehicleID,
		localNewPos, err, localAuxVolatile );

	// Turn our uncorrected displacement into a velocity
	// Note "uncorrected" only applies to horizontal movement.
	uncorrectedDisplacement.y = localNewPos.y - localOldPos.y;
	uncorrectedVelocity_->set(Vector4(uncorrectedDisplacement / dTime,0.f));

	// Finally, record how far the character just moved
	Vector3 displacement(newPos - oldPos);
	float distance = displacement.length();

	if (newPos.y > oldPos.y)
	{
		movedDistance_ = distance;
	}
	else
	{
		Vector2 newPos2D( newPos.x, newPos.z );
		Vector2 oldPos2D( oldPos.x, oldPos.z );
		Vector2 disp2D( newPos2D - oldPos2D );
		float dist2D = disp2D.length();
		movedDistance_ = dist2D;
	}
	movedDirection_ = displacement;

	// If we did move, let the DirectionCursor know about it.
	float speed = distance / dTime;
	this->setIsMoving(speed > movementThreshold_);

	if (moving_ && dc.lookSpring() && dc.lookSpringOnMove())
	{
		dc.forceLookSpring();
	}
}


/**
 *	This method calculates the new velocity based on whatever
 *	input method the player is using.
 *
 *	@param	velocityYaw		something
 *	@param	thisPitch		something else
 *	@param	useVelocity		[out] the velocity to use
 *	@param	dTime			delta time
 */
void Physics::calculateInputVelocity(const Angle& velocityYaw,
									 const Angle& thisPitch,
									 Vector3& useVelocity,
									 float dTime)
{
	BW_GUARD;
	bool usingKeyboard = !joystickEnabled_;

	if (!usingKeyboard)
	{
		this->calculateJoystickVelocity();
	}

	if (usingKeyboard || seeking_ || !userDriven_)
	{
		useVelocity = velocity_;

		if (brake_ > 0.f)
		{
			velocity_ *= powf( 0.5f, dTime / float(brake_) );
		}
	}
	else
	{
		useVelocity[0] = joystickVelocity_[0];
		useVelocity[1] = velocity_[1];
		useVelocity[2] = joystickVelocity_[2];

		if (brake_ > 0.f)
		{
			float f = powf( 0.5f, dTime / float(brake_) );
			runFwdSpeed_ *= f;
			runBackSpeed_ *= f;
		}
	}

	Matrix34	rotMatrix;
	rotMatrix.setRotateY( velocityYaw );

	switch (velocityMouse_)
	{
	case MA_MouseX:
		useVelocity = useVelocity * (velocityYaw - lastYaw_);
		break;
	case MA_MouseY:
		useVelocity = useVelocity * (thisPitch - lastPitch_);
		break;
	case MA_Direction:
		useVelocity = rotMatrix.applyPoint( useVelocity );
		break;
	}

	// Disable any smaller movements.
	const float MIN_SPEED = 0.6f;	// 0.6 metres per second.
	const float MIN_SPEED_SQR = MIN_SPEED * MIN_SPEED;

	if (useVelocity.lengthSquared() <= MIN_SPEED_SQR)
	{
		useVelocity.setZero();
	}
}


/**
 *	This method performs one tick of a seeking movement, which seeks towards
 *	an explicit world position.
 *
 *	@param	mc			MotionConstants information structure
 *	@param	useVelocity	The velocity at which to move
 *	@param	disableVelocitySmoothing	[out]flag to designate whether to
 *						disable velocity smoothing later on or not.
 *	@param	newHeadYaw	[out]Resultant head yaw
 *	@param	newPos		[out]Resulatant position
 *	@param	dTime		delta time.
 */
void Physics::doSeek( const MotionConstants& mc, const Vector3& useVelocity,
					 bool& disableVelocitySmoothing, Angle& newHeadYaw,
					 Vector3& newPos, float dTime )
{
	BW_GUARD;
	Vector3 seekPos( seek_[0], seek_[1], seek_[2] );

	// calculate stuff on where to move
	float uvLen = useVelocity.length();
	Vector3 dir(seekPos - newPos);
	float dirDisp = dir.length();
	float maxDisp = uvLen * dTime;

	// calculate stuff on if we're there yet
	float xDiff = newPos[0] - seekPos[0];
	float zDiff = newPos[2] - seekPos[2];
	float distSQ = xDiff * xDiff + zDiff * zDiff;

	float heightDiff = fabsf( newPos[1] - seekPos[1] );

	// ok, we've calculated the difference before we do anything,
	//  but we set the position here first before we call the
	//  callback in case it does some calculations based on it.
	// You can argue either way about whether its position should
	//  be what it wanted to set it to or the closest it can get,
	//  but at the moment 'what it wanted to set' is better for
	//  script/animation calculations, so I'll do it this way.
	if (dirDisp <= maxDisp)
	{
		newPos = seekPos;
		disableVelocitySmoothing = true;
	}
	else
	{
		newPos += dir * (maxDisp / dirDisp);
	}

	// set the yaw only in the last second of movement
	if (uvLen < 0.0001f || dirDisp / uvLen < 1.f)
	{
		// let the tracker take care of smoothing this yaw
		newHeadYaw = seek_[3] - mc.modelYaw;
	}
	else
	{
		newHeadYaw = atan2f( -xDiff, -zDiff ) - mc.modelYaw;
	}


	// If the X-Z distance is less than 2 centimetres (0.02 ^ 2)
	// and if the Y difference is less than the height tolerance,
	// the seek operation was a success.
	if ( distSQ < 0.0004f && heightDiff < seekHeightTolerance_ )
	{
		disableVelocitySmoothing = true;
		this->seekFinished( true );
		// Note: This callback could set another seek target,
		// so make sure this function stays safe if that happens.
	}
}


/**
 *	This method performs one tick of a chasing movement, whereby
 *	we follow the given entity.
 *
 *	@param	mc			MotionConstants information structure
 *	@param	useVelocity	The velocity at which to move
 *	@param	disableVelocitySmoothing	[out]flag to designate whether to
 *						disable velocity smoothing later on or not.
 *	@param	newHeadYaw	[out]Resultant head yaw
 *	@param	newPos		[out]Resulatant position
 *	@param	dTime		delta time.
 */
void Physics::doChaseTarget( const MotionConstants& mc,
							bool& disableVelocitySmoothing,
							Angle& newHeadYaw,
							Vector3& newPos, float dTime )
{
	BW_GUARD;
	// look at what we're chasing
		const Vector3 chasePos( chaseTarget_->position() );
		Vector3 dir( chasePos - newPos );
		if (dir.x == 0.f && dir.z == 0.f) dir.x = 0.1f;	// make it not zero

		// move perpendicularly by strafe amount
		Vector3 dirPerp( dir.z, 0, -dir.x );
		newPos += dirPerp * (velocity_.x * dTime / dirPerp.length());
		dir = Vector3( chasePos - newPos );
		if (dir.x == 0.f && dir.z == 0.f) dir.x = 0.1f;	// make it not zero

		// set desired yaw
		newHeadYaw = dir.yaw() - mc.modelYaw;

		// move back chase distance
		float dirOrigDisp = dir.length();
		float dirDisp = dirOrigDisp - chaseDistance_;
		dir *= dirDisp / dirOrigDisp;

		// go backwards if we have to
		if (dirDisp < 0.f) dirDisp = -dirDisp;

		// figure out the max speed
		float maxDisp = velocity_.z * dTime;

		// if we're close enough then don't move at all
		if (dirDisp < chaseTolerance_)
		{
			disableVelocitySmoothing = true;
		}
		// if we can get there this frame then do so ...
		else if (dirDisp <= maxDisp)
		{
			newPos += dir;

			disableVelocitySmoothing = true;
		}
		// ... otherwise move towards it
		else
		{
			newPos += dir * (maxDisp / dirDisp);
		}

		// and that should be it
}


/**
 *	This method targets a particular destination, by pointing the
 *	head pitch and yaw towards it.
 *
 *	@param	mc					MotionConstants information structure
 *	@param	desiredHeadPitch	[out]Resultant head pitch
 *	@param	desiredHeadYaw		[out]Resultant head yaw
 */
void Physics::doTargetDestination( const MotionConstants& mc,
								  Angle& desiredHeadPitch,
								  Angle& desiredHeadYaw )
{
	BW_GUARD;
	if (targetDest_)
	{
		Matrix mx;
		targetDest_->matrix( mx );

		Vector3 targetPos( mx.applyToOrigin() );
		Entity *self = Player::instance().entity();

		MF_ASSERT_DEV( self );

		if ( self )
		{
			Vector3 selfPos( self->position() );
			if (targetSource_)
			{
				targetSource_->matrix( mx );
				selfPos = mx.applyToOrigin();
			}
			Vector3 targetDirection = targetPos - selfPos;

			float targetYaw = atan2f(targetDirection.x, targetDirection.z);
			float targetPitch = atan2f( targetDirection.y,
				sqrtf( targetDirection.z * targetDirection.z +
					targetDirection.x * targetDirection.x ) );

			desiredHeadPitch = targetPitch - mc.modelPitch;
			desiredHeadYaw = targetYaw - mc.modelYaw;

			if ( dcLocked_ )
			{
				DirectionCursor& dc = DirectionCursor::instance();
				dc.yaw( targetYaw );
				dc.pitch( targetPitch );
			}
		}
	}
}


/**
 *	This method applies a smoothing to the velocity.
 *	If you chnage acceleration, it takes you smoothingTime_ to get there,
 *	so the action matcher has a chance to
 *		a) blend in to new speed
 *		b) figure out what the new speed will actually be
 *
 *	@param	disableVelocitySmoothing	whether or not to perform the smoothing
 *	@param	oldPos				last position
 *	@param	newPos				[out]Resultant position
 *	@param	thisDisplacement	[out]Resultant displacement
 *	@param	dTime				delta time
 *	@param	timeNow				this frame's timestamp
 *	@param	timeLast			last frame's timestamp
 */
void Physics::applyVelocitySmoothing( bool disableVelocitySmoothing,
										const Vector3& oldPos,
										Vector3& newPos,
										Vector3& thisDisplacement,
										float dTime,
										double timeNow,
										double timeLast )
{	
	BW_GUARD;
	Vector3 thisVelocity( dTime>0.f ? thisDisplacement/dTime : Vector3(0,0,0) );

	if (disableVelocitySmoothing)
	{
		thisVelocity.setZero();
		lastVelocity_ = thisVelocity;
		blendingFromFinish_ = 0.f;
	}

	float		lastSpeed = lastVelocity_.length();
	float		thisSpeed = thisVelocity.length();

	if (blendingReset_)
	{
		blendingFromSpeed_ = lastBlendedSpeed_;
		blendingFromFinish_ = timeLast + smoothingTime_;
		blendingFromDirection_ = lastVelocity_;
		blendingFromDirection_.normalise();
		blendingReset_ = false;
	}	

	lastVelocity_ = thisVelocity;

	if (blendingFromFinish_ > timeNow)
	{
		float requiredSpeed = thisSpeed + LINEAR_SCALE(
			blendingFromSpeed_ - thisSpeed,
			float(blendingFromFinish_ - timeNow),
			smoothingTime_ );

		if (thisSpeed < 0.1f)
		{		// go in the direction of blendingFromDirection_ if stopped.
			thisVelocity = blendingFromDirection_;
			thisSpeed = 1.0f;
		}
		thisVelocity *= requiredSpeed / thisSpeed;

		thisDisplacement = thisVelocity * dTime;
		newPos = oldPos + thisDisplacement;
	}

	lastBlendedSpeed_ = thisVelocity.length();
}


/**
 * This method maked sure the new position will put us the on top of the
 * terrain, unless of course we're in a shell (or an unloaded overlapping
 * region
 *
 *	@param	pEmb			ChunkEmbodiment, i.e. the model
 *	@param	mv				MotionConstants information structure
 *	@param	newPos			[out]Resultant position
 *	@param	posNotLoaded	[out]Whether or not the area is unloaded.
 */
void Physics::ensureAboveTerrain( const ChunkEmbodiment * pEmb,
								 const MotionConstants& mc,
								 Vector3& newPos,
								 bool& posNotLoaded )
{
	BW_GUARD;
	bool popToTerrain = false;	
	bool overlapperInfoComplete = true;

	// If we are in an outside chunk, check to see if we should disable
	// physics at this point if we are going to end up in an indoor chunk
	// (overlapper) which hasn't loaded yet (to avoid falling through the floor).
	if (pEmb != NULL && pEmb->chunk() != NULL &&
		pEmb->chunk()->isOutsideChunk())
	{
		popToTerrain = true;

		if (ChunkOverlappers::instance.exists( *pEmb->chunk() ))
		{
			ChunkOverlappers& o = ChunkOverlappers::instance( *pEmb->chunk() );
			const ChunkOverlappers::Overlappers& overlappers = o.overlappers();

			for (uint i = 0; i < overlappers.size(); i++)
			{
				ChunkOverlapper * pOverlapper = &*overlappers[i];
				if (pOverlapper->focussed()) continue;

				// If this overlapper hasn't loaded its bounding box yet, we should
				// wait until it has in case we're going to end up inside this overlapper.
				if (!pOverlapper->bbReady() || pOverlapper->bb().intersects( newPos, 0.1f ))
 				{
					//DEBUG_MSG( "We have an unloaded overlapper (%s).\n", 
					//	pOverlapper->pOverlapper()->label().c_str() );
					popToTerrain = false;
					posNotLoaded = true;
					break;
				}
			}
			if (!o.complete())
			{
				// M9: Only set posNotLoaded if we are not on the ground
				overlapperInfoComplete = false;
				//popToTerrain = false;
				//posNotLoaded = true;
			}
		}
	}

	if (pEmb != NULL && pEmb->chunk() == NULL)
	{
		posNotLoaded = true;
	}

	popToTerrain &= fall_;	// only pop if can fall

	if (popToTerrain)
	{
		ChunkSpace::Column * csCol;
		Chunk * oChunk;
		ChunkTerrain * pTer;
		if ((csCol=pEmb->chunk()->space()->column( newPos, false )) != NULL &&
			(oChunk = csCol->pOutsideChunk()) != NULL &&
			(pTer = ChunkTerrainCache::instance( *oChunk ).pTerrain()) != NULL)
		{
			const Matrix & trInv = oChunk->transformInverse();
			Vector3 terPos = trInv.applyPoint( newPos );
			float ground = pTer->block()->heightAt( terPos[0], terPos[2] ) -
				trInv.applyToOrigin()[1];
			if (!overlapperInfoComplete && (newPos[1] < ground ||
				newPos[1] > ground+mc.scrambleHeight*0.5f))
			{
				// M9: Only set posNotLoaded if we are not on the ground
				posNotLoaded = true;
			}
			else if (ground > newPos[1])
			{
				newPos[1] = ground;
			}
		}
	}
}


/**
 * This method scales down our speed if we're going uphill
 * (but you can fall as fast as you like!)
 * You can add rule(s) that if the hill is steep enough
 * avatar will slide on (i.e. going backwards)
 *
 *	@param	maxDistance		maximum allowable movement distance
 *	@param	oldPos			old position
 *	@param	newPos			[out]Resultant position.
 *	@return	float			new speed compared to original speed.
 */
float Physics::scaleUphillSpeed( float maxDistance, const Vector3& oldPos, Vector3& newPos )
{	
	BW_GUARD;
	if (!inWater_ && g_slowUpHill && newPos.y > oldPos.y)
	{
		Vector3 displacement(newPos - oldPos);
		float distance = displacement.length();
		float origDistance = distance;
		if (distance > 0.001f)
		{
			float sineAngle = sinf( displacement.y / distance );
			if (distance > maxDistance)
			{
				distance = maxDistance;
			}
			distance -= 0.75f * sineAngle * distance;
//			distance -= 0.5f * gravity_ * sineAngle * sqr( dTime );
/*			if (displacement.length() > maxDistance * 1.2f)
			{
				displacement *= maxDistance * 1.2f / displacement.length();
			}
*/
			displacement *= distance / displacement.length();
			newPos = oldPos + displacement;
			return ( displacement.length() / origDistance );
		}
	}
	return 1.f;
}


/**
 *	This method teleports the entity to the given teleport position.
 *
 *	@param	pEmb			Entity's Chunk Embodiment, i.e. its model
 *	@param	mc				MotionConstants information structure.
 *	@param	newPos			[out]Resultant position
 *	@param	newHeadYaw		[out]Resultant yaw
 *	@param	newHeadPitch	[out]Resultant pitch
 *	@param	spaceID			[out]Resultant space
 *	@param	vehicleID		[out]Resultant vehicle
 */
void Physics::doTeleport( const ChunkEmbodiment* pEmb,
						 MotionConstants& mc,
						 Vector3& newPos,
						 Angle& newHeadYaw,
						 Angle& newHeadPitch,
						 SpaceID& spaceID,
						 EntityID& vehicleID )
{
	BW_GUARD;
	if (teleportPos_.x != FLT_MAX)
	{
		newPos = teleportPos_;

		this->stop( true );

		teleportPos_.x = FLT_MAX;

		if (teleportDir_.x != FLT_MAX)
		{
			mc.modelYaw = teleportDir_.x; // overwriting constant intentional
			newHeadYaw = 0;
			newHeadPitch = teleportDir_.y;
			// roll is unused

			ChunkAttachment * pAtt = NULL;
			if (PyAttachment::Check( &**pEmb )) pAtt = (ChunkAttachment*)pEmb;
			if (pAtt != NULL)
			{
				Matrix modelTransform = pAtt->getMatrix();

				float oldYaw = atan2f( modelTransform.m[2][0], modelTransform.m[2][2] );
				modelTransform.preRotateY( mc.modelYaw - oldYaw );

				pAtt->setMatrix( modelTransform );
			}

			teleportDir_.x = FLT_MAX;
		}
	}
	if (teleportSpace_)
	{
		spaceID = teleportSpace_->id();

		if (teleportVehicle_ != EntityID(-1))
		{
			vehicleID = teleportVehicle_;

			teleportVehicle_ = EntityID(-1);
		}

		// We work in the co-ordinate system of the space here,
		// not the coordinate system of the vehicle.
		// That may well have to change, but for now we don't have
		// to do anything with the position here.

		teleportSpace_ = NULL;
	}

	// If we're on a vehicle, make sure it's still in the world
	// (even if we just teleported to it ... best to be safe)
	if (vehicleID != 0 &&
		EntityManager::instance().getEntity( vehicleID ) == NULL)
	{
		vehicleID = 0;
	}
}


/**
 *	Do weapon-emplacement processing
 */
void Physics::weaponEmplacementStyleTick( double timeNow, double timeLast )
{
	BW_GUARD;
	float		dTime = float(timeNow - timeLast);

	//Vector3 newPos = pSlave_->position();
	MotionConstants mc( *this );
	ChunkEmbodiment * pEmb = pSlave_->pPrimaryEmbodiment();

	double unused;
	SpaceID spaceID;
	EntityID vehicleID;
	Vector3 oldPos;
	Vector3 err( Vector3::zero() );
	float unused2[3];
	pSlave_->filter().getLastInput( unused,
		spaceID, vehicleID, oldPos, err, unused2 );
	this->transformLocalToCommon(
		spaceID, vehicleID, oldPos, *(Vector3*)unused2 );

	Vector3 newPos = oldPos;

	// Determine our DirectionCursor.
	DirectionCursor& dc = DirectionCursor::instance();


	float velocityYaw = dc.yaw();


	Angle newHeadYaw( velocityYaw - mc.modelYaw );
	Angle newHeadPitch( pSlave_->auxVolatile()[1] );

	Matrix34	rotMatrix;
	rotMatrix.setRotateY( velocityYaw );

	// Get our angles from the DirectionCursor.
	Angle	thisPitch	= -dc.pitch();


	bool	disableVelocitySmoothing = false;

	// Constants that should be read in somewhere else.
	const float HEAD_YAW_RANGE   = DEG_TO_RAD( 89.0f );
	const float HEAD_YAW_SPEED   = DEG_TO_RAD( 360.0f );
	const float HEAD_PITCH_RANGE = DEG_TO_RAD( 30.0f );
	const float HEAD_PITCH_SPEED = DEG_TO_RAD( 180.0f );

	// First calculate the input velocity, taking into account
	// braking, seekingnees, and keybaord or joystick mode.
	Vector3	useVelocity( 0.f, 0.f, 0.f );

	bool usingKeyboard = !joystickEnabled_;
	if (!usingKeyboard) this->calculateJoystickVelocity();

	if (usingKeyboard || seeking_ || !userDriven_)
	{
		useVelocity = velocity_;

		if (brake_ > 0.f)
		{
			velocity_ *= powf( 0.5f, dTime / float(brake_) );
		}
	}
	else
	{
		useVelocity[0] = joystickVelocity_[0];
		useVelocity[1] = velocity_[1];
		useVelocity[2] = joystickVelocity_[2];

		if (brake_ > 0.f)
		{
			float f = powf( 0.5f, dTime / float(brake_) );
			runFwdSpeed_ *= f;
			runBackSpeed_ *= f;
		}
	}

	switch (velocityMouse_)
	{
	case MA_MouseX:
		useVelocity = useVelocity * (velocityYaw - lastYaw_);
		break;
	case MA_MouseY:
		useVelocity = useVelocity * (thisPitch - lastPitch_);
		break;
	case MA_Direction:
		useVelocity = rotMatrix.applyPoint( useVelocity );
		break;
	}

	// Disable any smaller movements.
	const float MIN_SPEED = 0.6f;	// 0.6 metres per second.
	const float MIN_SPEED_SQR = MIN_SPEED * MIN_SPEED;

	if (useVelocity.lengthSquared() <= MIN_SPEED_SQR)
	{
		useVelocity.setZero();
	}

	float useAngular = float(angular_);

	while (seeking_ && seekDeadline_ < timeNow)
	{
		this->seekFinished( false );
		useVelocity.setZero();
	}

	if (seeking_)
	{
		newHeadYaw = seek_[0] - mc.modelYaw;
		newHeadPitch = seek_[1] - mc.modelPitch;
		//this->seekFinished( true );

		//Use current python interface to extract wanted head and seek positions
		//float difYaw = seek_[0] - mc.modelYaw;
		//float difPitch = seek_[1] - newHeadPitch;

		//newHeadYaw += Math::clamp( HEAD_YAW_SPEED * dTime, difYaw );
		//newHeadPitch += Math::clamp( HEAD_PITCH_SPEED * dTime, difPitch);

		//if (fabs(difYaw) < 0.001 && fabs(difPitch) < 0.001)
		//{
		//	this->seekFinished( true );
		//}
	}


	// Determine the desired head yaw and pitch this frame.
	Angle desiredHeadYaw( velocityYaw - mc.modelYaw );
	Angle desiredHeadPitch( thisPitch - mc.modelPitch );

	// Check if the player is targeting an Entity.
	if (targetDest_)
	{
		Matrix mx;
		targetDest_->matrix( mx );

		Vector3 targetPos( mx.applyToOrigin() );
		Entity *self = Player::instance().entity();

		MF_ASSERT_DEV( self );

		if ( self )
		{
			Vector3 selfPos( self->position() );
			if (targetSource_)
			{
				targetSource_->matrix( mx );
				selfPos = mx.applyToOrigin();
			}
			Vector3 targetDirection = targetPos - selfPos;

			float targetYaw = atan2f(targetDirection.x, targetDirection.z);
			float targetPitch = atan2f( targetDirection.y,
				sqrtf( targetDirection.z * targetDirection.z +
					targetDirection.x * targetDirection.x ) );

			desiredHeadPitch = targetPitch - mc.modelPitch;
			desiredHeadYaw = targetYaw - mc.modelYaw;

			if ( dcLocked_ )
			{
				dc.yaw( targetYaw );
				dc.pitch( targetPitch );
			}
		}
	}

	// Limit desired yaw and pitch angle values.
	if (!userDirected_)
	{
		desiredHeadYaw = newHeadYaw;
		desiredHeadPitch = newHeadPitch;
	}


	// Determine the actual head yaw and pitch this frame.
	newHeadYaw += Math::clamp( HEAD_YAW_SPEED * dTime,
		float(desiredHeadYaw) - float(newHeadYaw) );
	newHeadPitch += Math::clamp( HEAD_PITCH_SPEED * dTime,
		float(desiredHeadPitch) - float(newHeadPitch) );

	// Keep head pitch within range
	newHeadPitch = Math::clamp( float(HEAD_PITCH_RANGE),
		float(newHeadPitch) );

	// Regardless of those two, we should always be on top of the terrain,
	//  unless of course we're in a shell (or an unloaded overlapping region)
	bool popToTerrain = false;
	if (pEmb != NULL && pEmb->chunk() != NULL && pEmb->chunk()->isOutsideChunk())
	{
		popToTerrain = true;

		const ChunkOverlappers::Overlappers & overlappers =
			ChunkOverlappers::instance( *pEmb->chunk() ).overlappers();
		for (uint i = 0; i < overlappers.size(); i++)
		{
			ChunkOverlapper * pOverlapper = &*overlappers[i];
			if (pOverlapper->focussed()) continue;
			if (!pOverlapper->bbReady() || pOverlapper->bb().intersects( newPos ))
				popToTerrain = false;
		}
	}
	if (popToTerrain)
	{
		ChunkSpace::Column * csCol;
		Chunk * oChunk;
		ChunkTerrain * pTer;
		if ((csCol=pEmb->chunk()->space()->column( newPos, false )) != NULL &&
			(oChunk = csCol->pOutsideChunk()) != NULL &&
			(pTer = ChunkTerrainCache::instance( *oChunk ).pTerrain()) != NULL)
		{
			const Matrix & trInv = oChunk->transformInverse();
			Vector3 terPos = trInv.applyPoint( newPos );
			float ground = pTer->block()->heightAt( terPos[0], terPos[2] ) -
				trInv.applyToOrigin()[1];
			if (ground > newPos[1])
			{
				newPos[1] = ground;
				vehicleID = 0;
			}
		}
	}


	// Now clear stuff and prepare for next time.
	nudge_.set( 0.f, 0.f, 0.f );
	turn_ = 0;

	lastYaw_ = velocityYaw;
	lastPitch_ = thisPitch;

	spaceID = pSlave_->pSpace()->id();
	vehicleID = pSlave_->isOnVehicle() ?
		pSlave_->pVehicle()->id() : 0;

	// OK, that's where the entity wanted to go...
	desiredPosition_ = newPos;
	static const float relevantAccel = 1.f;


	// OK, we've finally got the new position, so add it to the filter.
	Vector3 localPos = newPos;
	float localAuxVolatile[3];


	localAuxVolatile[0] = mc.modelYaw + newHeadYaw;
	localAuxVolatile[1] = newHeadPitch;
	localAuxVolatile[2] = 0;

	if (!userDirected_)
	{
		localAuxVolatile[1] = 0;
	}



	this->transformCommonToLocal( spaceID, vehicleID,
		localPos, *(Vector3*)localAuxVolatile );

	pSlave_->filter().input( timeNow, spaceID, vehicleID,
		localPos, err, localAuxVolatile );





	// Finally, record how far the character just moved
	movedDistance_ = 0.0f;
	movedDirection_ = Vector3(0.0f,0.0f,0.0f);
	this->setIsMoving( false);
}




/**
 *	This private method transforms a position and direction in the common
 *	coordinate system into the local coordinate system that filters expect
 *	their input to be in.
 */
void Physics::transformCommonToLocal( SpaceID spaceID, EntityID vehicleID,
	Vector3 & pos, Vector3 & dir ) const
{
	BW_GUARD;
	if (vehicleID != 0)
	{
		Entity * pVehicle = pSlave_->pVehicle();
		if (pVehicle == NULL || pVehicle->id() != vehicleID)
			pVehicle = EntityManager::instance().getEntity( vehicleID );
		// don't look in cache ... only in-world vehicles here thanks
		if (pVehicle != NULL && pVehicle->isInWorld())
		{
			pVehicle->transformCommonToVehicle( pos, dir );
			return;
		}
	}
	if (spaceID != 0)
	{
		ChunkSpace * pSpace = &*pSlave_->pSpace();
		if (pSpace == NULL || pSpace->id() != spaceID)
			pSpace = &*ChunkManager::instance().space( spaceID, false );
		if (pSpace != NULL)
		{
			pSpace->transformCommonToSpace( pos, dir );
			return;
		}
	}
	// did no transform ... oh well.
}

/**
 *	This private method transforms a position and direction in the local
 *	coordinate system used by filters into the common coordinate system
 *	that physics likes to work with.
 */
void Physics::transformLocalToCommon( SpaceID spaceID, EntityID vehicleID,
	Vector3 & pos, Vector3 & dir ) const
{
	BW_GUARD;
	if (vehicleID != 0)
	{
		Entity * pVehicle = pSlave_->pVehicle();
		if (pVehicle == NULL || pVehicle->id() != vehicleID)
			pVehicle = EntityManager::instance().getEntity( vehicleID, true );
		// do look in cache ... for if vehicle disappeared underneath us
		if (pVehicle != NULL)
		{
			pVehicle->transformVehicleToCommon( pos, dir );
			return;
		}
	}
	if (spaceID != 0)
	{
		ChunkSpace * pSpace = &*pSlave_->pSpace();
		if (pSpace == NULL || pSpace->id() != spaceID)
			pSpace = &*ChunkManager::instance().space( spaceID, false );
		if (pSpace != NULL)
		{
			pSpace->transformSpaceToCommon( pos, dir );
			return;
		}
	}
	// did no transform ... oh well.
}


/**
 *	This private method sets the velocity to zero and calls the callback
 *	when a seek operation finishes.
 */
void Physics::seekFinished( bool successfully )
{
	BW_GUARD;
	PyObject * pFunction = seekCallback_;

	this->stop( false );

	seeking_ = false;
	seekCallback_ = NULL;

	Script::call( pFunction, Py_BuildValue( "(i)", int( successfully ) ),
		"Physics::seekFinished: ", true );
}

/*~ function Physics.stop
 *
 *	This method stops all movement.
 */
/**
 *	This private method stops all movement
 */
void Physics::stop( bool noslide )
{
	BW_GUARD;
	Py_XDECREF( chaseTarget_ );
	chaseTarget_ = NULL;

	velocity_.setZero();
	joystickVelocity_.setZero();
	runFwdSpeed_ = 0.f;
	runBackSpeed_ = 0.f;

	if (noslide)
	{
		lastVelocity_.setZero();
		blendingFromFinish_ = 0.f;
	}
}


/*~ function Physics.movingForward
 *
 *	This method finds out how far we can go 'moving forward.'
 *
 *	It stops whenever a collision would slow the velocity to less than
 *	half the original velocity, or when the input maxDistance is reached.
 *
 *	@param	maxDistance		This is a float, which specifies the maximum
 *							distance to check for forward movement, in the
 *							current direction the entity is facing.
 *	@param	destination		This is an optional Vector4Provider that overrides
 *							the maximum distance argument, if you specify this
 *							argument the destination is used as the end point,
 *							instead of current direction * maxDistance.
 *	@return	Vector3			The allowable end position for movement.
 */
/**
 *	This method finds out how far we can go 'moving forward' (marketing alert!)
 *
 *	It stops whenever a collision would slow the velocity to less than
 *	half the original velocity, or when the input maxDistance is reached.
 */
Vector3 Physics::movingForward( float maxDistance, Vector4ProviderPtr destinationPtr ) const
{
	BW_GUARD;
	if (pSlave_ == NULL) return Vector3::zero();

	Vector3 oldPos = pSlave_->position();
	Vector3 newPos = oldPos;

	EntityID vehicleID = pSlave_->isOnVehicle() ?
			pSlave_->pVehicle()->id() : 0;

	MotionConstants mc( *this );

	if (destinationPtr)
	{
		Vector4 destination4(0,0,0,0);
		destinationPtr->output( destination4 );
		Vector3 destination3( destination4[0], destination4[1], destination4[2] );

		Vector3 direction( destination3 - oldPos );
		mc.modelYaw = atan2f( direction[0], direction[2] );

		maxDistance = min( maxDistance, direction.length() );
	}

	Vector3 vel( sinf( mc.modelYaw ), 0.f, cosf( mc.modelYaw ) );
	vel *= 0.2f;	// move 20cm each step, i.e. at 30fps is 6m/s
	float dTime = 1.f/30.f;

	float odt = fallingVelocity_;
	while ((oldPos-newPos).lengthSquared() < maxDistance*maxDistance)
	{
		Vector3 midPos = newPos;

		newPos += vel;
		this->preventCollision( midPos, newPos, mc );
		float ndt = this->applyGravityAndBuoyancy( vehicleID, oldPos, newPos, mc, dTime );
		const_cast<Physics*>( this )->fallingVelocity_ = ndt;

		// if we moved less than half the desired distance, then stop
		if ((midPos-newPos).lengthSquared() < 0.1f*0.1f) break;
	}
	const_cast<Physics*>( this )->fallingVelocity_ = odt;

	return newPos;
}


/**
 *	This class find the flattest triangle encountered,
 *	limited to the range of a quadrilateral column.
 */
class FlattestTriangle : public CollisionCallback
{
public:
	FlattestTriangle():		
		best_( -1.1f )
	{
	}

	~FlattestTriangle()
	{
		BW_GUARD;	
#if DEBUG_PHYSICS
		if (g_debugDrawCollisions)
		{
			//bright blue for the max slope triangle
			s_dtriadd( minTri_, 0x00a0a0ff );
			Vector3 centre( minTri_.v0() + minTri_.v1() + minTri_.v2() );
			centre /= 3.0;
			Vector3 normal( minTri_.normal() );
			normal.normalise();
			s_darrowadd( centre, centre + normal, 0x00a0a0ff );
		}
#endif
	}

	WorldTriangle minSlope() const	{ return minTri_; }
	bool foundMin() const			{ return best_ > -1.1f; }
	
	float best_;
	WorldTriangle minTri_;

private:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		BW_GUARD;
		// transform into world space
		WorldTriangle wt(
			obstacle.transform_.applyPoint( triangle.v0() ),
			obstacle.transform_.applyPoint( triangle.v1() ),
			obstacle.transform_.applyPoint( triangle.v2() ) );

		// compare against our starting triangle
		Vector3 normal( wt.normal() );
		normal.normalise();

		if ( normal.y >= 0.f )
		{
			if ( normal.y > best_ )
			{
				minTri_ = wt;
				best_ = normal.y;
			}
		}

		return COLLIDE_ALL;
	}
};


/**
 *	This function prevents moving onto a slope greater than the maximum
 *	allowable slope.
 *
 *	@param	origDesiredDir	direction the physics was being pushed in
 *							originally (before any collision checks)
 *	@param	oldPos	old position
 *	@param	newPos	desired new position, returned as actual new position
 *	@param	mc		MotionConstants information structure
 *	@param	dTime	delta time
 *	@return	true	if the newPos was prevented from accessing a steep slope.
 */
bool Physics::preventSteepSlope( const Vector3& origDesiredDir,
								const Vector3& oldPos, Vector3& newPos,
								const MotionConstants& mc, float dTime  ) const
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace.hasObject())
	{
		return false;
	}
	
	// Construct a rectangle that will be advanced. This rectangle corresponds
	// to the width and height of the model with the bottom (hop height) cut
	// off.
	CollisionInfo ci;
	if( !this->collisionSetup( newPos, oldPos, mc, true, false, ci ) )
	{
		return false;
	}

	Vector3 v[4];
	v[0] = ci.bottomLeft_;
	v[1] = v[0] + ci.frontBuffer_;
	v[2] = v[1] + ci.widthVector_;
	v[3] = v[0] + ci.widthVector_;

	Vector3 toPos( v[0] );
	toPos.y -= mc.scrambleHeight;
	toPos.y -= 0.25f;

	WorldTriangle sweepTriangle1( v[0], v[1], v[2] );
	WorldTriangle sweepTriangle2( v[0], v[2], v[3] );

#if DEBUG_PHYSICS
	if (g_debugDrawCollisions)
	{
		// DEBUG: Draw the sweep triangle in orange
		s_dtriadd( sweepTriangle1, 0x00ff8000 );
		s_dtriadd( sweepTriangle2, 0x00ff8000 );
		//s_darrowadd( stPos, toPos, 0x00ff8000 );

		Vector3 delta = toPos - v[0];
		WorldTriangle sweepTriangle1Dest( v[0] + delta, v[1] + delta, v[2] + delta );
		WorldTriangle sweepTriangle2Dest( v[0] + delta, v[2] + delta, v[3] + delta );
		s_dtriadd( sweepTriangle1Dest, 0x008000ff );
		s_dtriadd( sweepTriangle2Dest, 0x008000ff );

	}
#endif	
	
	FlattestTriangle st;
	pSpace->collide( sweepTriangle1, toPos, st );
	pSpace->collide( sweepTriangle2, toPos, st );
		
	if ( st.foundMin() )
	{
		float slope = 0.f;
		WorldTriangle& wt = st.minSlope();
		Vector3& norm( wt.normal() );			
		norm.normalise();

		if (norm.y < 1.f)			
		{
			slope = 90.f - RAD_TO_DEG(atanf( norm.y / Vector2(norm.x, norm.z).length() ));
		}		

		// If slope is too large, remove the upward component of movement.
		// This will push the movement sideways along the steep triangle.
		if ( slope >= maximumSlope_ )
		{		
			//If the player originally pushed away from this triangle,
			//then just let them move, otherwise push them along the
			//steep triangle and not into it.
			if (origDesiredDir.dotProduct(norm) <= 0.f)
			{
				//Slide player directly along triangle face
				Vector3 flatMove( newPos - oldPos );
				flatMove.y = 0.f;				
				Vector3 flatNorm(norm);
				flatNorm.y = 0.f;
				flatNorm.normalise();
				
				newPos = oldPos;
				{
					ScopedDeltaPositionDebug spd(newPos,0x00ff0000);
					newPos = oldPos + flatMove;
				}

				float dotP = flatMove.dotProduct( flatNorm );
				newPos = oldPos;
				{
					ScopedDeltaPositionDebug spd(newPos,0x00ffff00);
					if (dotP < 0.f)
					{
						newPos = newPos + flatMove - flatNorm * dotP;
					}
				}				
			}
			return true;
		}
	}	

	return false;
}


bool g_inPrevCol = true;


/**
 *	This method sets up the current collision related information
 *	into the Collision Info object passed in.
 *
 *	@param	newPos		Current destination
 *	@param	oldPos		Previous position
 *	@param	mc			Motion Constants information structure
 *	@param	shouldSlide	Whether or not sliding is allowed
 *	@param	reverseCheck Whether or not this collision info should
 *						be doing the reverse collision check( backing
 *						out).
 *	@param	ci			[out]  resultant CollisionInfo structure.
 */
bool Physics::collisionSetup( const Vector3& newPos,
					const Vector3& oldPos,
					const MotionConstants& mc,
					bool shouldSlide,
					bool reverseCheck,
					CollisionInfo& ci ) const
{
	BW_GUARD;
	ci.delta_ = newPos - oldPos;

	//MF_ASSERT_DEV( ci.delta.y == 0.f );
	ci.delta_.y = 0.f;

	ci.maxAdvance_ = ci.delta_.length();

	if (ci.maxAdvance_ < 0.001f)
	{
		// We're not going anywhere.

		return false;
	}

	ci.flatDir_ = ci.delta_ / ci.maxAdvance_;

	// Construct a rectangle that will be advanced. This rectangle corresponds
	// to the width and height of the model with the bottom (hop height) cut
	// off.
	ci.widthVector_.set( mc.modelWidth * ci.flatDir_.z, 0.f, -mc.modelWidth * ci.flatDir_.x );
	ci.halfWidthVector_ = 0.5f * ci.widthVector_;
	ci.heightVector_.set( 0.f, mc.modelHeight - mc.scrambleHeight, 0.f );
	ci.bottomLeft_ = oldPos - ci.halfWidthVector_;
	ci.bottomLeft_.y += mc.scrambleHeight;

	// Make sure that we are at least this far away before we run into an object.

	ci.frontDistance_ = 0.0f;
	if (!reverseCheck)
		ci.frontDistance_ = 0.5f * sqrtf( sqr(mc.modelDepth) + sqr(mc.modelWidth) );

	// COLLISION_FUDGE is a bit of a fudge factor.
	ci.frontBuffer_ = ci.frontDistance_ * ci.flatDir_;
	ci.dest_ = ci.bottomLeft_ + ci.delta_ + ci.frontBuffer_;

	return true;
}


/**
 *	This function prevents a collision between newPos and oldPos,
 *	adjusting newPos if necessary.
 *
 *	@param	oldPos		previous position of player
 *	@param	newPos		desired position of player
 *	@param	mc			MotionConstants information
  *	@param	shouldSlide	Whether or not sliding is allowed
 *	@param	reverseCheck Whether or not this collision info should
 *						be doing the reverse collision check( backing
 *						out). 
 *	@return	bool		whether a collision occurred
 */
bool Physics::preventCollision( const Vector3 &oldPos, Vector3 &newPos,
	const MotionConstants & mc, bool shouldSlide, bool reverseCheck ) const
{
	BW_GUARD;
	bool collided = false;

	ChunkSpacePtr pSpace = pSlave_->pSpace();
	if (!pSpace) return collided;

	CollisionInfo ci;
	if (!collisionSetup( newPos, oldPos, mc, shouldSlide, reverseCheck, ci ))
	{
		return collided;
	}

	CollisionAdvance collisionAdvance( ci.bottomLeft_,
		ci.widthVector_, ci.heightVector_, ci.flatDir_, ci.maxAdvance_ + ci.frontDistance_ );
	collisionAdvance.ignoreFlags( TRIANGLE_NOCOLLIDE );

	// For now, we are just sweeping the triangle that has one edge as the base
	// of the rectangle and the other vertex is at the top, centre.
	WorldTriangle sweepTriangle(
		ci.bottomLeft_,
		ci.bottomLeft_ + ci.heightVector_ + ci.halfWidthVector_,
		ci.bottomLeft_ + ci.widthVector_ );

#if DEBUG_PHYSICS
	if (g_debugDrawCollisions)
	{
		DWORD cols[4] = { 0x00ffffff, 0x00808080, 0x0000ff00, 0x00008000 };
		DWORD col = cols[ (int)shouldSlide + 2 * (int)reverseCheck ];
		s_dtriadd( sweepTriangle, col );
		WorldTriangle bufTri(
			sweepTriangle.v0() + ci.frontBuffer_,
			sweepTriangle.v1() + ci.frontBuffer_,
			sweepTriangle.v2() + ci.frontBuffer_ );
		s_dtriadd( bufTri, col );
		WorldTriangle destTri(
			sweepTriangle.v0() + ci.delta_ + ci.frontBuffer_,
			sweepTriangle.v1() + ci.delta_ + ci.frontBuffer_,
			sweepTriangle.v2() + ci.delta_ + ci.frontBuffer_ );
		s_dtriadd( destTri, col );
	}	
#endif

	g_inPrevCol = true;
	pSpace->collide( sweepTriangle, ci.dest_, collisionAdvance );
	g_inPrevCol = false;

	float clear = collisionAdvance.advance() - ci.frontDistance_;

	Vector3 realDir = ci.flatDir_;
	realDir.y = (newPos.y - oldPos.y) / ci.maxAdvance_;

	newPos = oldPos;

	if (clear > 0.f)
	{
		// move as far as we can
		newPos += realDir * clear;
	}
	else
	{
		clear = 0.f;
	}

	collided = (clear < ci.maxAdvance_ - 0.0001f);
	if (shouldSlide && collided)
	{
		const float hitDepth = ci.maxAdvance_ - clear;
		Vector3 leftOver = hitDepth * ci.flatDir_;

		// Want to do this except with a horizontal normal.
		// collisionAdvance.hitTriangle().bounce( leftOver, g_collisionBounce /*0.f*/ );
		Vector3 normal = collisionAdvance.hitTriangle().normal();
		normal.y = 0.f;
		normal.normalise();

		// blend between projected and full
		// velocity when sliding against a wall.
		leftOver -= normal.dotProduct(leftOver) * normal;
		Vector3 newDir = leftOver;
		newDir.normalise();

		const Vector3 currPos = newPos;

		// the blending factor is the dot product between collision
		// normal and velocity, squared. Use alpha = 0 if you want
		// to use the full velocity all the time.
		float alpha = 1; //sqr(normal.dotProduct(flatDir));
		newPos += alpha * leftOver + (1-alpha) * newDir * hitDepth;
		this->preventCollision( currPos, newPos, mc, false );
	}

	if (g_standStill)
	{
		newPos = oldPos;
	}
	return collided;
}


/**
 *	This function applies gravity to the new position.
 *
 *	@param	vehicleID	vehcile the player is on
 *	@param	oldPos		previous position of player
 *	@param	newPos		desired position of player
 *	@param	mc			MotionConstants information
 *	@param	deltaTime	Delta time
 *
 *	@return Changed dropping time
 */
float Physics::applyGravityAndBuoyancy( EntityID & vehicleID, const Vector3 & oldPos, Vector3 & newPos,
	const MotionConstants & mc, float deltaTime ) const
{
	BW_GUARD;
	EntityID oldVehicleID = vehicleID;
	Vector3 testPos = newPos;
	testPos.y = oldPos.y;

	// make sure we have a space
	ChunkSpacePtr pSpace = pSlave_->pSpace();
	if (!pSpace)
		return fallingVelocity_;

	// see if we can get out of falling because something is unloaded
	ChunkEmbodiment * pModel = pSlave_->pPrimaryEmbodiment();
	if (pModel != NULL)
	{
		// no idea of chunk, so don't fall
		if (pModel->chunk() == NULL)
			return fallingVelocity_;

		if (pModel->chunk()->isOutsideChunk())
		{
			const ChunkOverlappers::Overlappers & overlappers =
				ChunkOverlappers::instance( *pModel->chunk() ).overlappers();
			for (uint i = 0; i < overlappers.size(); i++)
			{
				ChunkOverlapper * pOverlapper = &*overlappers[i];
				if (pOverlapper->focussed()) continue;

				// near an unloaded overlapper, so don't fall until it's loaded
				if (!pOverlapper->bbReady() || pOverlapper->bb().intersects( testPos ))
					return fallingVelocity_;
			}
		}
	}

	// ok we're definitely interested in falling then

	// first calculate any accumulated drop
	float newFallingVelocity = fallingVelocity_ + (gravity_ * deltaTime);

	if (inWater_ && newPos.y <= waterSurfaceHeight_)
	{
		newFallingVelocity -= buoyancy_ * deltaTime;

		if (newFallingVelocity > 0.f)
		{
			// Sinking
			newFallingVelocity -= viscosity_ * deltaTime;

			if (newFallingVelocity < 0.f)
			{
				newFallingVelocity = 0.f;
			}
		}
		else
		{
			// Floating
			newFallingVelocity += viscosity_ * deltaTime;

			if (newFallingVelocity > 0.f)
			{
				newFallingVelocity = 0.f;
			}
		}
	}

	float dropDistance = ( fallingVelocity_ + newFallingVelocity ) / 2 * deltaTime;

	if (inWater_ && newPos.y - dropDistance > waterSurfaceHeight_)
	{
		dropDistance = newPos.y - waterSurfaceHeight_;
		// Make sure the velocity gets capped here as well so that the velocity
		// doesn't start "floating" above the surface.
		newFallingVelocity = 0.f;
	}

	// we only need to check the max distance we can fall
	float maxDrop = dropDistance;

	if (oldVehicleID != 0)
	{
		// check the existing vehicle is still valid 
		// (e.g. we might have just teleported outside its AoI)
		Entity * pVehicle = pSlave_->pVehicle();
		if ( pVehicle != NULL && pVehicle->isInWorld() ) 
		{
			// check falling within vehicle reference frame.
			Position3D vpos = pVehicle->position();
			ChunkEmbodiment * pVModel = pVehicle->pPrimaryEmbodiment();
			BoundingBox bb;
			pVModel->localBoundingBox( bb, true );
			float vModelHeight = bb.maxBounds().y - bb.minBounds().y;

			maxDrop += vModelHeight + testPos.y - vpos.y;
			testPos.y += vModelHeight;
			dropDistance += vModelHeight;
		}
		else
		{
			oldVehicleID = 0;
		}
	}

	maxDrop += mc.scrambleHeight;
	dropDistance += mc.scrambleHeight;
	testPos.y += mc.scrambleHeight;

	// TODO: Should pass in the flat direction.
	Vector3 flatDir( 0.f, 0.f, 1.f );

	Vector3 perpFlatDir( flatDir.z, 0.f, -flatDir.x );

	// HACK:
//	modelDepth *= 0.1f;
//	modelWidth *= 0.1f;
	float modelWidth;
	float modelDepth;

	modelDepth = modelWidth = std::min( mc.modelDepth, mc.modelWidth );

	// Find the corners of the triangle to test.
	Vector3 corners[4];
	corners[0] = testPos - flatDir * modelDepth*0.5f - perpFlatDir * modelWidth*0.5f;
	corners[1] = testPos - flatDir * modelDepth*0.5f + perpFlatDir * modelWidth*0.5f;
	corners[2] = testPos + flatDir * modelDepth*0.5f - perpFlatDir * modelWidth*0.5f;
	corners[3] = testPos + flatDir * modelDepth*0.5f + perpFlatDir * modelWidth*0.5f;

	// make the object that checks them
	CollisionAdvance collisionAdvance( corners[0],
		corners[1] - corners[0], corners[2] - corners[0],
		maxDrop > 0.f ? Vector3( 0.f, -1.f, 0.f ) : Vector3( 0.f, 1.f, 0.f ),
		fabsf( maxDrop ) );

	collisionAdvance.ignoreFlags( TRIANGLE_NOCOLLIDE );
//	collisionAdvance.shouldFindCentre( true );

	// and test the two triangles
	WorldTriangle	dropOne( corners[0], corners[3], corners[1] );
	WorldTriangle	dropTwo( corners[0], corners[2], corners[3] );
	Vector3 dest( corners[0][0], corners[0][1] - maxDrop - 0.1f, corners[0][2] );

#if DEBUG_PHYSICS
	if (g_debugDrawCollisions)
	{
		// DEBUG: Draw the sweep triangle in orange
		s_dtriadd( dropOne, 0x00ff8000 );
		s_dtriadd( dropTwo, 0x00ff8000 );
		//s_darrowadd( stPos, toPos, 0x00ff8000 );

		Vector3 delta = dest - corners[0];
		WorldTriangle dropOneDest( corners[0] + delta, corners[3] + delta, corners[1] + delta );
		WorldTriangle dropTwoDest( corners[0] + delta, corners[2] + delta, corners[3] + delta );
		s_dtriadd( dropOneDest, 0x008000ff );
		s_dtriadd( dropTwoDest, 0x008000ff );

	}
#endif	

	pSpace->collide( dropOne, dest, collisionAdvance );
	pSpace->collide( dropTwo, dest, collisionAdvance );

	// reset vehicle
	EntityID newVehicleID = 0;
	if (collisionAdvance.advance() != fabsf(maxDrop))
	{
		// see if it's an ChunkEmbodiment with potential vehicle info
		ChunkItemPtr pHitItem = collisionAdvance.hitItem();

		if (pHitItem && (pHitItem->userFlags() & 4))
		{
			ChunkEmbodiment * pCE = (ChunkEmbodiment*)&*pHitItem;
			PyObject * pObj = &*pCE->pPyObject();
			// ask the python object for its vehicle ID
			PyObject * pVID = PyObject_GetAttrString( pObj, "vehicleID" );
			// try to parse it
			if (pVID == NULL || Script::setData( pVID, newVehicleID ) != 0)
			{	// if we got it out then great - otherwise leave it at zero
				PyErr_Clear();
			}
			Py_XDECREF( pVID );
		}
	}

	if (collisionAdvance.advance() < fabsf(dropDistance))
	{
		// Decelerate when we hit the ground, rathe than just resetting 
		// to zero allows the player to better stick to the ground
		// when walking down slopes).
		bool decelerate = true;
		float decelerationCoeff = inWater_ ? 8.0f : 2.0f;
		
		// Don't apply decelleration if we are touching the bottom of the lake
		// and have just started to rise due to bouyancy.
		if (inWater_ && newFallingVelocity <= 0.f)
		{
			decelerate = false;
		}

		if (decelerate)
		{
			newFallingVelocity -= decelerationCoeff * gravity_ * deltaTime;
			if (newFallingVelocity < 0.f) 
			{
				newFallingVelocity = 0.f;
			}
		}
	}

	// make sure we are within the bounds of the chunk space!
	testPos.x = Math::clamp( pSpace->minCoordX() + 0.05f,
							testPos.x,
							pSpace->maxCoordX() - 0.05f );
	testPos.z = Math::clamp( pSpace->minCoordZ() + 0.05f,
							testPos.z,
							pSpace->maxCoordZ() - 0.05f );

	if (vehicleID != 0 &&
		fabsf( dropDistance ) < collisionAdvance.advance())
	{
		testPos.y -= dropDistance;
	}
	else
	{
		if (dropDistance < 0.f)
		{
			testPos.y += collisionAdvance.advance();
		}
		else
		{
			testPos.y -= collisionAdvance.advance();
		}
	}

	if (oldVehicleID != newVehicleID)
	{
		vehicleID = newVehicleID;
		newFallingVelocity = 0;
	}

	newPos = testPos;

	return newFallingVelocity;
}



/**
 *  Static method to update the joystick velocity
 *	from this axis event, for all Physics instances
 */
bool Physics::handleAxisEventAll( const AxisEvent & event )
{
	BW_GUARD;
	bool handled = true;

	Physicians::iterator it;
	for (it = s_physicians_.begin(); it != s_physicians_.end(); it++)
	{
		// yes, we give it to all of them, even if an earlier one handles it
		Physics * pPhys = *it;
		if (pPhys->pSlave_ != NULL)
			handled |= pPhys->handleAxisEvent( event );
	}

	return handled;
}


/**
 *  Update the joystick velocity from this axis event.
 */
bool Physics::handleAxisEvent( const AxisEvent & event )
{
	BW_GUARD;
	bool handled = true;

	switch (event.axis())
	{
		case AxisEvent::AXIS_LX:	joystickPos_[0] = event.value();	break;
		case AxisEvent::AXIS_LY:	joystickPos_[1] = event.value();	break;
		default:					handled = false;					break;
	}

	if (pAxisEventNotifier_ && joystickPos_.lengthSquared() >=
											axisEventNotifierThreshold_)
	{
		Py_INCREF( pAxisEventNotifier_.getObject() );
		Script::call(
			pAxisEventNotifier_.getObject(),
			Py_BuildValue( "(ii)", int( event.axis() ), int( event.value() ) ),
			"Physics axisEvent notifier callback: " );
	}

	return handled;
}


/**
 *	This method calculates the joystick velocity from the current
 *	joystick position.
 */
void Physics::calculateJoystickVelocity()
{
	BW_GUARD;
	//use the last set joystick values
	float strafeValue = joystickPos_[0];
	float thrustValue = joystickPos_[1];

	//and clear them for next time
	joystickPos_[0] = 0.f;
	joystickPos_[1] = 0.f;

	//adjust thrust so that absolute left/right actually involves some
	// forward motion (note: currently unused)
	//float adjustedThrust = thrustValue + fabsf( strafeValue ) * 0.05f;
	//bool isForward = ( adjustedThrust >= 0.f );

	//check the length of the hypotenuse, clamp it to 1.f
	Vector2 newPosition( strafeValue, thrustValue );

	float lenSq = newPosition.lengthSquared();
	if (lenSq > 1.f)
	{
		newPosition.normalise();
		strafeValue = newPosition.x;
		thrustValue = newPosition.y;
	}

	//cancel out allow movements less than 20% of the max
	if (lenSq < MOVEMENT_DEAD_ZONE)
	{
		strafeValue = 0.f;
		thrustValue = 0.f;
	}

	//calculate the final joystick velocity values
	if ( thrustValue >= 0.f )
	{
		joystickVelocity_[0] = strafeValue * runFwdSpeed_;
		joystickVelocity_[2] = thrustValue * runFwdSpeed_;
	}
	else
	{
		joystickVelocity_[0] = strafeValue * runBackSpeed_;
		joystickVelocity_[2] = thrustValue * runBackSpeed_;
	}
}









/*~ function Physics.teleportSpace
 *
 *	This function moves the entity into the specified space.
 *	The change takes effect the next time Physics is run.
 *	This call can be combined with the other teleport calls.
 *
 *	@param	spaceID		The id of the space to teleport to
 */
/**
 *	This method teleports us to the given space.
 */
PyObject * Physics::teleportSpace( SpaceID spaceID )
{
	BW_GUARD;
	ChunkSpacePtr pNewSpace =
		ChunkManager::instance().space( spaceID, true );
	// TODO: Specify 'false' in space accessor above when we have gateways
	if (!pNewSpace)
	{
		PyErr_Format( PyExc_ValueError, "Physics.teleportSpace(): "
			"No such space ID %d\n", spaceID );
		return NULL;
	}

	if (!teleportSpace_)
		teleportVehicle_ = EntityID(-1);

	teleportSpace_ = pNewSpace;

	Py_Return;
}

/*~ function Physics.teleportVehicle
 *
 *	This function moves the entity into the specified vehicle.
 *	The change takes effect the next time Physics is run.
 *	This call can be combined with the other teleport calls.
 *  Note that a vehicle can be a moving platform or a more
 *  conventional transportation device (ie, car).  Because of
 *  this the physics fall attribute plays a major role in
 *  determining whether the player remains on the vehicle.
 *  For moving platforms, the fall attribute should remain as
 *  1, so when a player steps off the edge, the will 'leave'
 *  the vehicle.  In other cases, such as when a player is to
 *  control the vehicle, the fall attribute should be set to 0.
 *  For controlling a vehicle, refer to BigWorld.controlEntity()
 *  and the wards attribute on the Proxy...
 *
 *	@param	vehicleID	The id of the vehicle to teleport to
 */
/**
 *	This method teleports us to the given vehicle.
 */
PyObject * Physics::teleportVehicle( EntityPtr pVehicle )
{
	BW_GUARD;
	if (pVehicle && !pVehicle->isInWorld())
	{
		PyErr_SetString( PyExc_ValueError, "Physics.teleportVehicle(): "
			"Can only teleport to vehicles that are in the world\n" );
		return NULL;
	}

	if (!teleportSpace_)
		teleportSpace_ = pSlave_->pSpace();

	teleportVehicle_ = pVehicle ? pVehicle->id() : 0;

	Py_Return;
}


/*~ function Physics.teleport
 *
 *	This function moves the entity to the specified location, and optionally,
 *	direction. The change takes effect the next time Physics is run (this
 *	frame or next, depending on when the call is made - usually this frame)
 *	This call can be combined with the other teleport calls.
 *
 *	@param	position	A Vector3 that is the position to teleport to
 *	@param	direction	An optional Vector3 which if specified is the direction
 *						to teleport to (yaw, pitch, roll), otherwise the
 *						direction is unchanged.
 */
/**
 *	This method allows scripts to instantaneously teleport (next frame)
 */
void Physics::teleport( const Vector3 & pos, const Vector3 & dir )
{
	teleportPos_ = pos;
	teleportDir_ = dir;
}


/**
 *	This method cancels a pending teleport of this entity.
 */
void Physics::cancelTeleport()
{
	teleportPos_.x = FLT_MAX;
	teleportDir_.x = FLT_MAX;
}


/*~ function Physics.seek
 *
 *	The seek function is called to make the entity move to a particular
 *	location and yaw.  It moves at the speed contained in the velocity
 *	attribute of Physics.  It turns towards the destination, and moves in a
 *	straight line towards it until it is close, and then turns to face the
 *	desired yaw.  If it gets blocked by geometry, it keeps trying to move until
 *	it times out.
 *
 *	Once it either arrives or times out, it calls a callback function.
 *
 *  To cancel, call the seek function passing None for destination and callback.
 *
 *	@param	destination		This is a Vector4, with the first three elements
 *							being the location to seek to, the fourth being
 *							the yaw to seek to.
 *	@param	timeout			The time in seconds to seek for, before timing out.
 *							Use a negative value to have no time limit.
 *	@param	verticalRange	How close the entity has to be vertically to the
 *							destination to be considered to have arrived.
 *	@param	callback		The callable object to call once the seek ends.  It
 *							gets one argument, which is 1 if the seek
 *							succeeded, 0 otherwise.
 */
/**
 *	This method allows scripts to set the seek target
 */
PyObject * Physics::py_seek( PyObject * args )
{
	BW_GUARD;
	PyObject	* pyPose;
	Vector4		pose(0.f,0.f,0.f,0.f);
	float		timeLimit = -1.f;
	float		heightTolerance = 0.10f;
	PyObject	* pyCallback = NULL;
	bool		seeking;

	bool	good = false;
	if (PyArg_ParseTuple( args, "O|ffO",
		&pyPose, &timeLimit, &heightTolerance, &pyCallback ))
	{
		if (pyCallback == Py_None) pyCallback = NULL;

		if (pyCallback == NULL ||
			PyCallable_Check( pyCallback ))
		{
			if (pyPose == Py_None)
			{
				seeking = false;
				good = true;
			}
			else
			{
				seeking = true;
				good = (Script::setData( pyPose, pose ) == 0);
			}
		}
	}

	if (!good)
	{
		PyErr_SetString( PyExc_TypeError, "Argument parsing error: "
			"Physics.seek( Vector4 pose, float timeLimit, "
			"float heightTolerance, Callable callback )" );
		return NULL;
	}
	else
	{
		seeking_ = seeking;

		seek_ = pose;

		if (timeLimit >= 0.f )
		{
			seekDeadline_ = App::instance().getTime() + timeLimit;
		}
		else
		{
			seekDeadline_ = -1.f;
		}

		seekHeightTolerance_ = heightTolerance;

		PyObject * pOldCallback = seekCallback_;
		seekCallback_ = pyCallback;
		Py_XINCREF( seekCallback_ );
		if (pOldCallback != NULL)
		{
			// call old callback with failed ... but make sure this
			// if after all variables have been set.
			Script::call( pOldCallback, Py_BuildValue( "(i)", 0 ),
				"Physics::seekCancelled: ", true );
		}

		Py_Return;
	}
}



/*~ function Physics.chase
 *
 *	Calling the chase function makes the entity attached to the Physics start
 *	chasing the specified entity.  It faces towards it, and attempts to remain
 *	the specified distance away from it.  If it is within the optionally
 *	specified tolerance of the desired distance, then it doesn't bother trying
 *	to move away. Note that the entity will not move unless you also have
 *	physics.velocity set. The z component of velocity is the forward speed,
 *	and the x component of velocity is the strafe speed.
 *
 *	@param	entity		The Entity to chase after.
 *	@param	distance	The distance to try and remain from the entity.
 *	@param	tolerance	The optional tolerance range for distance.  If the
 *						entity is within this, it doesn't bother moving.
 */
/**
 *	This method sets a target entity to chase
 */
PyObject * Physics::py_chase( PyObject * args )
{
	BW_GUARD;
	PyObject * pTarget;
	float dist;
	float tol = 0.f;

	int nargs = PyTuple_Size(args);
	if ((nargs != 2 && nargs != 3) ||
		(!Entity::Check( pTarget = PyTuple_GetItem(args,0) ) && pTarget != Py_None) ||
		Script::setData( PyTuple_GetItem(args,1), dist ) != 0 ||
		(nargs == 3 && Script::setData( PyTuple_GetItem(args,2), tol ) != 0)
		)
	{
		PyErr_SetString( PyExc_TypeError,
			"Physics.chase( Entity, distance[, tolerance] ) got incorrect types" );
		return NULL;
	}

	Py_XDECREF( chaseTarget_ );

	chaseTarget_ = (pTarget == Py_None ? NULL : static_cast<Entity*>(pTarget));
	chaseDistance_ = dist;
	chaseTolerance_ = tol;

	Py_XINCREF( chaseTarget_ );

	Py_Return;
}



/**
 *	This method allows scripts to read the current physics settings
 */
PyObject * Physics::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}



/**
 *	This method allows scripts to control how the physics works.
 */
int Physics::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}




/**
 *	General method to get a Physics::MouseAxis as a python object
 *	TODO: Make this work with the general PyObjectPlus-sponsored system
 */
PyObject * getData( const Physics::MouseAxis data )
{
	BW_GUARD;
	char * str = NULL;
	switch (data)
	{
		case Physics::MA_MouseX:		str = "MouseX";		break;
		case Physics::MA_MouseY:		str = "MouseY";		break;
		case Physics::MA_Direction:		str = "Direction";	break;
		default:						Py_Return;			break;
	}
	return PyString_FromString( str );
}


/**
 *	General method to set a Physics::MouseAxis from a python object
 *	TODO: Make this work with the general PyObjectPlus-sponsored system
 */
int setData( PyObject * pObject, Physics::MouseAxis & rData,
		const char * varName )
{
	BW_GUARD;
	rData = Physics::MA_None;

	if (PyString_Check( pObject ))
	{
		char * valueStr = PyString_AsString( pObject );
		if (streq( valueStr, "MouseX" ))
		{
			rData = Physics::MA_MouseX;
		}
		else if (streq( valueStr, "MouseY" ))
		{
			rData = Physics::MA_MouseY;
		}
		else if (streq( valueStr, "Direction" ))
		{
			rData = Physics::MA_Direction;
		}
	}

	return 0;
}



/**
 *	Special get method for python attribute 'velocityMouse'
 */
PyObject * Physics::pyGet_velocityMouse()
{
	BW_GUARD;
	return getData( velocityMouse_ );
}

/**
 *	Special set method for python attribute 'velocityMouse'
 */
int Physics::pySet_velocityMouse( PyObject * value )
{
	BW_GUARD;
	return setData( value, velocityMouse_, "velocityMouse" );
}


/**
 *	Special get method for python attribute 'angularMouse'
 */
PyObject * Physics::pyGet_angularMouse()
{
	BW_GUARD;
	return getData( angularMouse_ );
}

/**
 *	Special set method for python attribute 'angularMouse'
 */
int Physics::pySet_angularMouse( PyObject * value )
{
	BW_GUARD;
	return setData( value, angularMouse_, "angularMouse" );
}


/**
 *	get method for python attribute 'fall'
 */
bool Physics::fall() const
{
	return fall_;
}


/**
 *	set method for python attribute 'fall'
 */
void Physics::fall( bool fall )
{
	BW_GUARD;
	bool oldFall = fall_;
	fall_ = fall;
	if (oldFall != fall_) fallingVelocity_ = 0.f;
}




PY_TYPEOBJECT( Physics )

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE Physics::

PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, fall, fall )

PY_BEGIN_METHODS( Physics )
	PY_METHOD( teleportSpace )
	PY_METHOD( teleportVehicle )
	PY_METHOD( teleport )

	PY_METHOD( seek )
	PY_METHOD( chase )
	PY_METHOD( stop )
		
	PY_METHOD( movingForward )

PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Physics )
	/*~ attribute Physics.velocity
	 *
	 *	The velocity in metres per second that the entity is trying to move at.
	 *	Forces acting on the entity do not change its velocity attribute.
	 *	If the entity collides with a wall, It will stop moving, but the
	 *	velocity attribute will remain set to its previous value.
	 *
	 *	The ActionMatcher doesn't use this attribute when choosing an action.
	 *	Instead, it matches using the displacement of the model from its
	 *	previous position.
	 *
	 *	@type	Vector3
	 */
	PY_ATTRIBUTE( velocity )
	/*~ attribute Physics.smoothingTime
	 *
	 *	For the Player entity, changes in velocity are smoothed over a
	 *	set amount of time to simulate momentum.  This smoothing time is
	 *	specified in seconds.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( smoothingTime )
	/*~ attribute Physics.uncorrectedVelocity
	 *
	 *	The velocity in metres per second that the entity actually moved at.
	 *	This takes into account the intended velocity, slowing down for
	 *	moving up slopes, gravity, etc.  It does not take into account
	 *	physics corrections for horizontal collisions or hitting the maximum 
	 *	slope limit. Note that the vertical velocity does take collision due 
	 *	to gravity into account.
	 *
	 *	The uncorrectVelocity is a Vector4Provider and its main raison d'etre
	 *	is to be used as a velocityProvider for the ActionMatcher.  Hooking up
	 *	these two can provide action matching that is slightly less twitchy
	 *	when colliding with objects, depending on how your actions are setup.
	 *
	 *	@type	Read-only Vector4Provider
	 */
	PY_ATTRIBUTE( uncorrectedVelocity )
	/*~ attribute Physics.joystickEnabled
	 *
	 *	If True, then the physics will take input from the connected joystick.
	 *	Defaults to False.
	 *
	 *	@type	Boolean
	 */
	PY_ATTRIBUTE( joystickEnabled )
	/*~ attribute Physics.nudge
	 *
	 *	Setting this attribute to a particular Vector3 adds that vector to the
	 *	entity's position in the next tick. The value is set back to the zero
	 *	vector after every tick. It can be read from but, unless it is read
	 *	immediately after setting it, it will always read as the zero vector.
	 *
	 *	@type	Vector3
	 */
	PY_ATTRIBUTE( nudge )
	/*~ attribute Physics.velocityMouse
	 *
	 *	The velocity mouse determines what role the mouse has in determining
	 *	the velocity of the entity.	It is a string, which can take three
	 *	(case sensative) possible values:
	 *
	 *	- "Direction"  which means that the mouse affects the direction of the
	 *	velocity only.  The	magnitude is as specified in the velocity
	 *	attribute, in the direction specified in the velocity attribute, as
	 *	applied relative to the frame of reference of the directioncursor.
	 *
	 *	- "MouseX" which means that horizontal movement of the mouse is used to
	 *	scale the magnitude	of the velocity attribute.  Direction is entirely
	 *	specified by the velocity attribute.  In order for this to be useful
	 *	the velocity attribute needs to have a scaling factor built in to it,
	 *	meaning	it is no longer specified in m/s, and in general needs to be
	 *	considerably bigger than it would normally be.  For example, (0,0,100)
	 *	moves the entity at roughly a walking pace.
	 *
	 *	- "MouseY" which means that vertical movement of the mouse is used to
	 *	scale the magnitude	of the velocity attribute.  Direction is entirely
	 *	specified by the velocity attribute.  In order for this to be useful
	 *	the velocity attribute needs to have a scaling factor built in to it
	 *	meaning	it is no longer specified in m/s, and in general needs to be
	 *	considerably bigger than it would normally be.  For example, (0,0,100)
	 *	moves the entity at roughly a walking pace.
	 *
	 *	@type String
	 */
	PY_ATTRIBUTE( velocityMouse )
	/*~ attribute Physics.userDriven
	 *
	 *	This attribute is designed for use with a joystick, rather than a
	 *	keyboard.  When using a joystick if this is set to zero, then the user
	 *	is not controlling the entity, otherwise they are.  When a keyboard is
	 *	the primary form of input, this has no effect.
	 *
	 *	This can be used on any entity, regardless of whether it is the player
	 *	entity or not.
	 *
	 *	@type Integer
	 */
	PY_ATTRIBUTE( userDriven )
	/*~ attribute Physics.angular
	 *
	 *	Entity angular velocity, in radians per second. The entity will try
	 *	to turn at this pace. The angular velocity will only be used if the
	 *	entity is not chasing another entity (see chase) nor seeking a specified
	 *	position (see seek).
	 *	@see chase
	 *	@see seek
	 *	@type	Float
	 */
	PY_ATTRIBUTE( angular )
	/*~ attribute Physics.turn
	 *
	 *	In avatar style physics, setting this attribute to a particular value
	 *	adds that value to the entity's yaw in the next tick. The value is set
	 *	back to zero after every tick. It can be read from but, unless it is
	 *	read immediately after setting it, it will always read as zero.
	 *
	 *	This attribute is not supported on physics styles other than avatar.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( turn )
	/*~ attribute Physics.angularMouse
	 *
	 *	This attribute is no longer supported.
	 */
	PY_ATTRIBUTE( angularMouse )
	/*~ attribute Physics.userDirected
	 *
	 *	This attribute determines whether the user is controlling the entity, or
	 *	it is being controlled by script.  If it is set to non-zero, then the
	 *	direction cursor is used to determine direction, otherwise yaw and
	 *	pitch can be directly set.
	 *
	 *	If this is set to non-zero on an entity which is not the player entity,
	 *	it also will use the direction cursor to determine direction.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( userDirected )
	/*~ attribute Physics.dcLocked
	 *
	 *	If this attribute is set to non-zero and there is a targetSource and
	 *	targetDest attribute set, then the entity will face in the
	 *	direction from targetSource to targetDest.  If this attribute is set to
	 *	zero, then this behaviour is switched off.
	 *
	 *	@type Integer
	 */
	PY_ATTRIBUTE( dcLocked )

	/*~ attribute Physics.collide
	 *
	 *	This attribute determines whether or not the entity performs collision
	 *	testing with models in the world.  If it is non-zero, then if the
	 *	entity collides with anything which it cannot walk up, it tries to slide
	 *	along it.  If this fails, then it stops.  If the attribute is zero,
	 *	then the entity's behaviour depends on the fall attribute.  If fall is
	 *	zero, then the entity will pass through objects without colliding,
	 *	although it glides over terrain.  If fall is non-zero, then the entity
	 *	will glide over both objects and terrain.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( collide )
	/*~ attribute Physics.fall
	 *
	 *	This attribute determines whether gravity effects the entity or not.
	 *	If it is non-zero, then it will fall towards the ground at a rate
	 *	determined using the gravity attribute.  If it is zero then the entity
	 *	will not fall towards the ground at all.
	 */
	PY_ATTRIBUTE( fall )
	/*~ attribute Physics.fallingVelocity
	 *
	 *	This read only attribute shows the current vertical velocity of the
	 *	entity due to gravity and/or buoyancy. Positive means falling.
	 *
	 *	@type	Read-Only Float
	 */
	PY_ATTRIBUTE( fallingVelocity )
	/*~ attribute Physics.gravity
	 *
	 *	This attribute determines how quickly the entity falls towards the
	 *	ground.  It is an acceleration measured in metres per second per second.
	 *	It defaults to 9.8.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( gravity )

	/*~ attribute Physics.thrust
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( thrust )
	/*~ attribute Physics.brake
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( brake )
	/*~ attribute Physics.thrustAxis
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( thrustAxis )

	/*~ attribute Physics.joystickFwdSpeed
	 *
	 *	This attribute is the speed (in metres per second) at which the entity
	 *	will move forward if the joystick is being used as a controller and is
	 *	pressed forwards.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( joystickFwdSpeed )
	/*~ attribute Physics.joystickBackSpeed
	 *
	 *	This attribute is the speed (in metres per second) at which the entity
	 *	will move backward if the joystick is being used as a controller and is
	 *	pressed backwards.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( joystickBackSpeed )

	/*~ attribute Physics.seeking
	 *
	 *	This read only attribute is set to 1 by the system while seeking
	 *	towards a particular point, initiated by calling the seek function.
	 *	Once the entity has arrived or failed, seeking is set to 0 again.
	 *
	 *	@type	Read-Only Integer
	 */
	PY_ATTRIBUTE( seeking )

	/*~ attribute Physics.targetSource
	 *
	 *	If the dcLocked attribute is set to non-zero, then the entity will face
	 *	in the yaw from the location of	targetSource to the location of
	 *	targetDest.
	 *
	 *	@type	MatrixProvider
	 */
	PY_ATTRIBUTE( targetSource )
	/*~ attribute Physics.targetDest
	 *
	 *	If the dcLocked attribute is set to non-zero, then the entity will face
	 *	in the yaw from the location of	targetSource to the location of
	 *	targetDest.
	 *
	 *	@type	MatrixProvider
	 */
	PY_ATTRIBUTE( targetDest )

	/*~ attribute Physics.modelHeight
	 *
	 *	This attribute is used to override the bounding box height of the
	 *	model, which is used for physics collision tests.  If this is set to
	 *	zero, then the value from the bounding box of the model is used instead,
	 *	otherwise this attribute is.
	 *
	 *	@type Float
	 */
	PY_ATTRIBUTE( modelHeight )
	/*~ attribute Physics.modelWidth
	 *
	 *	This attribute is used to override the bounding box width of the
	 *	model, which is used for physics collision tests.  If this is set to
	 *	zero, then the value from the bounding box of the model is used
	 *	instead, otherwise this attribute is.
	 *
	 *	@type Float
	 */
	PY_ATTRIBUTE( modelWidth )
	/*~ attribute Physics.modelDepth
	 *
	 *	This attribute is used to override the bounding box depth of the model,
	 *	which is used for physics collision tests.  If this is set to zero,
	 *	then the value from the bounding box of the model is used instead,
	 *	otherwise this attribute is.
	 *
	 *	@type Float
	 */
	PY_ATTRIBUTE( modelDepth )

	/*~ attribute Physics.scrambleHeight
	 *
	 *	This attribute is the height of the step up which the entity can climb.
	 *	When moving, it performs a collision test this high above the bottom of
	 *	the entity, in the direction of motion. If it hits anything, then the
	 *	entity is unable to move, otherwise it moves.
	 *
	 *  If it is set to zero, then Physics will use 1/3 of the height of the
	 *	model instead.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( scrambleHeight )

	/*~ attribute Physics.maximumSlope
	 *
	 *	This attribute is the angle, in degrees, of the maximum slope the entity
	 *	can move onto.	 	 
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( maximumSlope )	

	/*~ attribute Physics.inWater
	 *
	 *	This attribute tells the physics module whether it should treat the
	 *	entity as being in water or not.  If inWater is set to True, then
	 *	the buoyancy, viscosity and waterSurfaceHeight come into effect.	 
	 *
	 *	@type	Boolean
	 */
	PY_ATTRIBUTE( inWater )

	/*~ attribute Physics.waterSurfaceHeight
	 *
	 *	This attribute tells the physics module where the surface of the
	 *	water is.  It is used to make the entity float up to the appropriate
	 *	height.  It is only used if inWater is set to True.	 
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( waterSurfaceHeight )

	/*~ attribute Physics.buoyancy
	 *
	 *	This attribute affects how quickly an entity rises to the surface
	 *	of the water (set by waterSurfaceHeight) when it is in the water
	 *	(set by the inWater flag). Units are in m/s. You should set its value
	 *	greater than gravity to enable the entity floating up in water.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( buoyancy )

	/*~ attribute Physics.viscosity
	 *
	 *	This attribute affects how quickly an entities falling speed is
	 *	damped when it has entered water (set by the inWater flag). Units
	 *	are in m/s.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( viscosity )

	/*~ attribute Physics.chasing
	 *
	 *	This read only attribute is set to 1 if the entity is chasing a target,
	 *	0 otherwise.  Chasing is a behaviour where the physics moves the entity
	 *	towards a target entity, trying to remain a specified distance from it.
	 *	It is initiated by calling the chase method.
	 *
	 *	@type	Read-Only Integer
	 */
	PY_ATTRIBUTE( chasing )
	/*~ attribute Physics.moving
	 *
	 *	This read only attribute is set to 1 by the system if the entity moved
	 *	during the last tick, and to 0 otherwise.  If velocity is non-zero, but
	 *	it is unable to move due to geometry constraints, then the attribute
	 *	will be zero.
	 *
	 *	@type	Read-Only Integer
	 */
	PY_ATTRIBUTE( moving )

	PY_ATTRIBUTE( axisEventNotifier )
	PY_ATTRIBUTE( axisEventNotifierThreshold )
	/*~ attribute Physics.isMovingNotifier
	 *
	 *	This attribute is a callback function, which is called whenever the
	 *	entity transitions between moving and not moving or from not moving to
	 *	moving.  The function will be supplied with one argument, which is the
	 *	isMoving attribute.  It will be 1 if the entity has started moving, and
	 *	0 if it stops.
	 *
	 *	@type	Callable Object, taking one argument
	 */
	PY_ATTRIBUTE( isMovingNotifier )

	/*~ attribute Physics.ripperDesiredHeightFromGround
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperDesiredHeightFromGround )
	/*~ attribute Physics.ripperMinHeightFromGround
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperMinHeightFromGround )
	/*~ attribute Physics.ripperMaxThrust
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperMaxThrust )
	/*~ attribute Physics.ripperGravity
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperGravity )
	/*~ attribute Physics.ripperXDrag
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperXDrag )
	/*~ attribute Physics.ripperYUpDrag
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperYUpDrag )
	/*~ attribute Physics.ripperYDownDrag
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperYDownDrag )
	/*~ attribute Physics.ripperZDrag
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperZDrag )
	/*~ attribute Physics.ripperBrakeDrag
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperBrakeDrag )
	/*~ attribute Physics.ripperThrustDropOffHeight
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperThrustDropOffHeight )
	/*~ attribute Physics.ripperThrustDropOffRate
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperThrustDropOffRate )
	/*~ attribute Physics.ripperTurnRate
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperTurnRate )
	/*~ attribute Physics.ripperElasticity
	 *
	 *	This attribute is unsupported.
	 */
	PY_ATTRIBUTE( ripperElasticity )
PY_END_ATTRIBUTES()


/**
 *	This method sets the moving attribute of the object. If there is a callback
 *	wanting to know about this, let it know.
 */
void Physics::setIsMoving( bool newValue )
{
	BW_GUARD;
	if (newValue != moving_)
	{
		moving_ = newValue;

		if (pIsMovingNotifier_)
		{
			Py_INCREF( pIsMovingNotifier_.getObject() );
			Script::call(
				pIsMovingNotifier_.getObject(),
				Py_BuildValue( "(i)", int( moving_ ) ),
				"Physics isMoving notifier callback: " );
		}
	}
}










/// Do hover-style processing
void Physics::hoverStyleTick( double timeNow, double timeLast )
{
	BW_GUARD;
	float		dTime = float(timeNow - timeLast);

	collided_ = false;

	// First get the turn and thrust inputs
	if (joystickEnabled_)
	{
		turn_ = joystickPos_[0];
		thrust_ = joystickPos_[1];
		joystickPos_[0] = 0.f;
		joystickPos_[1] = 0.f;
	}

	// TODO: Remove this hack and use the action matcher properly!
	PyObject* pFunction = PyObject_GetAttrString( pSlave_, "checkAnims" );
	Script::call( pFunction,
		PyTuple_New(0),
		"Physics::hoverStyleTick: ", false );

	// OK, get the current pos
	Vector3 oldPos( pSlave_->position() );
	Vector3 position = oldPos;
	float yaw = pSlave_->auxVolatile()[0];
	float pitch = pSlave_->auxVolatile()[1];
	float roll = pSlave_->auxVolatile()[2];

	if (ripper_.zDrag_ < 1.f)
	{
		// Do the hover calculations
		const float DESIRED_ELAPSED_TIME = 0.01f;

		// Quick fix for if this takes longer than 0.01 seconds.
		float remainingTime = min( 0.1f, dTime );

		while (remainingTime > 0.f)
		{
			this->integrateHoverPosition( min(remainingTime, DESIRED_ELAPSED_TIME),
				position, yaw, pitch, roll );
			remainingTime -= DESIRED_ELAPSED_TIME;
		}
	}

	// Tell the entity's filter about it
	Vector3 localPos = position;
	float localAuxVolatile[3];
	localAuxVolatile[0] = yaw;
	localAuxVolatile[1] = pitch;
	localAuxVolatile[2] = roll;

	SpaceID spaceID = pSlave_->pSpace()->id();
	EntityID vehicleID = pSlave_->isOnVehicle() ?
		pSlave_->pVehicle()->id() : 0;

	this->transformCommonToLocal(
		spaceID, vehicleID, localPos, *(Vector3*)localAuxVolatile );

	pSlave_->filter().input( timeNow,
		spaceID, vehicleID, localPos, Vector3::zero(), localAuxVolatile );

	// And move the direction cursor around to match
	// We shouldn't really use the directioncursor as the camera here.
	DirectionCursor::instance().yaw( yaw );
	DirectionCursor::instance().roll( -roll );

	Vector3 displacement(position - oldPos);
	float distance = displacement.length();
	float speed = distance / dTime;
	this->setIsMoving( speed > movementThreshold_ );

	// Collision callbacks
	if ( collided_ )
	{
		PyObject* pFunction = PyObject_GetAttrString( pSlave_, "onCollide" );
		PyObject * pTuple = PyTuple_New( 4 );
		PyTuple_SetItem( pTuple, 0, Script::getData( collisionVector_ ) );
		PyTuple_SetItem( pTuple, 1, Script::getData( collisionPosition_ ) );
		PyTuple_SetItem( pTuple, 2, Script::getData( collisionSeverity_ ) );
		PyTuple_SetItem( pTuple, 3, Script::getData( collisionTriangleFlags_ ) );
		Script::call( pFunction, pTuple, "Physics::hoverStyleTick: ", false );
	}
}






// #include "hover.hpp"	// for static limits
#include "math/integrat.hpp"


// This functor is used in calculating the motion of the hover vehicle. The
// operator() function calculates the derivative of the momentum and position.
// This is then integrated using our integrator.
class HoverFunction
{
public:
	HoverFunction(
		const Physics::HoverSetting *settings,
		const PlaneEq * pPlaneEq,
		float thrustA, float brakeA, const Vector3 & thrustAxisA, Matrix34 & matrixA ) :
		settings_( settings ),
		pPlaneEq_(pPlaneEq),
		thrust(thrustA),
		brake(brakeA),
		thrustAxis(thrustAxisA),
		matrix(matrixA)
	{};


	//const Vector3 thrustAxis(0.f, sin(hover_.thrustAngle()), cos(hover_.thrustAngle()));

	Physics::Data operator()(float t, const Physics::Data & data ) const
	{
		BW_GUARD;	
//		const float GRAVITY = 150.f;	//100.f;//298.f;

		// Calculate the height from the ground.
		float groundHeight = pPlaneEq_ ? pPlaneEq_->y(data.position_.x, data.position_.z) : 0.f;
		float heightFromGround = data.position_.y - groundHeight - settings_->minHeightFromGround_;

		// Set a floor on the height from the ground.

		if (heightFromGround < 0.3f)
		{
			heightFromGround = 0.3f;
		}

		Physics::Data rv = data;
		rv.position_ = data.momentum_;

		// Find the components of the momentum in each direction of the vehicle. We want
		// more drag on sideways momentum than forward momentum, for example.

		float xMomentum = matrix.applyToUnitAxisVector(X_AXIS).dotProduct(data.momentum_);
		float yMomentum = matrix.applyToUnitAxisVector(Y_AXIS).dotProduct(data.momentum_);
		float zMomentum = matrix.applyToUnitAxisVector(Z_AXIS).dotProduct(data.momentum_);

		// Calculate the drag on each component of the momentum.
		const float X_DRAG = settings_->xDrag_; // Affects how much the vehicle slides
		const float Y_DRAG = yMomentum > 0.f ? settings_->yUpDrag_ : settings_->yDownDrag_; // Have more drag when flying up.
		const float Z_DRAG = settings_->zDrag_ + settings_->brakeDrag_ * brake; // More drag if we have the brake on.

		// Create the new momentum after considering drag.
		rv.momentum_ =
			-X_DRAG * xMomentum * matrix.applyToUnitAxisVector(X_AXIS) +
			-Y_DRAG * yMomentum * matrix.applyToUnitAxisVector(Y_AXIS) +
			-Z_DRAG * zMomentum * matrix.applyToUnitAxisVector(Z_AXIS);

		// Add the engine's thrust to the momentum. If the momentum is higher
		// from the ground we want the thrust to have less effect.
		const float heightMultiplier =
			(heightFromGround > settings_->thrustDropOffHeight_) ?
				powf(0.5f, (heightFromGround - settings_->thrustDropOffHeight_)/settings_->thrustDropOffRate_) : 1.f;

		rv.momentum_ += 0.5f * heightMultiplier *
			thrust * matrix.applyVector(thrustAxis);


		// Add a force to keep the vehicle above the ground.
		// Should we use the craft's y axis or the plane equation normal?

		float REPEL = settings_->desiredHeightFromGround_ / heightFromGround;

		if (pPlaneEq_)
		{
			const Vector3 & yAxis = pPlaneEq_->normal();

			if (data.momentum_.dotProduct(yAxis) < 0.f && REPEL > 1.5f)
			{
				REPEL *= REPEL;
			}
			else
			{
				REPEL *= 0.4f * REPEL;
			}

			rv.momentum_ += settings_->gravity_ * REPEL * yAxis;
		}

		rv.momentum_.y -= settings_->gravity_;

		return rv;
	}

private:
	const Physics::HoverSetting *settings_;
	const PlaneEq * pPlaneEq_;
	float thrust;
	float brake;
	const Vector3 & thrustAxis;
	Matrix34 & matrix;
};


/**
 *	This class finds the closest facing triangle hit and records it
 */
class ClosestFacingTriangle : public CollisionCallback
{
public:
	ClosestFacingTriangle( const Vector3 & dir ) :
		dir_( dir ), found_( false ) { }

private:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		BW_GUARD;
		WorldTriangle wt(
			obstacle.transform_.applyPoint( triangle.v0() ),
			obstacle.transform_.applyPoint( triangle.v1() ),
			obstacle.transform_.applyPoint( triangle.v2() ),
			triangle.flags() );

		Vector3 normal;
		normal.crossProduct(wt.v1() - wt.v0(), wt.v2() - wt.v0());
		if (normal.dotProduct( dir_ ) > 0.f)
		{
			if (!(wt.flags() & TRIANGLE_DOUBLESIDED))
				return COLLIDE_ALL;
			hit_ = WorldTriangle( wt.v0(), wt.v2(), wt.v1(), wt.flags() );
		}
		else
		{
			hit_ = wt;
		}
		found_ = true;
		return COLLIDE_BEFORE;
	}
public:
	Vector3			dir_;
	WorldTriangle	hit_;
	bool			found_;
};


/// Integrate the position of a hover vehicle over elapsedTime
void Physics::integrateHoverPosition( float elapsedTime, Vector3 & position,
	float & yaw, float & pitch, float & roll )
{
	BW_GUARD;
	float actualThrust = float( thrust_ * ripper_.maxThrust_ );
	yaw += float( elapsedTime * (-roll + turn_ * ripper_.turnRate_) );

	const Vector3 location = position;

	Matrix34	rotor;
	rotor.setRotateY(yaw);
	rotor.preRotateX(pitch);
	rotor.preRotateZ(roll);
	const Vector3 direction = rotor.applyToUnitAxisVector(Z_AXIS);

	const Vector3 oldPos = location;

	Matrix34	transform;
	transform.setRotateY( yaw );

	/*
	Moo::TerrainBlock::FindDetails f =
			Moo::TerrainBlock::findOutsideBlock( location );
	PlaneEq planeEq;

	if ( f.pBlock_ )
	{
		Vector3 relLoc = f.pInvMatrix_->applyPoint( location );
		relLoc.y = f.pBlock_->heightAt( relLoc.x, relLoc.z );
		planeEq = PlaneEq( f.pMatrix_->applyPoint( relLoc ),
			f.pBlock_->normalAt( relLoc.x, relLoc.z ) );
	}
	else
	{
		planeEq = PlaneEq( Vector3( 0.f, 1.f, 0.f ), 0.f );
	}
	*/
	PlaneEq planeEq;

	ClosestFacingTriangle cft( Vector3( 0.f, -1.f, 0.f ) );
	pSlave_->pSpace()->collide(
		Vector3( location.x, location.y + 1.f,   location.z ),
		Vector3( location.x, location.y - 300.f, location.z ),
		cft );
	if (cft.found_)
	{
		planeEq = PlaneEq( cft.hit_.v0(), cft.hit_.v1(), cft.hit_.v2() );
	}
	else
	{
		planeEq = PlaneEq( Vector3( 0.f, 1.f, 0.f ), 0.f );
	}

//HACK: to stop the hover when it's outside the terrain.
/*	PlaneEq flat( Vector3(0, 1, 0),
		HeightMapTerrain::NO_TERRAIN );*/

	/*
	if (!pPlaneEq)
	{
		transform[3] = location;
		return;
//		pPlaneEq = &flat;
	}
	*/

	PlaneEq collisionPlane;
	WorldTriangle * pCollisionTriangle = NULL;


	Vector3 landPos;

	const float currGroundHeight = planeEq.y(location[0], location[2]);

	float sinPitch = direction.dotProduct(planeEq.normal());
	float desiredPitch = asinf( Math::clamp( -1.f, sinPitch, 1.f ) );

	float sinRoll = transform.applyToUnitAxisVector(X_AXIS).dotProduct(planeEq.normal());
	float desiredRoll = -asinf( Math::clamp( -1.f, sinRoll, 1.f ) );

	float heightFromGround = location[1] - currGroundHeight - ripper_.minHeightFromGround_;

	if (heightFromGround < 0.01f)
		heightFromGround = 0.01f;

	const float heightFactor = 1.f - 0.8f * heightFromGround / (2.0f + heightFromGround);
//	desiredPitch *= heightFactor;
	desiredRoll *= heightFactor;

	float currentSpeed = this->data_.momentum_.length();

	if (currentSpeed > 0.f)
	{
		float turnFactor = (0.3f + 0.6f *
			currentSpeed / (currentSpeed + 20.f)) * -float(turn_);
		desiredRoll += turnFactor;
	}

	const float decayRate = 0.3f + 0.4f * heightFromGround / (2.f + heightFromGround);
	pitch += (1 - powf(decayRate, 2 * elapsedTime /* * 20.f/(heightFromGround + 3) */)) * (desiredPitch - pitch);
	roll  += (1 - powf(decayRate, 2 * elapsedTime /* * 20.f/(heightFromGround + 3)*/)) * (desiredRoll - roll);


	transform.preRotateX(pitch);
	transform.preRotateZ(roll);


	data_.position_ = position;

	HoverFunction F( &ripper_, &planeEq, actualThrust, float(brake_), thrustAxis_, transform);
	integrate(F, data_, elapsedTime );

	if (this->data_.momentum_[1] > 15.f)
		this->data_.momentum_[1] = 15.f;

	WorldTriangle * pHitTriangle = NULL;

	const float elasticity = ripper_.elasticity_;

	if (collide_)
	{
		Vector3 dirv( data_.position_ - oldPos );
		dirv.normalise();	// start ray from 10cm back from dir vector
		if (dirv.lengthSquared() < 0.9f) dirv = Vector3(0,-1,0);

		PlaneEq	colpl;
		ClosestFacingTriangle cft( dirv );
		pSlave_->pSpace()->collide( oldPos, data_.position_, cft );
		if (cft.found_)
		{
			/*
			colpl = PlaneEq( ct.hit_.v0(), ct.hit_.v1(), ct.hit_.v2() );

			const Vector3 & pos = data_.position_;
			const float planeHeight = fabs(colpl.normal().y) > 0.01f ?
				colpl.y( pos.x, pos.z ) : pos.y + 0.01f;

			if (pos.y < planeHeight)
			{
				Vector3 travel = pos - oldPos;
				data_.position_.y = planeHeight + 0.01f;

				Vector3 normal( colpl.normal() );
				data_.momentum_ -=
					(1 + elasticity) * normal.dotProduct( data_.momentum_ ) * normal;
			}
			*/

			if ( !collided_ )
			{
				Vector3 oldP = data_.momentum_;
				oldP.normalise();

				data_.position_ = oldPos;
				cft.hit_.bounce( data_.momentum_, elasticity );

				collided_ = true;
				collisionVector_ = data_.momentum_;
				Vector3 newP( data_.momentum_ );
				newP.normalise();
				collisionSeverity_ = oldP.dotProduct( newP );
				collisionPosition_ = ( cft.hit_.v0() + cft.hit_.v1() + cft.hit_.v2() ) / 3.f;
				collisionTriangleFlags_ = cft.hit_.flags();
			}
			else
			{
				data_.position_ = oldPos;
				cft.hit_.bounce( data_.momentum_, elasticity );
			}
		}
	}

	position = data_.position_;
}

#if 0
// this code is not used, because it has been extracted into a separate
// waypoint calculation tool, called 'wpgen'

#include "world.hpp"
/**
 *	This helper class provides the glue between the waypoint flood
 *	system and the physics system.
 */
class WaypointPhysicsHandler : public WaypointFlood::IPhysics
{
public:
	WaypointPhysicsHandler(Physics* pPhysics, bool outdoors) :
	  pPhysics_(pPhysics),
	  outdoors_(outdoors)
	{
	}

	bool findDropPoint(const Vector3& pos, float& y)
	{
		BW_GUARD;
		Vector3 srcPos;
		Vector3 dropPos;

		srcPos = pos;

		// findDropPoint fails occasionally. Looks like
		// a rounding error. This seems to fix it.
		srcPos.y += 0.1f;

		float dist = World::outside().space().collide(
			srcPos, srcPos+Vector3(0,-100,0), ClosestObstacle::s_default );
		if (dist >= 0.f)
		{
			y = srcPos.y - dist;
			return true;
		}
		else return false;
	}

	void adjustMove(const Vector3& src, const Vector3& dst,
		Vector3& dst2)
	{
		BW_GUARD;
		float y;
		dst2 = dst;

		MotionConstants mc( *pPhysics_ );

		pPhysics_->preventCollision( src, dst2, mc );

		if (outdoors_)
		{
			float ground = Moo::TerrainBlock::getHeight(dst2.x, dst2.z);
			if(ground > dst2.y)
				dst2.y = ground;
		}

		if (!this->findDropPoint(dst2, y))
		{
			DEBUG_MSG("Failed to find drop point at (%f, %f, f)\n",
				dst2.x, dst2.y, dst2.z);
			dst2 = src;
			return;
		}

		if (!outdoors_ && fabs(src.y - y) > 1.0f)
		{
			// shouldn't have to do this.
			dst2 = src;
			return;
		}

		dst2.y = y;
	}

private:
	bool outdoors_;
	Physics* pPhysics_;
};

/*~ function Physics.flood
 *
 *	This function is unsupported.
 */
/**
 *	This method performs a movement flood fill.
 */
PyObject * Physics::py_flood( PyObject * args )
{
	BW_GUARD;
	Vector3 bb_min, bb_max, seed;
	std::string name;
	int outdoors = 0;

	if( PyTuple_Size(args) != 5)
	{
		PyErr_SetString( PyExc_TypeError,
			"Physics.flood(name, (min), (max), (seed), outdoor): not enough arguments." );
		return NULL;
	}

	if (Script::setData( PyTuple_GetItem(args,0), name, "py_flood: " ) != 0)
	{
		return NULL;
	}

	if (Script::setData( PyTuple_GetItem(args,1), bb_min, "py_flood: " ) != 0)
	{
		return NULL;
	}

	if (Script::setData( PyTuple_GetItem(args,2), bb_max, "py_flood: " ) != 0)
	{
		return NULL;
	}

	if (Script::setData( PyTuple_GetItem(args,3), seed, "py_flood: " ) != 0)
	{
		return NULL;
	}

	if (Script::setData( PyTuple_GetItem(args,4), outdoors, "py_flood: " ) != 0)
	{
		return NULL;
	}

	WaypointPhysicsHandler physicsHandler(this, outdoors == 1);
	WaypointFlood waypointFlood;

	waypointFlood.setArea(bb_min, bb_max, 0.5f);
	waypointFlood.setPhysics(&physicsHandler);
	waypointFlood.fill(seed);
	waypointFlood.writeTGA(name.c_str());

	Py_Return;
}

#endif // 0


/// static initialiser
Physics::Physicians Physics::s_physicians_;

// physics.cpp
