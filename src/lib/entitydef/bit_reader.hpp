/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BIT_READER_HPP
#define BIT_READER_HPP

#include "cstdmf/stdmf.hpp"

class BinaryIStream;

/**
 *	This class is used to read from a stream of bits.
 */
class BitReader
{
public:
	BitReader( BinaryIStream & data );
	~BitReader();

	int get( int nbits );

private:
	BinaryIStream & data_;
	int	bitsLeft_;
	uint8 byte_;
};

#endif // BIT_READER_HPP
