/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_navigate.hpp"

#include "accelerate_along_path_controller.hpp"
#include "accelerate_to_entity_controller.hpp"
#include "accelerate_to_point_controller.hpp"
#include "cell.hpp"
#include "cell_chunk.hpp"
#include "entity.hpp"
#include "move_controller.hpp"
#include "navigation_controller.hpp"
#include "profile.hpp"
#include "real_entity.hpp"
#include "space.hpp"

#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"

#include "pyscript/script_math.hpp"

#include "waypoint/chunk_waypoint_set.hpp"
#include "waypoint/navigator.hpp"
#include "waypoint/navigator_cache.hpp"
#include "waypoint/waypoint_neighbour_iterator.hpp"

DECLARE_DEBUG_COMPONENT( 0 );

namespace // (anonymous)
{

PyObject * navigatePathPoints( Navigator & navigator, 
	ChunkSpace * pChunkSpace, const NavLoc & srcLoc, const Vector3 & dst, 
	float maxSearchDistance, float girth );

} // end namespace (anonymous)

// -----------------------------------------------------------------------------
// Section: EntityNavigate
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( EntityNavigate )

PY_BEGIN_METHODS( EntityNavigate )
	PY_METHOD( moveToPoint )
	PY_METHOD( moveToEntity )
	PY_METHOD( accelerateToPoint )
	PY_METHOD( accelerateAlongPath )
	PY_METHOD( accelerateToEntity )
	PY_METHOD( navigate )
	PY_METHOD( navigateStep )
	PY_METHOD( canNavigateTo )
	PY_METHOD( navigateFollow )
	PY_METHOD( waySetPathLength )
	PY_METHOD( getStopPoint )
	PY_METHOD( navigatePathPoints )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( EntityNavigate )
PY_END_ATTRIBUTES()


/// static initialiser
const EntityNavigate::Instance<EntityNavigate>
	EntityNavigate::instance( &EntityNavigate::s_attributes_.di_ );



/**
 *	Constructor
 */
EntityNavigate::EntityNavigate( Entity & e ) :
	EntityExtra( e )
{
}

/**
 *	Destructor
 */
EntityNavigate::~EntityNavigate()
{
}


/// Python get attribute method
PyObject * EntityNavigate::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return this->EntityExtra::pyGetAttribute( attr );
}

/// Python set attribute method
int EntityNavigate::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return this->EntityExtra::pySetAttribute( attr, value );
}


/*~ function Entity.moveToPoint
 *  @components{ cell }
 *
 *	This function moves the Entity in a straight line towards a given point,
 *	invoking a notification method on success or failure.  For a given Entity,
 *	only one navigation/movement controller is active at any one time.
 *	Returns a ControllerID that can be used to cancel the movement. For example,
 *	Entity.cancel( movementID ).  Movement can also be cancelled with
 *	Entity.cancel( "Movement" ). The notification methods are not called when
 *	movement is cancelled.
 *
 *	The notification methods are defined as follows;
 *	@{
 *		def onMove( self, controllerId, userData ):
 *		def onMoveFailure( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destination		(Vector3)			The destination point for the Entity
 *	@param	velocity		(float)				The speed to move the Entity in m/s
 *	@param	userData		(optional object)	Data that can be passed to notification method
 *	@param	faceMovement	(optional bool)		True if entity is to face in direction of movement (default).
 *												Should be false if another mechanism is to provide direction...
 *	@param	moveVertically	(optional bool)		True if Entity is allowed to move vertically, ie, fly.
 *												Set to false if Entity is to remain on ground (default).
 *
 *	@return					(integer)			The ID of the newly created controller
 */
/**
 *	This method is exposed to scripting. It creates a controller that
 *	will move an entity to the given point. The arguments below are passed
 *	via a python tuple.
 *
 *	@param destination			3D destination vector
 *	@param velocity				Movement velocity in m/s
 *	@param userArg				User data to be passed to the callback (opt)
 *	@param faceMovement			Should turn to face movement direction (opt)
 *	@param moveVertically		Should allow vertical movement (opt)
 *
 *	@return		The integer ID of the newly created controller.
 */
PyObject * EntityNavigate::moveToPoint( Vector3 destination,
	float velocity, int userArg, bool faceMovement, bool moveVertically )
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	ControllerPtr pController = new MoveToPointController(
			destination, "", 0, velocity, faceMovement, moveVertically );

	ControllerID controllerID;
	controllerID = entity_.addController( pController, userArg );
	return Script::getData( controllerID );
}


/*~ function Entity.moveToEntity
 *  @components{ cell }
 *
 *	This function moves the Entity in a straight line towards another Entity,
 *	invoking a notification method on success or failure.  For a given Entity,
 *	only one navigation/movement controller is active at any one time.
 *	Returns a ControllerID that can be used to cancel the movement. For example,
 *	Entity.cancel( movementID ).  Movement can also be cancelled with
 *	Entity.cancel( "Movement" ). The notification methods are not called when
 *	movement is cancelled.
 *
 * 	@{
 *		def onMove( self, controllerId, userData ):
 *		def onMoveFailure( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destEntityID	(int)				The ID of the target Entity
 *	@param	velocity		(float)				The speed to move the Entity in m/s
 *	@param	range			(float)				Range within destination to stop
 *	@param	userData		(optional object)	Data that can be passed to notification method
 *	@param	faceMovement	(optional bool)		True if entity is to face in direction of movement (default).
 *												Should be false if another mechanism is to provide direction...
 *	@param	moveVertically	(optional bool)		True if Entity is allowed to move vertically, ie, fly.
 *												Set to false if Entity is to remain on ground (default).
 *
 *	@return					(integer)			The ID of the newly created controller
*/
/**
 *	This method is exposed to scripting. It creates a controller that
 *	will move an entity towards another entity. The arguments below are passed
 *	via a python tuple.
 *
 *	@param destEntityID			Entity ID of destination
 *	@param velocity				Movement velocity in m/s
 *	@param range				Range within destination to stop
 *	@param userArg				User data to be passed to the callback
 *	@param faceMovement			True if entity is to face in direction of movement
 *								(default). Should be false if another mechanism is
 *								to provide direction.
 *	@param moveVertically		True if Entity is allowed to move vertically, ie,
 *								fly. Set to false if Entity is to remain on ground
 *								(default).
 *
 *	@return		The integer ID of the newly created controller.
 */
PyObject * EntityNavigate::moveToEntity( int destEntityID,
										float velocity,
										float range,
										int userArg,
										bool faceMovement,
										bool moveVertically )
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	ControllerPtr pController = new MoveToEntityController( destEntityID,
			velocity, range, faceMovement, moveVertically );

	ControllerID controllerID;
	controllerID = entity_.addController(pController, userArg);
	return Script::getData( controllerID );
}


/*~ function Entity.accelerateToPoint
 *  @components{ cell }
 *
 *	This function is equivalent to calling
 *	accelerateAlongPath( [destination], etc )
 *
 *	@see accelerateAlongPath
 *	@see cancel
 *
 *	@param	destination		(Vector3)		The destination to move to.
 *	@param	acceleration	(float)			The rate at which the entity will
 *											accelerate. units/second^2
 *	@param	maxSpeed		(float)			The maximum speed the entity will
 *											accelerate to.
 *	@param	facing			(integer)		(Optional) Defines direction the
 *											entity should face while it moves.
 *												0	Disabled
 *												1	Face velocity (default)
 *												2	Face acceleration
 *	@param	userData		(integer)		(Optional) Data that will be passed
 *											to notification method
 *
 *	@return					(ControllerID)	The ID of the newly created
 *											controller
 */
