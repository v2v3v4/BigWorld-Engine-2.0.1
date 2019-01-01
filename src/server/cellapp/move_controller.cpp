/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "move_controller.hpp"
#include "entity.hpp"
#include "cellapp.hpp"
#include "cellapp_config.hpp"
#include "cell.hpp"
#include "real_entity.hpp"

DECLARE_DEBUG_COMPONENT(0)

/*~ callback Entity.onMove
 *  @components{ cell }
 *	This method is related to the Entity.moveToPoint and Entity.moveToEntity
 *	methods. It is called when the entity has reached its destination.
 *	@param controllerID The id of the controller that triggered this callback.
 *	@param userData The user data that was passed to Entity.moveToPoint or
 *	Entity.moveToEntity.
 */
/*~ callback Entity.onMoveFailure
 *  @components{ cell }
 *	This method is related to the Entity.moveToPoint and Entity.moveToEntity
 *	methods. It is called if the entity fails to reach its destination.
 *	@param controllerID The id of the controller that triggered this callback.
 *	@param userData The user data that was passed to Entity.moveToPoint or
 *	Entity.moveToEntity.
 */


// -----------------------------------------------------------------------------
// Section: MoveController
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param velocity			Velocity in metres per second
 *	@param faceMovement		Whether the entity should face the direction of
 *							movement.
 *	@param moveVertically	Indicates whether or not the entity should be moved
 *							vertically (i.e. along the y-axis).
 */
MoveController::MoveController( float velocity, bool faceMovement,
		bool moveVertically ):
	faceMovement_( faceMovement ),
	moveVertically_( moveVertically )
{
	metresPerTick_ = velocity / CellAppConfig::updateHertz();
}


/**
 *	This method moves the entity towards the destination by metresPerTick_.
 *
 *	@param destination The position to move to.
 */
bool MoveController::move( const Position3D & destination )
{
	Position3D position = this->entity().position();
	Direction3D direction = this->entity().direction();

	Vector3 movement = destination - position;
	if (!moveVertically_)
	{
		movement.y = 0.f;
	}

	// If we are real close, then finish up.

	if(movement.length() < metresPerTick_)
	{
		position = destination;
	}
	else
	{
		movement.normalise();
		movement *= metresPerTick_;
		position += movement;
	}

	// Make sure we are facing the right direction.

	if (faceMovement_)
	{
		if (movement.x != 0.f || movement.z != 0.f)
		{
			direction.yaw = movement.yaw();
		}
		if (movement.y != 0.f)
		{
			direction.pitch = movement.pitch();
		}
	}

	// Keep ourselves alive until we have finished cleaning up,
	// with an extra reference count from a smart pointer.
	ControllerPtr pController = this;

	// No longer on the ground...
	// Might want to make this changeable from script
	// for entities that want to be on the ground.
	this->entity().isOnGround( false );
	this->entity().setPositionAndDirection( position, direction );

	if (!this->isAttached())
	{
		return false;
	}

	return position.x == destination.x &&
			position.z == destination.z &&
			(position.y == destination.y || !moveVertically_);
}


void MoveController::writeRealToStream( BinaryOStream & stream )
{
	this->Controller::writeRealToStream( stream );
	stream << metresPerTick_ << faceMovement_ << moveVertically_;
}


bool MoveController::readRealFromStream( BinaryIStream & stream )
{
	bool result = this->Controller::readRealFromStream( stream );
	stream >> metresPerTick_ >> faceMovement_ >> moveVertically_;

	return result;
}


/**
 *	This method overrides the Controller method.
 */
void MoveController::startReal( bool /*isInitialStart*/ )
{
	MF_ASSERT( entity().isReal() );
	CellApp::instance().registerForUpdate( this );
}


/**
 *	This method overrides the Controller method.
 */
void MoveController::stopReal( bool /*isFinalStop*/ )
{
	MF_VERIFY( CellApp::instance().deregisterForUpdate( this ) );
}


// -----------------------------------------------------------------------------
// Section: MoveToPointController
// -----------------------------------------------------------------------------

IMPLEMENT_EXCLUSIVE_CONTROLLER_TYPE(
		MoveToPointController, DOMAIN_REAL, "Movement" )

/**
 *	Constructor for MoveToPointController.
 *
 *	@param destination		The location to which we should move
 *	@param destinationChunk	The destination chunk.
 *	@param destinationWaypoint The id of the destination waypoint.
 *	@param velocity			Velocity in metres per second
 *	@param faceMovement		Whether or not the entity should face in the
 *							direction of movement.
 *	@param moveVertically	Whether or not the entity should move vertically.
 */
MoveToPointController::MoveToPointController( const Position3D & destination,
	   const std::string & destinationChunk, int32 destinationWaypoint,
		float velocity, bool faceMovement, bool moveVertically ) :
	MoveController( velocity, faceMovement, moveVertically ),
	destination_( destination ),
	destinationChunk_( destinationChunk ),
	destinationWaypoint_( destinationWaypoint )
{
}


/**
 *	This method writes our state to a stream.
 *
 *	@param stream		Stream to which we should write
 */
