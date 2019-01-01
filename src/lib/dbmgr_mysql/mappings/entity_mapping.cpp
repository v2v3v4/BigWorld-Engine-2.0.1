/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_mapping.hpp"

#include "entitydef/entity_description.hpp"


/**
 * 	Constructor.
 */
EntityMapping::EntityMapping( const EntityDescription & entityDesc,
		const PropertyMappings & properties, const std::string & tableNamePrefix ) :
	entityDesc_( entityDesc ),
	tableName_( tableNamePrefix + "_" + entityDesc.name() ),
	properties_( properties )
{
}


/**
 * 	Gets the type ID of the entity type associated with this entity mapping.
 */
EntityTypeID EntityMapping::getTypeID() const
{
	return entityDesc_.index();
}


/*
 * 	Gets the type name of the entity type associated with this entity mapping.
 */
const std::string & EntityMapping::typeName() const
{
	return entityDesc_.name();
}


/*
 * 	Override from TableProvider. Visit all our columns, except the ID column.
 */
bool EntityMapping::visitColumnsWith( ColumnVisitor & visitor )
{
	for (PropertyMappings::iterator iter = properties_.begin();
			iter != properties_.end(); ++iter)
	{
		if (!(*iter)->visitParentColumns( visitor ))
		{
			return false;
		}
	}

	return true;
}


/*
 * 	Override from TableProvider. Visit our ID column.
 */
bool EntityMapping::visitIDColumnWith(	ColumnVisitor & visitor )
{
	ColumnDescription idColumn( ID_COLUMN_NAME_STR, ID_COLUMN_TYPE,
			INDEX_TYPE_PRIMARY );

	return visitor.onVisitColumn( idColumn );
}


/*
 * 	Override from TableProvider. Visit all our sub-tables.
 */
bool EntityMapping::visitSubTablesWith( TableVisitor & visitor )
{
	for ( PropertyMappings::iterator iter = properties_.begin();
			iter != properties_.end(); ++iter )
	{
		if (!(*iter)->visitTables( visitor ))
		{
			return false;
		}
	}

	return true;
}

// entity_mapping.cpp
