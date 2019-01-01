/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "shared_data.hpp"

#include "cstdmf/binary_stream.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pickler.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

/**
 *	This static function is used to implement operator[] for the scripting
 *	object.
 */
int SharedData::s_ass_subscript( PyObject * self,
		PyObject * index, PyObject * value )
{
	return ((SharedData *) self)->ass_subscript( index, value );
}

/**
 *	This static function is used to implement operator[] for the scripting
 *	object.
 */
PyObject * SharedData::s_subscript( PyObject * self, PyObject * index )
{
	return ((SharedData *) self)->subscript( index );
}


/**
 * 	This static function returns the number of entities in the system.
 */
Py_ssize_t SharedData::s_length( PyObject * self )
{
	return ((SharedData *) self)->length();
}


/**
 *	This structure contains the function pointers necessary to provide
 * 	a Python Mapping interface.
 */
static PyMappingMethods g_sharedDataMapping =
{
	SharedData::s_length,			// mp_length
	SharedData::s_subscript,		// mp_subscript
	SharedData::s_ass_subscript	// mp_ass_subscript
};


/*~ function SharedData has_key
 *  @components{ base, cell }
 *  has_key reports whether a value with a specific key is listed in this
 *	SharedData object.
 *  @param key The key to be searched for.
 *  @return A boolean. True if the key was found, false if it was not.
 */
/*~ function SharedData keys
 *  @components{ base, cell }
 *  keys returns a list of the keys of all of the shared data in this object.
 *  @return A list containing all of the keys.
 */
/*~ function SharedData items
 *  @components{ base, cell }
 *  items returns a list of the items, as (key, value) pairs.
 *  @return A list containing all of the (key, value) pairs.
 */
/*~ function SharedData values
 *  @components{ base, cell }
 *  values returns a list of all the values associated with this object.
 *  @return A list containing all of the values.
 */
/*~ function SharedData get
 *  @components{ base, cell }
 *  This method returns the value with the input key or the default value if not
 *	found.
 *
 *  @return The found value.
 */
PY_TYPEOBJECT_WITH_MAPPING( SharedData, &g_sharedDataMapping )

PY_BEGIN_METHODS( SharedData )
	PY_METHOD( has_key )
	PY_METHOD( keys )
	PY_METHOD( items )
	PY_METHOD( values )
	PY_METHOD( get )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( SharedData )
PY_END_ATTRIBUTES()


// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	The constructor for SharedData.
 */
