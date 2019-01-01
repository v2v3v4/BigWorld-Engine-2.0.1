/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "cstdmf/config.hpp"

#if ENABLE_WATCHERS

#include "Python.h"

#include "cstdmf/debug.hpp"
DECLARE_DEBUG_COMPONENT2( "Script", 0 )

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/watcher.hpp"

#include "pyobject_plus.hpp"
#include "pywatcher.hpp"
#include "script.hpp"
#include "py_output_writer.hpp"


// ----------------------------------------------------------------------------
// Section: SequenceWatcher and MappingWatcher
// ----------------------------------------------------------------------------


/**
 *	Function to return a watcher for PySequences
 */
Watcher & PySequence_Watcher()
{
	static WatcherPtr pWatchMe = NULL;
	if (pWatchMe == NULL)
	{
		pWatchMe = new SequenceWatcher<PySequenceSTL>( *(PySequenceSTL*)NULL );

		/*pWatchMe->addChild( "*", new DataWatcher<PyObjectPtrRef>(
			*(PyObjectPtrRef*)NULL ) );*/
		pWatchMe->addChild( "*", new PyObjectWatcher(
			*(PyObjectPtrRef*)NULL ) );
	}
	return *pWatchMe;
}


/**
 *	Function to return a watcher for PyMappings
 */
Watcher & PyMapping_Watcher()
{
	static WatcherPtr pWatchMe = NULL;
	if (pWatchMe == NULL)
	{
		pWatchMe = new MapWatcher<PyMappingSTL>( *(PyMappingSTL*)NULL );

		/*pWatchMe->addChild( "*", new DataWatcher<PyObjectPtrRef>(
			*(PyObjectPtrRef*)NULL ) );*/
		pWatchMe->addChild( "*", new PyObjectWatcher(
			*(PyObjectPtrRef*)NULL ) );
	}
	return *pWatchMe;
}




// ----------------------------------------------------------------------------
// Section: PyObjectWatcher
// ----------------------------------------------------------------------------


// Static initialiser of our fallback watcher
SmartPointer<DataWatcher<PyObjectPtrRef> >	PyObjectWatcher::fallback_ = 
	new DataWatcher<PyObjectPtrRef>( *(PyObjectPtrRef*)NULL, Watcher::WT_READ_WRITE );


/**
 *	Constructor
 */
PyObjectWatcher::PyObjectWatcher( PyObjectPtrRef & popr ) : popr_( popr )
{
}


/**
 *	Get method
 */
bool PyObjectWatcher::getAsString( const void * base, const char * path,
	std::string & result, std::string & desc, Watcher::Mode & mode ) const
{
	// TODO: check this method makes sense and document (also in getAsStream)
	// - decref correct?
	PyObjectPtrRef & popr = *(PyObjectPtrRef*)( ((const uintptr)&popr_) + ((const uintptr)base) );

	// get out the PyObject *
	PyObject * pObject = popr;

	bool ret = false;

	// check against our registry to see if this is a directory
	Watcher * pSpecialWatcher = this->getSpecialWatcher( pObject );

	if (pSpecialWatcher != NULL)
	{	// yes it is - let it handle everything then
		ret = pSpecialWatcher->getAsString( &pObject, path, result, desc, mode );
	}		// (should return "<DIR>" for empty paths
	else
	{
		// the path had better be empty...
		if (isEmptyPath( path ))
		{	// call the datawatcher
			ret = static_cast<Watcher&>(*fallback_).getAsString(
				&popr, path, result, desc, mode );
		}
	}

	Py_DECREF( pObject );
	return ret;
}


/*
 *	Set method
 */
bool PyObjectWatcher::setFromString( void * base, const char * path,
	const char * valueStr )
{
	PyObjectPtrRef & popr = *(PyObjectPtrRef*)( ((const uintptr)&popr_) + ((const uintptr)base) );

	bool ret = false;

	// see if it's this watcher we're interested in or one of its chillun
	if (isEmptyPath( path ))
	{
		// ok, set away then
		ret = static_cast<Watcher&>(*fallback_).setFromString( &popr, path, valueStr );
	}
	else
	{
		// it had better be a directory...
		PyObject * pObject = popr;
		Watcher * pSpecialWatcher = this->getSpecialWatcher( pObject );

		if (pSpecialWatcher != NULL)
		{
			ret = pSpecialWatcher->setFromString( &pObject, path, valueStr );
		}

		Py_DECREF( pObject );
	}

	return ret;
}

bool PyObjectWatcher::getAsStream( const void * base, const char * path,
	WatcherPathRequestV2 & pathRequest ) const
{
	// TODO: check this method makes sense and document (also in getAsString)
	// - decref correct?
	PyObjectPtrRef & popr = *(PyObjectPtrRef*)( ((const uintptr)&popr_) + ((const uintptr)base) );

	// get out the PyObject *
	PyObject * pObject = popr;

	bool ret = false;

	// check against our registry to see if this is a directory
	Watcher * pSpecialWatcher = this->getSpecialWatcher( pObject );

	if (pSpecialWatcher != NULL)
	{	// yes it is - let it handle everything then
		ret = pSpecialWatcher->getAsStream( &pObject, path, pathRequest );
	}		// (should return "<DIR>" for empty paths
	else
	{
		// the path had better be empty...
		if (isEmptyPath( path ))
		{	// call the datawatcher
			ret = static_cast<Watcher&>(*fallback_).getAsStream(
				&popr, path, pathRequest );
		}
	}

	Py_DECREF( pObject );
	return ret;
}

/*
 *	Set method
 */
bool PyObjectWatcher::setFromStream( void * base, const char * path,
	WatcherPathRequestV2 &pathRequest )
{
	// TODO: check this method makes sense and document
	// - decref correct?

	PyObjectPtrRef & popr = *(PyObjectPtrRef*)( ((const uintptr)&popr_) + ((const uintptr)base) );

	bool ret = false;

	// see if it's this watcher we're interested in or one of its chillun
	if (isEmptyPath( path ))
	{
		// ok, set away then
		ret = static_cast<Watcher&>(*fallback_).setFromStream( &popr, path,
			pathRequest );
	}
	else
	{
		// it had better be a directory...
		PyObject * pObject = popr;
		Watcher * pSpecialWatcher = this->getSpecialWatcher( pObject );

		if (pSpecialWatcher != NULL)
		{
			ret = pSpecialWatcher->setFromStream( &pObject, path,
				pathRequest );
		}

		Py_DECREF( pObject );
	}

	return ret;
}


