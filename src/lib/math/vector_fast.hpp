/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VECTOR3_FAST_HPP
#define VECTOR3_FAST_HPP

#include "xp_math.hpp"
#include "cstdmf/debug.hpp"
#include <math.h>


// -----------------------------------------------------------------------------

/**
 *	This class implements an aligned vector of four floats with SIMD operations.
 *
 *	@ingroup Math
 */

class VectorFastBase : public Aligned
{

public:
							VectorFastBase();
							VectorFastBase( const VectorFastBase& v );
							VectorFastBase( __m128 v );
							VectorFastBase( float a, float b, float c, float d = 1.f );
	explicit				VectorFastBase( const Vector3& v );
	explicit				VectorFastBase( const Vector4& v );

	void					getVector3( Vector3& v ) const;
//	void					getVector4( Vector4& v ) const;

	void					castToInt( int* i ) const;

	void					setZero();
	void					saturate();

	VectorFastBase&			operator = ( const VectorFastBase& v );
	VectorFastBase&			operator = ( __m128 v );

	VectorFastBase&			operator += ( const VectorFastBase& v );
	VectorFastBase&			operator -= ( const VectorFastBase& v );
	VectorFastBase&			operator *= ( const VectorFastBase& v );
	VectorFastBase&			operator *= ( float s );
	VectorFastBase&			operator /= ( float s );

	VectorFastBase			operator - () const;

private:
	// This is to prevent construction like:
	//	VectorFastBase( 0 );
	// It would interpret this as a float * and later crash.
							VectorFastBase( int value );

public:

	union
	{
		__m128					v4;
		float					f[4];
		struct
		{
			float					x;
			float					y;
			float					z;
			float					w;
		};
	};
};


// -----------------------------------------------------------------------------

typedef VectorFastBase Vector3Fast;
typedef VectorFastBase Vector4Fast;

// -----------------------------------------------------------------------------

// Always inline
#include "vector_fast.ipp"

#endif // VECTOR3_FAST_HPP
