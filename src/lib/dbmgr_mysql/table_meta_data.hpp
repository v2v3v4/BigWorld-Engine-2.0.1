/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_TABLE_META_DATA_HPP
#define MYSQL_TABLE_META_DATA_HPP

#include "table.hpp"

#include <map>
#include <set>
#include <string>

typedef std::set< std::string > StrSet;

class MySql;
class TableIndices;

#define PARENTID_INDEX_NAME "parentIDIndex"

std::string generateIndexName( const std::string & colName );

namespace TableMetaData
{

class ColumnInfo
{
public:
	ColumnType		columnType;
	ColumnIndexType	indexType;

	ColumnInfo( const MYSQL_FIELD& field, const TableIndices& indices );
	ColumnInfo();

	bool operator==( const ColumnInfo& other ) const;
	bool operator!=( const ColumnInfo& other ) const
	{	return !this->operator==( other );	}

private:
	static ColumnIndexType deriveIndexType(
			const MYSQL_FIELD& field, const TableIndices& indices );
};

// Map of column name to ColumnInfo.
typedef std::map< std::string, ColumnInfo > NameToColInfoMap;

struct UpdatedColumnInfo : public ColumnInfo
{
	// ColumnType		oldColumnType;
	ColumnIndexType	oldIndexType;

	UpdatedColumnInfo( const ColumnInfo& newCol, const ColumnInfo& oldCol ) :
	   	ColumnInfo( newCol ), // oldColumnType( oldCol.columnType ),
		oldIndexType( oldCol.indexType )
	{}
};

// Map of column name to UpdatedColumnInfo.
typedef std::map< std::string, UpdatedColumnInfo > NameToUpdatedColInfoMap;

void getEntityTables( StrSet & tables, MySql & connection );
void getTableColumns( NameToColInfoMap & columns,
		MySql & connection, const std::string & tableName );

} // namespace TableMetaData



/**
 *	This class retrieves the fields and indexes of a table.
 */
class MySqlTableMetadata
{
public:
	MySqlTableMetadata( MySql & connection, const std::string tableName );
	~MySqlTableMetadata();

	bool isValid() const 				{ return result_; }
	unsigned int getNumFields() const	{ return numFields_; }
	const MYSQL_FIELD& getField( unsigned int index ) const
	{ return fields_[index]; }

private:
	MYSQL_RES*		result_;
	unsigned int 	numFields_;
	MYSQL_FIELD* 	fields_;
};


#endif // MYSQL_TABLE_META_DATA_HPP
