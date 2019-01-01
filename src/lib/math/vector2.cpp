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
#include "vector2.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "Math", 0 )

#ifndef CODE_INLINE
    #include "vector2.ipp"
#endif

#include <iomanip>

Vector2 Vector2::s_zero( 0.f, 0.f );

/**
 *	This function returns a description of the vector
 *
 */
std::string Vector2::desc() const
{
	char buf[128];
	bw_snprintf( buf, sizeof(buf), "(%g, %g)", x, y );

	return buf;
}

/**
 *	This function implements the output streaming operator for Vector2.
 *
 *	@relates Vector2
 */
std::ostream& operator <<( std::ostream& o, const Vector2& t )
{
	const int fieldWidth = 8;

	o.put('(');
	o.width( fieldWidth );
	o << t[0];
	o.put(',');
	o.width( fieldWidth );
	o << t[1];
	o.put(')');

    return o;
}

/**
 *	This function implements the input streaming operator for Vector2.
 *
 *	@relates Vector2
 */
std::istream& operator >>( std::istream& i, Vector2& t )
{
	char dummy;
    i >> dummy >> t[0] >> dummy >> t[1] >> dummy;

    return i;
}

// vector2.cpp
