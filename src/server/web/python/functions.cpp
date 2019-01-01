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

#include "web_integration.hpp"
#include "mailbox.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

#include "autotrace.hpp"

// ----------------------------------------------------------------------------
// Section: Python method implementations
// ----------------------------------------------------------------------------

/*~ function BigWorld.resetNetworkInterface
 *
 *	Recreates a new network interface which listens on the specified port. This
 *	also has the effect of clearing the locally cached database manager
 *	address, so that it is looked up on the next access.
 *
 *	@param port 	An integer specifying which port to use. If set to 0, then
 *					a random port is assigned. It defaults to 0.
 *
 */
PyObject * py_resetNetworkInterface( PyObject* args, PyObject * kwargs )
{
	int port = 0;
	const char * keywords[] = { "port", NULL };
	if (!PyArg_ParseTupleAndKeywords( args, kwargs,
			"|i:BigWorld.resetNetworkInterface",
			const_cast< char ** >( keywords ), &port ))
	{
		return NULL;
	}

	// TODO: Do we need to check if this succeeded?
	WebIntegration::instance().resetNetworkInterface( port );

	Py_RETURN_NONE;
}
PY_MODULE_FUNCTION_WITH_KEYWORDS_WITH_DOC( resetNetworkInterface, BigWorld,
"resetNetworkInterface( port=0 )\n\n"
"Recreates the communications nub on the specified port. It also has the \
effect of clearing the locally cached addresses to manager entities, so that \
they are looked up on the next access. \n\
If port is 0 (the default), then a random port is assigned. ")

PY_MODULE_FUNCTION_ALIAS_WITH_DOC( resetNetworkInterface, setNubPort, BigWorld,
"setNubPort( port=0 )\n\n"
"Deprecated. Alias for BigWorld.resetNetworkInterface.\n\n\
Recreates the communications nub on the specified port. It also has the \
effect of clearing the locally cached addresses to manager entities, so that \
they are looked up on the next access. \n\
If port is 0 (the default), then a random port is assigned. ")


/*~ BigWorld.lookUpEntityByName
 *
 *	Looks up an entity by its type name and its instance name.
 *	@param type the name of the entity type
 *	@param name the name of the entity
 *	@return a mailbox, if the entity is checked out, otherwise, either True if
 *	the entity exists but is not checked out, or False, indicating that no
 *	entity of that type exists.
 */
PyObject * py_lookUpEntityByName( PyObject* args, PyObject* kw )
{
	AutoTrace _at( "BigWorld.lookUpEntityByName()" );

	const char * keywords[] =
	{
		"type",
		"name",
		NULL
	};

	const char* entityTypeName = NULL;
	const char* entityName = NULL;

	int res = PyArg_ParseTupleAndKeywords( args, kw,
		"ss:BigWorld.lookUpEntityByName",
		const_cast< char ** >( keywords ),
		&entityTypeName, &entityName );

	if (!res)
	{
		return NULL;
	}

	return WebIntegration::instance().lookUpEntityByName(
		entityTypeName, entityName );

}
PY_MODULE_FUNCTION_WITH_KEYWORDS_WITH_DOC( lookUpEntityByName, BigWorld,
"lookUpEntityByName( entityType, name )\n\n"
"Looks up an entity by entity type (string) and name (string). If the entity \
exists and is not checked out, return a mailbox to the entity.\n"
"If the entity exists, but is not checked out from the database, return True. \
Otherwise, if no such entity exists, return False." )


/*~BigWorld.lookUpEntityByDBID
 *
 *	Looks up an entity by its type name and database ID.
 *
 *	@param type the name of the entity type
 *	@param databaseID the database ID of the entity
 *	@return a mailbox, if the entity is checked out, otherwise, either True if
 *	the entity exists but is not checked out, or False, indicating that no
 *	entity of that type exists.
 */
