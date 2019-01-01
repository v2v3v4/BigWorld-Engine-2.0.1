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

#include "ema.hpp"

#include <math.h>

/**
 *	This method calculates a bias required to give the most recent samples the
 *	desired weighting.
 *
 *	For example, if numSamples is 60 and weighting is 0.95, using the resulting
 *	bias in an EMA means that the latest 60 samples account for 95% of the
 *	average and all other samples account for 5%.
 *
 *	@param numSamples The number of samples having the desired weighting.
 *	@weighting The proportion of the average that these samples contribute.
 */
float EMA::calculateBiasFromNumSamples( float numSamples,
		float weighting )
{
	return (numSamples != 0.f) ?
		exp( log( 1.f - weighting )/numSamples ) :
		0.f;
}

// ema.cpp
