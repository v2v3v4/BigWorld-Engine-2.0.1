/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include <Python.h>
// Python C API includes
#include <cStringIO.h>

#include "functions.hpp"

#include "py_object_auto_ptr.hpp"
#include "py_type_mapping.hpp"

#include <string>


namespace // anonymous
{
// ----------------------------------------------------------------------------
// Section: Helper function declarations
// ----------------------------------------------------------------------------

void PyErr_ZendError( const char * msg = NULL );

const std::string PyErr_GetString();

long debug_verbosity();

// ----------------------------------------------------------------------------
// Section: Macros for debugging
// ----------------------------------------------------------------------------

enum DebugLevels
{
	DBG_LEVEL_ERROR,
	DBG_LEVEL_INFO,
	DBG_LEVEL_TRACE
};

#define ERROR_MSG  	zend_error
#define INFO_MSG 	if (debug_verbosity() >= DBG_LEVEL_INFO) 	zend_error
#define TRACE_MSG 	if (debug_verbosity() >= DBG_LEVEL_TRACE) 	zend_error


// ----------------------------------------------------------------------------
// Section: Helper class definitions
// ----------------------------------------------------------------------------

/**
 *	Calls efree() on the given pointer when destroyed. Useful when created as
 *	an automatic variable.
 */
class ZendAutoPtr
{
public:
	ZendAutoPtr( void* ptr ):
		ptr_( ptr )
	{
	}

	~ZendAutoPtr()
	{
		if (ptr_)
		{
			efree( ptr_ );
		}
	}

private:
	void * 	ptr_;
};



} // namespace (anonymous)

// ----------------------------------------------------------------------------
// Section: Destructor functions for PHP resources
// ----------------------------------------------------------------------------

/**
 *	The destructing function for a Python object PHP resource.
 */
ZEND_RSRC_DTOR_FUNC( PyObject_ResourceDestructionHandler )
{
	PyObject * pyObj = reinterpret_cast< PyObject * >( rsrc->ptr );
	Py_XDECREF( pyObj );
}


// ----------------------------------------------------------------------------
// Section: Zend function implementations
// ----------------------------------------------------------------------------

/**
 *	bw_logon( $username, $password, $allow_already_logged_on=FALSE )
 *
 *	Logs on a user given an input password. The method returns TRUE if the
 *	logon attempt was successful, otherwise it returns an error message string.
 *
 *	@param username 					The username.
 *	@param password 					The password.
 *	@param allow_already_logged_on 		If true, then if the entity is already
 *										checked out from the database, then no
 *										error will be raised, if false, then an
 *										error will be raised.
 *
 *	@return TRUE on success, or a string error message.
 */
PHP_FUNCTION( bw_logon )
{
	const char * username 	= NULL;
	int usernameLen 		= 0;
	const char * password 	= NULL;
	int passwordLen 		= 0;
	zend_bool allow_already_logged_on = 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"ss|b",
			&username, &usernameLen,
			&password, &passwordLen,
			&allow_already_logged_on ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_logon(%s, ********, allow_already_logged_on=%s)",
		username, allow_already_logged_on ? "true" : "false" );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_logon: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod( BWG( bwModule ),
		"logOn", "ssb",
		username, password, (char)allow_already_logged_on ) );

	if (!pResult)
	{
		std::string errString = PyErr_GetString();
		INFO_MSG( E_NOTICE, "bw_logon failed due to %s\n", errString.c_str() );
		RETURN_STRING( const_cast< char * >( errString.c_str() ),
			/*duplicate:*/1 );
	}

	RETURN_TRUE;
}


