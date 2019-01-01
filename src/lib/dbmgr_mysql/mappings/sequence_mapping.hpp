/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_SEQUENCE_MAPPING_HPP
#define MYSQL_SEQUENCE_MAPPING_HPP

#include "property_mapping.hpp"

#include "../query.hpp"
#include "../table.hpp" // For TableProvider

#include <memory>


/**
 *	This class maps sequences to tables.
 */
class SequenceMapping : public PropertyMapping, public TableProvider
{
public:
	SequenceMapping( const Namer & namer, const std::string & propName,
		PropertyMappingPtr pChild, int size = 0 );
	~SequenceMapping();

	const PropertyMapping & getChildMapping() const	{	return *pChild_;	}

	bool isFixedSize() const	{ return size_ != 0; }
	int getFixedSize() const	{ return size_;	}

	virtual void prepareSQL();

	int getNumElemsFromStrm( BinaryIStream & strm ) const;

	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const;

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const;

	int setNumElemsInStrm( BinaryOStream & strm, int numElems ) const;

	virtual void defaultToStream( BinaryOStream & strm ) const;

	virtual bool hasTable() const	{ return true; }

	virtual void deleteChildren( MySql & connection, DatabaseID databaseID ) const;

	virtual bool visitParentColumns( ColumnVisitor & visitor );

	virtual bool visitTables( TableVisitor & visitor );

	// TableProvider overrides
	virtual const std::string & getTableName() const;

	virtual bool visitColumnsWith( ColumnVisitor & visitor );

	virtual bool visitIDColumnWith( ColumnVisitor & visitor );

	virtual bool visitSubTablesWith( TableVisitor & visitor );

private:
	std::string tblName_;
	PropertyMappingPtr pChild_;
	int	size_;

	DatabaseID queryID_;
	DatabaseID childID_;
	bool childHasTable_;

	// auto_ptr's so we can delay instantiation
	Query selectQuery_;

	Query selectChildrenQuery_;
	Query deleteQuery_;
	Query deleteExtraQuery_;

	Query insertQuery_;
	Query updateQuery_;
};

#endif // MYSQL_SEQUENCE_MAPPING_HPP
