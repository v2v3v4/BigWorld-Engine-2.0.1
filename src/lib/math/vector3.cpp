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
#include "cstdmf/debug.hpp"
#include "cstdmf/binary_stream.hpp"
#include <sstream>

DECLARE_DEBUG_COMPONENT2( "Math", 0 )

#include "vector3.hpp"

#ifndef CODE_INLINE
    #include "vector3.ipp"
#endif

Vector3 Vector3::s_zero( 0.f, 0.f, 0.f );

/**
 *	This function returns a description of the vector
 *
 */
std::string Vector3::desc() const
{
	char buf[128];
	bw_snprintf( buf, sizeof(buf), "(%g, %g, %g)", x, y, z );

	return buf;
}


/**
 *	This function implements the output streaming operator for Vector3.
 *
 *	@relates Vector3
 */
std::ostream& operator <<( std::ostream& o, const Vector3& t )
{
	char buf[128];
	bw_snprintf( buf, sizeof(buf), "(%1.1f, %1.1f, %1.1f)", t.x, t.y, t.z );
	o << buf;

    return o;
}


/**
 *	This function implements the input streaming operator for Vector3.
 *
 *	@relates Vector3
 */
std::istream& operator >>( std::istream& i, Vector3& t )
{
	char dummy;
    i >> dummy >> t.x >> dummy >> t.y >> dummy >> t.z >>  dummy;

    return i;
}


BinaryIStream& operator>>( BinaryIStream &is, Vector3 &v )
{
	return is >> v.x >> v.y >> v.z;
}


BinaryOStream& operator<<( BinaryOStream &os, const Vector3 &v )
{
	return os << v.x << v.y << v.z;
}

// vector3.cpp
