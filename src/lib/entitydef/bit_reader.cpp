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

#include "bit_reader.hpp"

#include "cstdmf/binary_stream.hpp"


// -----------------------------------------------------------------------------
// Section: BitReader
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
BitReader::BitReader( BinaryIStream & data ) :
	data_( data ), bitsLeft_( 0 )
{
}


/**
 *	Destructor.
 */
BitReader::~BitReader()
{
	if ((bitsLeft_ > 0) && (byte_ != 0))
	{
		ERROR_MSG( "BitReader::~BitReader: bitsLeft_ = %d. byte_ = 0x%x\n",
				bitsLeft_, byte_ );
	}
}


/**
 *	This method reads the specified number of bits from the bit stream and
 *	returns them as an integer.
 */
int BitReader::get( int nbits )
{
	int	ret = 0;

	int gbits = 0;
	while (gbits < nbits)	// not as efficient as the writer...
	{
		if (bitsLeft_ == 0)
		{
			byte_ = *(uint8*)data_.retrieve( 1 );
			bitsLeft_ = 8;
		}

		int bitsTake = std::min( nbits-gbits, bitsLeft_ );
		ret = (ret << bitsTake) | (byte_ >> (8-bitsTake));
		byte_ <<= bitsTake;
		bitsLeft_ -= bitsTake;
		gbits += bitsTake;
	}

	return ret;
}

// bit_reader.cpp
