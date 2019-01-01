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

#include "filter_utility_functions.hpp"

#include "entity.hpp"
#include "entity_manager.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_space.hpp"
#include "math/vector3.hpp"
#include "romp/line_helper.hpp"
#include "terrain/base_terrain_block.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


namespace FilterUtilityFunctions
{


/**
 *	This method updates an entity to ensure it is in the given coordinate
 *	system.
 *
 *	@param	pEntity		The entity to update
 *	@param	spaceID		The server space the entity will now be in.
 *	@param	vehicleID	The ID of the vehicle who's coordinate system the
 *						entity will now be in.
 */
void coordinateSystemCheck(	Entity * pEntity,
							SpaceID spaceID,
							EntityID vehicleID )
{
	BW_GUARD;
	ChunkSpace * pSpace = &*pEntity->pSpace();
	if ( pSpace != NULL )
	{
		if ( spaceID != pSpace->id() )
		{
			pEntity->leaveWorld( true );
			pEntity->enterWorld( spaceID, vehicleID, true );
			return;
		}
	}
	else
	{
		WARNING_MSG( "coordinateSystemCheck: entity %d is not in any space\n",
			pEntity->id() );
		return;
	}

	Entity * pCurrVehicle = pEntity->pVehicle();

	if (pCurrVehicle && pCurrVehicle->isDestroyed())
	{
		Entity * pNewVehicle =
				EntityManager::instance().getEntity( vehicleID, true );

		// TODO: Should have some form a LIMBO so that the passenger is removed
		// from the scene.
		// Only update if there's a new entity otherwise leave where it was.
		if (pNewVehicle)
		{
			pEntity->pVehicle( pNewVehicle );
		}
	}

	if (pEntity->vehicleID() != vehicleID)
	{
		if (vehicleID)
		{
			Entity * pVehicle =
				EntityManager::instance().getEntity( vehicleID, true );
			pEntity->pVehicle( pVehicle );

			// TODO: What should be done if the vehicle cannot be found?
			if (pVehicle == NULL)
			{
				WARNING_MSG( "FilterUtilityFunctions::coordinateSystemCheck: "
						"Passenger %d cannot find vehicle %d\n",
					pEntity->id(), vehicleID );
			}
		}
		else
		{
			pEntity->pVehicle( NULL );
		}
	}
}

/**
 *	This function decodes the 'on ground' condition by sampling
 *	the terrain height.
 */
bool resolveOnGroundPosition(	Position3D & position,
								bool & onGround )
{
	BW_GUARD;
	onGround = false;

	if ( position.y < -12000.0f )
	{
		float terrainHeight = Terrain::BaseTerrainBlock::getHeight( position[0], 
																	position[2] );
		if ( terrainHeight == Terrain::BaseTerrainBlock::NO_TERRAIN )
			return false;


		position.y = terrainHeight;
		onGround = true;
	}

	return true;
}


/**
 *	This is a collision callback used by filterDropPoint() to find the closest
 *	solid triangle.
 */
class SolidCollisionCallback : public CollisionCallback
{
public:
	SolidCollisionCallback() : closestDist_(-1.0f) {}

	virtual int operator()(	const ChunkObstacle & obstacle,
							const WorldTriangle & triangle,
							float dist )
	{
		BW_GUARD;
		// Look in both directions if triangle is not solid.

		if (triangle.flags() & TRIANGLE_NOCOLLIDE)
		{
			return COLLIDE_ALL;
		}

		// Otherwise just find the closest.
		if (closestDist_ == -1.0f || dist < closestDist_)
		{
			closestDist_ = dist;
			tri_ = triangle;
		}

		return COLLIDE_BEFORE;
	}

	float distance() const { return closestDist_; }


