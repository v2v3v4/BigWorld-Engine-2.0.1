/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _INCLUDE_MATH_VECTOR2_HPP
#define _INCLUDE_MATH_VECTOR2_HPP

//---------------------------------------------------------
// vector2 implements a 2D vector, with templated coordinates,
// allowing for int32, real32, etc.
//---------------------------------------------------------
template<class T>
class vector2
{
public:
	inline vector2( const T& aX = 0, const T& aY = 0 );
	
	// NOT virtual, so don't subclass Float
	~vector2();

	T x;
	T y;

	// basis vectors
	static inline vector2<T> X();
	static inline vector2<T> Y();

	inline vector2<T>& normalise();
	inline T magnitude() const;
	inline T magnitudeSquared() const;
	inline T dot( const vector2<T>& aVector2 ) const;

	inline bool operator==( const vector2<T>& aVector2 ) const;
	inline bool operator!=( const vector2<T>& aVector2 ) const;

	inline vector2<T> operator-();
	
	inline vector2<T> operator*( const T& aT ) const;
	inline vector2<T> operator/( const T& aT ) const;
	inline vector2<T> operator+( const vector2<T>& aVector2 ) const;
	inline vector2<T> operator-( const vector2<T>& aVector2 ) const;

	inline vector2<T>& operator*=( const T& aT );
	inline vector2<T>& operator/=( const T& aT );
	inline vector2<T>& operator+=( const vector2<T>& aVector2 );
	inline vector2<T>& operator-=( const vector2<T>& aVector2 );
	
	inline T& operator[]( uint32 aIndex );

};

template<class T>
inline vector2<T> operator*( const T& aT, const vector2<T>& aVector );

#include "vector2.ipp"

#endif