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

#include "Python.h"	// forward declaration doesn't work :(

#include "cstdmf/stdmf.hpp"

#include "entitydef/entity_description_map.hpp"

class Entity;

/**
 *	This class maintains an entity template, as constructed from
 *	its definition file and python scripts.
 *
 *	It is used to create new python class objects and to retrieve
 *	static entity type information.
 */
class EntityType
{
private:
	EntityType( const EntityDescription & description,
		PyObject * pModule,
		PyTypeObject * pClass );

public:
	~EntityType();			// our destructor should be too, but VC++ doesn't like it

	static bool init();
	static bool reload();
	static void fini();

	static EntityType * find( uint type );
	static EntityType * find( const std::string & name );

	static PyObject * getPreloads();

	enum StreamContents { BASE_PLAYER_DATA, CELL_PLAYER_DATA,
		TAGGED_CELL_PLAYER_DATA, TAGGED_CELL_ENTITY_DATA };

	PyObject * newDictionary( DataSectionPtr pSection = NULL ) const;
	PyObject * newDictionary( BinaryIStream & stream,
		StreamContents contents ) const;

	int index() const					{ return description_.clientIndex(); }
	const std::string & name() const	{ return description_.name(); }

	const EntityDescription & description() const	{ return description_; }

	PyTypeObject * pClass() const		{ return pClass_; }
	PyTypeObject * pPlayerClass() const;

	Entity * newEntity( EntityID id, Vector3 & pos, float * pAuxVolatile,
		int enterCount, BinaryIStream & data, StreamContents contents,
		Entity * pSister );

private:
	static std::vector<EntityType*>				s_types_;
	static std::map<std::string,EntityTypeID>	s_typeNames_;

	void swizzleClass( PyTypeObject *& pOldClass );

	const EntityDescription & description_;
	PyObject * pModule_;
	PyTypeObject * pClass_;
	mutable PyTypeObject * pPlayerClass_;
};



#endif // ENTITY_TYPE_HPP
