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

#include <vector>

#include "Python.h"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/md5.hpp"
#include "entitydef/entity_description_map.hpp"

class Entity;


/**
 *	This class maintains an entity template, as constructed from
 *	its definition file. It has a static varaiable that contains
 *  a vector of all defined entity types, and static variable that
 *  contains a map of all defined entity descriptions.
 */
class EntityType
{
private:
	EntityType( const EntityDescription & description);

public:
	~EntityType();

	static bool init( const std::string &clientPath, MD5::Digest& digest );
	static bool fini();

	static EntityType * find( uint type );
	static EntityType * find( const std::string & name );

	PyObject * newDictionary( DataSectionPtr pSection = NULL ) const;
	PyObject * newDictionary( BinaryIStream & stream, int dataDomains ) const;

	const std::string name() const	{ return description_.name(); }
	const EntityDescription & description() const	{ return description_; }

	Entity * newEntity( EntityID id, const Vector3 & pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data, bool isBasePlayer );

private:
	typedef std::vector< EntityType * > Types;
	static Types		types_;
	static EntityDescriptionMap entityDescriptionMap_;

	const EntityDescription & description_;
};



#endif // ENTITY_TYPE_HPP
