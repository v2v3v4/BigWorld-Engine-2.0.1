/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_NUM_MAPPING_HPP
#define MYSQL_NUM_MAPPING_HPP

#include "property_mapping.hpp"

#include "../namer.hpp"
#include "../query_runner.hpp"
#include "../string_conv.hpp"
#include "../table.hpp"

#include "cstdmf/binary_stream.hpp"
#include "resmgr/datasection.hpp"

template <class STRM_NUM_TYPE>
class NumMapping : public PropertyMapping
{
public:
	NumMapping( const std::string & propName,
			DataSectionPtr pDefaultValue ) :
		PropertyMapping( propName ),
		colName_( propName ),
		defaultValue_( 0 )
	{
		if (pDefaultValue)
		{
			defaultValue_ = pDefaultValue->as<STRM_NUM_TYPE>();
		}
	}

	NumMapping( const Namer & namer, const std::string & propName,
			DataSectionPtr pDefaultValue ) :
		PropertyMapping( propName ),
		colName_( namer.buildColumnName( "sm", propName ) ),
		defaultValue_( 0 )
	{
		if (pDefaultValue)
		{
			defaultValue_ = pDefaultValue->as<STRM_NUM_TYPE>();
		}
	}

	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
	{
		STRM_NUM_TYPE i;
		strm >> i;
		queryRunner.pushArg( i );
	}

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const
	{
		STRM_NUM_TYPE i;
		results >> i;
		strm << i;
	}

	virtual void defaultToStream( BinaryOStream & strm ) const
	{
		strm << defaultValue_;
	}

	virtual bool visitParentColumns( ColumnVisitor & visitor )
	{
		ColumnType type(
				MySqlTypeTraits<STRM_NUM_TYPE>::colType,
				!std::numeric_limits<STRM_NUM_TYPE>::is_signed,
				0,
				StringConv::toStr( defaultValue_ ) );

		ColumnDescription columnDescription( colName_, type );

		return visitor.onVisitColumn( columnDescription );
	}

private:
	std::string colName_;
	STRM_NUM_TYPE defaultValue_;
};

#endif // MYSQL_NUM_MAPPING_HPP
