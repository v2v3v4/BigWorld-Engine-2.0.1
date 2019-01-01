/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VECTOR2_HPP
#define VECTOR2_HPP

#include <iostream>
#include "xp_math.hpp"
#include "cstdmf/debug.hpp"
#include <math.h>


/**
 *	This class implements a vector of two floats.
 *
 *	@ingroup Math
 */
class Vector2 : public Vector2Base
{

public:
	Vector2();
	Vector2( float a, float b );
	explicit Vector2( const Vector2Base & v2 );

	// Use the default compiler implementation for these methods. It is faster.
	// Vector2( const Vector2& v );
	// Vector2& operator  = ( const Vector2& v );

	void setZero( );
	void set( float a, float b ) ;
	void scale( const Vector2 &v, float s );
	float length() const;
	float lengthSquared() const;
	void normalise();
	Vector2 unitVector() const;

	float dotProduct( const Vector2& v ) const;
	float crossProduct( const Vector2& v ) const;

	void projectOnto( const Vector2& v1, const Vector2& v2 );
	Vector2 projectOnto( const Vector2 & v ) const;

	INLINE void operator += ( const Vector2& v );
	INLINE void operator -= ( const Vector2& v );
	INLINE void operator *= ( float s );
	INLINE void operator /= ( float s );

	std::string desc() const;

	static const Vector2 & zero()			{ return s_zero; }

private:
    friend std::ostream& operator <<( std::ostream&, const Vector2& );
    friend std::istream& operator >>( std::istream&, Vector2& );

	// This is to prevent construction like:
	//	Vector2( 0 );
	// It would interpret this as a float * and later crash.
	Vector2( int value );

	static Vector2	s_zero;
};

INLINE Vector2 operator +( const Vector2& v1, const Vector2& v2 );
INLINE Vector2 operator -( const Vector2& v1, const Vector2& v2 );
INLINE Vector2 operator *( const Vector2& v, float s );
INLINE Vector2 operator *( float s, const Vector2& v );
INLINE Vector2 operator *( const Vector2& v1, const Vector2& v2 );
INLINE Vector2 operator /( const Vector2& v, float s );
INLINE bool operator   ==( const Vector2& v1, const Vector2& v2 );
INLINE bool operator   !=( const Vector2& v1, const Vector2& v2 );
INLINE bool operator   < ( const Vector2& v1, const Vector2& v2 );

inline
bool operator   > ( const Vector2& v1, const Vector2& v2 ) { return v2 < v1; }
inline
bool operator   >=( const Vector2& v1, const Vector2& v2 ) { return !(v1<v2); }
inline
bool operator   <=( const Vector2& v1, const Vector2& v2 ) { return !(v2<v1); }

bool almostEqual( const Vector2& v1, const Vector2& v2, const float epsilon = 0.0004f );

#ifdef CODE_INLINE
	#include "vector2.ipp"
#endif

#endif // VECTOR2_HPP