/**
 *	Visit method
 */
bool PyObjectWatcher::visitChildren( const void * base, const char * path,
	WatcherPathRequest & pathRequest )
{
	PyObjectPtrRef & popr = *(PyObjectPtrRef*)( ((const uintptr)&popr_) + ((const uintptr)base) );

	// get out the PyObject *
	PyObject * pObject = popr;

	bool ret = false;

	// check against our registry to see if this is a directory
	Watcher * pSpecialWatcher = this->getSpecialWatcher( pObject );

	// can only pass it down (for whatever action) if it's a directory
	if (pSpecialWatcher != NULL)
	{
		ret = pSpecialWatcher->visitChildren( &pObject, path,
											  pathRequest );
	}
	// the datawatcher can't visit children (as it's not a directory)

	Py_DECREF( pObject );
	return ret;
}



/**
 *	Add child method
 */
bool PyObjectWatcher::addChild( const char * /*path*/, Watcher * /*pChild*/,
	void * /*withBase*/ )
{
	// Neither this nor any of the sub watchers support the
	// 'addChild' method, sorry [decision may be revised ... maybe
	// the list of special watchers should not be static, but rather
	// stored in each PyObjectWatcher as its children... with the
	// path as the hex of the object type's address ... hmmmm]
	return false;
}


/**
 *	This static method determines whether or not the object in pObject
 *	has its own special watcher. If not, then the default watcher is
 *	used, which just calls the object's 'Str' method. This watcher is
 *	not used for setAsString on the object itself - the default watcher
 *	is always used for that.
 */
Watcher * PyObjectWatcher::getSpecialWatcher( PyObject * pObject )
{
	if (PyString_Check( pObject ))
	{
		return NULL;
	}
	if (PySequenceSTL::Check( pObject ))
	{
		return &PySequence_Watcher();
	}
	else if(PyMappingSTL::Check( pObject ))
	{
		return &PyMapping_Watcher();
	}

	return NULL;
}


// -----------------------------------------------------------------------------
// Section: Misc
// -----------------------------------------------------------------------------

/*~ function BigWorld.setWatcher
 *	@components{ all }
 *
 *	This function interacts with the watcher debugging system, allowing the
 *	setting of watcher variables.
 *
 *	The watcher debugging system allows various variables to be displayed and
 *  updated from within the runtime.
 *
 *	Watchers are specified in the C-code using the MF_WATCH macro.  This takes
 *	three arguments, the first is a string specifying the path (parent groups
 *	followed by the item name, separated by forward slashes), the second is the
 *	expression to watch.  The third argument specifies such things as whether
 *	it is read only or not, and has a default of read-write.
 *
 *	The setWatcher function in Python allows the setting of a particular
 *	read-write value.
 *	For example:
 *
 *	@{
 *	>>>	BigWorld.setWatcher( "Client Settings/Terrain/draw", 0 )
 *	@}
 *
 *	@param	path	the path to the item to modify.
 *	@param	val		the new value for the item.
 */
/**
 *	This script function is used to set watcher values. The script function
 *	takes a string to the watcher value to set and the value to set it too.
 */
static PyObject * py_setWatcher( PyObject * args )
{
	char * path = NULL;
	PyObject * pValue = NULL;

	if (!PyArg_ParseTuple( args, "sO", &path, &pValue ))
	{
		return NULL;
	}

	PyObject * pStr = PyObject_Str( pValue );

	if (pStr)
	{
		char * pCStr = PyString_AsString( pStr );

		if (!Watcher::rootWatcher().setFromString( NULL, path, pCStr ))
		{
			PyErr_Format( PyExc_TypeError, "Failed to set %s to %s.",
				path, pCStr );
			Py_DECREF( pStr );
			return NULL;
		}

		Py_DECREF( pStr );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Invalid second argument." );
		return NULL;
	}

	Py_Return;
}
PY_MODULE_FUNCTION( setWatcher, BigWorld )


/*~ function BigWorld.getWatcher
 *	@components{ all }
 *
 *	This function interacts with the watcher debugging system, allowing the
 *	retrieval of watcher variables.
 *
 *	The watcher debugging system allows various variables to be displayed and
 *	updated from within the runtime.
 *
 *	Watchers are specified in the C-code using the MF_WATCH macro.  This takes
 *	three arguments, the first is a string specifying the path (parent groups
 *	followed by the item name, separated by forward slashes),
 *	the second is the expression to watch.  The third argument specifies such
 *	things as whether it is read only or not, and has a default of read-write.
 *
 *	The getWatcher function in Python allows the getting of a particular value.
 *	For example:
 *
 *	@{
 *	>>>	BigWorld.getWatcher( "Client Settings/Terrain/draw" )
 *	1
 *	@}
 *
 *	@param	path	the path to the item to modify.
 *
 *	@return			the value of that watcher variable.
 */
/**
 *	This script function is used to get watcher values. The script function
 *	takes a string to the watcher value to get. It returns the associated value
 *	as a string.
 */
static PyObject * py_getWatcher( PyObject * args )
{
	char * path = NULL;

	if (!PyArg_ParseTuple( args, "s", &path ))
	{
		return NULL;
	}

	std::string result;
	std::string desc;
	Watcher::Mode mode;

	if (!Watcher::rootWatcher().getAsString( NULL, path, result, desc, mode ))
	{
		PyErr_Format( PyExc_TypeError, "Failed to get %s.", path );
		return NULL;
	}

	return PyString_FromString( result.c_str() );
}
PY_MODULE_FUNCTION( getWatcher, BigWorld )


/*~ function BigWorld.getWatcherDir
 *	@components{ all }
 *
 *	This function interacts with the watcher debugging system, allowing the
 *	inspection of a directory watcher.
 *
 *	@param	path	the path to the item to modify.
 *
 *	@return			a list containing an entry for each child watcher. Each
 *					entry is a tuple of child type, label and value. The child
 *					type is 1 for read-only, 2 for read-write and 3 for a
 *					directory.
 */
namespace
{

	class MyVisitor : public WatcherVisitor
	{
	public:
		MyVisitor() : pList_( PyList_New( 0 ) )	{}
		~MyVisitor()							{ Py_DECREF( pList_ ); }

