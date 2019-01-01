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
#include "pyscript/script.hpp"

#ifndef CODE_INLINE
#include "py_entities.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

PY_MODULE_ATTRIBUTE( BigWorld, entities, new PyEntities )
PY_MODULE_ATTRIBUTE( BigWorld, cachedEntities, new PyEntities( false, true ) )
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
	PyObjectPlus( pType )
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

PY_BEGIN_METHODS( PyEntities )
	PY_METHOD( has_key )
	PY_METHOD( keys )
	PY_METHOD( items )
	PY_METHOD( values )
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

	PyObject * pEntity = NULL;

	Entity::EntityMap::iterator iter = Entity::entities_.find(id);
	if ( iter != Entity::entities_.end() )
	{
		pEntity = iter->second;
	}

	if (pEntity == NULL)
	{
		PyErr_Format( PyExc_KeyError, "%d", (int)id );
		return NULL;
	}

	Py_INCREF( pEntity );
	return pEntity;
}


/**
 * 	This method returns the number of entities in the system.
 */ 
int PyEntities::length()
{
	int length = Entity::entities_.size();

	return length;
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

	const bool hasKey = Entity::entities_.find(id) != Entity::entities_.end();

	return PyInt_FromLong( hasKey );
}


/**
 *	This function is used py_keys. It creates an object that goes into the keys
 *	list based on an element in the Entities map.
 */
PyObject * getKey( const Entity::EntityMap::value_type & item )
{
	return PyInt_FromLong( item.first );
}


/**
 *	This function is used py_values. It creates an object that goes into the
 *	values list based on an element in the Entities map.
 */
PyObject * getValue( const Entity::EntityMap::value_type & item )
{
	Py_INCREF( item.second );
	return item.second;
}


/**
 *	This function is used py_items. It creates an object that goes into the
 *	items list based on an element in the Entities map.
 */
PyObject * getItem( const Entity::EntityMap::value_type & item )
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

	Entity::EntityMap::iterator iter = Entity::entities_.begin();

	while (iter != Entity::entities_.end())
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

int PyEntities_token = 0;

// py_entities.cpp
