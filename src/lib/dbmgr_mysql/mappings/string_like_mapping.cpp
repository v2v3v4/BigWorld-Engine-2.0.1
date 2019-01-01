/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "string_like_mapping.hpp"

#include "../column_type.hpp"
#include "../namer.hpp"
#include "../query_runner.hpp"
#include "../result_set.hpp"

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/value_or_null.hpp"

/**
 *	Constructor for string-like types where the character length and the
 *	byte length are equivalent.
 */
StringLikeMapping::StringLikeMapping( const Namer & namer,
			const std::string & propName,
			bool isNameIndex,
			uint charLength,
	   		uint byteLength ):
		PropertyMapping( propName ),
		colName_( namer.buildColumnName( "sm", propName ) ),
		charLength_( charLength ),
		byteLength_( (byteLength != 0) ? byteLength : charLength ),
		isNameIndex_( isNameIndex ),
		defaultValue_()
{
}


/*
 *	Override from PropertyMapping.
 */
void StringLikeMapping::fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
{
	std::string value;
	strm >> value;
	if (strm.error())
	{
		ERROR_MSG( "StringLikeMapping::fromStreamToDatabase: "
					"Failed destreaming property '%s'.\n",
				this->propName().c_str() );
		return;
	}

	if (value.size() > byteLength_)
	{
		WARNING_MSG( "StringLikeMapping::fromStreamToDatabase: "
					"Truncating string property '%s' from %zd to %u.\n",
				this->propName().c_str(), value.size(), byteLength_ );
	}

	queryRunner.pushArg( value );
}


/*
 *	Override from PropertyMapping.
 */
void StringLikeMapping::fromDatabaseToStream( ResultToStreamHelper & helper,
			ResultStream & results,
			BinaryOStream & strm ) const
{
	ValueOrNull< std::string > value;

	results >> value;

	if (!value.isNull())
	{
		strm << *value.get();
	}
	else
	{
		strm << defaultValue_;
	}
}


/*
 *	Override from PropertyMapping.
 */
void StringLikeMapping::defaultToStream( BinaryOStream & strm ) const
{
	strm << defaultValue_;
}


/**
 * This method retrieves the MySQL field type of the column based on the
 * length of each character being represented by the string type.
 *
 * @returns The MySQL field type of the associated string column.
 */
enum_field_types StringLikeMapping::getColumnType() const
{
	return MySqlTypeTraits< std::string >::colType( charLength_ );
}


/*
 *	Override from PropertyMapping.
 */
bool StringLikeMapping::visitParentColumns( ColumnVisitor & visitor )
{
	ColumnType type( 
			this->getColumnType(),
			this->isBinary(),
			charLength_,
			defaultValue_ );

	if (type.fieldType == MYSQL_TYPE_LONG_BLOB)
	{
		// Can't put string > 16MB onto stream.
		CRITICAL_MSG( "StringLikeMapping::StringLikeMapping: "
				"Property '%s' has DatabaseLength %u that exceeds the maximum "
				"supported byte length of 16777215\n",
			this->propName().c_str(),
			charLength_ );
	}

	ColumnDescription description( colName_,
			type,
			isNameIndex_ ? INDEX_TYPE_NAME : INDEX_TYPE_NONE );

	return visitor.onVisitColumn( description );
}

// string_like_mapping.cpp
