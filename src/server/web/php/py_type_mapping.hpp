/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYTHON_TYPE_MAPPING_HPP
#define PYTHON_TYPE_MAPPING_HPP

#include <Python.h>

#include "zend_API.h"

/**
 *	List entry ID for PyObject* PHP resources.
 */
extern int le_pyobject;

/**
 *	This namespace contains functions to map between PHP types and Python types.
 */
namespace PyTypeMapping
{
	void mapPyTypeToPHP( PyObject * pyObj, zval * return_value );

	void mapPHPTypeToPy( zval * phpObj, PyObject ** ppReturnValue );

} // namespace PyTypeMapping

#endif // PYTHON_TYPE_MAPPING_HPP
