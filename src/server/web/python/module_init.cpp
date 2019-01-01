/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"
#include "osdefs.h" // need this for DELIM

#include "entitydef/entity_description_map.hpp"

#include "pyscript/py_import_paths.hpp"

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"
#include "server/bwservice.hpp"

#include "web_integration.hpp"

#include <string>

extern int Math_token;
extern int ResMgr_token;
extern int PyScript_token;
static int s_moduleTokens =
	Math_token | ResMgr_token | PyScript_token;

extern int ChunkStationGraph_token;
static int s_chunkStationGraphToken = ChunkStationGraph_token;
extern int UserDataObjectLinkDataType_token;
static int s_userDataObjectLinkDataType_token = UserDataObjectLinkDataType_token;

DECLARE_DEBUG_COMPONENT( 0 )

namespace // anonymous
{

/** Finalisation function. */
PyObject * bigworld_fini_fn( PyObject * self, PyObject * args );

/**
 * Method definition for finalisation function.
 */
PyMethodDef s_bigworld_fini =
{
	"_fini", 				/* char* ml_name */
	bigworld_fini_fn, 		/* PyCFunction ml_meth */
	METH_NOARGS,			/* int ml_flags */
	NULL,					/* char * ml_doc */
};


/**
 * Finalisation function.
 */
PyObject * bigworld_fini_fn( PyObject * self, PyObject * args )
{
	delete WebIntegration::pInstance();

	delete BWResource::pInstance();

	Script::fini( /* shouldFinalise */ false );

	TRACE_MSG( "BigWorld module unloaded\n" );

	Py_RETURN_NONE;
}

} // namespace (anonymous)


/**
 * Python dynamic extension module initialisation function.
 */
PyMODINIT_FUNC initBigWorld( void )
{
	new BWResource();
	BWResource::init( 0, NULL );
	BWConfig::init( 0, NULL );

	Py_InitModule3( "BigWorld", NULL,
		"BigWorld Web Integration module." );

	// hack to retain old sys.path

	// get new reference to sys module
	PyObjectPtr pSysModule( PyImport_ImportModule( "sys" ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pSysModule)
	{
		PyErr_SetString( PyExc_ImportError, "Could not import module 'sys'!" );
		return;
	}

	// get new reference to sys.path variable
	PyObjectPtr pPathList( PyObject_GetAttrString( pSysModule.get(), "path" ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pPathList || !PySequence_Check( pPathList.get() ))
	{
		PyErr_SetString( PyExc_ImportError,
			"Could not find sys.path, or is not a sequence" );
		return;
	}

	PyImportPaths pythonPaths;

	for (int i = 0; i < PySequence_Size( pPathList.get() ); ++i)
	{
		PyObjectPtr pPathString( PySequence_GetItem( pPathList.get(), i ),
				PyObjectPtr::STEAL_REFERENCE );
		// Note that the Python interpreter hasn't been initialised at this
		// point, and so the builtin types haven't been initialised with the
		// right tp_flags, so we can't use PyString_Check to check that they
		// are strings.
		pythonPaths.addPath( PyString_AS_STRING( pPathString.get() ) );
	}

	if (!Script::init( pythonPaths ))
	{
		PyErr_SetString( PyExc_ImportError, "Script::init failed\n" );
		return;
	}

	// reset sys.path to old value
	pythonPaths.setDelimiter( DELIM );
	PySys_SetPath( const_cast< char * >( pythonPaths.pathsAsString().c_str() ) );

	// register finalisation function with atexit
	PyObjectPtr pAtExitModule( PyImport_ImportModule( "atexit" ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pAtExitModule)
	{
		return;
	}

	PyObjectPtr pyCFunction( PyCFunction_New( &s_bigworld_fini, NULL ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pyCFunction)
	{
		PyErr_SetString( PyExc_ImportError,
			"Could not create finalisation function object" );
		return;
	}

	PyObjectPtr pResult( PyObject_CallMethod( pAtExitModule.get(),
			"register", "O", pyCFunction.get() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pResult)
	{
		return;
	}

	WebIntegration * pWebIntegration = new WebIntegration();

	if (!pWebIntegration->init())
	{
		// exception set in init on failure
		return;
	}

	START_MSG( "WebIntegrationModule" );
}

// module_init.cpp