		virtual bool visit( Watcher::Mode mode,
			const std::string & label,
			const std::string & desc,
			const std::string & valueStr )
		{
			PyObject * pTuple = PyTuple_New( 3 );
			PyTuple_SET_ITEM( pTuple, 0, Script::getData( int(mode) ) );
			PyTuple_SET_ITEM( pTuple, 1, Script::getData( label ) );
			PyTuple_SET_ITEM( pTuple, 2, Script::getData( valueStr ) );

			PyList_Append( pList_, pTuple );
			Py_DECREF( pTuple );

			return true;
		}

		PyObject * pList_;
	};

}

static PyObject * py_getWatcherDir( PyObject * args )
{
	char * path = NULL;

	if (!PyArg_ParseTuple( args, "s", &path ))
	{
		return NULL;
	}

	MyVisitor visitor;

	if (!Watcher::rootWatcher().visitChildren( NULL, path, visitor ))
	{
		PyErr_Format( PyExc_TypeError, "Failed to get %s.", path );
		return NULL;
	}

	Py_INCREF( visitor.pList_ );

	return visitor.pList_;

}
PY_MODULE_FUNCTION( getWatcherDir, BigWorld )


// Anonymous namespace
namespace
{

/*
 * Convert a stream containing a known data type into a Python object.
 *
 * @param stream BinaryIStream containing a SET2 watcher protocol data.
 * @param type   Watcher data type contained in the stream.
 *
 * @returns A new Python object containing the streamed data on success,
 *          Py_None on error.
 */
PyObject * extractPyObjectFromStream( BinaryIStream & stream )
{
	// Stream off the tuple element type
	char cType;
	stream >> cType;

	if (!stream.error())
	{
		WatcherDataType type = (WatcherDataType)cType;

		switch (type)
		{
			case WATCHER_TYPE_INT:
			{
				int64 value;
				if (watcherStreamToValueType( stream, value, type ))
				{
					return Script::getData( value );
				}
				break;
			}
			case WATCHER_TYPE_UINT:
			{
				uint64 value;
				if (watcherStreamToValueType( stream, value, type ))
				{
					return Script::getData( value );
				}
				break;
			}
			case WATCHER_TYPE_FLOAT:
			{
				double value;
				if (watcherStreamToValueType( stream, value, type ))
				{
					return Script::getData( value );
				}
				break;
			}
			case WATCHER_TYPE_BOOL:
			{
				bool value;
				if (watcherStreamToValueType( stream, value, type ))
				{
					return Script::getData( value );
				}
				break;
			}
			case WATCHER_TYPE_STRING:
			{
				std::string value;
				if (watcherStreamToValueType( stream, value, type ))
				{
					return Script::getData( value );
				}
				break;
			}
			default:
			{
				ERROR_MSG( "Unsupported type (%d) encountered while "
							"parsing input stream\n", type );
			}
		}
	}

	return NULL;
}

bool putPyObjectOnStream( BinaryOStream & stream, PyObject *pyObj,
							WatcherDataType & type )
{
	bool status = true;

	switch (type)
	{
		case WATCHER_TYPE_INT:
		case WATCHER_TYPE_UINT:
		{
			int64 value;
			if (Script::setData(pyObj, value, "int64") != -1)
			{
				watcherValueToStream( stream, value, Watcher::WT_READ_ONLY );
			}
			else
			{
				ERROR_MSG( "PyWatcher: putPyObjectOnStream failed to convert "
					"PyObject into an int64.\n" );

				if (PyErr_Occurred())
				{
					PyErr_Print();
				}

				status = false;
			}
			break;
		}
		case WATCHER_TYPE_FLOAT:
		{
			double value;
			if (Script::setData(pyObj, value, "double") != -1)
			{
				watcherValueToStream( stream, value, Watcher::WT_READ_ONLY );
			}
			else
			{
				ERROR_MSG( "PyWatcher: putPyObjectOnStream failed to convert "
					"PyObject into a double.\n" );

				if (PyErr_Occurred())
				{
					PyErr_Print();
				}

				status = false;
			}
			break;
		}
		case WATCHER_TYPE_BOOL:
		{
			bool value;
			if (Script::setData(pyObj, value, "bool") != -1)
			{
				watcherValueToStream( stream, value, Watcher::WT_READ_ONLY );
			}
			else
			{
				ERROR_MSG( "PyWatcher: putPyObjectOnStream failed to convert "
					"PyObject into a bool.\n" );

				if (PyErr_Occurred())
				{
					PyErr_Print();
				}

				status = false;
			}
			break;
		}
		case WATCHER_TYPE_STRING:
		{
			std::string value;
			if (Script::setData(pyObj, value, "std::string") != -1)
			{
				watcherValueToStream( stream, value, Watcher::WT_READ_ONLY );
			}
			else
			{
				ERROR_MSG( "PyWatcher: putPyObjectOnStream failed to convert "
					"PyObject into an std::string.\n" );

				if (PyErr_Occurred())
				{
					PyErr_Print();
				}

				status = false;
			}
			break;
		}
		case WATCHER_TYPE_UNKNOWN:
		{
			// In this case we will only pack on the type mode and a size of 0,
			// no data will be transferred.
			stream << (uchar)WATCHER_TYPE_UNKNOWN;
			stream << (uchar)Watcher::WT_READ_ONLY;
			stream << (uchar)0;
			break;
		}
		default:
		{
			ERROR_MSG( "PyWatcher: putPyObjectOnStream: Unsupported type (%d) "
					"encountered while generating output stream\n", type );
			status = false;
		}
	}

	return status;
}


void getPyObjectWatcherData( PyObject * pyObj,
							WatcherDataType & type, int32 & size )
{
	if (pyObj == NULL)
	{
		type = WATCHER_TYPE_UNKNOWN;
		size = 0;
	}
	else if (PyBool_Check( pyObj ))
	{
		type = WATCHER_TYPE_BOOL;
		size = sizeof(bool);
	}
	else if (PyInt_Check( pyObj ) ||
		PyLong_Check( pyObj ))
	{
		type = WATCHER_TYPE_INT;
		size = sizeof(int64);
	}
	else if (PyFloat_Check( pyObj ))
	{
		type = WATCHER_TYPE_FLOAT;
		size = sizeof(double);
	}
	else if (PyString_Check( pyObj ))
	{
		type = WATCHER_TYPE_STRING;
		size = PyString_Size( pyObj );
	}
	else
	{
		type = WATCHER_TYPE_UNKNOWN;
		size = 0;
	}

	return;
}

} // Anonymous namespace


