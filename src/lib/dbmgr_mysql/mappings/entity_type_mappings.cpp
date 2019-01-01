/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_type_mappings.hpp"

#include "entity_type_mapping.hpp"
#include "property_mappings_per_type.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"


/**
 *	Default constructor.
 */
EntityTypeMappings::EntityTypeMappings()
{
}


/**
 *	Constructor.
 */
EntityTypeMappings::EntityTypeMappings( const EntityDefs & entityDefs,
	MySql & connection )
{
	this->init( entityDefs, connection );
}


/**
 *	Destructor.
 */
EntityTypeMappings::~EntityTypeMappings()
{
	Container::iterator iter = container_.begin();

	while (iter != container_.end())
	{
		delete *iter;

		++iter;
	}
}


/**
 * 	This function creates EntityTypeMappings from the given
 * 	PropertyMappings.
 */
void EntityTypeMappings::init( const EntityDefs & entityDefs,
	MySql & connection )
{
	PropertyMappingsPerType propMappingsPerType;
	MF_VERIFY( propMappingsPerType.init( entityDefs ) );

	for (EntityTypeID typeID = 0;
			typeID < entityDefs.getNumEntityTypes();
			++typeID)
	{
		if (entityDefs.isValidEntityType( typeID ))
		{
			const EntityDescription & entityDesc =
				entityDefs.getEntityDescription( typeID );
			const DataDescription *pIdentifier = entityDesc.pIdentifier();

			std::string identiferProperty;
			if (pIdentifier)
			{
				identiferProperty = pIdentifier->name();
			}
			container_.push_back( new EntityTypeMapping( connection,
									entityDesc,
									propMappingsPerType[ typeID ],
									identiferProperty ) );
		}
		else
		{
			container_.push_back( NULL );
		}
	}
}

// entity_type_mappings.cpp
