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
#include "worldeditor/collisions/collision_callbacks.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "gizmo/item_functor.hpp"


DECLARE_DEBUG_COMPONENT2( "editor", 0 )


ObstacleLockCollisionCallback ObstacleLockCollisionCallback::s_default;


/**
 *  This operator is called by the collision code to test the collided
 *  triangles.
 *
 *  @param obstacle		ChunkObstacle that owns the triangle.
 *  @param triangle		Triangle that collided, in world coordinates.
 *	@param dist			Collision distance to the triangle.
 *	@return				Collision flags to stop/continue testing triangles.
 *
 *  @see ChunkObstacle
 */
int ObstacleLockCollisionCallback::operator() ( const ChunkObstacle & obstacle,
	const WorldTriangle & triangle, float dist )
{
	BW_GUARD;

	if( MatrixMover::moving() && !obstacle.pItem()->isShellModel() && WorldManager::instance().isItemSelected(obstacle.pItem()) )
		return COLLIDE_ALL;
	// ignore selected
	if ( !obstacle.pItem()->edIsSnappable() )
		return COLLIDE_ALL;

	// ignore back facing
	Vector3 trin = obstacle.transform_.applyVector( triangle.normal() );
	trin.normalise();
	Vector3 lookv = obstacle.transform_.applyPoint( triangle.v0() ) -
		Moo::rc().invView()[3];
	lookv.normalise();
	const float dotty = trin.dotProduct( lookv );
	if ( dotty > 0.f )
		return COLLIDE_ALL;

	// choose if closer
	if ( dist < dist_ )
	{
		dist_ = dist;
		normal_ = trin;
		pItem_ = obstacle.pItem();
		triangleNormal_ = triangle.normal();
	}

	//return COLLIDE_BEFORE;
	return COLLIDE_ALL;
}


/**
 *  This method releases the references/resources gathered during the collision
 *  test. This method should be called after performing a collision, when the
 *  callback result is no longer needed, or when finalising the app.
 */
void ObstacleLockCollisionCallback::clear()
{
	BW_GUARD;

	pItem_ = NULL;
}