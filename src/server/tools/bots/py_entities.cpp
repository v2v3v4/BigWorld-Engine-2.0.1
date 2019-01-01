/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "py_entities.hpp"

#ifndef CODE_INLINE
#include "py_entities.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

// -----------------------------------------------------------------------------
// Section: PyEntities
// -----------------------------------------------------------------------------

/**
 *	The constructor for PyEntities.
 */
PyEntities::PyEntities( ClientApp * pClientApp, PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pClientApp_( pClientApp )
{
}


/**
 *	Destructor.
 */
PyEntities::~PyEntities()
{
}


/**
 *	This function is used to implement operator[] for the scripting object.
 */
PyObject * PyEntities::s_subscript( PyObject * self, PyObject * index )
{
	return ((PyEntities *) self)->subscript( index );
}


/**
 * 	This function returns the number of entities in the system.
 */
Py_ssize_t PyEntities::s_length( PyObject * self )
{
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
	long id = PyInt_AsLong( entityID );

	if (PyErr_Occurred())
		return NULL;

	PyObject * pEntity = this->findEntity( id );

	if (pEntity == NULL)
	{	// now, entity is only created when it is
		// required for running corresponding script.
		//PyErr_Format( PyExc_KeyError, "%d", id );
		//return NULL;
		Py_RETURN_NONE;
	}

	Py_INCREF( pEntity );
	return pEntity;

}

/**
*	This method returns the entity with the input id.
*/
Entity * PyEntities::findEntity( EntityID id ) const
{
	Entity * pEntity = NULL;

	ClientApp::EntityMap::const_iterator iter = 
		pClientApp_->entities().find( id );

	if (iter != pClientApp_->entities().end())
	{
		pEntity = iter->second;
	}
	return pEntity;
}

/**
 * 	This method returns the number of entities in the system.
 */ 
int PyEntities::length()
{
	return pClientApp_->entities().size();
}


/**
 * 	This method returns true if the given entity exists.
 * 
 * 	@param args		A python tuple containing the arguments.
 */ 
PyObject * PyEntities::py_has_key(PyObject* args)
{
	long id;

	if (!PyArg_ParseTuple( args, "i", &id ))
		return NULL;

	const bool hasKey = (pClientApp_->entities().find( id ) != pClientApp_->entities().end());

	return PyInt_FromLong( hasKey );
}


/**
 *	This function is used py_keys. It creates an object that goes into the keys
 *	list based on an element in the Entities map.
 */
PyObject * getKey( const ClientApp::EntityMap::value_type & item )
{
	return PyInt_FromLong( item.first );
}


/**
 *	This function is used py_values. It creates an object that goes into the
 *	values list based on an element in the Entities map.
 */
PyObject * getValue( const ClientApp::EntityMap::value_type & item )
{
	if (item.second == NULL)
		Py_RETURN_NONE;

	Py_INCREF( item.second );
	return item.second;
}


/**
 *	This function is used py_items. It creates an object that goes into the
 *	items list based on an element in the Entities map.
 */
PyObject * getItem( const ClientApp::EntityMap::value_type & item )
{
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
	PyObject* pList = PyList_New( this->length() );
	int i = 0;

	ClientApp::EntityMap::const_iterator iter = pClientApp_->entities().begin();
	while (iter != pClientApp_->entities().end())
	{
		PyList_SET_ITEM( pList, i, objectFunc( *iter ) );

		i++;
		iter++;
	}

	return pList;
}


/**
 * 	This method returns a list of all the entity IDs in the system.
 */ 
PyObject* PyEntities::py_keys(PyObject* /*args*/)
{
	return this->makeList( getKey );
}


/**
 * 	This method returns a list of all the entity objects in the system.
 */ 
PyObject* PyEntities::py_values( PyObject* /*args*/ )
{
	return this->makeList( getValue );
}


/**
 * 	This method returns a list of tuples of all the entity IDs 
 *	and objects in the system.
 */ 
PyObject* PyEntities::py_items( PyObject* /*args*/ )
{
	return this->makeList( getItem );
}


/**
 *	This method returns the entity with the input index. If this entity cannot
 *	be found, the default value is returned.
 */
PyObject * PyEntities::py_get( PyObject * args )
{
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
// py_entities.cpp
