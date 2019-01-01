/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "unique_id_mapping.hpp"

#include "../namer.hpp"
#include "../query_runner.hpp"
#include "../result_set.hpp"

#include "cstdmf/binary_stream.hpp"
#include "resmgr/datasection.hpp"


/**
 *	Constructor.
 */
UniqueIDMapping::UniqueIDMapping( const Namer & namer,
		const std::string & propName,
		DataSectionPtr pDefaultValue ) :
	PropertyMapping( propName ),
	colName_( namer.buildColumnName( "sm", propName ) )
{
	if (pDefaultValue)
	{
		defaultValue_ = UniqueID( pDefaultValue->asString() );
	}
}


/*
 *	Override from PropertyMapping.
 */
void UniqueIDMapping::fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
{
	UniqueID uniqueId;
	strm >> uniqueId;
	if (strm.error())
	{
		ERROR_MSG( "UniqueIDMapping::fromStreamToDatabase: "
					"Failed destreaming property '%s'.\n",
				this->propName().c_str() );
		return;
	}

	std::string uniqueIdStr( (const char *)&uniqueId, sizeof( uniqueId ) );
	queryRunner.pushArg( uniqueIdStr );
}


/*
 *	Override from PropertyMapping.
 */
void UniqueIDMapping::fromDatabaseToStream( ResultToStreamHelper & helper,
			ResultStream & results,
			BinaryOStream & strm ) const
{
	std::string data;
	results >> data;

	if (data.size() != sizeof( UniqueID ))
	{
		results.setError();
	}
	else
	{
		UniqueID uniqueId;
		memcpy( &uniqueId, data.data(), sizeof( UniqueID ) );
	}
}


/*
 *	Override from PropertyMapping.
 */
void UniqueIDMapping::defaultToStream( BinaryOStream & strm ) const
{
	strm << defaultValue_;
}


/*
 *	Override from PropertyMapping.
 */
bool UniqueIDMapping::visitParentColumns( ColumnVisitor & visitor )
{
	ColumnType type( MYSQL_TYPE_STRING,
			true,
			sizeof( UniqueID ),
			// TODO: Make stingify method
			std::string( reinterpret_cast< const char * >( &defaultValue_ ),
					sizeof( defaultValue_ ) ) );
	ColumnDescription description( colName_, type );
	return visitor.onVisitColumn( description );
}

// unique_id_mapping.cpp
