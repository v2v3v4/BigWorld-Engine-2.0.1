/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_KEY_HPP
#define ENTITY_KEY_HPP

#include "network/basictypes.hpp"
#include <string>


/**
 *	This class represents a key to an entity record in the database.
 */
class EntityKey
{
public:
	EntityKey( EntityTypeID type, DatabaseID id ) :
		typeID( type ),
		dbID( id )
	{
	}

	bool operator<( const EntityKey & other ) const
	{
		return (typeID < other.typeID) ||
				((typeID == other.typeID) && (dbID < other.dbID));
	}

	EntityTypeID	typeID;
	DatabaseID 		dbID;
};


/**
 *	This class represents a key to an entity record in the database that can
 *	use either a DatabaseID or the entity's identifier string.
 */
class EntityDBKey : public EntityKey
{
public:
	EntityDBKey( EntityTypeID typeID, DatabaseID dbID,
			const std::string & s = std::string() ) :
		EntityKey( typeID, dbID ),
		name( s )
	{
	}

	explicit EntityDBKey( const EntityKey & key ) :
		EntityKey( key ),
		name()
	{
	}

	std::string		name;	///< used if dbID is zero
};

#endif // ENTITY_KEY_HPP
