/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <iostream>

#include "xp_math.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/debug.hpp"
#include "math/mathdef.hpp"
#include <math.h>


/**
 *	This class implements a vector of three floats.
 *
 *	@ingroup Math
 */
class Vector3 : public Vector3Base
{

public:
	Vector3();
	Vector3( float a, float b, float c );
	explicit Vector3( const Vector3Base & v );
#ifdef _WIN32
	Vector3( __m128 v4 );
#endif

	// Use the default compiler implementation for these methods. It is faster.
	//	Vector3( const Vector3& v);
	//	Vector3& operator  = ( const Vector3& v );

	void setZero();
	void set( float a, float b, float c );
//	void scale( const Vector3& v, float s );
	void setPitchYaw( float pitchInRadians, float yawInRadians );

	float dotProduct( const Vector3& v ) const;
	void crossProduct( const Vector3& v1, const Vector3& v2 );
	Vector3 crossProduct( const Vector3 & v ) const;
	void lerp( const Vector3 & v1, const Vector3 & v2, float alpha );
	void clamp( const Vector3 & lower, const Vector3 & upper );

	void projectOnto( const Vector3& v1, const Vector3& v2 );
	Vector3 projectOnto( const Vector3 & v ) const;

	INLINE float length() const;
	INLINE float lengthSquared() const;
	INLINE void normalise();
	INLINE Vector3 unitVector() const;

	INLINE void operator += ( const Vector3& v );
	INLINE void operator -= ( const Vector3& v );
	INLINE void operator *= ( float s );
	INLINE void operator /= ( float s );
	INLINE Vector3 operator-() const;

	float yaw() const;
	float pitch() const;
	std::string desc() const;

	static const Vector3 & zero()		{ return s_zero; }

private:
    friend std::ostream& operator <<( std::ostream&, const Vector3& );
    friend std::istream& operator >>( std::istream&, Vector3& );

	// This is to prevent construction like:
	//	Vector3( 0 );
	// It would interpret this as a float * and later crash.
	Vector3( int value );

	static Vector3 s_zero;
};

INLINE Vector3 operator +( const Vector3& v1, const Vector3& v2 );
INLINE Vector3 operator -( const Vector3& v1, const Vector3& v2 );
INLINE Vector3 operator *( const Vector3& v, float s );
INLINE Vector3 operator *( float s, const Vector3& v );
INLINE Vector3 operator *( const Vector3& v1, const Vector3& v2 );
INLINE Vector3 operator /( const Vector3& v, float s );
INLINE bool operator   ==( const Vector3& v1, const Vector3& v2 );
INLINE bool operator   !=( const Vector3& v1, const Vector3& v2 );
INLINE bool operator   < ( const Vector3& v1, const Vector3& v2 );

inline
bool operator   > ( const Vector3& v1, const Vector3& v2 ) { return v2 < v1; }
inline
bool operator   >=( const Vector3& v1, const Vector3& v2 ) { return !(v1<v2); }
inline
bool operator   <=( const Vector3& v1, const Vector3& v2 ) { return !(v2<v1); }


class BinaryIStream;
class BinaryOStream;
BinaryIStream& operator>>( BinaryIStream &is, Vector3 &v );
BinaryOStream& operator<<( BinaryOStream &is, const Vector3 &v );


inline bool almostEqual( const Vector3& v1, const Vector3& v2, const float epsilon = 0.0004f )
{
	return almostEqual( v1.x, v2.x, epsilon ) &&
		almostEqual( v1.y, v2.y, epsilon ) &&
		almostEqual( v1.z, v2.z, epsilon );
}

// Vector3 pitchYawToVector3( float pitchInRadians, float yawInRadians );

#ifdef CODE_INLINE
	#include "vector3.ipp"
#endif

#endif // VECTOR3_HPP
