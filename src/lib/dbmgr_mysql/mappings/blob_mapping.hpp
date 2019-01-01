/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_BLOB_MAPPING_HPP
#define MYSQL_BLOB_MAPPING_HPP

#include "string_like_mapping.hpp"

/**
 *	This class maps the BLOB type to the database.
 */
class BlobMapping : public StringLikeMapping
{
public:
	BlobMapping( const Namer & namer, const std::string & propName,
			bool isNameIndex, int length, DataSectionPtr pDefaultValue );

	virtual bool isBinary() const	{ return true; }

	// This method gets the section data as a base64 encoded string
	// and decodes it, placing the result in output.
	static void decodeSection( std::string & output, DataSectionPtr pSection );
};

#endif // MYSQL_BLOB_MAPPING_HPP
