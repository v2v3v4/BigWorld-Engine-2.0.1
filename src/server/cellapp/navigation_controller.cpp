/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "navigation_controller.hpp"
#include "entity.hpp"
#include "cell.hpp"
#include "cellapp.hpp"
#include "cellapp_config.hpp"
#include "real_entity.hpp"
#include "entity_navigate.hpp"
#include "profile.hpp"

DECLARE_DEBUG_COMPONENT( 0 )


// -----------------------------------------------------------------------------
// Section: NavigationController
// -----------------------------------------------------------------------------

IMPLEMENT_EXCLUSIVE_CONTROLLER_TYPE(
		NavigationController, DOMAIN_REAL, "Movement" )

/**
 *	Constructor.
 *
 *	@param destination		The location to which we should move
 *	@param velocity			Velocity in metres per second
 *	@param faceMovement		Whether or not the entity should face the direction
 *							of movement.
 *	@param girth			The girth of the entity in metres.
 *	@param closeEnough		The distance at which the entity is thought to be
 *							close enough.
 */
NavigationController::NavigationController( const Position3D & destination,
		float velocity,	bool faceMovement, float maxDistance, float girth,
		float closeEnough ):
	metresPerTick_( velocity / CellAppConfig::updateHertz() ),
	maxDistance_( maxDistance ),
	girth_( girth ),
	closeEnough_( closeEnough ),
	faceMovement_( faceMovement ),
	nextPosition_( Position3D() ),
	destination_( destination ),
	pDstLoc_( NULL ),
	currentNode_( 0 )
{
	if ( closeEnough_ < 0.001f )
	{
		WARNING_MSG( "NavigationController::NavigationController: "
				"closeEnough_ is very small (%f)\n",
			closeEnough_ );
	}
}


/**
 *	This method moves the entity towards the destination.
 *	Assume nextPosition_ is valid.
 */
NavigationController::NavigationStatus NavigationController::move()
{
	Position3D position = this->entity().position();
	Direction3D direction = this->entity().direction();

	Vector3 movement = nextPosition_ - position;

	if (movement.y < 0)
	{
		movement.y = 0.f;
	}

	// If we are real close, then finish up.

	if (movement.length() < metresPerTick_)
	{
		position = nextPosition_;
	}
	else
	{
		movement.normalise();
		movement *= metresPerTick_;
		position += movement;
		if( position.y < nextPosition_.y )
		{
			position.y = nextPosition_.y;
		}
	}

	// Make sure we are facing the right direction.

	if (faceMovement_ && (movement.x != 0.f || movement.z != 0.f))
	{
		direction.yaw = movement.yaw();
	}

	// No longer on the ground...
	// Might want to make this changeable from script
	// for entities that want to be on the ground.

	ControllerPtr pController = this;

	this->entity().isOnGround( false );
	this->entity().setPositionAndDirection( position, direction );

	// a script callback cancelled us
	if (!this->isAttached())
	{
		return NAVIGATION_CANCELLED;
	}

	// Check for completion
	bool reachedFinalPos = 
		(fabsf( position.x - destination_.x ) < closeEnough_)  &&
		(fabsf( position.z - destination_.z ) < closeEnough_);
	if (reachedFinalPos)
	{
		return NAVIGATION_COMPLETE;
	}

	// .. otherwise check if we are at the next update location
	bool reachedNextPos = 
		(fabs( position.x - nextPosition_.x ) < 0.001f)  &&
		(fabs( position.z - nextPosition_.z ) < 0.001f);
	if (reachedNextPos)
	{
		if (currentNode_ < (int)path_.size())
		{
			nextPosition_ = path_[ currentNode_ ];
			++currentNode_;
			return NAVIGATION_IN_PROGRESS;
		}

		EntityNavigate& en = EntityNavigate::instance( this->entity() );

		this->entity().pReal()->navigator().clearCachedWayPath();

		// figure out the source navloc
		NavLoc srcLoc;
		en.validateNavLoc( position, girth_, srcLoc );
		// complain if invalid
		if (!srcLoc.valid() || !pDstLoc_->valid())
		{
			//DEBUG_MSG( "NavigationController::move: "
			//	"Could not resolve source (%f,%f,%f) or "
			//	"destination position (%f,%f,%f)+%f",
			//	position.x, position.y, position.z,
			//	destination_.x, destination_.y, destination_.z,
			//	girth_ );
			return NAVIGATION_FAILED;
		}

		bool passedActivatedPortal;
		if (!en.getNavigatePosition( srcLoc, *pDstLoc_, maxDistance_, 
				nextPosition_, passedActivatedPortal, girth_ ))
		{
			return NAVIGATION_FAILED;
		}

		// If we are still navigating, setup the state for the next round
		// of movement.
		currentNode_ = 0;
		path_.clear();

		this->generateTraversalPath( srcLoc );

/*
		DEBUG_MSG("Found next point to move (%6.3f, %6.3f, %6.3f)\n",
			nextPosition_.x, nextPosition_.y, nextPosition_.z );
*/	
	}

	return NAVIGATION_IN_PROGRESS;
}


/**
 *	This method overrides the Controller method.
 */
