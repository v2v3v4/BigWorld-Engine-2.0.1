/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "namer.hpp"

#include "constants.hpp"


/**
 *	Constructor (1 of 2)
 */
Namer::Namer( const std::string & entityName,
			const std::string & tableNamePrefix ) :
		tableNamePrefix_( tableNamePrefix ),
		names_( 1, entityName ),
		tableLevels_( 1, 1 )
{
}


/**
 *	Constructor (2 of 2)
 */
Namer::Namer( const Namer & existing,
		const std::string & propName, bool isTable ) :
	tableNamePrefix_( existing.tableNamePrefix_ ),
	names_( existing.names_ ),
	tableLevels_( existing.tableLevels_ )
{
	if (propName.empty())
	{
		names_.push_back( isTable ? DEFAULT_SEQUENCE_TABLE_NAME :
							DEFAULT_SEQUENCE_COLUMN_NAME  );
	}
	else
	{
		names_.push_back( propName );
	}

	if (isTable)
	{
		tableLevels_.push_back( names_.size() );
	}
}


/**
 *
 */
std::string Namer::buildColumnName( const std::string & prefix,
							const std::string & propName ) const
{
	std::string suffix =
		(propName.empty()) ? DEFAULT_SEQUENCE_COLUMN_NAME : propName;

	return this->buildName( prefix, suffix, tableLevels_.back() );
}


/**
 *
 */
std::string Namer::buildTableName( const std::string & propName ) const
{
	std::string suffix =
		(propName.empty()) ? DEFAULT_SEQUENCE_TABLE_NAME : propName;

	return this->buildName( tableNamePrefix_, suffix, 0 );
}


/**
 *
 */
std::string Namer::buildName( const std::string & prefix,
					const std::string & suffix,
					Strings::size_type startIdx ) const
{
	std::string name = prefix;

	for (Strings::size_type i = startIdx; i < names_.size(); ++i)
	{
		name += '_';
		name += names_[i];
	}

	if (!suffix.empty())
	{
		name += '_';
		name += suffix;
	}

	return name;
}

// mysql_namer.cpp