/**
 *	This function is equivalent to calling
 *	accelerateAlongPath( [destination], ... )
 *
 *	@see EntityNavigate::accelerateAlongPath()
 *
 *	@param	destination			The destination to move to.
 *	@param	acceleration		The rate at which the entity will accelerate.
 *								units/second^2
 *	@param	maxSpeed			The maximum speed the entity will maintain.
 *	@param	intFacing			Defines direction the entity should face
 *								while it moves.
 *									0	Disabled
 *									1	Face velocity (default)
 *									2	Face acceleration
 *	@param	stopAtDestination	Describes whether the speed of the entity
 *								should be zero upon reaching its destination.
 *	@param	userArg				Data that will be passed to notification
 *								methods
 *
 *	@return					The ID of the newly created controller
 */
PyObject * EntityNavigate::accelerateToPoint(	Position3D destination,
												float acceleration,
												float maxSpeed,
												int intFacing,
												bool stopAtDestination,
												int userArg)
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	BaseAccelerationController::Facing facing;
	if( intFacing >= 0 && intFacing < BaseAccelerationController::FACING_MAX )
	{
		facing = BaseAccelerationController::Facing( intFacing );
	}
	else
	{
		PyErr_SetString(	PyExc_TypeError,
							"Facing must one of the value 0, 1 or 2" );
		return NULL;
	}

	ControllerPtr pController = new AccelerateToPointController(
															destination,
															acceleration,
															maxSpeed,
															facing,
															stopAtDestination);

	ControllerID controllerID;
	controllerID = entity_.addController( pController, userArg );
	return Script::getData( controllerID );
}


/*~ function Entity.accelerateAlongPath
 *  @components{ cell }
 *
 *	This function moves the Entity through a series of given waypoints by
 *	applying an acceleration. For a given Entity only one movement controller
 *	can be active and so this function will replace any existing controller.
 *
 *	The notification methods are defined as follows;
 * 	@{
 *		def onMove( self, controllerId, userData ):
 *		def onMoveFailure( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	waypoints		([Vector3])		The waypoints to pass through.
 *	@param	acceleration	(float)			The rate at which the entity will
 *											accelerate. units/second^2
 *	@param	maxSpeed		(float)			The maximum speed the entity will
 *											accelerate to.
 *	@param	facing			(integer)		(Optional) Defines direction the
 *											entity should face while it moves.
 *												0	Disabled
 *												1	Face velocity (default)
 *												2	Face acceleration
 *	@param	userData		(integer)		(Optional) Data that will be passed
 *											to notification method
 *
 *	@return					(ControllerID)	The ID of the newly created
 *											controller
 */
/**
 *	This is a python exposed function that creates a controller to moves the
 *	Entity through a series of waypoints by applying acceleration.
 *
 *	@param	waypoints		The waypoints to pass through.
 *	@param	acceleration	The rate at which the entity will accelerate.
 *							units/second^2
 *	@param	maxSpeed		The maximum speed the entity will maintain.
 *	@param	intFacing			Defines direction the entity should face
 *							while it moves.
 *								0	Disabled
 *								1	Face velocity (default)
 *								2	Face acceleration
 *	@param	userArg		Data that will be passed to notification
 *							methods
 *
 *	@return					The ID of the newly created controller
 */
PyObject * EntityNavigate::accelerateAlongPath(	
											std::vector<Position3D> waypoints,
											float acceleration,
											float maxSpeed,
											int intFacing,
											int userArg)
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	if( waypoints.empty() )
	{
		PyErr_SetString( PyExc_TypeError,
			"Waypoints must not be empty." );
		return NULL;
	}

	BaseAccelerationController::Facing facing;
	if( intFacing >= 0 && intFacing < BaseAccelerationController::FACING_MAX )
	{
		facing = BaseAccelerationController::Facing( intFacing );
	}
	else
	{
		PyErr_SetString(	PyExc_TypeError,
							"Facing must one of the value 0, 1 or 2" );
		return NULL;
	}

	ControllerPtr pController = new AccelerateAlongPathController(	
																waypoints,
																acceleration,
																maxSpeed,
																facing );

	ControllerID controllerID;
	controllerID = entity_.addController( pController, userArg );
	return Script::getData( controllerID );
}


/*~ function Entity.accelerateToEntity
 *  @components{ cell }
 *
 *	This function moves the entity towards the given target entity by applying
 *	an acceleration. For a given Entity only one movement controller can be
 *	active and so this function will replace any existing controller.
 *
 *	The notification methods are defined as follows;
 * 	@{
 *		def onMove( self, controllerId, userData ):
 *		def onMoveFailure( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destinationEntity		(EntityID)	The waypoints to pass through.
 *	@param	acceleration	(float)			The rate at which the entity will
 *											accelerate. units/second^2
 *	@param	maxSpeed		(float)			The maximum speed the entity will
 *											accelerate to.
 *	@param	range			(float)			Range within destination to stop
 *	@param	facing			(integer)		(Optional) Defines direction the
 *											entity should face while it moves.
 *												0	Disabled
 *												1	Face velocity (default)
 *												2	Face acceleration
 *	@param	userData		(integer)		(Optional) Data that will be passed
 *											to notification method
 *
 *	@return					(ControllerID)	The ID of the newly created
 *											controller
 */
/**
 *	This is a python exposed function that creates a controller to moves the
 *	Entity through a series of waypoints by applying acceleration.

 *	@param	destinationEntity	The waypoints to pass through.
 *	@param	acceleration		The rate at which the entity will accelerate.
 *								units/second^2
 *	@param	maxSpeed			The maximum speed the entity will accelerate to.
 *	@param	range				Range within destination to stop.
 *	@param	intFacing			Defines direction the entity should face while
 *								it moves.
 *									0	Disabled
 *									1	Face velocity (default)
 *									2	Face acceleration
 *	@param	userArg			Data that will be passed to notification method.
 *
 *	@return	The ID of the newly created controller
 */
PyObject * EntityNavigate::accelerateToEntity(	EntityID destinationEntity,
												float acceleration,
												float maxSpeed,
												float range,
												int intFacing,
												int userArg)
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	BaseAccelerationController::Facing facing;
	if( intFacing >= 0 && intFacing < BaseAccelerationController::FACING_MAX )
	{
		facing = BaseAccelerationController::Facing( intFacing );
	}
	else
	{
		PyErr_SetString(	PyExc_TypeError,
							"Facing must one of the value 0, 1 or 2" );
		return NULL;
	}

	ControllerPtr pController = new AccelerateToEntityController(	
															destinationEntity,
															acceleration,
															maxSpeed,
															range,
															facing );

	ControllerID controllerID;
	controllerID = entity_.addController( pController, userArg );
	return Script::getData( controllerID );
}


/**
 *	This private method validates our cached NavLoc and writes the result
 *	into in the input NavLoc. It does not update the cached one.
 */
