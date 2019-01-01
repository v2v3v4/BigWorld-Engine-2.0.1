/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BIT_WRITER_HPP
#define BIT_WRITER_HPP

#include "cstdmf/stdmf.hpp"

// -----------------------------------------------------------------------------
// Section: BitWriter
// -----------------------------------------------------------------------------

/**
 *	This class is used to manage writing to a stream of bits.
 */
class BitWriter
{
public:
	BitWriter();

	void add( int numBits, int bits );

	int		usedBytes() const 		{ return byteCount_ + (bitsLeft_ != 8); }
	const void * bytes() const		{ return bytes_; }

private:
	int		byteCount_;
	int		bitsLeft_;

	// TODO: Remove magic number. Maybe take a BinaryOStream instead.
	uint8	bytes_[224];
};

#endif // BIT_WRITER_HPP
