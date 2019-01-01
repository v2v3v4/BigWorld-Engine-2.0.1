/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@file
 */

#ifndef WORLD_POLYGON_HPP
#define WORLD_POLYGON_HPP

#include <vector>

#include "math/vector3.hpp"
#include "math/planeeq.hpp"

typedef std::vector< Vector3 > PolygonBase;

/**
 *	This class is used to represent a polygon.
 */
class WorldPolygon : public std::vector< Vector3 >
{
public:
	WorldPolygon() : PolygonBase() {}
	WorldPolygon( size_t size ) : PolygonBase( size ) {}

	void split( const PlaneEq & planeEq,
		WorldPolygon & frontPoly,
		WorldPolygon & backPoly ) const;

	bool chop( const PlaneEq & planeEq );

private:
};

typedef std::vector< WorldPolygon > WPolygonSet;

#ifdef CODE_INLINE
#include "worldpoly.ipp"
#endif

#endif // WORLD_POLYGON_HPP
