/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_UNIQUE_ID_MAPPING_HPP
#define MYSQL_UNIQUE_ID_MAPPING_HPP

#include "property_mapping.hpp"

#include "../column_type.hpp"

#include "cstdmf/unique_id.hpp"

/**
 *	Maps a UniqueID into MySQL. This is a base class to properties that
 * 	stores a UniqueID into the database istead of the actual object data.
 */
class UniqueIDMapping : public PropertyMapping
{
public:
	UniqueIDMapping( const Namer & namer, const std::string & propName,
			DataSectionPtr pDefaultValue );

	UniqueID getValue() const;

	// Overrides from PropertyMapping
	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const;

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const;

	virtual void defaultToStream( BinaryOStream & strm ) const;

	virtual bool visitParentColumns( ColumnVisitor & visitor );

private:
	std::string	colName_;
	UniqueID 	defaultValue_;
};

#endif // MYSQL_UNIQUE_ID_MAPPING_HPP
