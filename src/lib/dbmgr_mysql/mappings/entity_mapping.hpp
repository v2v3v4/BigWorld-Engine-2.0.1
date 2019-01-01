/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_ENTITY_MAPPING_HPP
#define MYSQL_ENTITY_MAPPING_HPP

#include "../table.hpp" // For TableProvider

#include "property_mapping.hpp" // For PropertyMappings

class EntityDescription;


/**
 * 	This class contains the property mappings for an entity type.
 */
class EntityMapping : public TableProvider
{
public:
	EntityMapping( const EntityDescription & entityDesc,
			const PropertyMappings & properties,
			const std::string & tableNamePrefix = TABLE_NAME_PREFIX );
	virtual ~EntityMapping() {};

	EntityTypeID getTypeID() const;
	const std::string & typeName() const;

	// TableProvider overrides
	virtual const std::string & getTableName() const {	return tableName_;	}

	virtual bool visitColumnsWith( ColumnVisitor & visitor );
	virtual bool visitIDColumnWith( ColumnVisitor & visitor );
	virtual bool visitSubTablesWith( TableVisitor & visitor );

protected:
	const PropertyMappings & getPropertyMappings() const
			{ return properties_; }
	const EntityDescription & getEntityDescription() const
			{ return entityDesc_; }

private:
	const EntityDescription & 	entityDesc_;
	std::string					tableName_;
	PropertyMappings			properties_;
};

#endif // MYSQL_ENTITY_MAPPING_HPP
