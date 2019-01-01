/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLLISON_ADVANCE_HPP
#define COLLISON_ADVANCE_HPP


#include "chunk/chunk_obstacle.hpp"
#include "math/planeeq.hpp"
#include "physics2/worldtri.hpp"

/**
 *	This is a helper class is used with preventCollision.
 */
class CollisionAdvance : public CollisionCallback
{
public:
	CollisionAdvance( const Vector3 & origin,
			const Vector3 & axis1, const Vector3 & axis2,
			const Vector3 & direction,
			float maxAdvance );
	~CollisionAdvance();

	float advance() const { return max( 0.f, advance_); }

	const WorldTriangle & hitTriangle() const	{ return hitTriangle_; }
	ChunkItemPtr hitItem() const				{ return hitItem_; }

	void ignoreFlags( uint8 flags )				{ ignoreFlags_ = flags; }

private:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist );

	PlaneEq planeEq0_;
	PlaneEq planeEq1_;
	PlaneEq planeEq2_;
	PlaneEq planeEq3_;
	PlaneEq planeEqBase_;
	const Vector3 dir_;
	float advance_;
	float adjust_;

	WorldTriangle	hitTriangle_;
	ChunkItemPtr	hitItem_;

	uint8 ignoreFlags_;
};



#endif // COLLISON_ADVANCE_HPP
