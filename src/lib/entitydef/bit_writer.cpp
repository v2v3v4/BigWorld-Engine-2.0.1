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

#include "bit_writer.hpp"
#include <string.h>

#include <string.h>

// -----------------------------------------------------------------------------
// Section: BitWriter
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
BitWriter::BitWriter() : byteCount_( 0 ), bitsLeft_( 8 )
{
	memset( bytes_, 0, sizeof(bytes_) );
}


/**
 *	This method writes out a number of bits.
 *
 *	@param numBits The number of bits to add.
 *	@param bits    The bits to add stored in the low bits
 *
 */
void BitWriter::add( int numBits, int bits )
{
	uint32 dataHigh = bits << (32-numBits);

	int bitAt = 0;

	while (bitAt < numBits)
	{
		bytes_[byteCount_] |= (dataHigh>>(32-bitsLeft_));
		dataHigh <<= bitsLeft_;

		bitAt += bitsLeft_;

		if (bitAt <= numBits)
		{
			bitsLeft_ = 8;
			byteCount_++;
		}
		else
		{
			bitsLeft_ = bitAt-numBits;
		}
	}
}

// bit_writer.cpp