void EntityNavigate::validateNavLoc( const Vector3 & testPosition,
		float girth, NavLoc & out )
{
	MF_ASSERT( entity_.pReal() != NULL );

	if (entity_.pReal() != NULL &&
		entity_.pReal()->navLoc().valid())
	{
		NavLoc navLoc = entity_.pReal()->navLoc();
		//DEBUG_MSG( "Entity.validateNavLoc: "
		//		"Guessing from pReal->navLoc (%f,%f,%f) in %s id %d [%s]\n",
		//	navLoc.point().x, navLoc.point().y, navLoc.point().z,
		//	navLoc.pSet()->chunk()->identifier().c_str(), navLoc.waypoint(),
		//	(navLoc.isWithinWP() ? "Exact": "Closest"));

		Vector3 position = testPosition;
		if (navLoc.waypoint() >= 0)
		{
			Vector3 v( position );

			navLoc.makeMaxHeight( v );
			position.y = std::min( position.y, v.y );
		}
		out = NavLoc( entity_.pReal()->navLoc(), position );
		//DEBUG_MSG( "Entity.validateNavLoc: "
		//		"out is (%f,%f,%f) in %s id %d [%s]\n",
		//	out.point().x, out.point().y, out.point().z,
		//	out.pSet()->chunk()->identifier().c_str(), out.waypoint(),
		//	(out.isWithinWP() ? "Exact": "Closest") );
	}
	else if (entity_.pChunk() != NULL)
	{
		out = NavLoc( entity_.pChunk(), testPosition, girth );
		//DEBUG_MSG( "Entity.validateNavLoc: "
		//		"srcLoc from pchunk(%s) is (%f,%f,%f) in %s id %d [%s]\n",
		//	entity_.pChunk()->identifier().c_str(),
		//	out.point().x, out.point().y, out.point().z,
		//	out.pSet()->chunk()->identifier().c_str(), out.waypoint(),
		//	(out.isWithinWP() ? "Exact": "Closest") );
	}
	else
	{
		out = NavLoc( entity_.pChunkSpace(), testPosition, girth );
		//DEBUG_MSG( "Entity.validateNavLoc: "
		//		"From nowhere : out is (%f,%f,%f) in %s id %d [%s]\n",
		//	out.point().x, out.point().y, out.point().z,
		//	out.pSet()->chunk()->identifier().c_str(), out.waypoint(),
		//	(out.isWithinWP() ? "Exact": "Closest") );
	}
}


/*~ function Entity.navigate
 *  @components{ cell }
 *
 *	This function uses the BigWorld navigation system to move the Entity to a
 *	destination point, invoking a notification method on success or failure.
 *	BigWorld can have several pre-generated navigation systems, each with a
 *	different girth (resulting in different navigation paths).  For a given
 *	Entity, only one navigation/movement controller can be active at any one
 *	time.
 *	Returns a ControllerID that can be used to cancel the movement. For example,
 *	Entity.cancel( movementID ).  Movement can also be cancelled with
 *	Entity.cancel( "Movement" ). The notification methods are not called when
 *	movement is cancelled.
 *
 *	The notification methods are defined as follows;
 *	@{
 *		def onNavigate( self, controllerId, userData ):
 *		def onNavigateFailed( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destination		(Vector3)			The destination point for the Entity
 *	@param	velocity		(float)				The speed to move the Entity in m/s
 *	@param	faceMovement	(optional bool)		True if entity is to face in direction of movement (default).
 *												Should be false if another mechanism is to provide direction...
 *	@param	maxSearchDistance (float)			Max distance to search
 *	@param	girth			(optional float)	Which navigation girth to use (default 0.5)
 *	@param	closeEnough		(optional float)	How close to end point we need to get before navigation is
 *												complete and callback is triggered.
 *													Note:  A value of 0 should never be used.
 *	@param	userData		(optional object)	Data that can be passed to notification method
 *
 *	@return					(integer)			The ID of the newly created controller
*/
/**
 *	This method is exposed to scripting. It creates a controller that
 *	will move an entity towards a destination point, following waypoints.
 *	The entity will continue moving until it reaches its destination,
 *	or an error occurs (or the controller is cancelled).
 *
 *	@param dstPosition			Destination point
 *	@param velocity				Movement velocity in m/s
 *	@param faceMovement         Whether to face the direction of movement or not ie strafe
 *	@param maxSearchDistance	Max distance to search
 *	@param girth				The girth of this entity
 *  @param closeEnough			How close to end point we need to get.
 *	@param userArg				User data to be passed to the callback
 *
 *	@return		The integer ID of the newly created controller.
 */
PyObject * EntityNavigate::navigate( const Vector3 & dstPosition,
		float velocity, bool faceMovement, float maxSearchDistance,
		float girth, float closeEnough, int userArg )
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	ControllerPtr pController = new NavigationController( dstPosition, velocity,
		faceMovement, maxSearchDistance, girth, closeEnough );

	return Script::getData( entity_.addController( pController, userArg ) );
}


/*~ function Entity.getStopPoint
 *  @components{ cell }
 *	This function should be called if we want to navigate an entity from its current
 *	position to destination. It is intended to be used as a replacement to canNavigateTo
 *	that knows about activated portals (see ChunkPortal.activate()).
 *	@param	destination	(Vector3) The final destination point for the Entity
 *	@param	ignoreFirstStopPoint (bool) Set to true to determine next stopping point past current
 *			stopping point, otherwise the current stopping point would get triggered again.
 *	@param	maxSearchDistance (optional float) Set the max distance of search (default to 500).
 *	@param	girth (optional float) Which navigation girth to use (default 0.5)
 *	@param	stopDist (optional float) Distance before portal to stop (default 1.5)
 *	@param	nearPortalDist (optional float) Distance back from portal that the Entity should
 *			be to be considered next to model blocking portal (eg, door) (default 1.8)
 *	@return (Vector3, bool) Returns None if Entity can't navigate to destination.  Otherwise,
 *			returns the point stopDist back along the navigation path from the activated
 *			portal and true if it reached the destination.
 */
/*
 * This function should be called if we want to navigate an entity from its
 * current position to destination. It is intended to be used as a replacement
 * to canNavigateTo that knows about doors.
 *
 * returns:
 *		None - can't navigate to destination.
 *		else: ( Vector3 stopPoint, bool reachedDestination )
 *
 *  where stopPoint is a point stopDist back along the navigation path from the
 *  activated portal (portal with a door). nearPortalDist is the distance along
 *  path back from activated portal to consider that entity is next to door.
 */
