/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ZIP_STREAM_HPP
#define ZIP_STREAM_HPP

#include "cstdmf/memory_stream.hpp"

/**
 *	This class is used to read from a stream that has compressed data on it.
 */
class ZipIStream : public MemoryIStream
{
public:
	ZipIStream( BinaryIStream & stream );
	ZipIStream();

	void init( BinaryIStream & stream );
	~ZipIStream();

private:
	unsigned char * pBuffer_;
};


/**
 *	This class is used compressed data to write to a stream.
 */
class ZipOStream : public BinaryOStream
{
public:
	ZipOStream( BinaryOStream & dstStream,
			int compressLevel = 1 /* Z_BEST_SPEED */ );
	ZipOStream();

	void init( BinaryOStream & dstStream,
			int compressLevel = 1 /* Z_BEST_SPEED */ );

	~ZipOStream();

	virtual void * reserve( int nBytes )
	{
		return outStream_.reserve( nBytes );
	}

	virtual int size() const
	{ 
		return outStream_.size();
	} 

private:
	MemoryOStream outStream_;
	BinaryOStream * pDstStream_;
	int compressLevel_;
};

#endif // ZIP_STREAM_HPP
