/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HEIGHT_MAP_COMPRESS_HPP
#define HEIGHT_MAP_COMPRESS_HPP

#include "moo/image.hpp"

namespace Terrain
{
	/*
	 *	These methods compress and decompress an image of floats to and from a 
	 *	BinaryBlock.  The heights can be quantised, and so there is a little 
	 *	loss of	precision.  Repeated compressions and decompressions however is 
	 *	stable.  The decompression method should be able to handle older 
	 *	formats but the	compression method always compresses using the best 
	 *	known compression method.  Currently the compression is done by 
	 *	quantising the heights to 1mm intervals and compressing the resultant 
	 *	integerised height map using PNG.  Typically this gives a compression 
	 *	ratio of 8:1.
	 */
	 
	BinaryPtr compressHeightMap(Moo::Image<float> const &heightMap);
	bool decompressHeightMap(BinaryPtr data, Moo::Image<float> &heightMap);
}

#endif // HEIGHT_MAP_COMPRESS_HPP
