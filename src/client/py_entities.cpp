/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "pch.hpp"
#include "py_entities.hpp"

#include "entity_manager.hpp"

#ifndef CODE_INLINE
#include "py_entities.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

/*~ attribute BigWorld entities
 *  This lists all of the entities currently in the world, which includes all
 *  entities that have not been cached. Entities are cached when
 *  they are known by the client but exist outside the player's area of 
 *  interest. This only occurs for entities which are provided by a server.
 *  @type Read-Only PyEntities.
 */
PY_MODULE_ATTRIBUTE( BigWorld, entities, new PyEntities )
/*~ attribute BigWorld cachedEntities
 *  This lists all of the entities currently cached. Entities are cached when
 *  they are known by the client but exist outside the player's area of 
 *  interest. This only occurs for entities which are provided by a server.
 *  
 *  This variable is not supported in client-only licenses of BigWorld.
 *  @type Read-Only PyEntities.
 */
PY_MODULE_ATTRIBUTE( BigWorld, cachedEntities, new PyEntities( false, true ) )
/*~ attribute BigWorld allEntities
 *  This lists all of the entities instantiated on the client.
 *  @type Read-Only PyEntities.
 */
PY_MODULE_ATTRIBUTE( BigWorld, allEntities, new PyEntities( true, true ) )

// -----------------------------------------------------------------------------
// Section: PyEntities
// -----------------------------------------------------------------------------

/**
 *	The constructor for PyEntities.
 */
PyEntities::PyEntities( bool considerEntered,
						bool considerCached,
						PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	considerEntered_( considerEntered ),
	considerCached_( considerCached )
{
	BW_GUARD;	
}


/**
 *	Destructor.
 */
PyEntities::~PyEntities()
{
	BW_GUARD;	
}


/**
 *	This function is used to implement operator[] for the scripting object.
 */
PyObject * PyEntities::s_subscript( PyObject * self, PyObject * index )
{
	BW_GUARD;
	return ((PyEntities *) self)->subscript( index );
}


/**
 * 	This function returns the number of entities in the system.
 */
int PyEntities::s_length( PyObject * self )
{
	BW_GUARD;
	return ((PyEntities *) self)->length();
}


/**
 *	This structure contains the function pointers necessary to provide
 * 	a Python Mapping interface. 
 */
static PyMappingMethods g_entitiesMapping =
{
	PyEntities::s_length,		// mp_length
	PyEntities::s_subscript,	// mp_subscript
	NULL						// mp_ass_subscript
};


PY_TYPEOBJECT_WITH_MAPPING( PyEntities, &g_entitiesMapping )

/*~ function PyEntities has_key
 *  Used to determine whether an entity with a specific ID
 *  is listed in this PyEntities object.
 *  @param key An integer key to look for.
 *  @return A boolean. True if the key was found, false if it was not.
 */
/*~ function PyEntities keys
 *  Generates a list of the IDs of all of the entities listed in
 *  this object.
 *  @return A list containing all of the keys, represented as integers.
 */
/*~ function PyEntities items
 *  Generates a list of the items (ID:entity pairs) listed in
 *  this object.
 *  @return A list containing all of the ID:entity pairs, represented as tuples
 *  containing one integer, and one entity.
 */
/*~ function PyEntities values
 *  Generates a list of all the entities listed in
 *  this object.
 *  @return A list containing all of the entities.
 */
/*~ function PyEntities get
 *  @components{ base }
 *	@param id The id of the entity to find.
 *	@param defaultValue An optional argument that is the return value if the
 *		entity cannot be found. This defaults to None.
 *	@return The entity with the input id or the default value if not found.
 */
PY_BEGIN_METHODS( PyEntities )
	PY_METHOD( has_key )
	PY_METHOD( keys )
	PY_METHOD( items )
	PY_METHOD( values )
	PY_METHOD( get )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyEntities )
PY_END_ATTRIBUTES()


/**
 *	This method overrides the PyObjectPlus method.
 */
PyObject * PyEntities::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method finds the entity with the input ID.
 *
 * 	@param	entityID 	The ID of the entity to locate.
 *
 *	@return	The object associated with the given entity ID. 
 */
PyObject * PyEntities::subscript( PyObject* entityID )
{
	BW_GUARD;
	long id = PyInt_AsLong( entityID );

	if (PyErr_Occurred())
		return NULL;

	PyObject * pEntity = this->findEntity( id );
	if (pEntity == NULL)
	{
		PyErr_Format( PyExc_KeyError, "%d", id );
		return NULL;
	}

	Py_INCREF( pEntity );
	return pEntity;
}


/**
 *	This method returns the entity with the input id.
 */
