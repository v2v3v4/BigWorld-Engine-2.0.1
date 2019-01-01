/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "utils.hpp"

#include "comma_sep_column_names_builder.hpp"

#include "cstdmf/binary_stream.hpp"

std::string buildCommaSeparatedQuestionMarks( int num )
{
	if (num <= 0)
		return std::string();

	std::string list;
	list.reserve( (num * 2) - 1 );
	list += '?';

	for ( int i=1; i<num; i++ )
	{
		list += ",?";
	}
	return list;
}


/**
 * 	Stores the default value of a sequence mapping into the stream.
 */
void defaultSequenceToStream( BinaryOStream & strm, int seqSize,
		PropertyMappingPtr pChildMapping_ )
{
	if (seqSize == 0)	// Variable-sized sequence
	{
		strm << int(0);	// Default value is no elements
	}
	else
	{
		strm << seqSize;

		for ( int i = 0; i < seqSize; ++i )
		{
			pChildMapping_->defaultToStream( strm );
		}
	}
}

std::string createInsertStatement( const std::string & tbl,
		const PropertyMappings & properties )
{
	std::string stmt = "INSERT INTO " + tbl + " (";
	CommaSepColNamesBuilder colNames( properties );
	stmt += colNames.getResult();
	int numColumns = colNames.getCount();
	stmt += ") VALUES (";

	stmt += buildCommaSeparatedQuestionMarks( numColumns );
	stmt += ')';

	return stmt;
}


std::string createUpdateStatement( const std::string& tbl,
		const PropertyMappings& properties )
{
	std::string stmt = "UPDATE " + tbl + " SET ";
	CommaSepColNamesBuilderWithSuffix colNames( properties, "=?" );
	stmt += colNames.getResult();

	if (colNames.getCount() == 0)
	{
		return std::string();
	}

	stmt += " WHERE id=?";

	return stmt;
}

std::string createSelectStatement( const std::string & tbl,
		const PropertyMappings & properties,
		const std::string & where )
{
	std::string stmt = "SELECT id,";

	CommaSepColNamesBuilder colNames( properties );
	stmt += colNames.getResult();

	if (colNames.getCount() == 0)
	{
		stmt.resize( stmt.length() - 1 );	// remove comma
	}

	stmt += " FROM " + tbl;

	if (where.length())
	{
		stmt += " WHERE " + where;
	}

	return stmt;
}


// utils.cpp