PyObject * EntityNavigate::getStopPoint( const Vector3 & destination,
		bool ignoreFirstStopPoint, float maxSearchDistance,
		float girth, float stopDist, float nearPortalDist )
{

	Vector3 source = entity_.position();

	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	// determine the source navloc
	NavLoc srcLoc;
	this->validateNavLoc( entity_.position(), girth, srcLoc );
	if ( !srcLoc.valid() ) // complain if invalid
	{
		PyErr_Format( PyExc_ValueError,
			"Entity.getStopPoint: Could not resolve source position" );
		return NULL;
	}

	// determine the dest navloc
	NavLoc dstLoc( entity_.pChunkSpace(), destination, girth );
	if ( !dstLoc.valid() ) // complain if invalid
	{
		PyErr_Format( PyExc_ValueError, "Entity.getStopPoint: "
			"Could not resolve destination position" );
		return NULL;
	}

	dstLoc.clip();

	// prepare jump over navPoly path.
	//entity_.pReal()->navigator().clearCache();
	Vector3 position = source; // entity_.position();
	std::vector<Vector3> stepPoints;
	stepPoints.push_back( position );
	int safetyCount = 500;
	bool skippedFirstPoint = false;
	while ( (fabs( position.x - destination.x ) > 0.01f
		     || fabs( position.z  - destination.z) > 0.01f) &&
			(--safetyCount > 0) )
	{

		bool passedActivatedPortal;

		if ( !this->getNavigatePosition( srcLoc, dstLoc, maxSearchDistance, 
				position, passedActivatedPortal, girth ) )
		{
			Py_Return;
		}

		this->validateNavLoc( position, girth, srcLoc );
		if ( !srcLoc.valid() )	// complain if srcLoc invalid
		{
			PyErr_Format( PyExc_ValueError, "Entity.getStopPoint: "
				"src location not valid" );
			return NULL;
		}

		stepPoints.push_back( position );

		if ( passedActivatedPortal )
		{
			// we've changed chunks. Calculate dist traveled from start.

			float dist = 0.0;

			for ( uint i=1; i<stepPoints.size(); ++i ) // HERE
			{
				dist += (float)sqrt( (stepPoints[i].x - stepPoints[i-1].x) * (stepPoints[i].x - stepPoints[i-1].x) +
									 (stepPoints[i].z - stepPoints[i-1].z) * (stepPoints[i].z - stepPoints[i-1].z) );
			}

			// if starting point is a long way away, then we definitely want
			// to stop at this portal.
			if ( dist > nearPortalDist )
			{
				// but if we're told to skip the first point, and we haven't yet, then
				// this is a problem.
				if ( !skippedFirstPoint && ignoreFirstStopPoint )
				{
					PyErr_Format( PyExc_ValueError, "Entity.getStopPoint: "
							"told to ignore first portal, but further than "
							"nearPortalDist away from it." );

					return NULL;
				}
				// the following will never happen, but put check in just for good measure.
				if ( skippedFirstPoint && !ignoreFirstStopPoint )
				{
					PyErr_Format( PyExc_AssertionError, "Entity.getStopPoint: "
						"Programmer logic error." );

					return NULL;
				}

				// this is the portal we want to stop at. Find the position stopDist
				// back from the portal.
				float backDist = 0.0;
				float prevBackDist = 0.0;
				float lastLength = 0.0;
				int i = (int)stepPoints.size()-1; // HERE
				for ( ; i>0; --i )
				{
					lastLength = (float)sqrt( (stepPoints[i].x - stepPoints[i-1].x) *
									          (stepPoints[i].x - stepPoints[i-1].x) +
									(stepPoints[i].z - stepPoints[i-1].z) *
									(stepPoints[i].z - stepPoints[i-1].z) );

					backDist += lastLength;

					if ( backDist > stopDist )
					{
						break;
					}

					prevBackDist = backDist;
				}

				// will never happen but check anyway.
				if ( backDist < stopDist )
				{
					PyErr_Format( PyExc_AssertionError, "Entity.getStopPoint: "
						"Programmer logic error (1)." );
					return NULL;
				}

				float prop = (stopDist - prevBackDist) / lastLength;

				Vector3 dir( (stepPoints[i-1].x - stepPoints[i].x), 0.0,
					(stepPoints[i-1].z - stepPoints[i].z) );

				dir.normalise();

				dir.x *= prop;
				dir.z *= prop;

				Vector3 stopPosition = stepPoints[i]; // HERE
				stopPosition += dir;

				PyObject * pTuple = PyTuple_New( 2 );
				PyTuple_SetItem( pTuple, 0, Script::getData( stopPosition ) );
				PyTuple_SetItem( pTuple, 1, PyBool_FromLong( 0 ) );
				return pTuple;
			}

			// otherwise starting point is close to this portal.
			else
			{
				// if we're not ignoring the first portal, then return the
				// start point.
				if ( !ignoreFirstStopPoint )
				{
					PyObject * pTuple = PyTuple_New( 2 );
					PyTuple_SetItem( pTuple, 0, Script::getData( source ) );
					PyTuple_SetItem( pTuple, 1, PyBool_FromLong( 0 ) );
					return pTuple;
				}
				else
				{
					// sanity check.
					if ( skippedFirstPoint )
					{
						PyErr_Format( PyExc_AssertionError,
							"Entity.getStopPoint: Programmer logic error "
							"(2).\n" );
						return NULL;
					}

					// continue around loop, but note that we've skipped first point.
					skippedFirstPoint = true;
				}
			}

		}

	}

    if ( safetyCount <= 0 )
	{
		PyErr_Format( PyExc_ValueError, "safety count exceeded." );
		return NULL;
	}

	// no doors encountered.
	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SetItem( pTuple, 0, Script::getData( destination ) );
	PyTuple_SetItem( pTuple, 1, PyBool_FromLong( 1 ) );
	return pTuple;

}



/**
 *	This method gets the next straight-line step position for navigation between
 *	the given NavLocs. Note: both NavLocs must be valid.
 */
bool EntityNavigate::getNavigatePosition( NavLoc srcLoc, NavLoc dstLoc,
		float maxSearchDistance, Vector3 & nextPosition, 
		bool & passedActivatedPortal, float girth )
{
	// TODO: Use the girth parameter.
	MF_ASSERT( srcLoc.valid() && dstLoc.valid() );

	// ok, we're ready to go
	/*
	DEBUG_MSG( "Entity.getNavigatePosition: %d "
			"srcLoc is (%f,%f,%f) in %s id %d [%s]\n",
		entity_.id(), srcLoc.point().x, srcLoc.point().y, srcLoc.point().z,
		srcLoc.pSet()->chunk()->identifier().c_str(), srcLoc.waypoint(),
		(srcLoc.isWithinWP() ? "Exact": "Closest"));

	DEBUG_MSG( "Entity.getNavigatePosition: "
			"dstLoc is (%f,%f,%f) in %s id %d [%s]\n",
		dstLoc.point().x, dstLoc.point().y, dstLoc.point().z,
		dstLoc.pSet()->chunk()->identifier().c_str(), dstLoc.waypoint(),
		(dstLoc.isWithinWP() ? "Exact": "Closest") );
	*/
	// if they are the same waypoint it is easy
	if (srcLoc.waypointsEqual( dstLoc ))
	{
		Vector3 wp = dstLoc.point();
		// set our y position now
		srcLoc.makeMaxHeight( wp );
		nextPosition = wp;
		return true;
	}
	else
	{
		NavLoc wayLoc;

		bool ok = entity_.pReal()->navigator().findPath(
			srcLoc, dstLoc, maxSearchDistance,
			/* blockNonPermissive */ true, 
			wayLoc, passedActivatedPortal );

		if (!ok)
		{
			/*
			Position3D dstPosition = srcLoc.pSet()->chunk()->transform().applyPoint(
				dstLoc.point() );
			DEBUG_MSG( "Entity.getNavigatePosition: No path "
				"from (%f,%f,%f) in %s "
				"to (%f,%f,%f) in %s with girth %f\n",
				entity_.position().x, entity_.position().y, entity_.position().z,
				srcLoc.pSet()->chunk()->identifier().c_str(),
				dstPosition.x, dstPosition.y, dstPosition.z,
				dstLoc.pSet()->chunk()->identifier().c_str(),
				girth );
			*/
			return false;
		}

		// we have found our way! (YOUR way? All ways are MY ways!)
		entity_.pReal()->navLoc( wayLoc );

		/*
		DEBUG_MSG( "Entity.getNavigatePosition: Path found: "
				"Next waypoint is (%f,%f,%f) in %s id %d [%s]\n",
			wayLoc.point().x, wayLoc.point().y, wayLoc.point().z,
			wayLoc.pSet()->chunk()->identifier().c_str(), wayLoc.waypoint(),
			(wayLoc.isWithinWP() ? "Exact": "Closest") );
		*/

		// calculate the way world position, and ensure it is not too far away
		Vector3 wp = wayLoc.point();

		// set our y position now
		NavLoc& heightLoc = (wayLoc.waypoint() < 0) ? srcLoc : wayLoc;
		heightLoc.makeMaxHeight( wp );

		//DEBUG_MSG( "Entity.getNavigatePosition: "
		//		"nextPosition is: (%6.3f, %6.3f, %6.3f)\n",
		//	wp.x, wp.y, wp.z );

		nextPosition = wp;
		return true;
	}
}


