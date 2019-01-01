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

#include "zip_stream.hpp"

#include "zip/zlib.h"

namespace
{

/**
 *	This helper class is used to manage the static buffer used by ZipIStream and
 *	ZipOStream.
 */
class StaticBuffer
{
public:
	StaticBuffer() :
		pBuf_( NULL ),
		size_( 0 ),
		isCheckedOut_( false )
	{
	}

	~StaticBuffer()
	{
		delete pBuf_;
		size_ = 0;
	}

	unsigned char * getBuffer( int size )
	{
		// Currently only support a single buffer.
		MF_ASSERT( !isCheckedOut_ );

		if (size > size_)
		{
			delete[] pBuf_;
			pBuf_ = new unsigned char[ size ];
			size_ = size;
		}

		isCheckedOut_ = true;

		return pBuf_;
	}

	void giveBuffer( unsigned char * pBuffer )
	{
		MF_ASSERT( pBuffer == pBuf_ );
		MF_ASSERT( isCheckedOut_ );
		isCheckedOut_ = false;
	}

private:
	unsigned char * pBuf_;
	int size_;
	bool isCheckedOut_;
};

StaticBuffer g_staticBuffer;

} // anonymous namespace


// -----------------------------------------------------------------------------
// Section: ZipIStream
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param stream	The stream to read the compressed data from. This should
 *		have been written with a ZipOStream.
 */
ZipIStream::ZipIStream( BinaryIStream & stream ) :
	MemoryIStream(),
	pBuffer_( NULL )
{
	this->init( stream );
}

/**
 *	Constructor.
 */
ZipIStream::ZipIStream() :
	MemoryIStream(),
	pBuffer_( NULL )
{
}


/**
 *	This method initialises the zip stream.
 */
void ZipIStream::init( BinaryIStream & stream )
{
	uLongf origLen = stream.readStringLength();
	int compressLen = stream.readStringLength();

	pBuffer_ = g_staticBuffer.getBuffer( origLen );
	int result = uncompress( pBuffer_, &origLen,
		static_cast< const Bytef * >( stream.retrieve( compressLen ) ),
		compressLen );

	if (origLen > 100)
	{
		DEBUG_MSG( "ZipIStream::ZipIStream: uncompress from %d to %d\n",
			compressLen, int( origLen ) );
	}

	MF_ASSERT( result == Z_OK );

	this->MemoryIStream::init( reinterpret_cast< char * >( pBuffer_ ),
			origLen );
}


/**
 *	Destructor.
 */
ZipIStream::~ZipIStream()
{
	if (pBuffer_)
	{
		g_staticBuffer.giveBuffer( pBuffer_ );
	}
}


// -----------------------------------------------------------------------------
// Section: ZipOStream
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param dstStream	The stream to write the compressed data to. Note that
 *			this class keeps a reference to this stream and only writes to it
 *			in the destructor.
 *	@param compressLevel The level of compression to use.
 */
ZipOStream::ZipOStream( BinaryOStream & dstStream,
		int compressLevel ) :
	outStream_(),
	pDstStream_( NULL ),
	compressLevel_( 0 )
{
	this->init( dstStream, compressLevel );
}


/**
 *	Constructor.
 */
ZipOStream::ZipOStream() :
	outStream_(),
	pDstStream_( NULL ),
	compressLevel_( 0 )
{
}


/**
 *	This method initialises the ZipOStream.
 */
void ZipOStream::init( BinaryOStream & dstStream, int compressLevel )
{
	pDstStream_ = &dstStream;
	compressLevel_ = compressLevel;
}


/**
 *	Destructor
 */
ZipOStream::~ZipOStream()
{
	if (pDstStream_ == NULL)
	{
		// Not initialised.
		return;
	}

	int srcLen = outStream_.size();
	const void * pSrc = outStream_.retrieve( srcLen );

	uLongf dstLen = compressBound( srcLen );
	unsigned char * pDst = g_staticBuffer.getBuffer( dstLen );

	int result = compress2( pDst, &dstLen,
			static_cast< const Bytef * >( pSrc ), srcLen,
			compressLevel_ );

	MF_ASSERT( result == Z_OK );

	if (dstLen > 100)
	{
		DEBUG_MSG( "ZipOStream::~ZipOStream: compress from %d to %d\n",
				srcLen, int( dstLen ) );
	}

	pDstStream_->writeStringLength( srcLen );
	pDstStream_->writeStringLength( dstLen );
	pDstStream_->addBlob( pDst, dstLen );

	g_staticBuffer.giveBuffer( pDst );
}

// zip_stream.cpp