SharedData::SharedData( SharedDataType dataType,
		SharedData::SetFn setFn,
		SharedData::DelFn delFn,
		SharedData::OnSetFn onSetFn,
		SharedData::OnDelFn onDelFn,
		Pickler * pPickler,
		PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	dataType_( dataType ),
	setFn_( setFn ),
	delFn_( delFn ),
	onSetFn_( onSetFn ),
	onDelFn_( onDelFn ),
	pPickler_( pPickler )
{
	pMap_ = PyDict_New();
}


/**
 *	Destructor.
 */
SharedData::~SharedData()
{
	Py_DECREF( pMap_ );
}


// -----------------------------------------------------------------------------
// Section: Script Mapping Related
// -----------------------------------------------------------------------------

/**
 *	This method overrides the PyObjectPlus method.
 */
PyObject * SharedData::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method finds the value associated with the input key.
 *
 * 	@param key 	The key of the value to find.
 *
 *	@return	A new reference to the object associated with the given key.
 */
PyObject * SharedData::subscript( PyObject * key )
{
	PyObject * pObject = PyDict_GetItem( pMap_, key );
	if (pObject == NULL)
	{
		PyErr_SetObject( PyExc_KeyError, key );
	}
	else
	{
		Py_INCREF( pObject );
	}

	return pObject;
}


/**
 *	This method sets a new shared data value.
 *
 * 	@param key 	The key of the value to set.
 * 	@param value 	The new value.
 *
 *	@return	0 on success, otherwise false.
 */
int SharedData::ass_subscript( PyObject* key, PyObject * value )
{
	std::string pickledKey = this->pickle( key );

	if (PyErr_Occurred())
	{
		ERROR_MSG( "SharedData::ass_subscript: Failed to pickle key\n" );
		return -1;
	}

	PyObjectPtr pUnpickledKey( this->unpickle( pickledKey ),
			PyObjectPtr::STEAL_REFERENCE );

	if (!pUnpickledKey ||
			(PyObject_Compare( key, pUnpickledKey.get() ) != 0))
	{
		PyErr_SetString( PyExc_TypeError,
				"Unpickled key is not equal to original key" );
		return -1;
	}

	if (value == NULL)
	{
		int result = PyDict_DelItem( pMap_, key );

		if (result == 0)
		{
			(*delFn_)( pickledKey, dataType_ );
		}

		return result;
	}
	else
	{
		std::string pickledValue = this->pickle( value );

		if (PyErr_Occurred())
		{
			ERROR_MSG( "SharedData::ass_subscript: Failed to pickle value\n" );
			return -1;
		}

		int result = PyDict_SetItem( pMap_, key, value );

		if (result == 0)
		{
			(*setFn_)( pickledKey, pickledValue, dataType_ );
		}

		return result;
	}
}


/**
 * 	This method returns the number of data entries.
 */
int SharedData::length()
{
	return PyDict_Size( pMap_ );
}


/**
 * 	This method returns true if the given entry exists.
 */
PyObject * SharedData::py_has_key( PyObject* args )
{
	return PyObject_CallMethod( pMap_, "has_key", "O", args );
}


/**
 * 	This method returns the value with the input key or the default value if not
 *	found.  
 *
 * 	@param args		A python tuple containing the arguments.
 */
PyObject * SharedData::py_get( PyObject* args )
{
	return PyObject_CallMethod( pMap_, "get", "O", args );
}


/**
 * 	This method returns a list of all the keys of this object.
 */
PyObject* SharedData::py_keys(PyObject* /*args*/)
{
	return PyDict_Keys( pMap_ );
}


/**
 * 	This method returns a list of all the values of this object.
 */
PyObject* SharedData::py_values( PyObject* /*args*/ )
{
	return PyDict_Values( pMap_ );
}


/**
 * 	This method returns a list of tuples of all the keys and values of this
 *	object.
 */
PyObject* SharedData::py_items( PyObject* /*args*/ )
{
	return PyDict_Items( pMap_ );
}


// -----------------------------------------------------------------------------
// Section: Misc.
// -----------------------------------------------------------------------------

/**
 *	This method sets a local SharedData entry from the input value.
 */
bool SharedData::setValue( const std::string & key, const std::string & value )
{
	PyObject * pKey = this->unpickle( key );
	PyObject * pValue = this->unpickle( value );
	bool isOkay = true;

	if (pKey && pValue)
	{
		if (PyDict_SetItem( pMap_, pKey, pValue ) == -1)
		{
			ERROR_MSG( "SharedData::setValue: Failed to set value\n" );
			PyErr_Clear();
			isOkay = false;
		}
		else if (onSetFn_)
		{
			(*onSetFn_)( pKey, pValue, dataType_ );
		}
	}
	else
	{
		ERROR_MSG( "SharedData::setValue: "
						"Unpickle failed. Invalid key(%p) or value(%p)\n",
				   pKey, pValue );
		PyErr_Print();
		isOkay = false;
	}

	Py_XDECREF( pKey );
	Py_XDECREF( pValue );

	return isOkay;
}


/**
 *	This method deletes a local SharedData entry from the input value.
 */
bool SharedData::delValue( const std::string & key )
{
	PyObject * pKey = this->unpickle( key );
	bool isOkay = true;

	if (pKey)
	{
		if (PyDict_GetItem( pMap_, pKey ) && 
			PyDict_DelItem( pMap_, pKey ) == -1)
		{
			// ERROR_MSG( "SharedData::delValue: Failed to delete key.\n" );
			// Probably because we were the one to delete it.
			PyErr_Clear();

			isOkay = false;
		}
		else if (onDelFn_)
		{
			(*onDelFn_)( pKey, dataType_ );
		}

		Py_DECREF( pKey );
	}
	else
	{
		ERROR_MSG( "SharedData::delValue: Invalid key to delete\n" );
		PyErr_Print();

		isOkay = false;
	}

	return isOkay;
}


/**
 *	This method adds the dictionary to the input stream.
 */
bool SharedData::addToStream( BinaryOStream & stream ) const
{
	uint32 size = PyDict_Size( pMap_ );
	stream << size;

	PyObject * pKey;
	PyObject * pValue;
	Py_ssize_t pos = 0;

	while (PyDict_Next( pMap_, &pos, &pKey, &pValue ))
	{
		stream << this->pickle( pKey ) << this->pickle( pValue );
	}

	MF_ASSERT( !PyErr_Occurred() );

	return true;
}


/**
 *	This method pickles the input object.
 */
std::string SharedData::pickle( PyObject * pObj ) const
{
	return pPickler_->pickle( pObj );
}


/**
 *	This method unpickles the input data.
 */
PyObject * SharedData::unpickle( const std::string & str ) const
{
	return pPickler_->unpickle( str );
}

// shared_data.cpp
