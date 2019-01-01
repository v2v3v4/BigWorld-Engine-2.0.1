/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SIMPLEX_NOISE_HPP
#define SIMPLEX_NOISE_HPP


#include <vector>


/**
 *	SimplexNoise is a class that generates simplex noise based upon Perlin's
 *	improvement to the Perlin noise algorithm.  The implementation is based 
 *	upon Stefan Gustavson's implementation as in:
 *
 *		http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
 *
 *	There are two ways of using this noise.  Most simply you can call
 *	
 *		SimplexNoise::generate(x, y, seed),
 *
 *	which generates noise in the range [0, 1] on a 1-unit scale.
 *
 *	The second method is to use the SimplexNoise class.  This lets you set up
 *	a collection of octaves of noise, including their frequencies, relative
 *	weights, the output range and a linear lookup which can be used to 
 *	accentuate, de-emphasize etc noise values.
 */
class SimplexNoise
{
public:
    struct Octave
    {
        float       waveLength_;    // The wavelength of the noise
        float       weight_;        // The weight contribution of this octave
        int         seed_;          // The seed value for the noise, repeats after 256 values

        Octave();
        Octave( float wavelength, float weight, int seed = 0 );
        
        float       frequency_;     // Set internally.
    };

    typedef std::vector< Octave > OctaveVec;

    static float generate( float x, float y, int seed );

    SimplexNoise();

    float operator()( float x, float y ) const;

	bool operator==( const SimplexNoise & other ) const;
	bool operator!=( const SimplexNoise & other ) const;

    void octaves( OctaveVec const &oct );
    OctaveVec const &octaves() const;

private:
    std::vector<Octave>             octaves_;
	float							totalWeight_;
};


#endif // SIMPLEX_NOISE_HPP