/**
 *	bw_look_up_entity_by_name( $entityType, $entityName )
 *
 *	Queries the BigWorld server for a specific entity with the given type and
 *	name. If the entity exists and an entity base has been loaded into the
 *	BigWorld system, then it returns an entity mailbox pointing to that can be
 *	used by bw_exec() to invoke methods on that entity base. If the entity
 *	exists but has not been loaded, then TRUE is returned. If the entity does
 *	not exist, then FALSE is returned.
 *
 *	@param entityType 	The entity type string.
 *	@param entityName 	The identifier string for the entity to be looked up.
 *	@return 			A mailbox if the entity is currently checked out of the
 *						database, TRUE if the entity exists but has not been
 *						checked out, or FALSE if the netity does not exist in
 *						the database.
 */
PHP_FUNCTION( bw_look_up_entity_by_name )
{
	const char * entityType = NULL;
	int entityTypeLen 		= 0;
	const char * entityName = NULL;
	int entityNameLen 		= 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"ss",
			&entityType, &entityTypeLen,
			&entityName, &entityNameLen ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_look_up_entity_by_name(%s, %s)",
		entityType, entityName );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_look_up_entity_by_name: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod(
		BWG( bwModule ),
		"lookUpEntityByName", "ss",
		entityType, entityName ) );

	if (!pResult)
	{
		std::string errString = PyErr_GetString();
		RETURN_STRING( const_cast< char* >( errString.c_str() ), 
			/*duplicate:*/1 );
	}

	PyTypeMapping::mapPyTypeToPHP( pResult, return_value );
}


/**
 *	bw_look_up_entity_by_dbid( $entityType, $databaseID )
 *
 *	Queries the BigWorld server for a specific entity with the given type and
 *	database ID. If the entity exists and an entity base has been loaded into
 *	the BigWorld system, then it returns an entity mailbox pointing to that can
 *	be used by bw_exec() to invoke methods on that entity base. If the entity
 *	exists but has not been loaded, then TRUE is returned. If the entity does
 *	not exist, then FALSE is returned.
 *
 *	@param entityType 	The entity type string.
 *	@param databaseID 	The database ID for the entity to be looked up.
 *	@return 			A mailbox if the entity is currently checked out of the
 *						database, TRUE if the entity exists but has not been
 *						checked out, or FALSE if the netity does not exist in
 *						the database.
 */
PHP_FUNCTION( bw_look_up_entity_by_dbid )
{
	const char* entityType 	= NULL;
	int entityTypeLen 		= 0;
	long dbId 				= 0;
	int entityNameLen 		= 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"sl",
			&entityType, &entityTypeLen,
			&dbId ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_look_up_entity_by_dbid(%s, %ld)",
		entityType, dbId );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_look_up_entity_by_dbid: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod(
		BWG( bwModule ),
		"lookUpEntityByDBID", "sl",
		entityType, dbId ) );

	if (!pResult)
	{
		std::string errString = PyErr_GetString();
		RETURN_STRING( const_cast<char*>( errString.c_str() ), 1 );
	}

	PyTypeMapping::mapPyTypeToPHP( pResult, return_value );
}


#if 0
/**
 *	bw_test(...)
 *
 *	Test function. Not registered.
 */
PHP_FUNCTION( bw_test )
{

}
#endif


/**
 *	bw_exec($mailbox, $methodName, ...)
 *
 *	Executes a remote method on an entity mailbox given as a PHP resource, as
 *	returned by bw_look_up_entity_by_name() and bw_look_up_entity_by_dbid().
 *
 *	This method has a variable size argument lists, which are converted to
 *	Python objects before being sent to the Python remote method. The arguments
 *	are defined in the entity definition file, and should match the order in
 *	which they appear.
 *
 *	If the remote method contains return values, then these are converted to
 *	PHP types. In particular, return values are always returned as a PHP array,
 *	with the keys being the return value names as defined in the function
 *	specification in the entity definition file.
 *
 *	If there are no return values, then NULL is returned.
 *
 *	@param mailbox resource
 *	@param methodName
 *
 */
