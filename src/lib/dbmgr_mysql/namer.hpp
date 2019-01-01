/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_NAMER_HPP
#define MYSQL_NAMER_HPP

#include "constants.hpp"

#include <string>
#include <vector>

/**
 *	This class helps to build names for table columns. Introduced when due to
 *	nested properties. It tries to achieve the following:
 *		- Table names are fully qualified.
 *		- Column names are relative to the current table.
 */
class Namer
{
public:
	Namer( const std::string & entityName,
			const std::string & tableNamePrefix = TABLE_NAME_PREFIX );

	Namer( const Namer & existing,
			const std::string & propName, bool isTable );

	std::string buildColumnName( const std::string & prefix,
								const std::string & propName ) const;

	std::string buildTableName( const std::string & propName ) const;

private:
	typedef std::vector< std::string >			Strings;
	typedef std::vector< Strings::size_type >	Levels;

	std::string buildName( const std::string & prefix,
						const std::string & suffix,
						Strings::size_type startIdx ) const;

	std::string tableNamePrefix_;
	Strings		names_;
	Levels		tableLevels_;
};

#endif // MYSQL_NAMER_HPP