/*~ function Entity.navigateStep
 *  @components{ cell }
 *
 *	This function uses the BigWorld navigation system to move the Entity towards a destination point,
 *	stopping at the next waypoint or given distance, invoking a notification method on success or failure.
 *	BigWorld can have several pre-generated navigation systems, each with a different girth (resulting in
 *	different navigation paths).  For a given Entity, only one navigation/movement controller is
 *	active at any one time.
 *	Returns a ControllerID that can be used to cancel the movement. For example,
 *	Entity.cancel( movementID ).  Movement can also be cancelled with
 *	Entity.cancel( "Movement" ). The notification methods are not called when
 *	movement is cancelled.
 *
 *	The notification method are defined as follows;
 *	@{
 *		def onMove( self, controllerId, userData ):
 *		def onNavigateFailed( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destination		(Vector3)			The destination point for the Entity to move towards
 *	@param	velocity		(float)				The speed to move the Entity in m/s
 *	@param	maxMoveDistance	(float)				Maximum distance to move
 *	@param	maxSearchDistance (float)			Maximum distance to search
 *	@param	faceMovement	(optional bool)		True if entity is to face in direction of movement (default).
 *												Should be false if another mechanism is to provide direction...
 *	@param	girth			(optional float)	Which navigation girth to use (default 0.5)
 *	@param	userData		(optional object)	Data that can be passed to notification method
 *
 *	@return					(integer)			The ID of the newly created controller
 */
/**
 *	This method is exposed to scripting. It creates a controller that
 *	will move an entity towards a destination point, following waypoints.
 *	The entity will stop moving when it reaches a new waypoint, or when
 *	it exceeds a maximum distance.
 *
 *	@param dstPosition			Destination point
 *	@param velocity				Movement velocity in m/s
 *	@param maxMoveDistance		Maximum distance to move
 *	@param maxSearchDistance	Maximum distance to search
 *	@param faceMovement			Whether or not the entity should face the
 *								direction of movement.
 *	@param girth				The girth of this entity
 *	@param userArg				User data to be passed to the callback
 *
 *	@return		The integer ID of the newly created controller.
 */
PyObject * EntityNavigate::navigateStep( const Vector3 & dstPosition,
		float velocity, float maxMoveDistance, float maxSearchDistance, 
		bool faceMovement, float girth, int userArg )
{
	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	// figure out the source navloc
	NavLoc srcLoc;
	this->validateNavLoc( entity_.position(), girth, srcLoc );
	// complain if invalid
	if (!srcLoc.valid())
	{
		PyErr_Format( PyExc_ValueError, "Entity.navigateStep: "
			"Could not resolve source position" );
		return NULL;

	}

	// figure out the dest navloc
	NavLoc dstLoc( entity_.pChunkSpace(), dstPosition, girth );
	// complain if invalid
	if (!dstLoc.valid())
	{
		PyErr_Format( PyExc_ValueError, "Entity.navigateStep: "
			"Could not resolve destination position" );
		return NULL;
	}

	dstLoc.clip();

	Vector3 wp;
	bool passedActivatedPortal;

	bool ok = this->getNavigatePosition( srcLoc, dstLoc,
		maxSearchDistance, wp, passedActivatedPortal, girth );

	if (!ok)
	{
		PyErr_SetString( PyExc_EnvironmentError,
				"Entity.navigate: No path found" );
		return NULL;
	}

	//dprintf( "Way world is (%f,%f,%f)\n", wp.x, wp.y, wp.z );
	Vector3 disp = wp - entity_.position();

	disp.y = 0.f;

	float wayDistance = disp.length();
	float dstY = wp.y;

	Vector3 wsv( wp );
	srcLoc.makeMaxHeight( wsv );
	float srcY = wsv.y;

	if (wayDistance > maxMoveDistance)
	{
		// shorten it
		wp = entity_.position() + disp * (maxMoveDistance / wayDistance);

		if (srcLoc.pSet()->waypoint( srcLoc.waypoint() ).
				containsProjection( *(srcLoc.pSet()),
					MappedVector3( wp, srcLoc.pSet()->chunk(),
						MappedVector3::WORLD_SPACE ) ))
		{
			//dprintf( "Way world shortened to (%f,%f,%f)\n", wp.x, wp.y, wp.z );
			// and keep our cached navloc at our source
			entity_.pReal()->navLoc( srcLoc );
			dstY = srcY;
		}
	}

	// set our y position now
	Position3D position = entity_.position();
	wp.y = std::max( dstY, srcY );
	position.y = wp.y;
	entity_.setPositionAndDirection( position, entity_.direction() );

	// and finally return a new controller
	ControllerPtr pController = new MoveToPointController(
		wp, std::string(), 0, velocity, faceMovement );
	return Script::getData( entity_.addController( pController, userArg ) );
}


/*~ function Entity.navigateFollow
 *  @components{ cell }
 *
 *	This function uses the BigWorld navigation system to move the Entity
 *	towards another Entity,	stopping at the next waypoint or given distance,
 *	invoking a notification method on success or failure.
 *
 *	BigWorld can have several pre-generated navigation systems, each with a
 *	different girth (resulting in different navigation paths).  For a given
 *	Entity, only one navigation / movement controller is active at any one
 *	time.
 *
 *	Returns a ControllerID that can be used to cancel the movement. For
 *	example; Entity.cancel( movementID ).  Movement can also be cancelled with
 *	Entity.cancel( "Movement" ).
 *
 *	The notification methods are not called when movement is cancelled.
 *
 *	The notification method are defined as follows;
 * 	@{
 *		def onMove( self, controllerId, userData ):
 *		def onNavigateFailed( self, controllerId, userData ):
 *	@}
 *
 *	@see cancel
 *
 *	@param	destEntity			(Entity) The target Entity
 *	@param	angle				(float)	The angle at which the Entity should
 *								attempt to follow (in radians)
 *	@param	distance			(float) The distance at which the Entity should
 *								attempt to follow
 *	@param	velocity			(float)	The speed to move the Entity in m/s
 *	@param	maxMoveDistance		(float)	Maximum distance to move
 *	@param	maxSearchDistance 	(optional float) The maximum search distance
 *								(default 500m)
 *	@param	faceMovement		(optional bool) True if entity is to face in
 *								direction of movement (default).
 *								Should be false if another mechanism is to
 *								provide direction.
 *	@param	girth				(optional float) Which navigation girth to use
 *								(default 0.5)
 *	@param	userData			(optional int) Data that can be passed to
 *								notification method
 *
 *	@return (integer) The ID of the newly created controller
 */
/**
 *	This method is exposed to scripting. It moves the entity to a position that
 *	is describe as an offset from another entity.
 *
 *	@param pEntity				The entity that the position is relative to.
 *	@param angle				The offset angle from the destination entity.
 *	@param distance				The offset distance (in metres) form the
 *								destination entity.
 *	@param velocity				Movement velocity in m/s
 *	@param maxMoveDistance		Maximum distance to move
 *	@param maxSearchDistance 	The maximum search distance
 *	@param faceMovement			Whether or not the entity should face the
 *								direction of movement.
 *	@param girth				The girth of this entity
 *	@param userArg				User data to be passed to the callback
 *
 *	@return		The integer ID of the newly created controller.
 */