PHP_FUNCTION( bw_exec )
{
	int numArgs = ZEND_NUM_ARGS();

	if (numArgs < 2)
	{
		WRONG_PARAM_COUNT;
	}

	// Allocate memory for holding the variable args
	zval *** argArray = reinterpret_cast< zval *** >(
		safe_emalloc( ZEND_NUM_ARGS(), sizeof( zval ** ), 0 ) );

	if (!argArray)
	{
		ERROR_MSG( E_ERROR,
			"bw_exec: could not allocate memory for arugments" );
		RETURN_FALSE;
	}

	// argArray is freed when _argArray goes out of scope
	ZendAutoPtr _argArray( argArray );

	// Get variable argument list.
	if (FAILURE == zend_get_parameters_array_ex( numArgs, argArray ))
	{
		WRONG_PARAM_COUNT;
	}

	// Get the Python mailbox object from the PHP resource, which is the first
	// arg.
	PyObject * pMailbox = NULL;
	// If there's an issue retrieving the resource, ZEND_FETCH_RESOURCE prints
	// an error and returns FALSE from this function.
	ZEND_FETCH_RESOURCE( pMailbox, PyObject *,
		argArray[0],
		-1, // default resource ID
		"PyObject", le_pyobject );

	// Create trace string to print out via zend_error
	PyObjectAutoPtr pMailboxString( PyObject_Str( pMailbox ) );
	std::string traceString( "bw_exec(" );
	traceString += PyString_AsString( pMailboxString );
	traceString += ", ";

	// Get the method name, the second arg.
	if (Z_TYPE_PP( argArray[1] ) != IS_STRING)
	{
		zend_error( E_ERROR, "bw_exec: Method name is not a string" );
		RETURN_FALSE;
	}

	const char * methodName = Z_STRVAL_PP( argArray[1] );
	traceString += methodName;

	// see if the method exists!
	PyObjectAutoPtr pMethod( PyObject_GetAttrString( pMailbox,
		const_cast<char*>( methodName ) ) );

	if (!pMethod)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	if (!PyCallable_Check( pMethod ))
	{
		PyErr_Format( PyExc_TypeError,
				"Mailbox does not have method called %s\n",
			methodName );
		RETURN_FALSE;
	}

	// get the rest of the method arguments
	PyObjectAutoPtr pMethodArgs( PyTuple_New( numArgs - 2 ) );

	if (!pMethodArgs)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	for (int i = 2; i < numArgs; ++i)
	{
		PyObject * pyArg = NULL;
		// map each php typed argument to a newly referenced
		// python object which is stolen by PyTuple_SetItem
		PyTypeMapping::mapPHPTypeToPy( *argArray[i], &pyArg );

		if (!pyArg)
		{
			zend_error( E_ERROR, "Could not map %dth item to Python object "
					"for call to %s",
				i - 2, methodName );
			RETURN_FALSE;
		}

		PyObjectAutoPtr pArgString( PyObject_Str( pyArg ) );
		traceString += ", ";
		traceString += PyString_AsString( pArgString );

		// PyTuple_SetItem steals a reference to pyArg
		if (0 != PyTuple_SetItem( pMethodArgs, i - 2, pyArg ))
		{
			zend_error( E_ERROR, "Could not set %dth item in "
					"argument tuple for call to %s",
				i - 2, methodName );
			RETURN_FALSE;
		}
	}

	traceString += ")";
	INFO_MSG( E_NOTICE, traceString.c_str() );

	// make the call, get back a dictionary of return values
	PyObjectAutoPtr pResult( PyObject_CallObject( pMethod, pMethodArgs ) );

	if (!pResult)
	{
		std::string errString = PyErr_GetString();
		RETURN_STRING( const_cast< char * >( errString.c_str() ), 1 );
	}

	// map it back to PHP typed return value
	PyTypeMapping::mapPyTypeToPHP( pResult, return_value );
}


/**
 *	bw_reset_network_interface($port)
 *
 *	Recreates the network interface, and binds to the given port if it is
 *	non-zero, otherwise it rebinds to a random port.
 *
 *	@param port the port number, or 0 for a random port.
 */
