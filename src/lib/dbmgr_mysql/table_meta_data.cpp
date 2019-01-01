/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "table_meta_data.hpp"

#include "query.hpp"
#include "result_set.hpp"

#include <sstream>

/**
 *	This function generates an index name based on column name. The name
 * 	of the index isn't really all that important but it's nice to have some
 * 	consistency.
 */
std::string generateIndexName( const std::string & colName )
{
	std::string::size_type underscorePos = colName.find( '_' );
	return (underscorePos == std::string::npos) ?
		colName + "Index" :
		colName.substr( underscorePos + 1 ) + "Index";
}


// -----------------------------------------------------------------------------
// Section: Table meta data
// -----------------------------------------------------------------------------

/**
 *	This class executes the "SHOW INDEX" query on a table and stores the
 * 	list of index names
 */
// NOTE: Can't be in anonymous namespace because forward declared in header
class TableIndices
{
public:
	TableIndices( MySql & connection, const std::string & tableName );

	const std::string * getIndexName( const std::string & colName ) const
	{
		ColumnToIndexMap::const_iterator found = colToIndexMap_.find( colName );

		return (found != colToIndexMap_.end()) ? &found->second : NULL;
	}

private:
	// Maps column names to index name.
	typedef std::map< std::string, std::string > ColumnToIndexMap;
	ColumnToIndexMap	colToIndexMap_;
};


/**
 *	Constructor. Retrieves the index information for the given table.
 */
TableIndices::TableIndices( MySql & connection, const std::string & tableName )
{
	// Get index info
	std::stringstream queryStr;
	queryStr << "SHOW INDEX FROM " << tableName;
	Query query( queryStr.str() );

	ResultSet resultSet;
	query.execute( connection, &resultSet );

	ResultRow resultRow;

	while (resultRow.fetchNextFrom( resultSet ))
	{
		std::string columnName;
		std::string keyName;

		resultRow.getField( 4, columnName );
		resultRow.getField( 2, keyName );

		// Build column name to index name map. Assume no multi-column index.
		colToIndexMap_[ columnName ] = keyName;
	}
}


/**
 * 	Constructor. Initialises from a MYSQL_FIELD and TableIndices.
 */
TableMetaData::ColumnInfo::ColumnInfo( const MYSQL_FIELD & field,
		const TableIndices & indices ) :
	columnType( field ), indexType( deriveIndexType( field, indices ) )
{}


/**
 * Default constructor. Required for insertion into std::map
 */
TableMetaData::ColumnInfo::ColumnInfo() :
	columnType( MYSQL_TYPE_NULL, false, 0, std::string() ),
	indexType( INDEX_TYPE_NONE )
{}


bool TableMetaData::ColumnInfo::operator==( const ColumnInfo & other ) const
{
	return (this->columnType == other.columnType) &&
			(this->indexType == other.indexType);
}


/**
 *	Returns the correct IMySqlColumnMapping::IndexType based on the information
 * 	in MYSQL_FIELD and TableIndices.
 */
ColumnIndexType TableMetaData::ColumnInfo::deriveIndexType(
		const MYSQL_FIELD & field, const TableIndices & indices )
{
	if (field.flags & PRI_KEY_FLAG)
	{
		return INDEX_TYPE_PRIMARY;
	}
	else if (field.flags & UNIQUE_KEY_FLAG)
	{
		const std::string * pIndexName = indices.getIndexName( field.name );
		MF_ASSERT( pIndexName );

		if (*pIndexName == generateIndexName( field.name ))
		{
			// One of ours
			return INDEX_TYPE_NAME;
		}
		else
		{
			WARNING_MSG( "TableMetaData::ColumnInfo::deriveIndexType: "
					"Found unknown unique index %s for column %s\n",
					pIndexName->c_str(), field.name );
		}
	}
	else if (field.flags & MULTIPLE_KEY_FLAG)
	{
		const std::string * pIndexName = indices.getIndexName( field.name );
		MF_ASSERT( pIndexName );

		if (*pIndexName == PARENTID_INDEX_NAME)
		{
			return INDEX_TYPE_PARENT_ID;
		}
		else
		{
			WARNING_MSG( "TableMetaData::ColumnInfo::deriveIndexType: "
					"Found unknown multiple key index %s for column %s\n",
					pIndexName->c_str(), field.name );
		}
	}

	return INDEX_TYPE_NONE;
}


/**
 *	Retrieves all the names of entity tables currently in the database.
 *
 * 	@param	This parameter receives the list of tables.
 */
void TableMetaData::getEntityTables( StrSet & tables, MySql & connection )
{
	const Query query( "SHOW TABLES LIKE '"TABLE_NAME_PREFIX"_%'" );

	ResultSet resultSet;
	query.execute( connection, &resultSet );

	std::string tableName;

	while (resultSet.getResult( tableName ))
	{
		tables.insert( tableName );
	}
}


/**
 * 	Retrieves meta data of all the columns for a given table.
 *
 * 	@param	columns	This output parameter receives the list of columns.
 * 	The key is the column name and the data is the column type.
 * 	@param	tableName	The name of the table to get the columns for.
 */
void TableMetaData::getTableColumns( TableMetaData::NameToColInfoMap & columns,
		MySql & connection, const std::string & tableName )
{
	MySqlTableMetadata	tableMetadata( connection, tableName );
	if (tableMetadata.isValid())	// table exists
	{
		TableIndices tableIndices( connection, tableName );
		for (unsigned int i = 0; i < tableMetadata.getNumFields(); i++)
		{
			const MYSQL_FIELD& field = tableMetadata.getField( i );
			columns[ field.name ] =
				TableMetaData::ColumnInfo( field, tableIndices );
		}
	}
}


// -----------------------------------------------------------------------------
// Section: class MySqlTableMetadata
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MySqlTableMetadata::MySqlTableMetadata( MySql & connection, 
		const std::string tableName ) :
	result_( mysql_list_fields( connection.get(), tableName.c_str(), NULL ) )
{
	if (result_)
	{
		numFields_ = mysql_num_fields( result_ );
		fields_ = mysql_fetch_fields( result_ );
	}
	else
	{
		numFields_ = 0;
		fields_ = NULL;
	}
}


/**
 *	Destructor.
 */
MySqlTableMetadata::~MySqlTableMetadata()
{
	if (result_)
	{
		mysql_free_result( result_ );
	}
}

// mysql_table_meta_data.cpp
