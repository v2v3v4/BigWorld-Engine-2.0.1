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
#include "vector4.hpp"

#include <iostream>

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "Math", 0 )

#ifndef CODE_INLINE
    #include "vector4.ipp"
#endif

#include <iomanip>


Vector4 Vector4::s_zero( 0.f, 0.f, 0.f, 0.f );


/**
 *	This function implements the output streaming operator for Vector4.
 *
 *	@relates Vector4
 */
std::ostream& operator <<( std::ostream& o, const Vector4& t )
{
	const int fieldWidth = 8;

	o.put('(');
	o.width( fieldWidth );
	o << t[0];
	o.put(',');
	o.width( fieldWidth );
	o << t[1];
	o.put(',');
	o.width( fieldWidth );
	o << t[2];
	o.put(',');
	o.width( fieldWidth );
	o << t[3];
	o.put(')');

    return o;
}


/**
 *	This function implements the input streaming operator for Vector2.
 *
 *	@relates Vector4
 */
std::istream& operator >>( std::istream& i, Vector4& t )
{
	char dummy;
    i >>	dummy >> t[0] >>
			dummy >> t[1] >>
			dummy >> t[2] >>
			dummy >> t[3] >> dummy;

    return i;
}

// vector4.cpp