PyObject * EntityNavigate::navigateFollow( PyObjectPtr pEntityObj, float angle,
		float distance, float velocity, float maxMoveDistance, 
		float maxSearchDistance, bool faceMovement, float girth, int userArg )
{
	// Check that the entity parameter actually has an entity
	if (!PyObject_IsInstance( pEntityObj.get(), (PyObject*)(&Entity::s_type_) ))
	{
		PyErr_SetString( PyExc_TypeError, 
			"parameter 1 must be an Entity instance" );
		return NULL;
	}

	Entity * pOtherEntity = static_cast< Entity * >( pEntityObj.get() );

	// A destroyed entity has no real location
	if (pOtherEntity->isDestroyed())
	{
		PyErr_SetString( PyExc_ValueError,
			"cannot follow a destroyed entity" );
		return NULL;
	}

	// Check that the entity is in the same space
	if (this->entity().space().id() != pOtherEntity->space().id())
	{
		PyErr_SetString( PyExc_ValueError,
			"followed entity is not in the same space" );
		return NULL;
	}

	if (maxSearchDistance == 0.f)
	{
		PyErr_SetString( PyExc_ValueError,
			"maxSearchDistance cannot be 0.0" );
		return NULL;
	}

	float yaw = pOtherEntity->direction().yaw;
	float totalYaw = yaw + angle;
	Vector3 offset = Vector3( distance * sinf( totalYaw ),
			0, distance * cosf( totalYaw ) );
	Vector3 position = pOtherEntity->position() + offset;

	return this->navigateStep( position, velocity, maxMoveDistance, 
		maxSearchDistance, faceMovement, girth, userArg );
}


/*~ function Entity.canNavigateTo
 *  @components{ cell }
 *
 *	This function uses the BigWorld navigation system to determine whether the
 *	Entity can move to the given destination.  BigWorld can have several
 *	pre-generated navigation systems, each with a different girth (resulting in
 *	different navigation paths).
 *
 *	@see cancel
 *
 *	@param	destination        (Vector3)        The destination point for the
 *												navigation test
 *	@param	maxSearchDistance  (optional float) Max distance to search
 *	                                            (default 500m)
 *	@param	girth              (optional float) Which navigation girth to use
 *												for the test (default 0.5m)
 *
 *	@return					(Vector3)			The closest position that the
 *												Entity can navigate to, or None
 *												if no path is found.
 */
/**
 *	This method is exposed to scripting. It checks to see if this entity
 *	can move towards a destination point, following waypoints.
 *
 *	@param dstPosition			Destination point
 *	@param maxSearchDistance	Max distance to search (default 500)
 *	@param girth				The girth of this entity for the test
 *
 *	@return		whether or not the destination can be navigated to.
 */
PyObject * EntityNavigate::canNavigateTo( const Vector3 & dstPosition,
		float maxSearchDistance, float girth )
{
	AUTO_SCOPED_PROFILE( "canNavigateTo" );

	if (!entity_.isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"This method can only be called on a real entity." );
		return NULL;
	}

	// figure out the source navloc
	NavLoc srcLoc( entity_.pChunkSpace(), entity_.position(), girth );
	// NavLoc srcLoc;
	//this->validateNavLoc( entity_.position(), girth, srcLoc );
	// complain if invalid
	if (!srcLoc.valid())
	{
		/*
		DEBUG_MSG( "Entity.canNavigateTo: "
			"Could not resolve source position (%f,%f,%f)+%f",
			entity_.position().x, entity_.position().y, entity_.position().z, girth );
		*/
		Py_Return;
	}

	// figure out the dest navloc
	NavLoc dstLoc( entity_.pChunkSpace(), dstPosition, girth );
	// complain if invalid
	if (!dstLoc.valid())
	{
		/*
		DEBUG_MSG( "Entity.canNavigateTo: "
			"Could not resolve destination position (%f,%f,%f)+%f\n",
			dstPosition.x, dstPosition.y, dstPosition.z, girth );
		*/
		Py_Return;
	}

	dstLoc.clip();

	// if they are the same waypoint it is easy
	if (srcLoc.waypointsEqual( dstLoc ))
	{
		Vector3 wp = dstLoc.point();
		// set our y position now
		srcLoc.makeMaxHeight( wp );
		return Script::getData( wp );
	}
	else
	{
		// need to clear navigation cache. It is possible that when a door was open
		// and a search from srcLoc -> dstLoc was stored, which is still there. The
		// door may now be closed.
		//entity_.pReal()->navigator().clearCache();
		NavLoc wayLoc;
		bool passedActivatedPortal;
		bool ok;

		{
			AUTO_SCOPED_PROFILE( "findPath" );
			ok = entity_.pReal()->navigator().findPath( srcLoc, dstLoc, 
					maxSearchDistance, /* blockNonPermissive: */ true, 
					wayLoc, passedActivatedPortal );
		}

		if (entity_.pReal()->navigator().infiniteLoopProblem())
		{
			ERROR_MSG( "EntityNavigate::canNavigateTo: infinite loop problem: "
					"Entity %u src pos: (%f, %f, %f), src chunk: %s "
					"dst pos (%f %f %f)\n",
				entity_.id(),
				entity_.position().x, entity_.position().y, entity_.position().z,
				entity_.pChunk() ? entity_.pChunk()->identifier().c_str() : "??",
				dstPosition.x, dstPosition.y, dstPosition.z );
		}

		if (ok)
		{
			Vector3 wp = dstLoc.point();
			// set our y position now
			dstLoc.makeMaxHeight( wp );

			//DEBUG_MSG( "Entity.canNavigateTo: destPosition is: (%6.3f, %6.3f, %6.3f)\n",
			//	wp.x, wp.y, wp.z );
			return Script::getData( wp );
		}
		else
		{
			Py_Return;
		}
	}
}

/*~ function Entity.waySetPathLength
 *  @components{ cell }
 *
 *	Returns the length of the most recently determined navigation path
 *
 *	@return					(float)			The length of the most recently determined navigation path
 */
/**
 *	This method is exposed to scripting. It returns the wayset path length of
 *	the last cached path.
 *
 *	@return		The length of last way set path
 */
int EntityNavigate::waySetPathLength()
{
	// TODO: Move this to RealEntity if it ever supports script extensions.
	if (!entity_.isRealToScript())
	{

		ERROR_MSG( "EntityNavigate::waySetPathLength: "
			"This method can only be called on a real entity." );
		return 0;
	}

	return entity_.pReal()->navigator().getWaySetPathSize();
}



/*~	function Entity.navigatePathPoints
 *	@components{ cell }
 *
 *	This function returns the points along a path from this entity position 
 *	to the given destination position.
 *
 *	@param 	destination 		(Vector3) 	The destination point.
 *	@param 	maxSearchDistance 	(float)		The maximum distance to search.
 *	@param 	girth				(float)		The navigation girth (defaults to 
											0.5).
 */
/**
 *	This method returns the points along a path from this entity position to
 *	the destination position.
 */
PyObject * EntityNavigate::navigatePathPoints( const Vector3 & dstPosition,
		float maxSearchDistance, float girth )
{
	NavLoc srcLoc;
	this->validateNavLoc( entity_.position(), girth, srcLoc );

	// Don't clip the source point, as then the entity's actual position will
	// be out of sync.

	Navigator & navigator = entity_.pReal()->navigator();

	return ::navigatePathPoints( navigator, entity_.pChunkSpace(), srcLoc, 
		dstPosition, maxSearchDistance, girth );
}


// -----------------------------------------------------------------------------
// Section: Navigation script methods in the BigWorld module
// -----------------------------------------------------------------------------

