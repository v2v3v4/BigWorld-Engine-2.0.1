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

#include "entity_manager.hpp"

#include "pyscript/pyobject_plus.hpp"
/*~ class BigWorld.PyEntities
 *  A class which emulates a dictionary of PyEntity objects indexed by their id
 *  attributes. Instances of this class do not support item assignment, but can
 *  be used with the subscript operator. Note that the key must be an integer, 
 *  and that a key error will be thrown if the key given does not exist in the
 *  dictionary. Instances of this class are used by the engine to present lists
 *  of entities to script. They cannot be created via script, nor can they be 
 *  modified.
 *
 *  Example:
 *  @{
 *  e = BigWorld.entities # this is a PyEntities object
 *  e[ 100 ] # returns the entity with ID 100
 *  len( e ) # returns the number of entities in this dictionary
 *  @}
 */
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

	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )
	PY_METHOD_DECLARE( py_get )

	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static int			s_length( PyObject * self );

private:
	PyEntities( const PyEntities& );
	PyEntities& operator=( const PyEntities& );

	Entity * findEntity( EntityID id ) const;

	typedef PyObject * (*GetFunc)( const Entities::value_type & item );
	PyObject * makeList( GetFunc objectFunc );

	bool considerEntered_;
	bool considerCached_;
};


#ifdef CODE_INLINE
#include "py_entities.ipp"
#endif

#endif // PY_ENTITIES_HPP
