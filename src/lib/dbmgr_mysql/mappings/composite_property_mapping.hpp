/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_COMPOSITE_PROPERTY_MAPPING_HPP
#define MYSQL_COMPOSITE_PROPERTY_MAPPING_HPP

#include "property_mapping.hpp"

/**
 *	This class handles mapping multiple property types into a single collection
 *	such as is required for USER_DATA types.
 */
class CompositePropertyMapping : public PropertyMapping
{
public:
	CompositePropertyMapping( const std::string & propName );

	void addChild( PropertyMappingPtr child );

	int getNumChildren() const;

	// Overrides from PropertyMapping
	virtual void prepareSQL();

	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const;


	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const;

	virtual void defaultToStream( BinaryOStream & strm ) const;

	virtual bool hasTable() const;

	virtual void deleteChildren( MySql & connection,
				DatabaseID databaseID ) const;

	virtual bool visitParentColumns( ColumnVisitor & visitor );

	virtual bool visitTables( TableVisitor & visitor );

	typedef std::vector< PropertyMappingPtr > Children;

protected:
	Children children_;
};

typedef SmartPointer<CompositePropertyMapping> CompositePropertyMappingPtr;

#endif // MYSQL_COMPOSITE_PROPERTY_MAPPING_HPP
