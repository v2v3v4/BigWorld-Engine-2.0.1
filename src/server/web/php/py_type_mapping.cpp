/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "py_type_mapping.hpp"

#include "py_object_auto_ptr.hpp"

#include <limits.h>

namespace PyTypeMapping
{

// Typemapping from specific Python objects to PHP objects

void mapPyDictToPHP( PyObject * pyDict, zval * return_value );

void mapPySequenceToPHP( PyObject * pySeq, zval * return_value );

void mapPyStringToPHP( PyObject * pyStr, zval * return_value );

void mapPyIntToPHP( PyObject * pyInt, zval * return_value );

void mapPyLongToPHP( PyObject * pyInt, zval * return_value );

void mapPyFloatToPHP( PyObject * pyInt, zval * return_value );

void mapPyObjToPHP( PyObject * pyObj, zval * return_value );

// Typemapping from specific PHP objects to Python objects

bool phpArrayIsList( zval * array );

void mapPHPListArrayToPy( zval * zendList, PyObject ** ppReturnValue );

void mapPHPDictArrayToPy( zval * zendDict, PyObject ** ppReturnValue );



// ----------------------------------------------------------------------------
// Section: Type-mapping functions from Python objects to PHP types
// ----------------------------------------------------------------------------

/**
 *	Maps a Python object to a PHP object.
 *
 *	@param pyObj			The Python object.
 *	@param return_value 	The PHP return value.
 */
void mapPyTypeToPHP( PyObject * pyObj, zval * return_value )
{
	if (PyDict_Check( pyObj ))
	{
		mapPyDictToPHP( pyObj, return_value );
	}
	else if (PyString_Check( pyObj ))
	{
		mapPyStringToPHP( pyObj, return_value );
	}
	else if (PySequence_Check( pyObj ))
	{
		mapPySequenceToPHP( pyObj, return_value );
	}
	else if (PyInt_Check( pyObj ))
	{
		// includes bool
		mapPyIntToPHP( pyObj, return_value );
	}
	else if (PyLong_Check( pyObj ))
	{
		mapPyLongToPHP( pyObj, return_value );
	}
	else if (PyFloat_Check( pyObj ))
	{
		mapPyFloatToPHP( pyObj, return_value );
	}
	else if (pyObj == Py_None)
	{
		RETURN_NULL();
	}
	else
	{
		mapPyObjToPHP( pyObj, return_value );
	}
}


/**
 *	Maps a Python dictionary object to a PHP hash array.
 *
 *	@param pyDict			The Python dictionary object.
 *	@param return_value 	The PHP return value.
 */
void mapPyDictToPHP( PyObject * pyDict, zval * return_value )
{
	array_init( return_value );

	Py_ssize_t pos = 0;
	PyObject * pyKey = NULL;
	PyObject * pyValue = NULL;

	while (PyDict_Next( pyDict, &pos, &pyKey, &pyValue ))
	{
		// pyKey and pyValue are borrowed

		PyObjectAutoPtr pKeyString( PyObject_Str( pyKey ) );

		char * phpKey = PyString_AsString( pKeyString );

		zval * phpValue = NULL;
		MAKE_STD_ZVAL( phpValue );

		mapPyTypeToPHP( pyValue, phpValue );

		if (SUCCESS != add_assoc_zval( return_value, phpKey, phpValue ))
		{
			zend_error( E_ERROR, "Could not add value for key = %s", phpKey );
			return;
		}
	}
}


/**
 *	Maps a Python sequence object to a PHP numerically indexed hash array. This
 *	includes lists and tuples, but not strings (handled by mapPyStringToPHP).
 *
 *	@param pySequence		The Python sequence object.
 *	@param return_value 	The PHP return value.
 */
void mapPySequenceToPHP( PyObject * pySequence, zval * return_value )
{
	array_init( return_value );

	for (int i = 0; i < PySequence_Size( pySequence ); ++i)
	{
		PyObject * pyValue = PySequence_GetItem( pySequence, i ); // borrowed

		zval * phpValue = NULL;
		MAKE_STD_ZVAL( phpValue );

		mapPyTypeToPHP( pyValue, phpValue );

		if (SUCCESS != add_index_zval( return_value, i, phpValue ))
		{
			zend_error( E_ERROR, "Could not add value for index = %d", i );
			return;
		}
	}
}


/**
 *	Maps a Python string to a PHP string.
 *
 *	@param pyStr			The Python string object.
 *	@param return_value 	The PHP return value.
 */
void mapPyStringToPHP( PyObject * pyStr, zval * return_value )
{
	RETURN_STRING( PyString_AsString( pyStr ), /*duplicate: */1 );
}


/**
 *	Maps a Python plain integer to a PHP integer.
 *
 *	@param pyInt			The Python plain integer object.
 *	@param return_value 	The PHP return value.
 */
void mapPyIntToPHP( PyObject * pyInt, zval * return_value )
{
	if (PyBool_Check( pyInt ))
	{
		if (pyInt == Py_True)
		{
			RETURN_TRUE;
		}
		else
		{
			RETURN_FALSE;
		}
	}
	RETURN_LONG( PyInt_AsLong( pyInt ) );
}


/**
 *	Maps a Python long integer to either a PHP long if it can fit, or a PHP
 *	string representation if it cannot, in which case the string can then be
 *	manipulated using the bc* functions for arbitrary precision arithmetic in
 *	PHP.
 *
 *	@param pyInt			The Python long integer object.
 *	@param return_value 	The PHP return value.
 */
void mapPyLongToPHP( PyObject * pyLong, zval * return_value )
{
	if (PyLong_AsLongLong( pyLong ) > LONG_MAX || 
			PyLong_AsLongLong( pyLong ) < LONG_MIN)
	{
		PyObjectAutoPtr pLongString( PyObject_Str( pyLong ) );
		mapPyStringToPHP( pLongString, return_value );
	}
	else
	{
		RETURN_LONG( PyLong_AsLongLong( pyLong ) );
	}
}


/**
 *	Maps a Python float to a PHP double.
 *
 *	@param pyFloat 			The Python floating point object.
 *	@param return_value		The PHP return value.
 */
void mapPyFloatToPHP( PyObject * pyFloat, zval * return_value )
{
	RETURN_DOUBLE( PyFloat_AsDouble( pyFloat ) );
}


/**
 *	Maps a generic Python object to a PHP resource, by storing a reference to
 *	the Python object inside the new PHP resource.
 *
 *	@param pyObj			the Python object
 *	@param return_value 	the zval return value.
 */
void mapPyObjToPHP( PyObject * pyObj, zval * return_value )
{
	// General Python Object -> PHP resource
	Py_INCREF( pyObj );
	ZEND_REGISTER_RESOURCE( return_value, pyObj, le_pyobject );
}


// ----------------------------------------------------------------------------
// Section: Type-mapping functions from PHP types to Python objects
// ----------------------------------------------------------------------------

/**
 *	Map a PHP object to a new reference to the equivalent Python object.
 *
 *	@param phpObj 			The PHP object.
 *	@param ppReturnValue 	This pointer is filled with the new reference to
 *							the equivalent Python object.
 */
void mapPHPTypeToPy( zval * phpObj, PyObject ** ppReturnValue )
{
	*ppReturnValue = NULL;

	switch (Z_TYPE_P( phpObj ))
	{
		case IS_NULL:
			Py_INCREF( Py_None );
			*ppReturnValue = Py_None;
		break;

		case IS_LONG:
			*ppReturnValue = PyInt_FromLong( Z_LVAL_P( phpObj ) );
		break;

		case IS_DOUBLE:		// Python float
			*ppReturnValue = PyFloat_FromDouble( Z_DVAL_P( phpObj ) );
		break;

		case IS_STRING:		// Python string
			*ppReturnValue = PyString_FromStringAndSize(
				Z_STRVAL_P( phpObj ), Z_STRLEN_P( phpObj ) );
		break;

		case IS_ARRAY:
		{
			if (phpArrayIsList( phpObj ))
			{
				mapPHPListArrayToPy( phpObj, ppReturnValue );
			}
			else
			{
				mapPHPDictArrayToPy( phpObj, ppReturnValue );
			}
		}

		break;

		case IS_BOOL:	// Python plain integer
		{
			if (Z_LVAL_P( phpObj ) == 0)
			{
				Py_INCREF( Py_False );
				*ppReturnValue = Py_False;
			}
			else
			{
				Py_INCREF( Py_True );
				*ppReturnValue = Py_True;
			}
		}
		break;

		case IS_CONSTANT:	// Python string
			*ppReturnValue = PyString_FromStringAndSize(
				Z_STRVAL_P( phpObj ), Z_STRLEN_P( phpObj ) );
		break;

		case IS_RESOURCE:	 // Python resource
		{
			// get the Python object in the resource, if it is a Python object
			PyObject * pyObj = NULL;

			pyObj = (PyObject *) zend_fetch_resource(
				&phpObj TSRMLS_CC,
				-1, // default resource ID
				"PyObject", NULL, 1, le_pyobject );

			if (!pyObj)
			{
				return;
			}

			Py_INCREF( pyObj );
			*ppReturnValue = pyObj;
		}
		break;

		case IS_OBJECT: // Python object with incref
		case IS_CONSTANT_ARRAY:	// Python object with incref
		default:
			zend_error( E_ERROR, "Could not get arg format type for "
				"unknown PHP type: %d", phpObj->type );

		break;
	}
	return;
}


/**
 *	Return true if the given array zval is a list, that is, its keys are
 *	numerical, sequential and start from 0.

 *	@param array 	The PHP array.
 *	@return 		true if the array is a list, otherwise false.
 */
bool phpArrayIsList( zval * array )
{
	HashPosition pos;
	zval ** entry = NULL;
	char * key = NULL;
	uint keyLen = 0;
	ulong index = 0;

	bool isFirst = true;
	ulong lastIndex = 0;

	zend_hash_internal_pointer_reset_ex( HASH_OF( array ), &pos );

	while (zend_hash_get_current_data_ex(
			HASH_OF( array ), ( void** ) &entry, &pos ) == SUCCESS)
	{
		zend_hash_get_current_key_ex( HASH_OF( array ),
			&key, &keyLen, &index, 0, &pos );

		if (key)
		{
			return false;
		}
		else
		{
			if (( isFirst && index != 0 ) ||
				( !isFirst && index != lastIndex + 1 ))
			{
				return false;
			}
			lastIndex = index;
		}
		isFirst = false;
		zend_hash_move_forward_ex( HASH_OF( array ), &pos );
	}

	return true;
}


/**
 *	Maps a PHP list array (where the keys are consecutive numeric keys starting
 *	from 0) to a Python list.
 *
 *	@param zendArray 		The PHP array object.
 *	@param ppReturnValue 	A pointer will be filled with a new reference to a
 *							Python list with the equivalent Python object
 *							elements.
 */
void mapPHPListArrayToPy( zval * zendArray, PyObject ** ppReturnValue )
{
	*ppReturnValue = NULL;

	int numElements = zend_hash_num_elements( HASH_OF( zendArray ) );
	PyObjectAutoPtr pList( PyList_New( numElements ) );

	HashPosition pos;
	zval ** entry = NULL;
	char * key = NULL;
	uint keyLen = 0;
	ulong index = 0;

	zend_hash_internal_pointer_reset_ex( HASH_OF( zendArray ), &pos );

	while (zend_hash_get_current_data_ex(
			HASH_OF( zendArray ), ( void** )&entry, &pos ) == SUCCESS)
	{
		zend_hash_get_current_key_ex( HASH_OF( zendArray ),
			&key, &keyLen, &index, 0, &pos );

		PyObject * pValue = NULL;
		mapPHPTypeToPy( *entry, &pValue );

		if (pValue == NULL)
		{
			return;
		}

		PyList_SetItem( pList, index, pValue ); // ref to pValue is stolen

		zend_hash_move_forward_ex( HASH_OF( zendArray ), &pos );
	}

	Py_INCREF( pList.get() );
	*ppReturnValue = pList;
}


/**
 *	Maps a PHP dictionary array (a PHP array without consecutive numeric keys
 *	that start from 0) to a Python dictionary.
 *
 *	@param zendDict			The PHP array object, containing either string keys
 *							or non-consecutive integer keys.
 *	@param ppReturnValue 	a pointer to a PyObject * where the new PyDict will be
 *							created
 */
void mapPHPDictArrayToPy( zval * zendDict, PyObject ** ppReturnValue )
{
	*ppReturnValue = NULL;

	PyObjectAutoPtr pDict( PyDict_New() );

	HashPosition pos;
	zval ** entry = NULL;
	char * key = NULL;
	uint keyLen = 0;
	ulong index = 0;

	zend_hash_internal_pointer_reset_ex( HASH_OF( zendDict ), &pos );

	while (zend_hash_get_current_data_ex(
			HASH_OF( zendDict ), ( void** )&entry, &pos ) == SUCCESS)
	{
		zend_hash_get_current_key_ex( HASH_OF( zendDict ),
			&key, &keyLen, &index, 0, &pos );

		PyObjectAutoPtr pKey( key ?
			PyString_FromString( key ) :
			PyLong_FromLong( index ) );

		PyObject * valueObj = NULL;
		mapPHPTypeToPy( *entry, &valueObj );

		if (valueObj == NULL)
		{
			return;
		}

		PyObjectAutoPtr pValue( valueObj );

		PyDict_SetItem( pDict, pKey, pValue );

		zend_hash_move_forward_ex( HASH_OF( zendDict ), &pos );
	}

	Py_INCREF( pDict.get() );
	*ppReturnValue = pDict;
}

} // namespace PyTypeMapping

// py_type_mapping.cpp
