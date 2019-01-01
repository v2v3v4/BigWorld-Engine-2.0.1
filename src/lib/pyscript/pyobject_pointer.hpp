/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYOBJECT_POINTER_HPP
#define PYOBJECT_POINTER_HPP

#include "cstdmf/smartpointer.hpp"

typedef SmartPointer<PyObject> PyObjectPtr;

// -----------------------------------------------------------------------------
// Section: SmartPointer glue
// -----------------------------------------------------------------------------


/**
 *	This template specialisation allows us to have a SmartPointer to
 *	ordinary an PyObject (PyObjectPlus works fine without these)
 */
template <>
inline void incrementReferenceCount( const PyObject & Q )
{
	Py_INCREF( const_cast<PyObject*>( &Q ) );	// Q guaranteed non-null
}

/**
 *	This template specialisation allows us to have a SmartPointer to
 *	ordinary an PyObject (PyObjectPlus works fine without these)
 */
template <>
inline void decrementReferenceCount( const PyObject & Q )
{
	Py_DECREF( const_cast<PyObject*>( &Q ) );	// Q guaranteed non-null
}

/**
 *	This template specialisation allows us to have a SmartPointer to
 *	an ordinary PyObject (PyObjectPlus works fine without these)
 */
template <>
inline bool hasZeroReferenceCount( const PyObject & Q )
{
	return (const_cast<PyObject*>( &Q )->ob_refcnt == 0);	// Q guaranteed non-null
}

#endif // PYOBJECT_POINTER_HPP
