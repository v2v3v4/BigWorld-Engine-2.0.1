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
#include "math/math_extra.hpp"
#include "cstdmf/debug.hpp"

/**
 *	This applies Gram-Schmidt orgthonalisation to the vectors v1, v2 and v3 and
 *	returns the result in e1, e2 and e3.  The vectors v1, v2, v3 should be
 *	linearly indepedant, non-zero vectors.
 *
 *  See http://en.wikipedia.org/wiki/Gram-Schmidt_process for details of 
 *	Gram-Schmidt orthogonalisation.
 *
 *  @param v1, v2, v3	The vectors to orthonormalise.
 *  @param e1, e2, e3	The orthonormalised vectors.
 */
void BW::orthogonalise(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
				 Vector3 &e1, Vector3 &e2, Vector3 &e3)
{
	Vector3 u1 = v1; 

	Vector3 p1; 
	p1.projectOnto(v2, u1);
	Vector3 u2 = v2 - p1;
	
	Vector3 p2; 
	p2.projectOnto(v3, u1);
	Vector3 p3; 
	p3.projectOnto(v3, u2);
	Vector3 u3 = v3 - p2 - p3;
	
	u1.normalise();
	u2.normalise();
	u3.normalise();

	e1 = u1; e2 = u2; e3 = u3;
}

//This is the max power of two on 32 bit machines
static unsigned long MAX_POW_2 = 2147483648UL;

uint32 BW::largerPow2( uint32 number )
{
    //Do the best we can - return a smaller pow 2 element
	if (number > MAX_POW_2)
	{
		WARNING_MSG("BW::largerPow2 returning a smaller power of 2 as %u is too large", number);
		return MAX_POW_2;
	}
	const float LOG2_10 = 3.3219280948873626f;
	float n = log10f( float( number ) ) * LOG2_10;
	uint32 shift = uint32( ceil( n ) );
	if (n-floor( n ) < 0.01f)
	{
		shift = uint32( floor( n ) );
	}
	uint32 ret = 1 << shift;
	if (ret < number)
	{
		ret <<= 1;
	}
	return ret;
}


uint32 BW::smallerPow2( uint32 number )
{
	return BW::largerPow2( number ) / 2;
}
