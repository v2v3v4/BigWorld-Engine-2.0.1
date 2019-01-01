/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __vertex_hpp__
#define __vertex_hpp__

#include "bonevertex.hpp"

struct Point2
{
	Point2( float u = 0, float v = 0 );
		
	float u, v;

	inline bool operator==( const Point2& p ) const { return u == p.u && v == p.v; }
	inline bool operator!=( const Point2& p ) const { return !( *this == p ); }
};

#endif // __vertex_hpp__