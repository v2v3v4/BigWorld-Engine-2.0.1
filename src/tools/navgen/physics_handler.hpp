/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHYSICS_HANDLER_HPP
#define PHYSICS_HANDLER_HPP

#include "chunk/chunk_space.hpp"

#include "chunk/chunk_obstacle.hpp"

#include "waypoint_generator/waypoint_flood.hpp"
#include "girth.hpp"

/**
 *	Class to do physics checking in our chunk
 */
class PhysicsHandler : public IPhysics
{
public:
	PhysicsHandler( ChunkSpace * pSpace, girth gSpec );

	Vector3 getGirth() const;
	float getScrambleHeight() const;
	bool findDropPoint(const Vector3& pos, float& y);
	bool findDropSeedPoint(const Vector3& pos, float& y);

	/**
	 *	This function prevents a collision between newPos and oldPos,
	 *	adjusting newPos if necessary.
	 *
	 *	Copied from bigworld/src/client/app.cpp
	 */
	void preventCollision( const Vector3 &oldPos, Vector3 &newPos,
		float hopMax, float modelWidth, float modelHeight, float modelDepth );

	void applyGravity( Vector3 &newPos, float hopMax,
		float modelWidth, float modelDepth );

	void adjustMove(const Vector3& src, const Vector3& dst,
			Vector3& dst2);

	float scrambleHeight_;
	float modelWidth_;
	float modelHeight_;
	float modelDepth_;

	bool	debug_;

private:
	ChunkSpace * pChunkSpace_;
};

#endif