/**
 *	This class is used to associate a watcher variable with
 *	Python scripts. The functionality is similar to FunctionWatcher.
 *
 *	The getFunction is called to retrieve the value of watcher
 *	variable, while setFunction, if exists, is called when
 *	watcher variable is modified.
 *
 * 	If setFunction is absent the watcher will be read-only.
 *
 */
class PyAccessorWatcher : public Watcher
{
public:
	/// @name Construction/Destruction
	//@{
	PyAccessorWatcher( const char * path, PyObject * getFunction,
			PyObject * setFunction = NULL, const char * desc = NULL ) :
		path_(path),
		getFunction_( getFunction ),
		setFunction_( setFunction )
	{
		Py_INCREF( getFunction_ );
		Py_XINCREF( setFunction_ );
		if (desc)
		{
			std::string s( desc );
			comment_ = s;
		}
	}

	~PyAccessorWatcher()
	{
		// Only release these references if Python is still alive. We are able
		// to destruct after Python has been shutdown because the watcher system
		// is closed down at static de-init time.
		if (Py_IsInitialized())
		{
			Py_XDECREF( getFunction_ );
			Py_XDECREF( setFunction_ );
		}
		else if (getFunction_ != NULL || setFunction_ != NULL)
		{
			WARNING_MSG("~PyAccessorWatcher: There were still Python references to a watcher after Python was finalized ('%s'). "
						"You should release all Python references to watchers on shutdown - did you get a Python exception?\n", path_.c_str());
		}
	}

	//@}

protected:
	/// @name Overrides from Watcher.
	//@{
	// Override from Watcher.
	virtual bool getAsString( const void * base, const char * path,
		std::string & result, std::string & desc, Watcher::Mode & mode ) const
	{
		if (isEmptyPath( path ))
		{
			Py_INCREF( getFunction_ );
			PyObject * pValue = Script::ask( getFunction_,
				PyTuple_New( 0 ), "PyAccessorWatcher" );

			if (pValue)
			{
				PyObject * pValStr = PyObject_Str( pValue );

				if (pValStr)
				{
					result = PyString_AsString( pValStr );
					mode =
						(setFunction_ != NULL) ? WT_READ_WRITE : WT_READ_ONLY;
					desc = comment_;
					Py_DECREF( pValStr );

					return true;
				}
				else
				{
					PyErr_PrintEx(0);
				}
				Py_DECREF( pValue );
			}
			else
			{
				PyErr_PrintEx(0);
			}
		}

		return false;
	}

	// Override from Watcher.
	virtual bool setFromString( void * base, const char * path,
		const char * valueStr )
	{
		bool result = false;

		if (isEmptyPath( path ) && (setFunction_ != NULL))
		{
			Py_INCREF( setFunction_ );
			result = Script::call( setFunction_,
				Py_BuildValue( "(s)", valueStr ), "PyAccessorWatcher" );
		}

		return result;
	}

	// Override from Watcher.
	virtual bool getAsStream( const void * base, const char * path,
		WatcherPathRequestV2 & pathRequest ) const
	{
		if (isEmptyPath( path ))
		{
			Py_INCREF( getFunction_ );
			PyObject * pValue = Script::ask( getFunction_,
				PyTuple_New( 0 ), "PyAccessorWatcher" );

			if (pValue)
			{
				PyObject * pValStr = PyObject_Str( pValue );

				if (pValStr)
				{
					Watcher::Mode mode =
						(setFunction_ != NULL) ? WT_READ_WRITE : WT_READ_ONLY;
					watcherValueToStream( pathRequest.getResultStream(), 
							PyString_AsString( pValStr ), mode );
					pathRequest.setResult( comment_, mode, this, base );
					Py_DECREF( pValStr );

					return true;
				}
				else
				{
					PyErr_PrintEx(0);
				}
				Py_DECREF( pValue );
			}
			else
			{
				PyErr_PrintEx(0);
			}
		}
		else if (isDocPath( path ))
		{
			// <watcher>/__doc__ handling
			Watcher::Mode mode = Watcher::WT_READ_ONLY;
			watcherValueToStream( pathRequest.getResultStream(),
					comment_, mode );
			pathRequest.setResult( comment_, mode, this, base );
			return true;
		}

		return false;
	}


	// Override from Watcher.
	virtual bool setFromStream( void * base, const char * path,
		WatcherPathRequestV2 & pathRequest )
	{
		bool status = false;

		if (isEmptyPath( path ) && (setFunction_ != NULL))
		{
			// De-stream the result only if the path is empty to help when
			// supporting the <watcher>/__doc__ et al watchers.
			BinaryIStream *istream = pathRequest.getValueStream();
			if (istream == NULL)
			{
				ERROR_MSG( "PyAccessorWatcher::setFromStream: NULL "
						"value stream within WatcherPathRequest.\n" );
				return false;
			}


			PyObject *pyObj = extractPyObjectFromStream( *istream );
			if (pyObj == NULL)
			{
				ERROR_MSG( "PyAccessorWatcher::setFromStream: NULL object "
						"extracted from stream. Unable to set watcher.\n" );
				return false;
			}

			Py_INCREF( setFunction_ );
			// Status of the set operation is appended to the result stream
			// prior to generating the final packet in watcher_nub.cpp
			status = Script::call( setFunction_,
				PyTuple_Pack( 1, pyObj ), "PyAccessorWatcher" );
			Py_DECREF( pyObj );
			pyObj = NULL;

			if (!status)
			{
				ERROR_MSG( "PyAccessorWatcher::setFromStream: Failed to set "
						   "python watcher value from set function.\n" );
			}
			else
			{
				// Get the result from the set operation now
				if (!this->getAsStream( base, path, pathRequest))
				{
					ERROR_MSG( "PyAccessorWatcher::setFromStream: Failed to "
						   "retrieve python watcher value after set.\n" );
					status = false;
				}
			}

		}

		return status;
	}

	//@}

private:

	std::string path_;

	PyObject * getFunction_;
	PyObject * setFunction_;
};



/**
 * Python Function Watcher.
 *
 * This class provides the ability for python functions to be exposed as
 * watcher values which can be called with arbitrary arguments and return
 * a result along with anything print to standard output to the caller.
 */