PyObject * py_lookUpEntityByDBID( PyObject* args, PyObject* kw )
{
	AutoTrace _at( "BigWorld.lookUpEntityByID()" );

	const char * keywords[] =
	{
		"type",
		"databaseID",
		NULL
	};
	const char * entityTypeName = NULL;
	DatabaseID databaseID = 0;

	int res = PyArg_ParseTupleAndKeywords( args, kw,
		"sL:BigWorld.lookUpEntityByID",
		const_cast< char ** >( keywords ),
		&entityTypeName, &databaseID );

	if (!res)
	{
		return NULL;
	}

	return WebIntegration::instance().lookUpEntityByDBID(
		entityTypeName, databaseID );
}
PY_MODULE_FUNCTION_WITH_KEYWORDS_WITH_DOC( lookUpEntityByDBID, BigWorld,
"pyBigWorld.lookUpEntityByDBID( entityType, databaseID )\n\n"
"Look up an entity from its entity type (string) and the database ID (long). \
If the entity exists and is not checked out, return a mailbox to the entity.\n"
"If the entity exists, but is not checked out from the database, return True. \
Otherwise, if no such entity exists, return False." )



/*~BigWorld.logOn(username, password, allow_already_logged_on=False)
 *
 *	Performs logon process to dbmgr with a username and password. Returns None
 *	on success, otherwise, an appropriate exception is thrown.
 *
 *	@param username 				The username.
 *	@param password 				The password.
 *	@param allow_already_logged_on 	If this is not set, then an exception
 *									is thrown if multiple logons are not
 *									allowed and the DBMgr reports that the
 *									logon was rejected because they are already
 *									logged on, otherwise, it is ignored.
 *
 *	@return None
 */
PyObject * py_logOn( PyObject* args, PyObject*kw )
{
	AutoTrace _at( "BigWorld.logOn()" );
	const char * username = NULL;
	const char * password = NULL;
	PyObject * allowAlreadyLoggedOn = Py_False;
	const char * keywords[] =
	{
		"username",
		"password",
		"allow_already_logged_on",
		NULL
	};

	int res = PyArg_ParseTupleAndKeywords( args, kw, "ss|O",
		const_cast< char ** >( keywords ),
		&username, &password, &allowAlreadyLoggedOn );

	if (!res)
	{
		return NULL;
	}

	if (WebIntegration::instance().logOn( username, password,
			PyObject_IsTrue( allowAlreadyLoggedOn ) ) == -1)
	{
		return NULL;
	}
	Py_RETURN_NONE;
}
PY_MODULE_FUNCTION_WITH_KEYWORDS_WITH_DOC( logOn, BigWorld,
"logOn( username, password )\n\n"
"Log on to the registered entity that is mapped to by the given username and \
password. This checks out entity from the database onto a base, where it can \
be looked up using the BigWorld.lookUp* methods.\n\n"
"Returns None on success." )


/**
 *	Set the default keep alive interval, in seconds. This is what the keep
 *	alive will be for new mailbox objects that are created through the
 *	BigWorld.lookUp* module methods.
 *
 *	They can be customised per mailbox instance through the keepAliveSeconds()
 *	C++ method, or the Python keepAliveSeconds attribute.
 */
PyObject * py_setDefaultKeepAliveSeconds( PyObject * args, PyObject * kwargs )
{
	uint32 keepAliveInterval = 0;
	const char* keywords[] =
	{
		"keepAliveSeconds",
		NULL
	};

	if (!PyArg_ParseTupleAndKeywords( args, kwargs, "k",
			const_cast< char ** >( keywords ),
			&keepAliveInterval ))
	{
		return NULL;
	}

	WebEntityMailBox::defaultKeepAliveSeconds( keepAliveInterval );

	Py_RETURN_NONE;
}
PY_MODULE_FUNCTION_WITH_KEYWORDS_WITH_DOC( setDefaultKeepAliveSeconds,
	BigWorld,
"setDefaultKeepAliveSeconds( seconds )\n\n"
"Set the default keep-alive interval (in seconds) given to new mailboxes \
when they are created, for example the return values for lookUp*() methods." )


// functions.cpp
