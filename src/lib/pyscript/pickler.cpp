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

#include "pickler.hpp"
#include "cstdmf/debug.hpp"
#include "pyscript/pyobject_plus.hpp"

DECLARE_DEBUG_COMPONENT2( "Script", 0)

// -----------------------------------------------------------------------------
// Section: FailedUnpickle
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( FailedUnpickle )

PY_BEGIN_METHODS( FailedUnpickle )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( FailedUnpickle )
PY_END_ATTRIBUTES()


// -----------------------------------------------------------------------------
// Section: Pickler
// -----------------------------------------------------------------------------

int 		Pickler::s_refCount			= 0;
PyObject * 	Pickler::s_pPickleMethod	= NULL;
PyObject * 	Pickler::s_pUnpickleMethod	= NULL;

/**
 * 	This static method initialises Pickler. It loads the Python pickle module
 * 	and queries methods needed.
 */
bool Pickler::init()
{
	PyObject * pPickleModule = PyImport_ImportModule( "cPickle" );

	if (pPickleModule != NULL)
	{
		if (!s_pPickleMethod)
		{
			s_pPickleMethod = PyObject_GetAttrString( pPickleModule, "dumps" );
			if (!s_pPickleMethod)
			{
				ERROR_MSG( "Pickler::init: Failed to get dumps\n" );
				PyErr_PrintEx(0);
			}
		}

		if (!s_pUnpickleMethod)
		{
			s_pUnpickleMethod = PyObject_GetAttrString(pPickleModule, "loads");

			if (!s_pUnpickleMethod)
			{
				ERROR_MSG( "Pickler::init: Failed to get loads\n" );
				PyErr_PrintEx(0);
			}
		}

		Py_DECREF( pPickleModule );
	}
	else
	{
		PyErr_PrintEx(0);
#ifdef _WIN32
		ERROR_MSG( "Failed to import cPickle module.\n" );
#else
		ERROR_MSG( "Failed to import cPickle module. "
			"Is your resource path set correctly?\n"
			"\tThis requires scripts/common/lib-dynload/cPickle.so to be "
			"relative to a resource path (usually bigworld/res).\n" );
#endif
	}

	return (s_pPickleMethod != NULL) && (s_pUnpickleMethod != NULL);
}


/**
 * 	This method pickles the given object into a binary string.
 *
 * 	@param pObj		The Python object to pickle.
 *	@param output 	The string to fill with the pickled string.
 * 	@return			True if successfully pickled, otherwise false.
 */
std::string Pickler::pickle( PyObject* pObj )
{
	if (!pObj)
	{
		ERROR_MSG( "Pickler::pickle: attempting to pickle NULL\n" );
	}
	else if (pObj->ob_type == &FailedUnpickle::s_type_)
	{
		return static_cast< FailedUnpickle * >( pObj )->pickleData();
	}
	else if (s_pPickleMethod != NULL)
	{
		PyObject * pResult;

		pResult = PyObject_CallFunction( s_pPickleMethod, "(Oi)", pObj, 2 );

		if (pResult == NULL)
		{
			ERROR_MSG( "Pickler::pickle: failed to pickle object\n" );
			PyErr_Print();
		}
		else
		{
			std::string str;

			str.assign( PyString_AsString( pResult ), PyString_Size( pResult ));
			Py_DECREF( pResult );

			return str;
		}
	}

	return "";
}


/**
 * 	This method unpickles the given binary string into a python object.
 *
 * 	@param str	The pickled string
 * 	@return		Object to pickle
 */
PyObject* Pickler::unpickle( const std::string& str )
{
	PyObject* pResult = NULL;

	if (s_pUnpickleMethod != NULL)
	{
		pResult = PyObject_CallFunction( s_pUnpickleMethod, "(s#)",
				str.data(), str.length() );

		if (pResult == NULL)
		{
			NOTICE_MSG( "Pickler::unpickle: "
					"Failed to unpickle. Using stand-in object.\n" );
			PyErr_Print();
		}
	}

	if (pResult == NULL)
	{
		pResult = new FailedUnpickle( str );
	}

	return pResult;
}


/**
 *	This method deletes the global pickle and unpickle methods.
 */
void Pickler::finalise()
{	
	Py_XDECREF(s_pPickleMethod);
	Py_XDECREF(s_pUnpickleMethod);
	s_pPickleMethod = NULL;
	s_pUnpickleMethod = NULL;
}

// pickler.cpp
