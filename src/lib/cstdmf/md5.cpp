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

#include "md5.hpp"
#include <string.h>
#include "binary_stream.hpp"

// -----------------------------------------------------------------------------
// Section: MD5 C++ wrapper
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MD5::MD5()
{
	MD5_Init( &state_ );
}

/**
 *	This method appends data to the MD5.
 */
void MD5::append( const void * data, int numBytes )
{
	MD5_Update( &state_, (const unsigned char*)data, numBytes );
}


/**
 *	This method gets the digest associated with this MD5.
 */
void MD5::getDigest( MD5::Digest & digest )
{
	MD5_Final( digest.bytes, &state_ );
}


// -----------------------------------------------------------------------------
// Section: MD5::Digest
// -----------------------------------------------------------------------------

/**
 *	This method clears this digest to all 0s
 */
void MD5::Digest::clear()
{
	memset( this, 0, sizeof(*this) );
}


/**
 *	This method returns whether or not two Digests are equal.
 *
 *	@return True if equal, otherwise false.
 */
bool MD5::Digest::operator==( const Digest & other ) const
{
	return memcmp( this->bytes, other.bytes, sizeof( Digest ) ) == 0;
}

/**
 *	This method returns the sort ordering of two digests.
 *
 *	@return True if this less than other, otherwise false.
 */
bool MD5::Digest::operator<( const Digest & other ) const
{
	return memcmp( this->bytes, other.bytes, sizeof( Digest ) ) < 0;
}


/**
 *	Quote the given digest to something that can live in XML.
 */
std::string MD5::Digest::quote() const
{
	const char hexTbl[17] = "0123456789ABCDEF";

	char buf[32];
	for (uint i = 0; i < 16; i++)
	{
		buf[(i<<1)|0] = hexTbl[ bytes[i]>>4 ];
		buf[(i<<1)|1] = hexTbl[ bytes[i]&0x0F ];
	}

	return std::string( buf, 32 );
}

// helper function to unquote a nibble
static inline unsigned char unquoteNibble( char c )
{
	return c > '9' ? c - ('A'-10) : c - '0';
}

/**
 *	Reverse the operation of the quote method.
 *	Returns true if successful.
 */
bool MD5::Digest::unquote( const std::string & quotedDigest )
{
	if (quotedDigest.length() == 32)
	{
		const char * buf = quotedDigest.c_str();
		for (uint i = 0; i < 16; i++)
		{
			bytes[i] = (unquoteNibble( buf[(i<<1)|0] ) << 4) |
				unquoteNibble( buf[(i<<1)|1] );
		}
		return true;
	}
	else
	{
		this->clear();
		return false;
	}
}

BinaryIStream& operator>>( BinaryIStream &is, MD5::Digest &d )
{
	memcpy( d.bytes, is.retrieve( sizeof( d.bytes ) ), sizeof( d.bytes ) );
	return is;
}

BinaryOStream& operator<<( BinaryOStream &os, const MD5::Digest &d )
{
	os.addBlob( d.bytes, sizeof( d.bytes ) );
	return os;
}

// md5.cpp
