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

/*~ class NoModule.PyEntities
 *  @components{ cell }
 *  An instance of PyEntities emulates a dictionary of PyEntity objects, 
 *  indexed by its id attribute. It does not support item assignment, but can
 *  be used with the subscript operator. Note that the key must be an integer, 
 *  and that a key error will be thrown if the key does not exist in the 
 *  dictionary. Instances of this class are used by the engine to make 
 *  lists of entities available to python. They cannot be created via script, 
 *  nor can they be modified.
 *
 *  Code Example:
 *  @{
 *  e = BigWorld.entities # this is a PyEntities object
 *  print "The entity with ID 100 is", e[ 100 ]
 *  print "There are", len( e ), "entities"
 *  @}
 */
/**
 *	This class is used to expose the collection of entities to scripting.
 */
class PyEntities : public PyObjectPlus
{
	Py_Header( PyEntities, PyObjectPlus )

	public:
		PyEntities( PyTypePlus * pType = &PyEntities::s_type_ );

		PyObject * 			pyGetAttribute( const char * attr );

		PyObject * 			subscript( PyObject * entityID );
		int					length();

		PY_METHOD_DECLARE(py_has_key)
		PY_METHOD_DECLARE(py_keys)
		PY_METHOD_DECLARE(py_values)
		PY_METHOD_DECLARE(py_items)

		PY_METHOD_DECLARE( py_get )

		static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
		static Py_ssize_t	s_length( PyObject * self );
};

#ifdef CODE_INLINE
// #include "py_entities.ipp"
#endif

#endif // PY_ENTITIES_HPP
