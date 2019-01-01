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
#include "polygon.hpp"
#include "vector2.hpp"


namespace Math
{

///////////////////////////////////////////////////////////////////////////////
// Section: Polygon
///////////////////////////////////////////////////////////////////////////////

/**
 *  This method is a 2D (Vector2) specialisation of the 'intersects' template
 *  method. It performs intersection using the separation of axes theorem. This
 *  implementation doesn't assume any particular ordering of the polygon's
 *  points.
 *
 *  @param other	The other Polygon object to test for intersection.
 *  @return			True if the polygons intersect, false otherwise.
 */
bool Polygon<Vector2>::intersects( const Polygon<Vector2>& other ) const 
{
	std::vector< Polygon<Vector2> > polys;
	polys.push_back( *this );
	polys.push_back( other );
	for( std::vector< Polygon<Vector2> >::iterator it = polys.begin();
		it != polys.end(); ++it )
	{
		Polygon<Vector2> poly = *it;
		for (int i0 = 0, i1 = (int)poly.size()-1; i0 < (int)poly.size(); i1 = i0, i0++)
		{
			Vector2 edge = ( poly.point(i0) - poly.point(i1) );
			Vector2 edgeNormal = Vector2( edge[1], -edge[0] );

			std::pair<float,float> thisMinMax = this->calcMinMax( edgeNormal );
			std::pair<float,float> otherMinMax = other.calcMinMax( edgeNormal );
			if ( thisMinMax.first > otherMinMax.second || otherMinMax.first > thisMinMax.second )
				return false;
		}
	}

	// No separation found, so the 2D polys intersect.
	return true;
}

} // namespace Math
