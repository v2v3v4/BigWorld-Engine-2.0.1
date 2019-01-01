/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_TIMESTAMP_MAPPING_HPP
#define MYSQL_TIMESTAMP_MAPPING_HPP

#include "property_mapping.hpp"

#include "../column_type.hpp"
#include "../constants.hpp"
#include "../wrapper.hpp"


class TimestampMapping : public PropertyMapping
{
public:
	TimestampMapping();

	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
	{
		// Timestamp should not be provided for the query.
		// The ColumnDescription in visitParentColumns is created with
		// shouldIgnore = true, which means we should be skipped by the visitor
	}

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const
	{
		// The timestamp should never be read back off by BigWorld.
	}

	virtual void defaultToStream( BinaryOStream & strm ) const {}

	virtual bool visitParentColumns( ColumnVisitor & visitor );
};

#endif // MYSQL_TIMESTAMP_MAPPING_HPP
