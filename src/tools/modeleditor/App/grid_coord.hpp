/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GRID_COORD_HPP
#define GRID_COORD_HPP

#include "math/Vector2.hpp"

/** Basically an int vector */
class GridCoord
{
public:
	GridCoord( int xc, int yc );
	GridCoord( Vector2 v );

	int x;
	int y;

	GridCoord operator+ (const GridCoord& rhs );

	static GridCoord zero();
};

class GridRect
{
public:
	GridRect( GridCoord bl, GridCoord tr );

	GridCoord bottomLeft;
	GridCoord topRight;

	bool valid();

	GridRect operator+ (const GridCoord& rhs );

	static GridRect zero();
	/** Create a GridRect from any two points */
	static GridRect fromCoords( GridCoord a, GridCoord b);
};

#endif // GRID_COORD_HPP