class PyFunctionWatcher : public CallableWatcher
{
public:
	/// @name Construction/Destruction
	//@{
	PyFunctionWatcher( ExposeHint hint, const char * comment ) :
		CallableWatcher( hint, comment ),
		pFunction_( NULL ) {}


	// Place an argument name / argument type into the function watcher's
	// arg list, performing type checking from python types -> watcher
	// protocol types in the process.
	bool addArg( PyObject * pArgType, const char * argName = "" )
	{
		WatcherDataType type = WATCHER_TYPE_STRING;

		if ((PyTypeObject *)pArgType == &PyBool_Type)
		{
			type = WATCHER_TYPE_BOOL;
		}
		else if (((PyTypeObject *)pArgType == &PyInt_Type) ||
				((PyTypeObject *)pArgType == &PyLong_Type))
		{
			type = WATCHER_TYPE_INT;
		}
		else if ((PyTypeObject *)pArgType == &PyFloat_Type)
		{
			type = WATCHER_TYPE_FLOAT;
		}
		else if ((PyTypeObject *)pArgType == &PyString_Type)
		{
			type = WATCHER_TYPE_STRING;
		}
		else
		{
			// If we can't determine the type of the object in the argument
			// tuple, set an error then bail out.
			PyObject *pyStr = PyObject_Str( pArgType );
			const char *typeStr = (pyStr != NULL) ?
					PyString_AsString( pyStr ) : "(unknown)";

			PyErr_Format( PyExc_RuntimeError,
				"Invalid type %s encountered while processing argument list.",
				typeStr);

			Py_XDECREF( pyStr );
			return false;
		}

		this->CallableWatcher::addArg( type, argName );

		return true;
	}

	bool init( const char * path, PyObject * function, PyObject * argList )
	{
		if (!PyCallable_Check( function ) || 
			(argList == NULL) || !PySequence_Check( argList ))
		{
			PyErr_SetString( PyExc_RuntimeError, "Invalid arguments" );
			return false;
		}

		Py_ssize_t seqSize = PySequence_Size( argList );

		for (Py_ssize_t i = 0; i < seqSize; i++)
		{
			PyObject * pArg = PySequence_GetItem( argList, i );
			PyObjectPtr argHolder( pArg, PyObjectPtr::STEAL_REFERENCE );

			if (pArg == NULL)
			{
				ERROR_MSG( "PyFunctionWatcher::init: Unable to get "
					"element %"PRIzd" from argList sequence.\n", i );

				// NB: not consuming any error messages here, so they
				//     are returned to the calling python context
				return false;
			}


			if (PyType_Check( pArg ))
			{
				if (!this->addArg( pArg ))
					return false;
			}
			else if (PyTuple_Check( pArg ) && (PyTuple_Size( pArg ) == 2))
			{
				PyObject * pName = PySequence_GetItem( pArg, 0 );
				PyObject * pType = PySequence_GetItem( pArg, 1 );

				if ((pName == NULL) || (pType == NULL))
				{
					PyErr_Format( PyExc_RuntimeError,
							"Failed to decode argument tuple for element %"PRIzd" "
							"of arg list sequence.", i );

					return false;
				}

				if (!PyString_Check( pName ) || !PyType_Check( pType ))
				{
					PyErr_Format( PyExc_ValueError,
							"Invalid argument tuple types for element %"PRIzd" "
							"of arg list sequence.", i );
					return false;

				}

				char *argName = PyString_AsString( pName );
				if (!argName)
					return false;

				// Add the extracted argName and type to the arg list.
				if (!this->addArg( pType, argName ))
					return false;

			}
			else
			{
				PyErr_Format( PyExc_RuntimeError,
					"Invalid type at element %"PRIzd" of arg list sequence.",
					i );
				return false;
			}

		}

		pFunction_ = function;

		return true;
	}

	//@}

protected:

	/**
	 *	This class is a helper to collect script output to send in the reply.
	 */
	class PyFunctionOutputWriter : public PyOutputWriter
	{
	public:
		PyFunctionOutputWriter( std::string * pStr, PyObject *pOldOutput ) :
			PyOutputWriter( NULL, false ),
			pStr_( pStr ),
			pOldOutput_( pOldOutput )
		{
			Py_INCREF( pOldOutput_);
		}

		~PyFunctionOutputWriter()
		{
			Py_DECREF( pOldOutput_ );
		}

	protected:
		void printMessage( const std::string & msg )
		{
			// push this message onto the std::string
			if (pStr_)
			{
				pStr_->append( msg );
			}

			// Now make sure it's still written to stdout
			PyObject * pResult = PyObject_CallMethod( pOldOutput_, "write", "s#", msg.c_str(), msg.length() );
			if (!pResult)
			{
				PyErr_Print();
			}
			else
			{
				Py_DECREF( pResult );
			}
		}

	private:
		std::string * pStr_;	/// Pointer the string to output messages to
		PyObject *pOldOutput_;	/// The python output stream to forward to
	};



	/**
	 * Converts a watcher protocol stream to a Python object.
	 *
	 */
	bool watcherStreamToValue( BinaryIStream & stream, PyObject* & pyArgs )
	{
		pyArgs = NULL;

		char type;
		stream >> type;
		int size = stream.readPackedInt();

		if (stream.error() || ((WatcherDataType)type != WATCHER_TYPE_TUPLE))
		{
			ERROR_MSG("PyFunctionWatcher::watcherStreamToValue: Function "
					  "Watcher called with non-tuple data as argument list. "
					  "Received %d, Wanted %d\n", (WatcherDataType)type,
					  WATCHER_TYPE_TUPLE);
			// Read over the rest of the current entry
			stream.retrieve( size );
			return false;
		}

		// Read off the number of entries in the tuple
		int numArgs = stream.readPackedInt();

		// The python tuple that will become our argument list
		pyArgs = PyTuple_New( numArgs );
		if (!pyArgs)
		{
			//TODO: PyErr_Clear() - clears error buff / PyErr_Print() 
			ERROR_MSG("PyFunctionWatcher::watcherStreamToValue: Failed to "
					  "create tuple for argument list. Cannot continue.\n");
			return false;
		}

		for (int i=0; i<numArgs; i++)
		{
			// now destream the individual entries.
			PyObject *pyObj = extractPyObjectFromStream( stream );
			if (pyObj == NULL)
			{
				// TODO: this should probably cleanup the tuple and then return
				//       false. if one arg is bad, the set operation should not
				//       continue.
				ERROR_MSG( "PyFunctionWatcher::watcherStreamToValue: NULL "
						   "returned from argument list extraction for "
						   "element %d.\n", i );
				Py_INCREF( Py_None );
				pyObj = Py_None;
			}

			MF_VERIFY( PyTuple_SetItem( pyArgs, i, pyObj ) == 0 );
		}

		return true;
	}

