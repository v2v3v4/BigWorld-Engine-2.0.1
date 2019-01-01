/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COMPRESSION_STREAM_HPP
#define COMPRESSION_STREAM_HPP

#include "compression_type.hpp"
#include "zip_stream.hpp"

/**
 *	This class is used to encapsulate streaming from a potentially compressed
 *	stream.
 */
class CompressionIStream
{
public:
	CompressionIStream( BinaryIStream & stream );

	operator BinaryIStream &()	{ return *pCurrStream_; }

private:
	BinaryIStream * pCurrStream_;
	ZipIStream zipStream_;
};


/**
 *	This class is used to encapsulate streaming to a potentially compressed
 *	stream.
 */
class CompressionOStream
{
public:
	CompressionOStream( BinaryOStream & stream,
		BWCompressionType compressType = BW_COMPRESSION_DEFAULT_INTERNAL );

	operator BinaryOStream &()	{ return *pCurrStream_; }

	static bool initDefaults( DataSectionPtr pSection );

private:

	BinaryOStream * pCurrStream_;
	ZipOStream zipStream_;

	static BWCompressionType s_defaultInternalCompression;
	static BWCompressionType s_defaultExternalCompression;
};

#endif // COMPRESSION_STREAM_HPP