namespace // (anonymous)
{



/**
 *	Find a point nearby random point in a connected navmesh
 *
 *	@param funcName 	The name of python function for error report
 *	@param spaceID 	The id of the space in which to operate.
 *	@param position	The position of the point
 *	@param minRadius	The minimum radius to search
 *	@param maxRadius	The maximum radius to search
 *	@param girth	Which navigation girth to use (optional and default to 0.5)
 *	@return			The random point found, as a Vector3
 */
PyObject * findRandomNeighbourPointWithRange( const char* funcName, 
		SpaceID spaceID, Vector3 position, 
		float minRadius, float maxRadius, float girth )
{
	ChunkSpacePtr pChunkSpace =
		ChunkManager::instance().space( spaceID, false );

	if (!pChunkSpace)
	{
		PyErr_Format( PyExc_ValueError, "%s: Invalid space ID %d",
			funcName, int(spaceID) );
		return NULL;
	}

	// Start from a bit above so we drop to the correct navpoly
	position.y += 0.1f;

	NavLoc navLoc( pChunkSpace.get(), position, girth );

	if (!navLoc.valid())
	{
		char buf[64];
		bw_snprintf( buf, sizeof( buf ), "(%f, %f, %f)",
			 position.x, position.y, position.z );

		PyErr_Format( PyExc_ValueError, 
			"%s: Cannot find a navmesh at position %s",
			funcName, buf );
		return NULL;
	}

	float radius = float( rand() % 100 ) * ( maxRadius - minRadius ) / 100.0f + 
		minRadius;

	float angle = float( rand() % 360 ) / 180 * MATH_PI;
	position.x += radius * cos( angle );
	position.z += radius * sin( angle );

	ChunkWaypointSetPtr pSet = navLoc.pSet();
	int waypoint = navLoc.waypoint();

	ChunkWaypointSetPtr pLastSet;
	int lastWaypoint = -1;

	WorldSpaceVector3 clipPos( position );
	navLoc.clip( clipPos );
	float minDistSquared = 
		( clipPos.x - position.x ) * ( clipPos.x - position.x ) +
		( clipPos.z - position.z ) * ( clipPos.z - position.z );

	bool foundNew = true;

	while (foundNew)
	{
		foundNew = false;

		for (WaypointNeighbourIterator wni( pSet, waypoint );
				!wni.ended(); 
				wni.advance())
		{
			if (wni.pNeighbourSet() == pLastSet &&
					wni.neighbourWaypointIndex() == lastWaypoint)
			{
				continue;
			}

			ChunkWaypoint& wp = wni.neighbourWaypoint();

			clipPos = WorldSpaceVector3( position );
			wp.clip( *(wni.pNeighbourSet()), wni.pNeighbourSet()->chunk(), 
				clipPos );

			float distSquared = 
				( clipPos.x - position.x ) * ( clipPos.x - position.x ) +
				( clipPos.z - position.z ) * ( clipPos.z - position.z );

			const float DISTANCE_ADJUST_FACTOR = 0.01f;
			if (distSquared + DISTANCE_ADJUST_FACTOR < minDistSquared)
			{
				minDistSquared = distSquared;
				pSet = wni.pNeighbourSet();
				waypoint = wni.neighbourWaypointIndex();
				foundNew = true;
			}
		}

		lastWaypoint = waypoint;
		pLastSet = pSet;
	}

	ChunkWaypoint & wp = pSet->waypoint( waypoint );
	clipPos = WorldSpaceVector3( position );
	wp.clip( *pSet, pSet->chunk(), clipPos );
	wp.makeMaxHeight( pSet->chunk(), clipPos );

	NavLoc result( pChunkSpace.get(), clipPos, girth );

	return Script::getData( result.point() );
}


/*~ function BigWorld findRandomNeighbourPointWithRange
 *  @components{ cell }
 *	This function can be used to find a random point in a connected navmesh.
 *	The result point is guaranteed to be connectable to the point.
 *	Note that in some conditions the result point might be closer than minRadius.
 *
 *	@param spaceID 		(int)				The ID of the space in which to operate.
 *	@param position		(Vector3)			The position of the point
 *	@param minRadius	(float) 			The minimum radius to search
 *	@param maxRadius	(float) 			The maximum radius to search
 *	@param girth		(optional float) 	Which navigation girth to use (optional and default to 0.5)
 *	@return				(Vector3)			The random point found
 */
/**
 *	Find a point nearby random point in a connected navmesh
 *
 *	@param spaceID 	The id of the space in which to operate.
 *	@param position	The position of the point
 *	@param minRadius	The minimum radius to search
 *	@param maxRadius	The maximum radius to search
 *	@param girth	Which navigation girth to use (optional and default to 0.5)
 *	@return			The random point found, as a Vector3
 */
PyObject * findRandomNeighbourPointWithRange( SpaceID spaceID,
		const Vector3 & position, float minRadius, float maxRadius,
		float girth = 0.5 )
{
	return findRandomNeighbourPointWithRange( 
		"BigWorld.findRandomNeighbourPointWithRange",
		spaceID, position, minRadius, maxRadius, girth );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, findRandomNeighbourPointWithRange,
	ARG( SpaceID, ARG( Vector3, ARG( float, ARG( float, 
		OPTARG( float, 0.5f, END ) ) ) ) ), BigWorld )


/*~ function BigWorld findRandomNeighbourPoint
 *  @components{ cell }
 *	This function can be used to find a random point in a connected navmesh.
 *	The result point is guaranteed to be connectable to the point.
 *
 *	@param spaceID 	(int)				The ID of the space in which to operate.
 *	@param position	(Vector3)			The position of the point
 *	@param radius	(float) 			The radius to search
 *	@param girth	(optional float) 	Which navigation girth to use (optional and default to 0.5)
 *	@return			(Vector3)			The random point found
 */
/**
 *	Find a point nearby random point in a connected navmesh
 *
 *	@param spaceID 	The id of the space in which to operate.
 *	@param position	The position of the point
 *	@param radius	The radius to search
 *	@param girth	Which navigation girth to use (optional and default to 0.5)
 *	@return			The random point found, as a Vector3
 */
PyObject * findRandomNeighbourPoint( SpaceID spaceID,
	Vector3 position, float radius, float girth = 0.5 )
{
	return findRandomNeighbourPointWithRange( "BigWorld.findRandomNeighbourPoint",
		spaceID, position, 0.f, radius, girth );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, findRandomNeighbourPoint,
	ARG( SpaceID, ARG( Vector3, ARG( float, OPTARG( float, 0.5f, END ) ) ) ), BigWorld )

	
/**
 *	NavInfo flags
 *	TODO: Rename this to something more appropriate
 */
enum ResolvePointFlags
{
	// NAVINFO_UNKNOWN	= 0x00,	// chunkID and waypointID are invalid
	NAVINFO_VALID	= 0x01, // chunkID and waypointID are valid
	// NAVINFO_GUESS	= 0x02,	// chunkID and waypointID are out of date
	NAVINFO_CLOSEST	= 0x04,	// no match. using closest chunk/waypoint.
};


/*~ function BigWorld configureConnection
 *  @components{ cell }
 *	This function has now been deprecated. Use Entity.addPortalConfig instead.
 *
 *	This function configures the connection between two demonstrative points
 *	that straddle a portal. The connection may be either open or closed.
 *
 *	@param spaceID 	The id of the space in which to operate.
 *	@param point1	The first demonstrative point.
 *	@param point2	The second demonstrative point.
 *	@param isOpen	A boolean value indicating whether or not the portal is
 *					open.
 */
/**
 *	Configure the connection between the demonstrative points pta and ptb
 *	in spaceID. The connection may be either open or closed.
 *	The demonstrative points must straddle a portal.
 */
PyObject * configureConnection( SpaceID spaceID,
		const Vector3 & pta, const Vector3 & ptb, bool connect )
{
	ERROR_MSG( "BigWorld.configureConnection: "
			"This function has been deprecated."
			"Use BigWorld.addPortalConfig instead\n" );
	// TODO: configureConnection should be implemented with a ghost controller
	// on the entity that is near the connection.

	ChunkSpacePtr pChunkSpace =
		ChunkManager::instance().space( spaceID, false );
	if (!pChunkSpace)
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.configureConnection(): "
			"No space ID %d", int(spaceID) );
		return NULL;
	}

	NavLoc na( pChunkSpace.get(), pta, 0.5f );
	NavLoc nb( pChunkSpace.get(), ptb, 0.5f );
	// the same portals are used for all girths, so we don't need to
	// worry about what we pass here (as long as it is small enough)

	if (!na.valid() || !nb.valid())
	{
		PyErr_SetString( PyExc_ValueError, "BigWorld.configureConnection: "
			"Could not resolve demonstrative points" );
		return NULL;
	}

	if (na.pSet() == nb.pSet())
	{
		PyErr_SetString( PyExc_ValueError, "BigWorld.configureConnection: "
			"Both demonstrative points are in the same waypoint set" );
		return NULL;
	}

	// ok, find the connection. since nothing is loaded dynamically,
	// we do not need to worry about what happens when it goes away.
	// and this is still broken for multiple cells.
	ChunkWaypointConns::iterator iterA = na.pSet()->connectionsBegin();
	for (;iterA != na.pSet()->connectionsEnd(); ++iterA)
	{
		if (iterA->first == nb.pSet())
		{
			if (iterA->second != NULL)
			{
				iterA->second->permissive = connect;
/*
				DEBUG_MSG("set portal %s connection on chunk %s to %s\n",
							conn.portal_->label.c_str(),
							conn.portal_->pChunk->identifier().c_str(),
							connect ? "True" : "False");
*/
			}
			else
			{
				iterA = na.pSet()->connectionsEnd();
			}
			break;
		}
	}

	ChunkWaypointConns::iterator iterB = nb.pSet()->connectionsEnd();
	for (;iterB != nb.pSet()->connectionsEnd(); ++iterB)
	{
		if (iterB->first == na.pSet())
		{
			if (iterB->second != NULL)
			{
				iterB->second->permissive = connect;
/*
				DEBUG_MSG("set portal %s connection on chunk %s to %s\n",
							conn.portal_->label.c_str(),
							conn.portal_->pChunk->identifier().c_str(),
							connect ? "True" : "False");
*/
			}
			else
			{
				iterB = nb.pSet()->connectionsEnd();
			}
			break;
		}
	}

	// see if we succeeded
	if (iterA == na.pSet()->connectionsEnd() &&
			iterB == nb.pSet()->connectionsEnd())
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.configureConnection: "
			"Set 0x%p in chunk %s is not adjacent to set 0x%p in chunk %s "
			"in the waypoint graph",
			na.pSet().get(), na.pSet()->chunk()->identifier().c_str(),
			nb.pSet().get(), nb.pSet()->chunk()->identifier().c_str() );
		return NULL;
	}
	if (iterA == na.pSet()->connectionsEnd() ||
			iterB == nb.pSet()->connectionsEnd())
	{
		// this is a system problem, don't trouble Python with it.
		ERROR_MSG( "configureConnection: Waypoint sets not mutually adjacent - "
			"connection configured in one direction only. "
			"na 0x%p in %s, nb 0x%p in %s\n",
			na.pSet().get(), na.pSet()->chunk()->identifier().c_str(),
			nb.pSet().get(), nb.pSet()->chunk()->identifier().c_str() );
	}

	Py_Return;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, configureConnection,
	ARG( SpaceID, ARG( Vector3, ARG( Vector3, ARG( bool, END ) ) ) ), BigWorld )