PHP_FUNCTION( bw_reset_network_interface )
{
	long port = 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"l", &port ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_reset_network_interface(%d)", port );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_reset_network_interface: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod( BWG( bwModule ),
		"resetNetworkInterface", "l",
		port ) );

	if (!pResult)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


/**
 *	bw_serialise( $mailbox )
 *
 *	Serialises the given mailbox.
 *
 *	@param mailbox the mailbox resource for the entity
 *	@return string the serialised mailbox string
 */
PHP_FUNCTION( bw_serialise )
{
	zval * resource = NULL;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&resource ))
	{
		RETURN_FALSE;
	}

	PyObject * pMailbox = NULL;
	ZEND_FETCH_RESOURCE( pMailbox, PyObject *, &resource, -1, "PyObject",
		le_pyobject );

	PyObjectAutoPtr pMailboxString( PyObject_Str( pMailbox ) );

	INFO_MSG( E_NOTICE, "bw_serialise(%s)",
		PyString_AsString( pMailboxString ) );

	PyObjectAutoPtr pResult( PyObject_CallMethod(
		pMailbox, "serialise", NULL  ) );

	if (!pResult)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	RETURN_STRING( PyString_AsString( pResult ), 1 );
}


/**
 *	bw_deserialise( $string )
 *
 *	Deserialises the given serialised mailbox string.
 *
 *	@param string the serialised mailbox string
 *	@return resource the deserialised mailbox
 */
PHP_FUNCTION( bw_deserialise )
{
	const char * serialised = NULL;
	int serialisedLen = 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
				"s", &serialised, &serialisedLen ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_deserialise(%s)", serialised );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_deserialise: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod( BWG( bwModule ),
		"deserialise", "s#", serialised, serialisedLen ) );

	if (!pResult)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	PyTypeMapping::mapPyTypeToPHP( pResult, return_value );
}

/**
 *	bw_pyprint( $pyResource )
 *
 *	Returns a string of the PyResource's string representation, e.g. str(obj)
 *
 *	@param pyResource 	the Python object PHP resource
 *	@return string
 */
PHP_FUNCTION( bw_pystring )
{
	zval * resource = NULL;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"r", &resource ))
	{
		RETURN_FALSE;
	}

	PyObject * pyObj = NULL;
	ZEND_FETCH_RESOURCE( pyObj, PyObject *, &resource, -1, "PyObject",
		le_pyobject );

	PyObjectAutoPtr pResult( PyObject_Str( pyObj ) );
	INFO_MSG( E_NOTICE, "bw_pystring(%s)", PyString_AsString( pResult ) );

	RETURN_STRING( PyString_AsString( pResult ), 1 );
}

/**
 *	bw_set_keep_alive_seconds( $keepAliveIntervalSeconds )
 *
 *	Set the keep-alive period for the given mailbox.
 *
 *	@param keepAliveIntervalSeconds the keep-alive interval in seconds.
 *	@return bool
 */
PHP_FUNCTION( bw_set_keep_alive_seconds )
{
	zval * mailboxResource = NULL;
	long keepAlive = 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"rl", &mailboxResource, &keepAlive ))
	{
		RETURN_FALSE;
	}

	PyObject * pMailbox = NULL;
	ZEND_FETCH_RESOURCE( pMailbox, PyObject *, &mailboxResource, -1,
			"PyObject", le_pyobject );

	PyObjectAutoPtr pMailboxString( PyObject_Str( pMailbox ) );

	INFO_MSG( E_NOTICE, "bw_set_keep_alive_seconds( %s, %ld )",
		PyString_AsString( pMailboxString ), keepAlive );

	PyObjectAutoPtr pKeepAlive( PyLong_FromLong( keepAlive ) );

	if (-1 == PyObject_SetAttrString( pMailbox,
			"keepAliveSeconds", pKeepAlive ))
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 *	bw_get_keep_alive_seconds()
 *
 *	Get the keep-alive period for the given mailbox.
 *
 *	@return int
 */
