/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_COMMA_SEP_COLUMN_NAMES_BUILDER_HPP
#define MYSQL_COMMA_SEP_COLUMN_NAMES_BUILDER_HPP

#include <sstream>

class PropertyMapping;

#include "mappings/property_mapping.hpp" // For PropertyMappings

#include "table.hpp" // For ColumnVisitor

/**
 *	This helper class builds a comma separated list string of column names,
 * 	with a suffix attached to the name.
 */
class CommaSepColNamesBuilder : public ColumnVisitor
{
public:
	// Constructor for a single PropertyMapping
	CommaSepColNamesBuilder( PropertyMapping & property );

	// Constructor for many PropertyMappings
	CommaSepColNamesBuilder( const PropertyMappings & properties );

	// Constructor for a TableProvider
	CommaSepColNamesBuilder( TableProvider & table, bool visitIDCol );

	std::string getResult() const	{ return commaSepColumnNames_.str(); }
	int 		getCount() const	{ return count_; }

	// ColumnVisitor override
	bool onVisitColumn( const ColumnDescription & column );

protected:
	std::stringstream	commaSepColumnNames_;
	int 				count_;

	CommaSepColNamesBuilder() : count_( 0 ) {}

};

/**
 *	This helper class builds a comma separated list string of column names,
 * 	with a suffix attached to the name.
 */
class CommaSepColNamesBuilderWithSuffix : public CommaSepColNamesBuilder
{
public:
	// Constructor for many PropertyMappings
	CommaSepColNamesBuilderWithSuffix( const PropertyMappings & properties,
			const std::string & suffix = std::string() );

	// Constructor for a single PropertyMapping
	CommaSepColNamesBuilderWithSuffix( PropertyMapping & property,
			const std::string & suffix = std::string() );

	bool onVisitColumn( const ColumnDescription & column );

private:
	std::string suffix_;
};

#endif // MYSQL_COMMA_SEP_COLUMN_NAMES_BUILDER_HPP
