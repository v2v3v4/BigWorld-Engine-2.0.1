/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_BASES_HPP
#define PY_BASES_HPP

#include "pyscript/pyobject_plus.hpp"

class Bases;

/**
 *	This class is used to expose the collection of bases to scripting.
 */
class PyBases : public PyObjectPlus
{
	Py_Header( PyBases, PyObjectPlus )

public:
	PyBases( const Bases & bases, PyTypePlus * pType = &PyBases::s_type_ );

	PyObject * 			pyGetAttribute( const char * attr );

	PyObject * 			subscript( PyObject * entityID );
	int					length();

	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )
	PY_METHOD_DECLARE( py_get )

	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static Py_ssize_t	s_length( PyObject * self );

private:
	const Bases & bases_;
};

#ifdef CODE_INLINE
// #include "py_bases.ipp"
#endif

#endif // PY_BASES_HPP