void NavigationController::startReal( bool /*isInitialStart*/ )
{
	//DEBUG_MSG("Starting navigation controller(%6.3f, %6.3f, %6.3f)\n",
	//	destination_.x, destination_.y, destination_.z );
	currentNode_  = 0;
	path_.clear();

	EntityNavigate & en = EntityNavigate::instance( this->entity() );

	// figure out the source navloc
	NavLoc srcLoc( this->entity().pChunkSpace(), this->entity().position(), 
		girth_ );

	// figure out the dest navloc
	if (pDstLoc_)
	{
		delete pDstLoc_;
	}

	pDstLoc_ = new NavLoc( this->entity().pChunkSpace(), destination_, girth_ );

	// complain if invalid
	if (pDstLoc_->valid() && srcLoc.valid())
	{
		pDstLoc_->clip();

		destination_ = pDstLoc_->point();
		//DEBUG_MSG("Adjusted destination (%6.3f, %6.3f, %6.3f)\n",
		//	destination_.x, destination_.y, destination_.z );

		this->entity().pReal()->navigator().clearCachedWayPath();
		this->entity().pReal()->navigator().clearCachedWaySetPath();

		bool passedActivatedPortal;
		if (!en.getNavigatePosition( srcLoc, *pDstLoc_, maxDistance_, 
				nextPosition_, passedActivatedPortal, girth_ ))
		{
			nextPosition_ = this->entity().position();
		}
		else
		{
			this->generateTraversalPath( srcLoc );
		}
	}
	else
	{
		nextPosition_ = this->entity().position();
	}

	CellApp::instance().registerForUpdate( this );
}


/**
 *	This method overrides the Controller method.
 */
void NavigationController::stopReal( bool /*isFinalStop*/ )
{
	MF_VERIFY( CellApp::instance().deregisterForUpdate( this ) );

	delete pDstLoc_;
	pDstLoc_ = NULL;
}


/**
 *	This method writes our state to a stream.
 *
 *	@param stream		Stream to which we should write
 */
void NavigationController::writeRealToStream(BinaryOStream & stream)
{
	stream << destination_ << metresPerTick_ << faceMovement_ << 
		maxDistance_ << girth_;
}


/**
 *	This method reads our state from a stream.
 *
 *	@param stream		Stream from which to read
 *
 *	@return				true if successful, false otherwise
 */
bool NavigationController::readRealFromStream( BinaryIStream & stream )
{
	stream >> destination_ >> metresPerTick_ >> faceMovement_ >> 
		maxDistance_ >> girth_;
	nextPosition_ = destination_;

	if (pDstLoc_)
	{
		delete pDstLoc_;
	}
	pDstLoc_ = new NavLoc( this->entity().pChunkSpace(), destination_, girth_ );

	return true;
}


/*~ callback Entity.onNavigate
 *  @components{ cell }
 *	This callback is associated with the Entity.navigate and
 *	Entity.navigateStep methods. It is called when the entity reaches the
 *	desired location.
 *
 *	@param controllerID 	The id of the controller that triggered this
 *							callback.
 *	@param userData 		The user data that was passed to Entity.navigate or
 *							Entity.navigateStep.
 */
/*~ callback Entity.onNavigateFailed
 *  @components{ cell }
 *	This callback is associated with the Entity.navigate and
 *	Entity.navigateStep methods. It is called when the entity fails to reach the
 *	desired location.
 *
 *	@param controllerID 	The id of the controller that triggered this
 *							callback.
 *	@param userData 		The user data that was passed to Entity.navigate or
 *							Entity.navigateStep.
 */
/**
 *	This method is called every tick.
 */
void NavigationController::update()
{
	static ProfileVal localProfile( "onNavigate" );

	// Keep ourselves alive until we have finished cleaning up,
	// with an extra reference count from a smart pointer.
	ControllerPtr pController = this;

	NavigationStatus navStatus = this->move();

	if (navStatus == NAVIGATION_COMPLETE)
	{
		//DEBUG_MSG( "Reached destination\n" );
		{
			SCOPED_PROFILE( localProfile );
			this->standardCallback( "onNavigate" );
		}

		if (this->isAttached())
		{
			this->cancel();
		}
	}
	else if (navStatus == NAVIGATION_FAILED)
	{
		//DEBUG_MSG( "Navigate failed\n" );
		{
			SCOPED_PROFILE( localProfile );
			this->standardCallback( "onNavigateFailed" );
		}

		if (this->isAttached())
		{
			this->cancel();
		}
	}
	//else still moving to destination( = 0), or cancelled( = -2)
}


/**
 *  This method generates a new set of destination positions to navigate
 *  through.
 */
void NavigationController::generateTraversalPath( const NavLoc & srcLoc )
{
	Navigator & navigator = this->entity().pReal()->navigator();
	navigator.getCachedWaypointPath( path_ );

	if (!path_.empty())
	{
		return;
	}

	nextPosition_ = path_[ 0 ];

	if (srcLoc.pSet() == pDstLoc_->pSet())
	{
		path_.push_back( destination_ );
	}
}

// navigation_controller.cpp
