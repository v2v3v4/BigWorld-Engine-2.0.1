/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blob_or_null.hpp"

#include "cstdmf/binary_stream.hpp"

/**
 *	This function is used to serialise a potentially NULL blob into a stream.
 */
BinaryOStream & operator<<( BinaryOStream & stream, const BlobOrNull & blob )
{
	if (blob.pData() && blob.length())
	{
		stream.appendString( blob.pData(), blob.length() );
	}
	else	// NULL value or just empty string
	{
		stream.appendString( "", 0 );
		stream << uint8(!blob.isNull());
	}

	return stream;
}

/**
 *	This function is used to deserialise a potentially NULL blob from a stream.
 */
BinaryIStream & operator>>( BinaryIStream & stream, BlobOrNull & blob )
{
	int length = stream.readStringLength();

	if (length > 0)
	{
		blob = BlobOrNull( (char*) stream.retrieve( length ), length );
	}
	else
	{
		uint8 isNotNull;
		stream >> isNotNull;

		blob = isNotNull ? BlobOrNull( "", 0 ) : BlobOrNull();
	}

	return stream;
}

// blob_or_null.cpp
