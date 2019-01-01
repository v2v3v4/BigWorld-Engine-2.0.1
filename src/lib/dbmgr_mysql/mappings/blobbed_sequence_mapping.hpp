/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_BLOBBED_SEQUENCE_MAPPING_HPP
#define MYSQL_BLOBBED_SEQUENCE_MAPPING_HPP

#include "string_like_mapping.hpp"

/**
 * 	This class maps sequences (like ARRAY and TUPLE) to a BLOB column in the
 * 	database.
 */
class BlobbedSequenceMapping : public StringLikeMapping
{
public:
	BlobbedSequenceMapping( const Namer & namer, const std::string & propName,
				PropertyMappingPtr child, int size, int dbLen );

	virtual bool isBinary() const	{ return true; }
};

#endif // MYSQL_BLOBBED_SEQUENCE_MAPPING_HPP
