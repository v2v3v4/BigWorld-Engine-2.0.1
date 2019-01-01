/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROPERTY_MAPPINGS_PER_TYPE_HPP
#define PROPERTY_MAPPINGS_PER_TYPE_HPP

#include "property_mapping.hpp"

class EntityDefs;
class EntityDescription;
class TableInspector;

/**
 *
 */
class PropertyMappingsPerType
{
public:
	PropertyMappingsPerType();
	bool init( const EntityDefs & entityDefs );

	PropertyMappings & operator[]( size_t typeID )
		{ return container_[ typeID ]; }

	bool visit( const EntityDefs& entityDefs, TableInspector & visitor );

private:
	PropertyMappingsPerType( const PropertyMappingsPerType & );


	static bool initPropertyMappings( PropertyMappings & properties,
		const EntityDescription & entityDescription );

	typedef std::vector< PropertyMappings > Container;

	Container container_;
};

#endif // PROPERTY_MAPPINGS_PER_TYPE_HPP
