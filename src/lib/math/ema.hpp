/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __EMA_HPP__
#define __EMA_HPP__

/**
 *	This class represents an exponentially weighted moving average.
 *
 *	The nth sample (given via the sample() method) updates the average
 *	according to:
 *		avg(n) = (1 - bias) * sample( n ) + bias * avg( n - 1 )
 *
 *	This class is entirely inlined.
 */
class EMA
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param bias 	Determines the exponential bias.
	 *	@param initial 	The initial value of the average.
	 */
	explicit EMA( float bias, float initial=0.f ):
		bias_( bias ),
		average_( initial )
	{}

	/**
	 *	Sample a value into the average.
	 *
	 *	@param value 	The value to be sampled.
	 */
	void sample( float value )
	{
		average_ = (1.f - bias_) * value + bias_ * average_;
	}

	/**
	 *	Return the current value of the average.
	 */
	float average() const 	{ return average_; }

	static float calculateBiasFromNumSamples( float numSamples,
			float weighting = 0.95f );

private:
	/// The bias.
	float bias_;

	/// The current value of the average.
	float average_;

};

/**
 *	This templated class is a helper class for accumulating values, and
 *	periodically sampling that value into an EMA.
 */
template< typename TYPE >
class AccumulatingEMA
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param bias 			Determines the exponential bias.
	 *	@param initialAverage	The initial value of the average.
	 *	@param initialValue 	The initial value of the accumulated value.
	 */
	AccumulatingEMA( float bias, float initialAverage=0.f,
			TYPE initialValue=TYPE() ):
		average_( bias, initialAverage ),
		accum_( initialValue )
	{
	}


	/**
	 *	Accumulated value accessor.
	 */
	TYPE & value() 				{ return accum_; }


	/**
	 *	Accumulated value const accessor.
	 */
	const TYPE & value() const 	{ return accum_; }


	/**
	 *	Average accessor.
	 */
	float average()	const		{ return average_.average(); }


	/**
	 *	Sample method. This should be called periodically to sample the
	 *	accumulated value into the weighted average. The accumulated value
	 *	is converted to a float before sampling.
	 *
	 *	@param shouldReset 		If true, the accumulated value is reset to the
	 *							TYPE default (e.g. 0 for numeric TYPEs) after
	 *							sampling. If false, it is unchanged.
	 */
	void sample( bool shouldReset=true )
	{
		average_.sample( float( accum_ ) );
		if (shouldReset)
		{
			accum_ = TYPE();
		}
	}

private:
	/// The exponentially weighted average.
	EMA 	average_;
	/// The accumulated value.
	TYPE 	accum_;
};
#endif // __EMA_HPP__
