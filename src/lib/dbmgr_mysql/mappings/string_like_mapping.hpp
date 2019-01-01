/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_STRING_LIKE_MAPPING_HPP
#define MYSQL_STRING_LIKE_MAPPING_HPP

#include "property_mapping.hpp"
#include "../wrapper.hpp" // For enum_field_types

class Namer;

class StringLikeMapping : public PropertyMapping
{
public:
	StringLikeMapping( const Namer & namer, const std::string & propName,
				bool isNameIndex, uint charLength, uint byteLength = 0 );

	// Overrides from PropertyMapping
	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const;

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const;

	virtual void defaultToStream( BinaryOStream & strm ) const;

	virtual bool visitParentColumns( ColumnVisitor & visitor );

	// New virtual method
	virtual bool isBinary() const = 0;
	virtual enum_field_types getColumnType() const;

protected:
	std::string colName_;
	uint		charLength_;
	uint		byteLength_;
	bool		isNameIndex_;
	std::string	defaultValue_;
};

#endif // MYSQL_STRING_LIKE_MAPPING_HPP