Entity * PyEntities::findEntity( EntityID id ) const
{
	BW_GUARD;
	EntityManager & mgr = EntityManager::instance();
	Entity * pEntity = NULL;

	if (considerEntered_)
	{
		Entities::iterator iter = mgr.entities().find( id );
		if (iter != mgr.entities().end())
		{
			pEntity = iter->second;
		}
	}

	if (considerCached_ && (pEntity == NULL))
	{
		Entities::const_iterator iter = mgr.cachedEntities().find( id );
		if (iter != mgr.cachedEntities().end())
		{
			pEntity = iter->second;
		}
	}

	return pEntity;
}



/**
 * 	This method returns the number of entities in the system.
 */ 
int PyEntities::length()
{
	BW_GUARD;
	int length = considerEntered_ ?
		EntityManager::instance().entities().size() : 0;

	if (considerCached_)
	{
		length += EntityManager::instance().cachedEntities().size();
	}

	return length;
}


/**
 * 	This method returns true if the given entity exists.
 * 
 * 	@param args		A python tuple containing the arguments.
 */ 
PyObject * PyEntities::py_has_key(PyObject* args)
{
	BW_GUARD;
	long id;

	if (!PyArg_ParseTuple( args, "i", &id ))
		return NULL;

	EntityManager & mgr = EntityManager::instance();

	const bool hasKey =
		(considerEntered_ &&
			mgr.entities().find( id ) != mgr.entities().end()) ||
		(considerCached_ &&
			mgr.cachedEntities().find( id ) != mgr.cachedEntities().end());

	return PyInt_FromLong( hasKey );
}


/**
 *	This function is used py_keys. It creates an object that goes into the keys
 *	list based on an element in the Entities map.
 */
PyObject * getKey( const Entities::value_type & item )
{
	BW_GUARD;
	return PyInt_FromLong( item.first );
}


/**
 *	This function is used py_values. It creates an object that goes into the
 *	values list based on an element in the Entities map.
 */
PyObject * getValue( const Entities::value_type & item )
{
	BW_GUARD;
	Py_INCREF( item.second );
	return item.second;
}


/**
 *	This function is used py_items. It creates an object that goes into the
 *	items list based on an element in the Entities map.
 */
PyObject * getItem( const Entities::value_type & item )
{
	BW_GUARD;
	PyObject * pTuple = PyTuple_New( 2 );

	PyTuple_SET_ITEM( pTuple, 0, getKey( item ) );
	PyTuple_SET_ITEM( pTuple, 1, getValue( item ) );

	return pTuple;
}


/**
 *	This method is a helper used by py_keys, py_values and py_items. It returns
 *	a list, each element is created by using the input function.
 */
PyObject * PyEntities::makeList( GetFunc objectFunc )
{
	BW_GUARD;
	PyObject* pList = PyList_New( this->length() );
	EntityManager & mgr = EntityManager::instance();
	int i = 0;

	if (considerEntered_)
	{
		Entities::iterator iter = mgr.entities().begin();

		while (iter != mgr.entities().end())
		{
			PyList_SET_ITEM( pList, i, objectFunc( *iter ) );

			i++;
			iter++;
		}
	}

	if (considerCached_)
	{
		Entities::const_iterator iter = mgr.cachedEntities().begin();

		while (iter != mgr.cachedEntities().end())
		{
			PyList_SET_ITEM( pList, i, objectFunc( *iter ) );

			i++;
			iter++;
		}
	}

	return pList;
}


/**
 * 	This method returns a list of all the entity IDs in the system.
 */ 
PyObject* PyEntities::py_keys(PyObject* /*args*/)
{
	BW_GUARD;
	return this->makeList( getKey );
}


/**
 * 	This method returns a list of all the entity objects in the system.
 */ 
PyObject* PyEntities::py_values( PyObject* /*args*/ )
{
	BW_GUARD;
	return this->makeList( getValue );
}


/**
 * 	This method returns a list of tuples of all the entity IDs 
 *	and objects in the system.
 */ 
PyObject* PyEntities::py_items( PyObject* /*args*/ )
{
	BW_GUARD;
	return this->makeList( getItem );
}


/**
 *	This method returns the entity with the input index. If this entity cannot
 *	be found, the default value is returned.
 */
PyObject * PyEntities::py_get( PyObject * args )
{
	BW_GUARD;
	PyObject * pDefault = Py_None;
	int id = 0;
	if (!PyArg_ParseTuple( args, "i|O", &id, &pDefault ))
	{
		return NULL;
	}

	PyObject * pEntity = this->findEntity( id );

	if (!pEntity)
	{
		pEntity = pDefault;
	}

	Py_INCREF( pEntity );
	return pEntity;
}


int PyEntities_token = 0;

// py_entities.cpp
