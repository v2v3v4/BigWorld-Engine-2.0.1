/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_ENTITIES_HPP
#define PY_ENTITIES_HPP

#include "pyscript/pyobject_plus.hpp"
#include "entity.hpp"

/**
 *	This class is used to expose the collection of entities to scripting.
 */
class PyEntities : public PyObjectPlus
{
	Py_Header( PyEntities, PyObjectPlus )

public:
	PyEntities( bool considerEntered = true,
		bool considerCached = false,
		PyTypePlus * pType = &PyEntities::s_type_ );
	~PyEntities();

	PyObject * 			pyGetAttribute( const char * attr );

	PyObject * 			subscript( PyObject * entityID );
	int					length();

	PY_METHOD_DECLARE(py_has_key)
	PY_METHOD_DECLARE(py_keys)
	PY_METHOD_DECLARE(py_values)
	PY_METHOD_DECLARE(py_items)

	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static Py_ssize_t	s_length( PyObject * self );

private:
	PyEntities( const PyEntities& );
	PyEntities& operator=( const PyEntities& );

	typedef PyObject * (*GetFunc)( const Entity::EntityMap::value_type & item );
	PyObject * makeList( GetFunc objectFunc );
};


#ifdef CODE_INLINE
#include "py_entities.ipp"
#endif

#endif // PY_ENTITIES_HPP
