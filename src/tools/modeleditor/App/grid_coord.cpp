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

#include "grid_coord.hpp"

DECLARE_DEBUG_COMPONENT2( "ModelEditor", 0 )


GridCoord::GridCoord( int xc, int yc ) : x( xc ), y( yc )
{
}

GridCoord::GridCoord( Vector2 v ) : x( (int) floorf( v.x )), y( (int) floorf( v.y ) )
{
}

GridCoord GridCoord::operator+ (const GridCoord& rhs )
{
	return GridCoord( x + rhs.x, y + rhs.y );
}

GridCoord GridCoord::zero()
{
	return GridCoord( 0, 0 );
}



GridRect::GridRect( GridCoord bl, GridCoord tr ) : bottomLeft( bl ), topRight( tr )
{
}

bool GridRect::valid()
{
	return bottomLeft.x < topRight.x && bottomLeft.y < topRight.y;
}

GridRect GridRect::operator+ (const GridCoord& rhs )
{
	return GridRect( bottomLeft + rhs, topRight + rhs );
}

GridRect GridRect::zero()
{
	return GridRect( GridCoord::zero(), GridCoord::zero() );
}

GridRect GridRect::fromCoords( GridCoord a, GridCoord b)
{
	return GridRect(
		GridCoord( min( a.x, b.x ), min( a.y, b.y )),
		GridCoord( max( a.x, b.x ), max( a.y, b.y ))
		);
}