	// Override from Watcher.
	virtual bool setFromStream( void * base, const char * path,
		WatcherPathRequestV2 &pathRequest )
	{
		bool status = false;
		PyObject * pArgs = NULL;

		// Grab the stream to write to
		BinaryIStream *istream = pathRequest.getValueStream();
		BinaryOStream & resultStream = pathRequest.getResultStream();

		// Now if all is OK proceed with the function call
		if (istream &&
			this->watcherStreamToValue( *istream, pArgs ) &&
			isEmptyPath( path ) && 
			(pFunction_ != NULL))
		{
			PyObject * pModule = PyImport_ImportModule( "sys" );
			if (pModule == NULL)
			{
				ERROR_MSG( "PyFunctionWatcher::setFromStream: Failed to "
							"import sys module. Unable to continue.\n" );
				Py_DECREF( pArgs );
				return false;
			}

			// Redirect stdout + stderr to our output writer
			std::string outputStr;
			PyObject * pStdOutOld = PyObject_GetAttrString( pModule, "stdout" );
			PyObject * pStdErrOld = PyObject_GetAttrString( pModule, "stderr" );
			PyObject * pStdOutNew = new PyFunctionOutputWriter(
												&outputStr, pStdOutOld );
			PyObject * pStdErrNew = new PyFunctionOutputWriter(
												&outputStr, pStdErrOld );
			PyObject_SetAttrString( pModule, "stdout", pStdOutNew );
			PyObject_SetAttrString( pModule, "stderr", pStdErrNew );

			// Call the Python function now with our compiled arg list
			Py_INCREF( pFunction_.getObject() );

			// Returning NULL here indicates a failure on the call. Rather than
			// bailing out, we should try to extract an error from python to
			// send back to the caller.
			PyObject * pReturn = Script::ask( pFunction_.getObject(), pArgs,
										"PyFunctionWatcher", false, true );
			// DECREF'ed in Script::call
			pArgs = NULL;

			// Redirect output back to the original output handlers
			PyObject_SetAttrString( pModule, "stdout", pStdOutOld );
			PyObject_SetAttrString( pModule, "stderr", pStdErrOld );

			// Set the return status of the set operation to 'fail'
			if (pReturn == NULL)
			{
				status = false;
			}

			Watcher::Mode mode = WT_READ_ONLY;
			// Fill in the result stream with the status from our run.
			// This unfortunately involves stuffing around with MemoryStreams
			// in order to determine the size of the entire message so we can
			// prefix it.
			resultStream << (uint8)WATCHER_TYPE_TUPLE;
			resultStream << (uint8)mode;

			// Determine the size of the output stream
			int resultSize = 0;
			 // Count (# tuple elements)
			resultSize  = BinaryOStream::calculatePackedIntSize( 2 );

			// Output stream
			resultSize += this->tupleElementStreamSize( outputStr.length() );

			// Return value
			WatcherDataType objectType = WATCHER_TYPE_UNKNOWN;
			int32 objectSize = 0;
			getPyObjectWatcherData( pReturn, objectType, objectSize );
			resultSize += this->tupleElementStreamSize( objectSize );

			resultStream.writePackedInt( resultSize );
			//TODO: add warnings re: size here > 64k
			// Now we know the size of the 2 string messages, keep generating
			// the result stream.
			resultStream.writePackedInt( 2 ); // # of tuple elements

			// First tuple element (stdout/stderr)
			watcherValueToStream( resultStream, outputStr,
				Watcher::WT_READ_ONLY );

			// Second tuple element (return data)
			putPyObjectOnStream( resultStream, pReturn, objectType );
			Py_XDECREF( pReturn );

			Py_DECREF( pModule );
			Py_DECREF( pStdOutNew );
			Py_DECREF( pStdOutOld );
			Py_DECREF( pStdErrNew );
			Py_DECREF( pStdErrOld );

			pathRequest.setResult( this->getComment(), mode, this, base );
		}

		Py_XDECREF( pArgs );

		return status;
	}

	//@}

private:

	PyObjectPtr pFunction_;
};



/*~ function BigWorld.addWatcher
 *	@components{ all }
 * 
 *
 *	This function interacts with the watcher debugging system, allowing the
 *	creation of watcher variables.
 *
 *	The addWatcher function in Python allows the creation of a particular
 *	watcher variable.
 *	For example:
 *
 *	@{
 *	>>> maxBandwidth = 20000
 *	>>> def getMaxBps( ):
 *	>>>     return str(maxBandwidth)
 *	>>> def setMaxBps( bps ):
 *	>>>     maxBandwidth = int(bps)
 *	>>>     setBandwidthPerSecond( int(bps) )
 *	>>>
 *	>>> BigWorld.addWatcher( "Comms/Max bandwidth per second", getMaxBps, setMaxBps )
 *	@}
 *
 *	This adds a watcher variable under "Comms" watcher directory. The function getMaxBps
 *  is called to obtain the watcher value and setMaxBps is called when the watcher is modified.
 *
 *	@param	path	the path to the item to create.
 *	@param	getFunction	the function to call when retrieving the watcher variable.
 *						This function takes no argument and returns a string representing
 *						the watcher value.
 *	@param	setFunction	(optional)the function to call when setting the watcher variable
 *						This function takes a string argument as the new watcher value.
 *						This function does not return a value. If this function does not
 * 						exist the created watcher is a read-only watcher variable.
 */
static PyObject * py_addWatcher( PyObject * args )
{
	char * path = NULL;
	char * desc = NULL;
	PyObject * getFunction = NULL;
	PyObject * setFunction = NULL;

	if (!PyArg_ParseTuple( args, "sO|Os", &path, &getFunction, &setFunction, &desc ))
	{
		return NULL;
	}

	if (!PyCallable_Check( getFunction ))
	{
		PyErr_SetString( PyExc_TypeError, "get function must be callable" );
		return NULL;
	}

	if ( (setFunction != NULL) && !PyCallable_Check( setFunction ) )
	{
		PyErr_SetString( PyExc_TypeError, "set function must be callable" );
		return NULL;
	}

	//TODO PyAccessorWatcher() + init() for better error handling + return status

	Watcher::rootWatcher().addChild( 
		path, new PyAccessorWatcher( path, getFunction, setFunction, desc ) );

	Py_Return;

}
PY_MODULE_FUNCTION( addWatcher, BigWorld )


