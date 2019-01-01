/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BLOB_OR_NULL_HPP
#define BLOB_OR_NULL_HPP

#include "cstdmf/stdmf.hpp"

class BinaryIStream;
class BinaryOStream;

/**
 *	This class is used to represent a Binary Large OBject.
 */
class BlobOrNull
{
public:
	BlobOrNull() : pData_( NULL ), length_( 0 ) {}	// NULL blob
	BlobOrNull( const char * pData, uint32 len ) :
		pData_( pData ), length_( len ) {}

	bool isNull() const 	{ return (pData_ == NULL); }

	const char * pData() const	{ return pData_; }
	uint32 length() const		{ return length_; }

private:
	const char *	pData_;
	uint32			length_;
};

BinaryOStream & operator<<( BinaryOStream & stream, const BlobOrNull & blob );
BinaryIStream & operator>>( BinaryIStream & stream, BlobOrNull & blob );

#endif // BLOB_OR_NULL_HPP
