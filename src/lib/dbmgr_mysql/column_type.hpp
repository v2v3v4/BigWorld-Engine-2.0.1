/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLUMN_TYPE_HPP
#define COLUMN_TYPE_HPP

#include "constants.hpp" 
#include "type_traits.hpp" 
#include "wrapper.hpp" 

#include "network/basictypes.hpp"

#include <mysql/mysql.h>

// The following enum values are stored in bigworldTableMetadata.idx.
// If you change their numerical values, you must update upgradeDatabase().
enum ColumnIndexType
{
	INDEX_TYPE_NONE 		= 0,
	INDEX_TYPE_PRIMARY		= 1,
	INDEX_TYPE_NAME			= 2,
	INDEX_TYPE_PARENT_ID	= 3
};


/**
 *	This class is used to describe the type of a column.
 */
class ColumnType
{
public:
	enum_field_types	fieldType;
	bool				isUnsignedOrBinary;	// Dual use field
	uint				length; // In characters for text types, otherwise
								// in bytes.
	std::string			defaultValue;
	std::string			onUpdateCmd;
	bool				isAutoIncrement;

	/**
	 *	Constructor used when creating a new field in the database.
	 */
	ColumnType( enum_field_types type = MYSQL_TYPE_NULL,
			bool isUnsignedOrBin = false, 
			uint len = 0,
			const std::string defVal = std::string(),
			bool isAutoInc = false ) :
		fieldType( type ),
		isUnsignedOrBinary( isUnsignedOrBin ),
		length( len ),
		defaultValue( defVal ),
		isAutoIncrement( isAutoInc )
	{}


	/**
	 *	Constructor used when populating from a retrieved MYSQL_FIELD value.
	 */
	ColumnType( const MYSQL_FIELD & field ) :
		fieldType( field.type ),
		isUnsignedOrBinary( deriveIsUnsignedOrBinary( field ) ),
		length( field.length / MySql::charsetWidth( field.charsetnr ) ),
		defaultValue( field.def ? field.def : std::string() ),
		isAutoIncrement( field.flags & AUTO_INCREMENT_FLAG )
	{
		if (this->fieldType == MYSQL_TYPE_BLOB)
		{
			this->fieldType = MySqlTypeTraits<std::string>::colType( this->length );
		}

	}

	// Only applies to integer fields
	bool isUnsigned() const			{ return isUnsignedOrBinary; }
	void setIsUnsigned( bool val )	{ isUnsignedOrBinary = val; }

	// Only applies to string or blob fields
	bool isBinary()	const			{ return isUnsignedOrBinary; }
	void setIsBinary( bool val ) 	{ isUnsignedOrBinary = val; }

	std::string getAsString( MySql& connection, ColumnIndexType idxType ) const;
	std::string getDefaultValueAsString( MySql& connection ) const;
	bool isDefaultValueSupported() const;
	bool isStringType() const;
	bool isSimpleNumericalType() const;

	bool operator==( const ColumnType& other ) const;
	bool operator!=( const ColumnType& other ) const
	{	return !this->operator==( other );	}

	static bool deriveIsUnsignedOrBinary( const MYSQL_FIELD& field );


private:
	void adjustBlobTypeForSize();
};


/**
 *	This is a description of a column.
 */
class ColumnDescription
{
public:
	ColumnDescription( const std::string & columnName,
			const ColumnType & type,
			ColumnIndexType indexType = INDEX_TYPE_NONE,
			bool shouldIgnore = false ) :
		columnName_( columnName ),
		columnType_( type ),
		indexType_( indexType ),
		shouldIgnore_( shouldIgnore )
	{
	}

	const std::string & columnName() const			{ return columnName_; }
	const ColumnType & columnType() const			{ return columnType_; }
	ColumnIndexType columnIndexType() const			{ return indexType_; }

	bool shouldIgnore() const	{ return shouldIgnore_; }

private:
	const std::string columnName_;
	ColumnType columnType_;
	ColumnIndexType indexType_;
	bool shouldIgnore_;
};


/**
 *	This class describes an interface. Classes derived from this can be used to
 *	visit columns.
 */
class ColumnVisitor
{
public:
	// NOTE: For efficiency reasons the IMySqlColumnMapping passed to the
	// visitor may be a temporary on the stack i.e. you should not store
	// its address.
	virtual bool onVisitColumn( const ColumnDescription & description ) = 0;
};


const ColumnType PARENTID_COLUMN_TYPE( MYSQL_TYPE_LONGLONG );
const ColumnType ID_COLUMN_TYPE( MYSQL_TYPE_LONGLONG, false, 
		0, std::string(), true );	// Auto-increment column

#endif // COLUMN_TYPE_HPP