void MoveToPointController::writeRealToStream( BinaryOStream & stream )
{
	this->MoveController::writeRealToStream( stream );
	stream << destination_ << destinationChunk_ << destinationWaypoint_;
}


/**
 *	This method reads our state from a stream.
 *
 *	@param stream		Stream from which to read
 *	@return				true if successful, false otherwise
 */
bool MoveToPointController::readRealFromStream( BinaryIStream & stream )
{
	this->MoveController::readRealFromStream( stream );
	stream >> destination_ >> destinationChunk_ >> destinationWaypoint_;
	return true;
}



/**
 *	This method is called every tick.
 */
void MoveToPointController::update()
{
	if (this->move( destination_ ))
	{
		// Keep ourselves alive until we have finished cleaning up,
		// with an extra reference count from a smart pointer.
		ControllerPtr pController = this;

		{
			SCOPED_PROFILE( ON_MOVE_PROFILE );
			this->standardCallback( "onMove" );
		}

		if (this->isAttached())
		{
			this->cancel();
		}
	}
}


// -----------------------------------------------------------------------------
// Section: MoveToEntityController
// -----------------------------------------------------------------------------

IMPLEMENT_EXCLUSIVE_CONTROLLER_TYPE(
		MoveToEntityController, DOMAIN_REAL, "Movement" )

/**
 *	Constructor for MoveToEntityController.
 *
 *	@param destEntityID		Entity to follow
 *	@param velocity			Velocity in metres per second
 *	@param range			Stop once we are within this range
 *	@param faceMovement		Whether or not the entity should face the direction
 *							of movement.
 *	@param moveVertically	Whether or not the entity should move vertically.
 */
MoveToEntityController::MoveToEntityController( EntityID destEntityID,
			float velocity, float range, bool faceMovement,
			bool moveVertically ) :
	MoveController( velocity, faceMovement, moveVertically ),
	destEntityID_( destEntityID ),
	pDestEntity_( NULL ),
	offset_( 0.f, 0.f, 0.f ),
	range_( range )
{
}


/**
 *	This method writes our state to a stream.
 *
 *	@param stream		Stream to which we should write
 */
void MoveToEntityController::writeRealToStream( BinaryOStream & stream )
{
	this->MoveController::writeRealToStream( stream );
	stream << destEntityID_ << range_;
}

/**
 *	This method reads our state from a stream.
 *
 *	@param stream		Stream from which to read
 *	@return				true if successful, false otherwise
 */
bool MoveToEntityController::readRealFromStream( BinaryIStream & stream )
{
	this->MoveController::readRealFromStream( stream );
	stream >> destEntityID_ >> range_;
	return true;
}



/**
 *	Make sure we decrement the reference count of the destination
 *	entity when we stop.
 */
void MoveToEntityController::stopReal( bool isFinalStop )
{
	this->MoveController::stopReal( isFinalStop );
	pDestEntity_ = NULL;
}


/**
 *  Pick a random offset, within range of the destination.
 */
void MoveToEntityController::recalcOffset()
{
	float angle = (rand() % 360) * (MATH_PI / 180.0f);
	offset_.x = range_ * cosf(angle);
	offset_.z = range_ * sinf(angle);
}


/**
 *	This method is called every tick.
 */
void MoveToEntityController::update()
{
	// Resolve the entity ID to an entity pointer.

	if (!pDestEntity_)
	{
		pDestEntity_ = CellApp::instance().findEntity( destEntityID_ );
		this->recalcOffset();
	}

	if (pDestEntity_ && pDestEntity_->isDestroyed())
	{
		pDestEntity_ = NULL;
	}

	if (!pDestEntity_)
	{
		ControllerPtr pController = this;
		this->standardCallback( "onMoveFailure" );
		if (this->isAttached())
		{
			this->cancel();
		}
		return;
	}

	Position3D destination = this->pDestEntity_->position();
	Position3D currentPosition = this->entity().position();

	// calculate remaining distance: don't take into account y-position when we
	// are not moving vertically; instead take 2D dist projected to x-z plane
	Vector3 remaining = destination - this->entity().position();

	if (!this->moveVertically())
	{
		remaining.y = 0.f;
		destination.y = currentPosition.y;
	}
	bool closeEnough = remaining.length() < range_;

	// Keep ourselves alive until we have finished cleaning up,
	// with an extra reference count from a smart pointer.
	ControllerPtr pController = this;

	if (closeEnough || this->move( destination + offset_ ))
	{
		if (closeEnough)
		{
			// In case we were moving towards a point near the entity,
			// make sure that we are now facing it exactly.

			const Position3D & position = this->entity().position();
			Vector3 vector = destination - position;

			Direction3D direction = this->entity().direction();
			if (this->faceMovement())
			{
				direction.yaw = vector.yaw();
			}
			this->entity().setPositionAndDirection( position, direction );
		}

		if (this->isAttached())
		{
			this->standardCallback( "onMove" );
			if (this->isAttached())
			{
				this->cancel();
			}
		}
	}
}

// move_controller.cpp