PHP_FUNCTION( bw_get_keep_alive_seconds )
{
	zval * mailboxResource = NULL;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
		"r", &mailboxResource ))
	{
		RETURN_FALSE;
	}

	PyObject * pMailbox = NULL;
	ZEND_FETCH_RESOURCE( pMailbox, PyObject *, &mailboxResource, -1,
		"PyObject", le_pyobject );

	PyObjectAutoPtr pMailboxString( PyObject_Str( pMailbox ) );

	INFO_MSG( E_NOTICE, "bw_get_keep_alive_seconds( %s )",
		PyString_AsString( pMailboxString ) );

	PyObjectAutoPtr pKeepAliveSeconds( PyObject_GetAttrString(
		pMailbox, "keepAliveSeconds" ) );

	if (!pKeepAliveSeconds)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	RETURN_LONG( PyLong_AsLong( pKeepAliveSeconds ) );
}


/**
 *	bw_set_default_keep_alive_seconds( $keepAliveIntervalSeconds )
 *
 *	Set the default keep-alive period that is set for newly created mailboxes.
 *
 *	@param keepAliveIntervalSeconds the keep-alive interval in seconds.
 *	@return bool
 */
PHP_FUNCTION( bw_set_default_keep_alive_seconds )
{
	long defaultKeepAlive = 0;

	if (FAILURE == zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,
			"l", &defaultKeepAlive ))
	{
		RETURN_FALSE;
	}

	INFO_MSG( E_NOTICE, "bw_set_default_keep_alive_seconds( %ld )",
		defaultKeepAlive );

	if (!BWG( bwModule ))
	{
		ERROR_MSG( E_ERROR, "bw_set_default_keep_alive_seconds: no BigWorld module" );
		RETURN_FALSE;
	}

	PyObjectAutoPtr pResult( PyObject_CallMethod( BWG( bwModule ),
		"setDefaultKeepAliveSeconds", "l",
		defaultKeepAlive ) );

	if (!pResult)
	{
		PyErr_ZendError();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

// ----------------------------------------------------------------------------
// Section: Helper method implementations
// ----------------------------------------------------------------------------
namespace // anonymous
{

/**
 *	Return a string describing the current Python exception, and clear the
 *	exception.
 */
const std::string PyErr_GetString()
{
	if (PyErr_Occurred())
	{
		PyObject * errType = NULL;
		PyObject * errValue = NULL;
		PyObject * errTraceback = NULL;

		PyErr_Fetch( &errType, &errValue, &errTraceback );
		// PyErr_Fetch clears the exception.

		// errValue and errTraceback may be NULL
		PyObject * errValueString = NULL;

		PyObjectAutoPtr pErrValueString(
			errValue ? PyObject_Str( errValue ) : NULL );

		std::string out( ((PyTypeObject*)errType)->tp_name );

		if (pErrValueString)
		{
			out += ": " + std::string( PyString_AsString( pErrValueString ) );
		}

		Py_DECREF( errType );
		Py_XDECREF( errValue );
		Py_XDECREF( errTraceback );

		return out;
	}
	else
	{
		return std::string( "Unknown error" );
	}
}


/**
 *	If a Python Exception has occurred, then the error type and string
 *	representation are printed out through zend_error with level E_ERROR.
 */
void PyErr_ZendError( const char * msg )
{
	std::string pyErr = PyErr_GetString();

	if (msg)
	{
		ERROR_MSG( E_ERROR, "%s: Python Exception: %s",
			msg, pyErr.c_str() );
	}
	else
	{
		ERROR_MSG( E_ERROR, "Python Exception: %s",
			pyErr.c_str() );
	}
}


/**
 *	Return the debug verbosity as set in the PHP config.
 */
inline long debug_verbosity()
{
	return BWG( debugLevel );
}

} // namespace (anonymous)

// functions.cpp