/**
 *	This helper function returns a list of points to navigate between the given
 *	source location to the given destination, for the given ChunkSpace. This is
 *	called by Entity.navigatePathPoints() and BigWorld.navigatePathPoints().
 */ 
PyObject * navigatePathPoints( Navigator & navigator, 
		ChunkSpace * pChunkSpace, const NavLoc & srcLoc, const Vector3 & dst, 
		float maxSearchDistance, float girth )
{
	char positionString[64];
	char girthString[8];

	if (!srcLoc.valid())
	{
		// Py_ErrFormat doesn't support %f
		bw_snprintf( positionString, sizeof( positionString ), 
			"(%.02f, %.02f, %.02f)", 
			srcLoc.point().x,
			srcLoc.point().y,
			srcLoc.point().z );
		bw_snprintf( girthString, sizeof( girthString ),
			"%.02f", girth );

		PyErr_Format( PyExc_ValueError, 
			"Source position %s is not in an area with "
				"valid navmesh (girth=%s)", 
			positionString, girthString );
		return NULL;
	}

	// Py_ErrFormat doesn't support %f
	NavLoc dstLoc( pChunkSpace, dst, girth );
	if (!dstLoc.valid())
	{
		bw_snprintf( positionString, sizeof( positionString ), 
			"(%.02f, %.02f, %.02f)", 
			dst.x, dst.y, dst.z );
		bw_snprintf( girthString, sizeof( girthString ),
			"%.02f", girth );
		PyErr_Format( PyExc_ValueError, 
			"Destination position %s is not in an area with "
				"valid navmesh (girth=%s)", 
			positionString, girthString );
		return NULL;
	}

	dstLoc.clip();

	Vector3Path pathPoints;

	if (!navigator.findFullPath( srcLoc, dstLoc, maxSearchDistance, 
			/* blockNonPermissive: */ true, pathPoints ))
	{
		PyErr_SetString( PyExc_ValueError, 
			"Full navigation path could not be found" );
		return NULL;
	}

	PyObject * pList = PyList_New( pathPoints.size() );

	Vector3Path::const_iterator iPoint = pathPoints.begin();
	while (iPoint != pathPoints.end())
	{
		PyList_SetItem( pList, iPoint - pathPoints.begin(), 
			Script::getData( *iPoint ) );
		++iPoint;
	}

	return pList;

}


/*~	function BigWorld.navigatePathPoints
 * 	@components{ cell }
 *
 * 	Return a path of points between the given source and destination points in
 * 	the space of the given space ID. The space must be loaded on this CellApp. 
 *
 * 	@param spaceID 	(int)		The space ID.
 * 	@param src	(Vector3)		The source point in the space.
 * 	@param dst	(Vector3)		The destination point in the space.
 * 	@param maxSearchDistance (float)
 * 								The maximum search distance, defaults to 500m.
 * 	@param girth (float) 		The navigation girth grid to use, defaults to
 * 								0.5m.
 *
 * 	@return (list) 	A list of Vector3 points between the source point to the
 * 					destination point.
 *
 */
PyObject * navigatePathPoints( SpaceID spaceID, const Vector3 & src, 
		const Vector3 & dst, float maxSearchDistance, float girth )
{
	static Navigator navigator;

	ChunkSpacePtr pChunkSpace =
		ChunkManager::instance().space( spaceID, false );

	NavLoc srcLoc( pChunkSpace.get(), src, girth );

	// We clip the srcLoc first, unlike in Entity.navigatePathPoints().
	srcLoc.clip();

	PyObject * pList = ::navigatePathPoints( navigator, pChunkSpace.get(), 
		srcLoc, dst, maxSearchDistance, girth );

	if (!pList)
	{
		return NULL;
	}

	if (!almostEqual( srcLoc.point(), src )) 
	{
		// Clipping the point resulted in a different point than the source,
		// insert it into the list at the beginning.
		PyList_Insert( pList, 0, Script::getData( srcLoc.point() ) );
	}

	return pList;
}

PY_AUTO_MODULE_FUNCTION( RETOWN, navigatePathPoints, ARG( SpaceID, ARG( Vector3, 
		ARG( Vector3, OPTARG( float, 500.f, OPTARG( float, 0.5f, END ) ) ) ) ), 
	BigWorld )

} // end (anonymous) namespace

// entity_navigate.cpp