	WorldTriangle	tri_;

private:
	float closestDist_;
};


/**
 *	This function takes the input start position and drops it onto the
 *	collision scene. A small upward fudge factor is applied to prevent
 *	the position falling through a collision poly it is resting on.
 *
 *	@param	pSpace			The space in which the position is defined.
 *	@param	startPosition	The initial position to be dropped onto the
 *							collision scene.
 *	@param	land			The position vector into which the final
 *							resting position of the point will be stored.
 *	@param	maxDrop			The maximum distance to search for a place to land.
 *	@param	groundNormal	(Optional Output) The surface normal of the geometry
 *							the drop point dropped onto. Untouched if no drop
 *							point was found.
 *
 *	@return			Returns true if a drop point was found.
 */
bool filterDropPoint(	ChunkSpace * pSpace,
						const Position3D & startPosition,
						Position3D & land,
						float maxDrop,
						Vector3 * groundNormal )
{
	BW_GUARD;
	Vector3 adjustedStartPosition = startPosition;
	adjustedStartPosition.y += 0.1f;

	SolidCollisionCallback scc;

	land = adjustedStartPosition;
	adjustedStartPosition.x += 0.005f;// add some fudge to avoid collision error
	adjustedStartPosition.z += 0.005f;// add some fudge to avoid collision error

	if ( pSpace == NULL )
	{	
		return false;
	}

	pSpace->collide(	adjustedStartPosition,
						Vector3(	adjustedStartPosition.x,
									adjustedStartPosition.y - maxDrop,
									adjustedStartPosition.z ),
						scc );

	if ( scc.distance() < 0.0f )
	{
		return false;
	}

	if (groundNormal)
	{
		*groundNormal = scc.tri_.normal();
		groundNormal->normalise();
	}

	land.y -= scc.distance();
	return true;
}


/**
 *	This function transforms the given position and direction into
 *	common/world space. 
 *
 *	@param	s	The server space in which the give position and direction
 *				are defined.
 *	@param	v	The vehicle who's coordinate system the given position and
 *				direction are defined.
 *	@param	pos	The local space position to be translated into common space.
 *	@param	dir	The local space direction to be translated into common space.
 */
void transformIntoCommon(	SpaceID s,
							EntityID v,
							Position3D & pos,
							Vector3 & dir )
{
	BW_GUARD;
	Entity * pVehicle = EntityManager::instance().getEntity( v );
	if (pVehicle != NULL)
	{
		pVehicle->transformVehicleToCommon( pos, dir );
	}
	else
	{
		ChunkSpace * pOur = &*ChunkManager::instance().space( s, false );
		if (pOur != NULL)
		{
			pOur->transformSpaceToCommon( pos, dir );
		}
	}
}


/**
 *	This function transforms the given position to its local representation.
 *
 *	@param	s	The server space into which the give position and direction
 *				will be placed.
 *	@param	v	The vehicle in who's coordinate system the given position
 *				and direction will now exist.
 *	@param	pos	The common space position to be translated into local space.
 *	@param	dir	The common space direction to be translated into local space.
 */
void transformFromCommon(	SpaceID s,
							EntityID v,
							Position3D & pos,
							Vector3 & dir )
{
	BW_GUARD;
	Entity * pVehicle = EntityManager::instance().getEntity( v );
	if (pVehicle != NULL)
	{
		pVehicle->transformCommonToVehicle( pos, dir );
	}
	else
	{
		ChunkSpace * pOur = &*ChunkManager::instance().space( s, false );
		if (pOur != NULL)
		{
			pOur->transformCommonToSpace( pos, dir );
		}
	}
}

} // namespace FilterUtilityFunctions

/**
 * This function allows interactive testing of terrain collision.  It will 
 * draw a red line if the test "missed" terrain, or a white line if collision
 * was successful.
 */
static void adfTest( SpaceID spaceID, const Vector4ProviderPtr pos )
{
	BW_GUARD;
	// Generate start and end points
	Vector4 vec;
	pos->output( vec );

	Vector3 start = Vector3( vec.x, vec.y, vec.z );
	Vector3 end = start;
	end.y -= 100.0f;

	// Draw
	LineHelper::instance().drawLine(start, end, 0xFFFFFFFF, 300 );

	// Collide
	ChunkSpacePtr space = ChunkManager::instance().space( spaceID, false );
	FilterUtilityFunctions::SolidCollisionCallback scc;

	space->collide( start, end,  scc );
	if ( scc.distance() < 0.0f )
	{
		// draw miss state
		LineHelper::instance().drawLine(start, end, 0xFFFF0000, 300 );
	}
}

PY_AUTO_MODULE_FUNCTION( RETVOID, adfTest, ARG( uint32, ARG( Vector4ProviderPtr, END ) ), BigWorld )

// filter_utility_functions.cpp
