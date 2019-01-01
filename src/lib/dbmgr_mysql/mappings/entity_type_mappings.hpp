/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_ENTITY_TYPE_MAPPINGS_HPP
#define MYSQL_ENTITY_TYPE_MAPPINGS_HPP

#include <vector>

class EntityDefs;
class EntityTypeMapping;
class MySql;


/**
 *	This class is a collection of EntityTypeMapping instances for each entity
 *	type.
 */
class EntityTypeMappings
{
public:
	EntityTypeMappings();
	EntityTypeMappings( const EntityDefs & entityDefs,
			MySql & connection );
	~EntityTypeMappings();

	void init( const EntityDefs & entityDefs,
			MySql & connection );

	const EntityTypeMapping * operator[]( size_t typeID ) const
	{
		return container_[ typeID ];
	}

private:
	typedef std::vector< EntityTypeMapping * > Container;
	Container container_;
};

#endif // MYSQL_ENTITY_TYPE_MAPPINGS_HPP
