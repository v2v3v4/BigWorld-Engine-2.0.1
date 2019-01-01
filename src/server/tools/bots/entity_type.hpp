/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_TYPE_HPP
#define ENTITY_TYPE_HPP

#include "Python.h"

#include <vector>

#include "cstdmf/stdmf.hpp"
#include "entitydef/entity_description_map.hpp"

class Entity;
class ClientApp;


/**
 *	This class maintains an entity template, as constructed from
 *	its definition file. It has a static variable that contains
 *  a vector of all defined entity types, and static variable that
 *  contains a map of all defined entity descriptions.
 */
class EntityType
{
private:
	EntityType( const EntityDescription & description, PyTypeObject * pType );

public:
	~EntityType();			// our destructor should be too, but VC++ doesn't like it

	static int init( const std::string & standinEntity );
	static bool fini();

	static EntityType * find( uint type );
	static EntityType * find( const std::string & name );

	enum StreamContents { BASE_PLAYER_DATA, CELL_PLAYER_DATA,
		TAGGED_CELL_PLAYER_DATA, TAGGED_CELL_ENTITY_DATA };

	PyObject * newDictionary( DataSectionPtr pSection = NULL ) const;
	PyObject * newDictionary( BinaryIStream & stream,
		StreamContents contents ) const;

	int index() const								{ return description_.clientIndex(); }
	const std::string name() const					{ return description_.name(); }
	const EntityDescription & description() const	{ return description_; }
	PyTypeObject * pType() const					{ return pType_; }

	Entity * newEntity( const ClientApp & clientApp, EntityID id, const Vector3 & pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data, bool isBasePlayer );

private:
	static std::vector< EntityType * > s_types_;
	static EntityDescriptionMap entityDescriptionMap_;

	const EntityDescription & description_;
	PyTypeObject * pType_;
};

#endif // ENTITY_TYPE_HPP
