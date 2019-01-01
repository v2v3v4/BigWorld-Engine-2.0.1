/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_TABLE_INITIALISER_HPP
#define MYSQL_TABLE_INITIALISER_HPP

#include "dbmgr_mysql/table_inspector.hpp"

class AlterTableHelper;

class TableInitialiser : public TableInspector
{
public:
	TableInitialiser( MySql & con, bool allowNew,
			const std::string & characterSet, const std::string & collation );

	// Exposed to allow sync_db to update from 1.7 to 1.8
	void createIndex( const std::string & tableName,
		const std::string & colName,
		const TableMetaData::ColumnInfo & colInfo );

protected:
	// TableInspector overrides.
	virtual bool onExistingTable( const std::string & tableName );

	virtual bool onNeedNewTable( const std::string & tableName,
			const TableMetaData::NameToColInfoMap & columns );

	virtual bool onNeedUpdateTable( const std::string & tableName,
			const TableMetaData::NameToColInfoMap & obsoleteColumns,
			const TableMetaData::NameToColInfoMap & newColumns,
			const TableMetaData::NameToUpdatedColInfoMap & updatedColumns );

	virtual bool onNeedDeleteTables( const StrSet & tableNames );

private:
	void addColumns( const std::string & tableName,
			const TableMetaData::NameToColInfoMap & columns,
			AlterTableHelper & helper, bool shouldPrintInfo );

	void dropColumns( const std::string & tableName,
			const TableMetaData::NameToColInfoMap & columns,
			AlterTableHelper & helper, bool shouldPrintInfo );

	void updateColumns( const std::string & tableName,
			const TableMetaData::NameToUpdatedColInfoMap & columns,
			AlterTableHelper & helper, bool shouldPrintInfo );

	void removeIndex( const std::string & tableName,
		const std::string & colName,
		ColumnIndexType indexType );

	template < class COLUMN_MAP >
	void initialiseColumns( const std::string & tableName,
		COLUMN_MAP & columns, bool shouldApplyDefaultValue );

	void initialiseColumn( const std::string & tableName,
		const std::string & columnName,
		const TableMetaData::ColumnInfo & columnInfo,
		bool shouldApplyDefaultValue );

	bool allowNew_;
	std::string characterSet_;
	std::string collation_;
};

#endif // MYSQL_TABLE_INITIALISER_HPP
