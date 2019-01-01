/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vector2.hpp"

/**
 *	This class is the Perlin noise.
 *	What is Perlin noise: Perlin noise is a noise function (seeded random
 *	number generator) which is just several Interpolated Noise functions at different
 *	scales/frequencies added together. 
 */
class PerlinNoise
{
public:
	//herein lies Perlin's original implementation
	/* coherent noise function over 1, 2 or 3 dimensions */
	/* (copyright Ken Perlin) */
	void init(void);
	float noise1( float arg );
	float noise2( const Vector2 & vec );
	float noise3( float vec[3] );
	void normalise2( float v[2] );
	void normalise3( float v[3] );

	//hereafter lies MF's original extensions
	float sumHarmonics2( const Vector2 & vec, float persistence,
						 float frequency, float nIterations );
private:
};


#endif