/*~ function BigWorld.addFunctionWatcher
 *	@components{ all }
 * 
 *
 *	This function interacts with the watcher debugging system, allowing the
 *	creation of watcher variables.
 *
 *	The addFunctionWatcher function in Python allows the creation of a watcher
 *  path that can be queried as well as set with a variable length argument
 *  list.
 *
 *	For example:
 *
 *  BigWorld.addFunctionWatcher( "command/addGuards", util.patrollingGuards,
 *        [("Number of guards to add", int)], BigWorld.EXPOSE_LEAST_LOADED,
 *        "Add an arbitrary number of patrolling guards into the world.") 
 *
 *	@param	path	The path the watcher will be created as. Function watchers
 *					are	generally placed under the "command/" path structure.
 *	@param	function	The function to call to with the provided argument list.
 *  @param  argumentList	The argument list (provided as a tuple) of types and
 *							descriptions that the function accepts. See the
 *							example for more details about its structure.
 *  @param  exposedHint The default exposure hint to be used when forwarding
 *                      the watcher call from Managers to Components.
 *	@param	desc	(Optional) A description of the function watcher behaviour
 *					and usage. While this field is optional, it is strongly
 *					recommended to provide this description in order to allow
 *					usage through WebConsole to be more intuitive.
 */
static PyObject * py_addFunctionWatcher( PyObject * args )
{
	char * path = NULL;
	char * desc = NULL;
	int exposedHint = 0;
	PyObject * function = NULL;
	PyObject * argumentList = NULL;

	if (!PyArg_ParseTuple( args, "sOOi|s:addFunctionWatcher", 
			&path, &function, &argumentList, &exposedHint, &desc ))
	{
		return NULL;
	}

	if (!PyCallable_Check( function ) )
	{
		PyErr_SetString( PyExc_TypeError, "Function must be callable" );
		return NULL;
	}

	PyFunctionWatcher *pyFW = new PyFunctionWatcher(
			(CallableWatcher::ExposeHint)exposedHint,
			desc );

	if (pyFW)
	{
		if (!pyFW->init( path, function,
				argumentList ))
		{
			delete pyFW;
			pyFW = NULL;
		}
	}

	// Attempt to add the new watcher as a child
	if (pyFW)
	{
		if (Watcher::rootWatcher().addChild( path, pyFW ))
		{
			Py_Return;
		}

		ERROR_MSG( "py_addFunctionWatcher: Failed to add watcher '%s'\n",
				path );
		PyErr_SetString( PyExc_RuntimeError, "Failed to add watcher to the "
				"watcher tree." );
	}

	return NULL;
}
PY_MODULE_FUNCTION( addFunctionWatcher, BigWorld )

/*~ function BigWorld.delWatcher
 *	@components{ all }
 *	This function interacts with the watcher debugging system, allowing the
 *	deletion of watcher variables.
 *
 *	@param path the path of the watcher to delete.
 */
static PyObject * py_delWatcher( PyObject * args )                              
{                                                                               
	char * path = NULL;                                                     
	if (!PyArg_ParseTuple( args, "s", &path ))                              
	{                                                                       
		return NULL;                                                    
	}                                                                       
	if (!Watcher::rootWatcher().removeChild(path))                          
	{                                                                       
		PyErr_Format( PyExc_ValueError, "Watcher path not found: %s.", path);
		return NULL;                                                    
	}                                                                       
	Py_Return;                                                              
}                                                                               
PY_MODULE_FUNCTION( delWatcher, BigWorld )                                      


int PyWatcher_token = 1;
int PyScript_token = PyWatcher_token;

PyObject * SimplePythonWatcher::pythonChildBase( PyObject * pPyObject, const char * path )
{
	char * pSeparator = strchr( (char*)path, WATCHER_SEPARATOR );

	unsigned int compareLength =
		(pSeparator == NULL) ? strlen( path ) : (pSeparator - path);

	std::string head = std::string( path, compareLength );

	if (PyMapping_Check( pPyObject ))
	{
		PyObject * pPyChild = 
			PyMapping_GetItemString( pPyObject, 
									 const_cast< char * >( head.c_str() ) );

		if (pPyChild != NULL)
		{
			return pPyChild;
		}
	}

	if ( PySequence_Check( pPyObject ) )
	{
		std::stringstream stream;
		stream << head;
		stream.seekg( 0, std::ios::beg );
		Py_ssize_t index;
		stream >> index;
		if (!stream.bad())
		{
			PyObject * pPyChild = PySequence_GetItem( pPyObject, index);
			if (pPyChild != NULL)
			{
				return pPyChild;
			}
		}
	}

	return PyObject_GetAttrString( pPyObject, head.c_str() );
}

bool SimplePythonWatcher::getAsString( const void * base, const char * path,
								 std::string & result, std::string & desc, 
								 Watcher::Mode & mode ) const
{
	PyObject *pPyObject = (PyObject *)base;

	if (pPyObject == NULL)
	{
		result = "NULL";
		mode = WT_READ_ONLY;
		desc = comment_;
		return true;
	}

	if (isEmptyPath( path ))
	{
		if ( PyString_Check( pPyObject ) )
		{
			result = PyString_AsString( pPyObject );
			mode = WT_READ_ONLY;
			desc = comment_;
			return true;
		}
		else if ( PyMapping_Check( pPyObject ) || 
				  PySequence_Check( pPyObject ) ||
				  PyObject_HasAttrString( pPyObject, "__dict__" ) )
		{
			result = "<DIR>";
			mode = WT_DIRECTORY;
			desc = comment_;
			return true;
		}
		else
		{
			PyObject *pPyString = PyObject_Str( pPyObject );
			if (PyString_Check(	pPyString ))
			{
				result = PyString_AsString( pPyObject );
				Py_DECREF( pPyString );
			}
			else
			{
				result = "";
			} 
			mode = WT_READ_ONLY;
			desc = comment_;
			return true;
		}
	}
	else
	{
		PyObject * pPyChild = pythonChildBase( pPyObject, path );
		if ( pPyChild )
		{
			// We recurse here because PyWatcher is its own child
			bool returnValue = this->getAsString( pPyChild, 
												  DirectoryWatcher::tail( path ), 
												  result, desc, mode );

			Py_DECREF( pPyChild );
			return returnValue;
		}
		else
		{
			result = "";
			mode = WT_READ_ONLY;
			desc = comment_;
			return true;	
		}
	}
}

bool SimplePythonWatcher::setFromString( void * base, const char * path,
								   const char * valueStr )
{
	return true;
}

bool SimplePythonWatcher::getAsStream( const void * base, const char * path,
								 WatcherPathRequestV2 & pathRequest ) const
{
	PyObject *pPyObject = (PyObject *)base;
	Watcher::Mode mode = WT_READ_ONLY;

	if (pPyObject == NULL)
	{
		watcherValueToStream( pathRequest.getResultStream(), "NULL", 
							  mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}

	if (isEmptyPath( path ))
	{
		if ( PyString_Check( pPyObject ) )
		{
			watcherValueToStream( pathRequest.getResultStream(), 
								  PyString_AsString( pPyObject ), mode );
			pathRequest.setResult( comment_, mode, this, base );
			return true;

		}
		else if ( PyMapping_Check( pPyObject ) || 
				  PySequence_Check( pPyObject ) ||
				  PyObject_HasAttrString( pPyObject, "__dict__" ) )
		{
			mode = WT_DIRECTORY;
			watcherValueToStream( pathRequest.getResultStream(), "<DIR>", 
								  mode );
			pathRequest.setResult( comment_, mode, this, base );
			return true;
		}
		else
		{
			PyObject *pPyString = PyObject_Str( pPyObject );
			if (PyString_Check(	pPyString ))
			{
				watcherValueToStream( pathRequest.getResultStream(), 
									  PyString_AsString( pPyString ), 
									  mode );
				Py_DECREF( pPyString );
			}
			else
			{
				watcherValueToStream( pathRequest.getResultStream(), "", mode );
			}
			pathRequest.setResult( comment_, mode, this, base );
			return true;
		}
	}
	else
	{
		PyObject * pPyChild = pythonChildBase( pPyObject, path );
		if ( pPyChild )
		{
			// We recurse here because PyWatcher is its own child
			bool returnValue = this->getAsStream( pPyChild, DirectoryWatcher::tail( path ),
												  pathRequest );
			Py_DECREF( pPyChild );
			return returnValue;
		}
		else
		{
			watcherValueToStream( pathRequest.getResultStream(), "", mode );
			pathRequest.setResult( comment_, mode, this, base );
			return true;
		}
	}
}


bool SimplePythonWatcher::setFromStream( void * base, const char * path,
								   WatcherPathRequestV2 & pathRequest )
{
	return true;
}

bool SimplePythonWatcher::visitChildren( const void * base, const char * path,
								   WatcherPathRequest & pathRequest )
{
	PyObject *pPyObject = (PyObject *)base;

	bool handled = false;

	if (pPyObject == NULL)
	{
		return false;
	}
	else if (isEmptyPath(path))
	{
		// If we have a mapping, get the keys and values
		PyObject *pKeys = NULL, *pValues = NULL, *pMapping = NULL;

		const char * message = "error examining python in watchers";

		if (PyMapping_Check( pPyObject ))
		{
			pMapping = pPyObject;
			Py_INCREF( pMapping );
		}
		else if (PyObject_HasAttrString( pPyObject, "__dict__" ))
		{
			pMapping = PyObject_GetAttrString( pPyObject, "__dict__" );
		}

		if (pMapping != NULL)
		{
			PyObject * slow;
			slow = PyMapping_Keys( pMapping );
			pKeys = PySequence_Fast( slow, message );
			Py_DECREF( slow );
			slow = PyMapping_Values( pMapping );
			pValues = PySequence_Fast( slow, message );
			Py_DECREF( slow );
			Py_DECREF( pMapping );
		}
		// If not, get a fast sequence
		else if (PySequence_Check( pPyObject ))
		{
			pValues = PySequence_Fast( pPyObject, message );
		}

		Py_ssize_t count = PySequence_Fast_GET_SIZE( pValues );

		pathRequest.addWatcherCount( count );

		for (Py_ssize_t iter = 0; iter < count; iter++ )
		{
			std::string label;
			// If we have a key use it for the label
			if (pKeys != NULL)
			{
				PyObject * pKey = 
					PyObject_Str(PySequence_Fast_GET_ITEM( pKeys, iter ));
				label = std::string(PyString_AsString( pKey ));
				Py_DECREF( pKey );
			}
			// If not, just use the index
			else
			{
				std::stringstream stream;
				stream << iter;
				stream.seekg( 0, std::ios::beg );
				stream >> label;
			}

			if (!pathRequest.
				addWatcherPath(PySequence_Fast_GET_ITEM( pValues, iter ),
							   NULL, label, *this ))
			{
				break;
			}
		}

		Py_DECREF( pValues );
		if (pKeys != NULL)
		{
			Py_DECREF( pKeys );
		}
		handled = true;
	}
	else
	{
		PyObject * pChild = pythonChildBase( pPyObject, path );

		if (pChild != NULL)
		{
			handled = this->
				visitChildren( pChild, DirectoryWatcher::tail( path ), 
							   pathRequest );
			Py_DECREF( pChild );
		}
	}

	return handled;
}


#else // ENABLE_WATCHERS

// Add sub functions so that these can still be called in script.

static PyObject * py_setWatcher( PyObject * args )			
{															
	Py_Return;												
}															
PY_MODULE_FUNCTION( setWatcher, BigWorld )					

static PyObject * py_getWatcher( PyObject * args )			
{															
	Py_Return;												
}															
PY_MODULE_FUNCTION( getWatcher, BigWorld )					

static PyObject * py_getWatcherDir( PyObject * args )		
{															
	Py_Return;												
}															

PY_MODULE_FUNCTION( getWatcherDir, BigWorld )				

static PyObject * py_delWatcher( PyObject * args )			
{															
	Py_Return;												
}															
PY_MODULE_FUNCTION( delWatcher, BigWorld )					


static PyObject * py_addWatcher( PyObject * args )
{
	Py_Return;
}
PY_MODULE_FUNCTION( addWatcher, BigWorld )


int PyScript_token = 1;



#endif // ENABLE_WATCHERS

// pywatcher.cpp
