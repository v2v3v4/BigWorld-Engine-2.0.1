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

#include "script.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/memory_tracker.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/watcher.hpp"

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"

#include "network/basictypes.hpp"

#ifndef MF_SERVER
#include "physics2/material_kinds.hpp"
#endif /* MF_SERVER */

#include "resmgr/bwresource.hpp"
#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"

#include "entitydef/constants.hpp"

#include "frozen_modules.hpp"
#include "pickler.hpp"
#include "py_import_paths.hpp"
#include "py_output_writer.hpp"
#include "res_mgr_import.hpp"
#include "script_math.hpp"

#include <stdarg.h>

#include "frameobject.h"

#ifndef CODE_INLINE
#include "script.ipp"
#endif


DECLARE_DEBUG_COMPONENT2( "Script", 0 )

#ifndef _WIN32
	#define _stricmp strcasecmp
#endif

// -----------------------------------------------------------------------------
// Section: Profiling
// -----------------------------------------------------------------------------

static bool s_outputPythonTrace;

static int profileFunc(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
	static int depth;

	const uint OUT_SIZE = 256;
	const uint OUT_LAST = OUT_SIZE - 1;
	char out[OUT_SIZE];

	memset( out, ' ', OUT_LAST );
	out[OUT_LAST] = '\0';

	switch ( what )
	{
		case PyTrace_CALL:
		{
#if ENABLE_STACK_TRACKER
			StackTracker::push(
				PyString_AsString( frame->f_code->co_name ),
				PyString_AsString( frame->f_code->co_filename ),
				frame->f_code->co_firstlineno );
#endif

			if ( s_outputPythonTrace )
			{
				uint padding = depth * 4;
				bw_snprintf( out + padding, OUT_LAST - padding,
					"PyTrace_CALL: [%s] [%s] [%d]\n",
					PyString_AsString( frame->f_code->co_filename ),
					PyString_AsString( frame->f_code->co_name ),
					frame->f_code->co_firstlineno );

				DEBUG_MSG( out );
			}

			depth++;
			break;
		}

		case PyTrace_RETURN:
		{
			depth--;

			if ( s_outputPythonTrace )
			{
				uint padding = depth * 4;
				bw_snprintf( out + padding, OUT_LAST - padding,
					"PyTrace_RETURN\n" );
				DEBUG_MSG( out );
			}

#if ENABLE_STACK_TRACKER
			StackTracker::pop();
#endif
			break;
		}

		case PyTrace_C_CALL:
		{
			if ( s_outputPythonTrace )
			{
				uint padding = depth * 4;
				bw_snprintf( out + padding, OUT_LAST - padding,
					"PyTrace_C_CALL: %s\n", PyString_AsString( PyObject_Str( arg ) ) );
				DEBUG_MSG( out );
			}

			depth++;
			break;
		}

		case PyTrace_C_RETURN:
		{
			depth--;

			if ( s_outputPythonTrace )
			{
				uint padding = depth * 4;
				bw_snprintf( out + padding, OUT_LAST - padding,
					"PyTrace_C_RETURN\n" );
				DEBUG_MSG( out );
			}

			break;
		}
	}

	return 0;
}

namespace Script
{
MEMTRACKER_DECLARE( Script_ask, "Script::ask", 0 );

static const int MAX_ARGS = 20;
int g_scriptArgc = 0;
char* g_scriptArgv[ MAX_ARGS ];

PyObject * s_pProfiler = NULL;
std::string s_profileName;

bool getProfileRunning()
{
	return s_pProfiler != NULL;
}


/*
 *	This function is exposed as a watcher property. It is used to start and stop
 *	Python profiling. Setting the value to true starts profiling and stores the
 *	profile log in a file specified by s_profileName. Setting the value to false
 *	stops the profiling and closes the log.
 *
 *	This log can later be inspected using the hotshot.stats module.
 */
void setProfileRunning( bool isRunning )
{
	if (isRunning)
	{
		if (s_pProfiler == NULL)
		{
			PyObject * pHotShot = PyImport_ImportModule( "_hotshot" );

			if (pHotShot)
			{
				s_pProfiler =
					Script::ask( PyObject_GetAttrString( pHotShot, "profiler" ),
						Py_BuildValue( "(s)", s_profileName.c_str() ) );

				if (s_pProfiler)
				{
					Script::call(
						PyObject_GetAttrString( s_pProfiler, "start" ),
						PyTuple_New( 0 ) );
				}

				Py_DECREF( pHotShot );
			}
			else
			{
				WARNING_MSG( "Script::setProfileRunning: "
						"Cannot import _hotshot\n" );
				PyErr_PrintEx(0);
			}
		}
	}
	else
	{
		if (s_pProfiler)
		{
			if (!Script::call( PyObject_GetAttrString( s_pProfiler, "close" ),
						PyTuple_New( 0 ) ))
			{
				ERROR_MSG( "Script::setProfileRunning: close failed.\n" );
			}

			Py_DECREF( s_pProfiler );
			s_pProfiler = NULL;
		}
	}
}

};


// -----------------------------------------------------------------------------
// Section: Script class methods
// -----------------------------------------------------------------------------

void runInitTimeJobs();
void runFiniTimeJobs();

#include "osdefs.h"

static MultiFileSystem * s_pMainPythonFS;
static MultiFileSystem* s_pPythonFS;

#ifndef MF_SINGLE_THREADED
static PyObject * s_pOurInitTimeModules;
static PyThreadState * s_pMainThreadState;
static THREADLOCAL(PyThreadState*) s_defaultContext;
#endif

/*
 *  Wrap Python's memory allocations
 */

MEMTRACKER_DECLARE( Script_Python, "Script - Python", 0 );

extern "C"
{
	void* malloc_py( size_t size );
	void free_py( void* mem );
	void* realloc_py( void* mem, size_t size );
}

void* malloc_py( size_t size )
{
	MEMTRACKER_SCOPED( Script_Python );
	return malloc( size );
}

void free_py( void* mem )
{
	MEMTRACKER_SCOPED( Script_Python );
	free( mem );
}

void* realloc_py( void* mem, size_t size )
{
	MEMTRACKER_SCOPED( Script_Python );
	return realloc( mem, size );
}


/**
 *	This static method initialises the scripting system.
 *	The paths must be separated with semicolons.
 */
bool Script::init( const PyImportPaths & appPaths,
					const char * componentName,
					bool shouldOverrideSysMembers )
{
	// Figure out the IFileSystem that Python works within
	s_pMainPythonFS = new MultiFileSystem(
		*BWResource::instance().fileSystem() );
	s_pPythonFS = s_pMainPythonFS;

	// Build the list of python code file paths relative to our IFileSystem
	PyImportPaths sysPaths( DELIM );

#ifndef PY_EXTERNAL_FOPEN
	sysPaths.setPrefix( "../res/" );
#endif // PY_EXTERNAL_FOPEN

	sysPaths.append( appPaths );

	// Add extra paths to the input ones
	std::string commonPath( EntityDef::Constants::commonPath() );
	sysPaths.addPath( commonPath );
	sysPaths.addPath( commonPath + "/Lib" );
#ifdef _WIN32
	sysPaths.addPath( commonPath + "/DLLs");
#elif !defined(_LP64)
	sysPaths.addPath( commonPath + "/lib-dynload" );
#else
	sysPaths.addPath( commonPath + "/lib-dynload64" );
#endif

	// Initialise python
	// Py_VerboseFlag = 2;
	Py_FrozenFlag = 1;

	// Warn if tab and spaces are mixed in indentation.
	Py_TabcheckFlag = 1;
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;
	Py_Initialize();

#if !CONSUMER_CLIENT_BUILD
	PyEval_SetProfile( profileFunc, NULL );
#endif

#if BWCLIENT_AS_PYTHON_MODULE
	if (g_scriptArgc)
#endif
	{
		PySys_SetArgv( g_scriptArgc, g_scriptArgv );
	}


#if !BWCLIENT_AS_PYTHON_MODULE
#ifdef USE_RES_MGR_IMPORT_HOOK
	PyImport_FrozenModules = BigWorldFrozenModules;
#endif // USE_RES_MGR_IMPORT_HOOK
	PySys_SetPath( const_cast< char * >( sysPaths.pathsAsString().c_str() ) );
#endif // !BWCLIENT_AS_PYTHON_MODULE

	if (shouldOverrideSysMembers)
	{
		if (!PyOutputWriter::overrideSysMembers( false ))
		{
			return false;
		}
	}

	// Run any init time jobs, including creating modules and adding methods
	::runInitTimeJobs();

#if !BWCLIENT_AS_PYTHON_MODULE
#ifdef USE_RES_MGR_IMPORT_HOOK
	// Put a ResMgrImportHook into sys.path_hooks
	int error;
	PyObject* importHook = new PyResMgrImportHook();
	PyObject* path_hooks = PySys_GetObject("path_hooks");
	MF_ASSERT( path_hooks != NULL && PyList_Check(path_hooks) );
	error = PyList_Append( path_hooks, importHook );
	if ( error )
	{
		PyErr_Print();
		MF_ASSERT( 0 );
	}
	Py_DECREF( importHook );

	// Clear the path_importer_cache. During Py_InitialiseEx, 
	// sys.path_importer_cache is created _and_ the root node
	// gets populated with NULL

	// So, any modules needed before this point need to be frozen
	// See frozen_modules.cpp for how and where to do this
	PyObject* blankDict = PyDict_New();
	MF_ASSERT( blankDict != NULL );
	error = PySys_SetObject("path_importer_cache", blankDict );
	if ( error )
	{
		PyErr_Print();
		MF_ASSERT( 0 );
	}
	Py_DECREF( blankDict );
#endif // USE_RES_MGR_IMPORT_HOOK
#endif // !BWCLIENT_AS_PYTHON_MODULE

	// Disable garbage collection
	PyObject * pGCModule = PyImport_ImportModule( "gc" );
	if (!pGCModule)
	{
		// GC is not compiled in
		// clear the import error
		PyErr_Clear();
	}
	else
	{
		PyObject* pResult = PyObject_CallMethod( pGCModule, "disable", NULL );
		Py_DECREF( pResult );
	}
	Py_XDECREF( pGCModule );

	PyObject * pBigWorld = PyImport_AddModule( "BigWorld" );
	/*~ attribute BigWorld.component
	 *  @components{ all }
	 *
	 *	This is the component that is executing the present python environment.
	 *	Possible values are (so far) 'cell', 'base', 'client', 'database', 'bot'
	 *	and 'editor'.
	 */
	PyObject * pStr = PyString_FromString( componentName );
	if (PyObject_SetAttrString( pBigWorld, "component", pStr ) == -1 )
	{
		ERROR_MSG( "Script::init: Unable to set BigWorld.component to %s\n",
			componentName );
	}
	Py_XDECREF( pStr );


#ifndef MF_SINGLE_THREADED
	s_pOurInitTimeModules = PyDict_Copy( PySys_GetObject( "modules" ) );
	s_pMainThreadState = PyThreadState_Get();
	s_defaultContext = s_pMainThreadState;
	PyEval_InitThreads();

	BWConcurrency::setMainThreadIdleFunctions(
		&Script::releaseLock, &Script::acquireLock );
#endif

	s_profileName = std::string( componentName ) + ".prof";

	MF_WATCH( "pythonProfile/Output trace", s_outputPythonTrace, Watcher::WT_READ_WRITE );

	MF_WATCH( "pythonProfile/running",
		&Script::getProfileRunning, &Script::setProfileRunning,
		"Set to true to start Python profiling and false to stop" );
	MF_WATCH( "pythonProfile/filename", s_profileName, Watcher::WT_READ_WRITE,
		   "Specifies the location to store the Python profiling output" );

	if (!Pickler::init())
	{
		ERROR_MSG( "Script::init: Pickler failed to initialise\n" );
		return false;
	}

#ifndef MF_SERVER
	// MaterialKinds is used in py_common.cpp, which is actually only used in
	// the vcproj build, not the Makefile build
	if (!MaterialKinds::init())
	{
		ERROR_MSG( "Script::init: Material Kinds failed to initialise\n" );
		return false;
	}
#endif /* MF_SERVER */

	PyObject * pAutoImport = PyImport_ImportModule( "BWAutoImport" );

	if (pAutoImport)
	{
		Py_DECREF( pAutoImport );
	}
	else
	{
		NOTICE_MSG( "Script::init: Unable to import BWAutoImport.\n" );
		PyErr_Print();
	}

	return true;
}

namespace
{
bool s_isFinalised = false;
}


/**
 *	This static method terminates the scripting system.
 */
void Script::fini( bool shouldFinalise )
{
#ifndef MF_SERVER
	MaterialKinds::fini();
#endif /* MF_SERVER */
	Pickler::finalise();

	::runFiniTimeJobs();

#ifndef MF_SINGLE_THREADED
	if (s_pOurInitTimeModules != NULL)
	{
		Py_DECREF( s_pOurInitTimeModules );
		s_pOurInitTimeModules = NULL;
	}
#endif

#if ENABLE_WATCHERS
	Watcher::fini();
#endif

	if (shouldFinalise)
	{
#if !BWCLIENT_AS_PYTHON_MODULE
		Py_Finalize();
#endif // !BWCLIENT_AS_PYTHON_MODULE
	}

	delete s_pMainPythonFS;
	s_pMainPythonFS = NULL;
	s_isFinalised = true;
}


bool Script::isFinalised()
{
	return s_isFinalised;
}


/**
 *	This function creates a new interpreter for single-threaded
 * 	applications. The new interpreter will have the same state as
 * 	the default interpreter freshly created by Script::init().
 * 	You can switch between interpreters by calling Script::swapInterpreter().
 * 	Multi-threaded applications should use Script::initThread() instead.
 */
PyThreadState* Script::createInterpreter()
{
	PyThreadState* 	pCurInterpreter = PyThreadState_Get();
	PyObject * 		pCurPath = PySys_GetObject( "path" );

	PyThreadState* pNewInterpreter = Py_NewInterpreter();
	if (pNewInterpreter)
	{
		PySys_SetObject( "path", pCurPath );
#ifndef MF_SINGLE_THREADED
		PyDict_Merge( PySys_GetObject( "modules" ), s_pOurInitTimeModules, 0 );
#endif
		// Restore original intepreter.
		PyThreadState* pSwapped = PyThreadState_Swap( pCurInterpreter );
		IF_NOT_MF_ASSERT_DEV( pSwapped == pNewInterpreter )
		{
			MF_EXIT( "error creating new python interpreter" );
		}
	}

	return pNewInterpreter;
}

/**
 * 	This function destroys an interpreter created by
 * 	Script::createInterpreter().
 */
void Script::destroyInterpreter( PyThreadState* pInterpreter )
{
	// Can't destroy current interpreter.
	IF_NOT_MF_ASSERT_DEV( pInterpreter != PyThreadState_Get() )
	{
		MF_EXIT( "trying to destroy current interpreter" );
	}

	// Can't call Py_EndInterpreter() see Script::finiThread().
	PyInterpreterState_Clear( pInterpreter->interp );
	PyInterpreterState_Delete( pInterpreter->interp );
}

/**
 *	This function swaps the current interpreter with the one provided.
 * 	It returns the swapped out interpreter.
 */
PyThreadState* Script::swapInterpreter( PyThreadState* pInterpreter )
{
#ifndef MF_SINGLE_THREADED
	s_defaultContext = pInterpreter;
#endif
	return PyThreadState_Swap( pInterpreter );
}

#ifndef MF_SINGLE_THREADED
/**
 *	This static method initialises scripting for a new thread.
 *
 *	It should be called from the new thread, after the main thread has called
 *	the main 'init' method. If plusOwnInterpreter is true, then the new thread
 *	is set up with its own blank interpreter. Otherwise, the new thread is
 *	associated with the main thread's interpreter.
 *
 *	The function returns with the global lock acquired.
 */
void Script::initThread( bool plusOwnInterpreter )
{
	IF_NOT_MF_ASSERT_DEV( s_defaultContext == NULL )
	{
		MF_EXIT( "trying to initialise scripting when already initialised" );
	}

	s_pPythonFS = s_pMainPythonFS;

	PyEval_AcquireLock();

	PyThreadState * newTState = NULL;

	if (plusOwnInterpreter)
	{
		newTState = Py_NewInterpreter();

		// set the path again
		PyObject * pMainPyPath = PyDict_GetItemString(
			s_pMainThreadState->interp->sysdict, "path" );
		PySys_SetObject( "path", pMainPyPath );

		// put in any modules created by our init-time jobs
		PyDict_Merge( PySys_GetObject( "modules" ), s_pOurInitTimeModules,
			/*override:*/0 );
	}
	else
	{
		newTState = PyThreadState_New( s_pMainThreadState->interp );
	}

	IF_NOT_MF_ASSERT_DEV( newTState != NULL )
	{
		MF_EXIT( "failed to create a new thread object" );
	}

	PyEval_ReleaseLock();

#if 0

	PyInterpreterState * newInterp = PyInterpreterState_New();
	PyInterpreterState * oldInterp = s_pMainThreadState->interp;

	int expectedSize = 32;

	// build the new state structures
	newInterp->modules = oldInterp->modules;	Py_INCREF( newInterp->modules );
	newInterp->sysdict = oldInterp->sysdict;	Py_INCREF( newInterp->sysdict );
	newInterp->builtins = oldInterp->builtins;	Py_INCREF( newInterp->builtins);
	newInterp->codec_search_path = oldInterp->codec_search_path;
	newInterp->codec_search_cache = oldInterp->codec_search_cache;
	newInterp->codec_error_registry = oldInterp->codec_error_registry;
	Py_XINCREF( newInterp->codec_search_path );
	Py_XINCREF( newInterp->codec_search_cache );
	Py_XINCREF( newInterp->codec_error_registry );
#ifdef HAVE_DLOPEN
	newInterp->dlopenflags = oldInterp->dlopenflags;
	expectedSize += 4;
#endif
#ifdef WITH_TSC	// whatever that is...
	newInterp->tscdump = oldInterp->tscdump;
	expectedSize += 4;
#endif
	PyEval_ReleaseLock();

	MF_ASSERT( sizeof(PyInterpreterState) == expectedSize );
	// Currently updated to Python 2.4.0

	PyThreadState * newTState = PyThreadState_New( newInterp );
#endif


	// and make our thread be the one global python one
	s_defaultContext = newTState;
	Script::acquireLock();
}

/**
 *	This static method finalises scripting for a thread (not the main one).
 *
 *	It must be called with the current thread in possession of the lock.
 *	(When it returns the lock is not held.)
 */
void Script::finiThread( bool plusOwnInterpreter )
{
	IF_NOT_MF_ASSERT_DEV( s_defaultContext == PyThreadState_Get() )
	{
		MF_EXIT( "trying to finalise script thread when not in default context" );
	}

	if (plusOwnInterpreter)
	{
		//Py_EndInterpreter( s_defaultContext );
		// for now we do not want our modules + sys dict destroyed...
		// ... really should make a way of migrating between threads
		// but for now this will do
		{
			//PyImport_Cleanup();	// this is the one we can't call
			PyInterpreterState_Clear( s_defaultContext->interp );
			PyThreadState_Swap( NULL );
			PyInterpreterState_Delete( s_defaultContext->interp );
		}

		PyEval_ReleaseLock();
	}
	else
	{
		PyThreadState_Clear( s_defaultContext );
		PyThreadState_DeleteCurrent();	// releases GIL
	}

	s_defaultContext = NULL;

#if 0
	PyThreadState * tstate = PyThreadState_Get();
	PyInterpreterState * interp = tstate->interp;

	PyThreadState_Clear( tstate );
	//PyThreadState_Delete( tstate );
	PyThreadState_DeleteCurrent();

	PyInterpreterState_Clear( interp );
	PyInterpreterState_Delete( interp );

	// let other threads into the action
	//Script::releaseLock();	// done by DeleteCurrent above
	s_defaultContext = NULL;
#endif

	if ((MultiFileSystem*)s_pPythonFS != (MultiFileSystem*)s_pMainPythonFS)
	{	// cast necessary for some TLS systems
		delete s_pPythonFS;
		s_pPythonFS = s_pMainPythonFS;
	}

}


/**
 *	This static method acquires the lock for the current thread and
 *	sets the python context to the default one for this thread.
 */
void Script::acquireLock()
{
	if (s_defaultContext == NULL) return;

	//MF_ASSERT( PyThreadState_Get() != s_defaultContext );
	// can't do assert above because PyThreadState_Get can't (since 2.4)
	// be called when the thread state is null - it generates a fatal
	// error. NULL is what we expect it to be as set by releaseLock anyway...
	// there doesn't appear to be a good way to assert this here. Oh well.
	PyEval_RestoreThread( s_defaultContext );
}

/**
 *	This static method releases the lock on the python context held by
 *	the current thread and allows other threads the execute (python) code
 */
void Script::releaseLock()
{
	if (s_defaultContext == NULL) return;

	PyThreadState * oldState = PyEval_SaveThread();
	IF_NOT_MF_ASSERT_DEV( oldState == s_defaultContext )
	{
		MF_EXIT( "releaseLock: default context is incorrect" );
	}
}

#else // MF_SINGLE_THREADED
void Script::acquireLock() { }
void Script::releaseLock() { }
#endif // MF_SINGLE_THREADED



/**
 *	Static function to call a callable object with error checking.
 *	Note that it decrements the reference counts of both of its arguments.
 *	Any error generates an exception, which is printed out if printException is
 *	true, otherwise it is left as the set python exception.
 *	If it is OK that the function is NULL, and an error occurred, then we
 *	assume that the error occurred during the pushing of the parameters onto
 *	the stack, and we clear the error ( e.g. PyObject_GetAttrString( func ) is
 *	used in the parameter list. )
 *
 *	@return the result of the call, or NULL if some problem occurred
 */
PyObject * Script::ask( PyObject * pFunction, PyObject * pArgs,
	const char * errorPrefix, bool okIfFunctionNull, bool printException )
{
	BW_GUARD;
	// Can't use the MEMTRACKER_SCOPED macro because GCC and VS can't agree
	// on wether it refers to Script namespace or global
	#ifdef ENABLE_MEMTRACKER
	ScopedMemTracker scopedMemTracker_Script_ask( Script::g_memTrackerSlot_Script_ask );
	#endif

	if (PyErr_Occurred() && !(okIfFunctionNull && pFunction == NULL))
	{
		PyErr_PrintEx(0);
	}
	MF_ASSERT_DEV(!PyErr_Occurred() || (okIfFunctionNull && pFunction == NULL));


	PyObject * pResult = NULL;

	if (pFunction != NULL && pArgs != NULL)
	{
		if (PyCallable_Check( pFunction ))
		{
			pResult = PyObject_CallObject( pFunction, pArgs );
			// may set an exception - we fall through and print it out later
		}
		else
		{
			PyErr_Format( PyExc_TypeError,
				"%sScript call attempted on a non-callable object!",
				errorPrefix );
		}
	}
	else
	{
		if (pArgs == NULL || !okIfFunctionNull)
		{
			PyErr_Format( PyExc_ValueError,
				"%sScript call attempted with\n"
				"\tFunction at 0x%p, Args at 0x%p",
				errorPrefix, pFunction, pArgs );
		}
		else
		{
			// the function is NULL but it's allowed, so clear
			// any error occurring from trying to find it.
			PyErr_Clear();
		}
	}

	Py_XDECREF( pFunction );
	Py_XDECREF( pArgs );

	if (printException)
	{
		PyObject * pErr = PyErr_Occurred();
		if (pErr != NULL)
		{
			PyObject *pType, *pValue, *pTraceback;
			PyErr_Fetch( &pType, &pValue, &pTraceback );

			std::string finalError;

			if ( pValue != NULL )
			{
				// there is extended error info, so put it, and put the generic
				// python error in pErr between parenthesis.
				PyObject * pErrStr = PyObject_Str( pValue );
				if ( pErrStr != NULL )
				{
					finalError +=
						std::string( PyString_AsString( pErrStr ) ) + " (";
					Py_DECREF( pErrStr );
				}
			}

			// add the defaul python error
			PyObject * pErrStr = PyObject_Str( pErr );
			if ( pErrStr != NULL )
			{
				finalError += PyString_AsString( pErrStr );
				Py_DECREF( pErrStr );
			}

			if ( pValue != NULL )
			{
				// there is extended error info, so close the parenthesis.
				finalError += ")";
			}

			// and output the error.
			ERROR_MSG( "%s %s\n",
				errorPrefix,
				finalError.c_str() );

			PyErr_Restore( pType, pValue, pTraceback );
			PyErr_PrintEx(0);
		}
		PyErr_Clear();
	}

	return pResult;
}


/**
 *	This static utility function returns a new instance of the
 *	class pClass, without calling its __init__.
 */
PyObject * Script::newClassInstance( PyObject * pClass )
{
	// This code was inspired by new_instance function in Modules/newmodule.c in
	// the Python source code.

	PyInstanceObject * pNewObject =
		PyObject_New( PyInstanceObject, &PyInstance_Type );

	Py_INCREF( pClass );
	pNewObject->in_class = (PyClassObject *)pClass;
	pNewObject->in_dict = PyDict_New();

	PyObject_GC_Init( pNewObject );

	return (PyObject *)pNewObject;
}

/**
 *	This static utility function unloads a module by deleting its entry in
 * 	sys.modules. Returns true if successful, false if the module is not loaded.
 */
bool Script::unloadModule( const char * moduleName )
{
	PyObject* pModulesDict = PyImport_GetModuleDict();
	MF_ASSERT( PyDict_Check( pModulesDict ) );
	return PyDict_DelItemString( pModulesDict, moduleName ) == 0;
}

/**
 *	Script utility function to match a class name in a PyClassObject's
 *	inheritance hierarchy, and return a pointer to its class object.
 *
 *	It does not increment the reference count of the class object
 *	if it finds it.
 *
 *	@return	Borrowed reference to class object
 */
PyObject * PyClass_MatchNameInHierarchy( PyClassObject * pClass,
	const char * name )
{
	if (streq( PyString_AsString( pClass->cl_name ), name ))
	{
		return (PyObject*)pClass;
	}

	PyObject * pFoundClass = NULL;
	int n = PyTuple_Size( pClass->cl_bases );
	for (int i = 0; i < n && pFoundClass == NULL; i++)
	{
		pFoundClass = PyClass_MatchNameInHierarchy(
			(PyClassObject*)PyTuple_GetItem( pClass->cl_bases, i ),
			name );
	}
	return pFoundClass;
}


/**
 *	Script utility function to run an expression string. It calls
 *	another PyRun_String using the __main__ dictionary and the
 *	Py_eval_input or Py_single_input start symbol.
 *
 *	If printResult is false, then neither the result nor any
 *	exception is printed - just like PyObject_CallObject. However,
 *	statements such as 'print ...' and 'import ...' are not allowed.
 *	If printResult is true, then those statements are allowed,
 *	the result is printed, but None is always returned.
 *
 *	If anyone can find a way to fix this (so that the result is
 *	never printed, but 'print' and 'import' are allowed) then it
 *	would be most wonderful!
 *
 *	@return	The result of the input expression, or None if printResult is true
 */
PyObject * Script::runString( const char * expr, bool printResult )
{
	PyObject * m = PyImport_AddModule( "__main__" );
	if (m == NULL)
	{
		PyErr_SetString( PyExc_SystemError, "Module __main__ not found!" );
		return NULL;
	}

	PyObject * d = PyModule_GetDict( m );

	PyCompilerFlags cf = { PyCF_SOURCE_IS_UTF8 };
	return PyRun_StringFlags( const_cast<char*>( expr ),
		printResult ? Py_single_input : Py_eval_input,
		d, d, &cf );

	/*
	OK, here's what the different exported start symbols do:
	(they're used in compiling, not executing)

	Py_single_input:	None returned, appends PRINT_EXPR instruction to code
	Py_file_input:		None returned, pops result from stack and discards it
	Py_eval_input:		Result returned, doesn't allow statements (only expressions)

	See compile_node in Python/compile.c for the proof
	*/
}


/**
 *	This method returns a tuple from a vector of strings.
 */
PyObject * PyTuple_FromStringVector( const std::vector< std::string > & v )
{
	int sz = v.size();
	PyObject * t = PyTuple_New( sz );
	for (int i = 0; i < sz; i++)
	{
		PyTuple_SetItem( t, i, PyString_FromString( v[i].c_str() ) );
	}
	return t;
}


// -----------------------------------------------------------------------------
// Section: PY_EXTERNAL_FOPEN
// -----------------------------------------------------------------------------

#ifdef PY_EXTERNAL_FOPEN

/**
 *	This method implements an external filename namespace for Python
 */
FILE * PyOS_fopen( const char * filename, const char * mode )
{
	FILE * f = s_pPythonFS->posixFileOpen( filename, mode );
#ifndef _WIN32
	return f;
#else
	if (f == NULL) return NULL;

	// make sure the case matches something in the directory
	// this is incredibly dodgy and a great pain!
	const char * fend1 = strrchr( filename, '/' );
	const char * fend2 = strrchr( filename, '\\' );
	if (fend2 && fend1 < fend2) fend1 = fend2;
	std::string dirname( filename, fend1?fend1-filename:0 );
	std::string entname( fend1?fend1+1:filename );

	IFileSystem::Directory d = s_pPythonFS->readDirectory( dirname );
	for (uint i = 0; i < d.size(); i++)
		if (d[i] == entname) return f;

	fclose( f );
	return NULL;
#endif
}

/**
 *	This method implements an external filename namespace for Python
 */
int PyOS_statType( const char * filename )
{
	IFileSystem::FileType ft = s_pPythonFS->getFileType( filename );
	switch (ft)
	{
		case IFileSystem::FT_FILE:			return 0;
		case IFileSystem::FT_DIRECTORY:		return 1;
		default:							return -1;
	};
}

/**
 *	This method implements an external filename namespace for Python
 */
PyObject * PyOS_listdir( const char * dirname )
{
	IFileSystem::FileType ft = s_pPythonFS->getFileType( dirname );
	if (ft != IFileSystem::FT_DIRECTORY)
	{
		PyObject * pyDirName = Script::getData( dirname );
		PyErr_SetObject( PyExc_OSError,
			Py_BuildValue("(isO)", -1, "Not a directory", pyDirName) );
		Py_DECREF( pyDirName );
		return NULL;
	}

	IFileSystem::Directory rd = s_pPythonFS->readDirectory( dirname );

	PyObject * pd = PyList_New( rd.size() );
	if (pd == NULL) return NULL;

	for (uint i = 0; i < rd.size(); i++)
	{
		PyList_SET_ITEM( pd, i, Script::getData( rd[i] ) );
	}

	return pd;
}

/**
 *	This method implements an external filename namespace for Python
 */
void * PyOS_dlopen( const char * filename, int flags )
{
	// unfortunately these require a real file, can't make do with a FILE *
	// which is a major pain for funky FSs like say zip.

	IFileSystem::FileType ft =
		BWResource::instance().fileSystem()->getFileType( filename );

	if (ft == IFileSystem::FT_FILE)
	{
		// Getting the absolute path through BWResolver, which is not ideal.
		// Must consider doing something different here, to be able to load
		// from Zip files, etc.
		std::string fullname = BWResolver::resolveFilename( filename );

		TRACE_MSG( "Script::PyOS_dlopen: dlopening '%s'\n",
			fullname.c_str() );
		return PyOS_dlopenDefault( fullname.c_str(), flags );
	}

	return NULL;
}


/// static method to add a new overriding res path
void Script::addResPath( const std::string & resPath )
{
	if (s_pPythonFS == s_pMainPythonFS)
		s_pPythonFS = new MultiFileSystem( *s_pMainPythonFS );

	MF_ASSERT( *resPath.rbegin() == '/' );
	s_pPythonFS->addBaseFileSystem( NativeFileSystem::create( resPath ), 0 );
}

/// static method to remove the foremost res path
void Script::delResPath()
{
	MF_ASSERT( s_pPythonFS != s_pMainPythonFS );
	s_pPythonFS->delBaseFileSystem( 0 );
}

#endif

// -----------------------------------------------------------------------------
// Section: Python core (i.e. NOT PyObjectPlus) memory allocation tracking
// -----------------------------------------------------------------------------

extern "C"
{
void * log_malloc( size_t n );
void * log_realloc( void * m, size_t n );
void log_free( void * m );
};

/**
 *	Logging interceptor for python 'malloc' calls
 */
void * log_malloc( unsigned int n )
{
#if defined( _DEBUG ) && defined( _WIN32 )
	if (n >= 1024*1024)
	{
		// python allocating more than 1 MB!
		ENTER_DEBUGGER()
	}
#endif

	uint16 * m = (uint16*)malloc( n );
	return m;
}

/**
 *	Logging interceptor for python 'realloc' calls
 */
void * log_realloc( void * m, size_t n )
{
#if defined( _DEBUG ) && defined( _WIN32 )
	if (n >= 1024*1024)
	{
		// python allocating more than 1 MB!
		ENTER_DEBUGGER()
	}
#endif

	m = realloc( m, n );
	return m;
}

/**
 *	Logging interceptor for python 'free' calls
 */
void log_free( void * m )
{
	free( m );
}




// -----------------------------------------------------------------------------
// Section: PyModuleMethodLink
// -----------------------------------------------------------------------------

/**
 *	Constructor taking PyCFunction.
 */
PyModuleMethodLink::PyModuleMethodLink( const char * moduleName,
		const char * methodName, PyCFunction method,
		const char * docString ) :
	Script::InitTimeJob( 0 ),
	moduleName_( moduleName ),
	methodName_( methodName )
{
	mdReal_.ml_name = const_cast< char * >( methodName_ );
	mdReal_.ml_meth = method;
	mdReal_.ml_flags = METH_VARARGS;
	mdReal_.ml_doc = const_cast< char * >( docString );

	mdStop_.ml_name = NULL;
	mdStop_.ml_meth = NULL;
	mdStop_.ml_flags = 0;
	mdStop_.ml_doc = NULL;
}


/**
 *	Constructor taking PyCFunctionWithKeywords.
 */
PyModuleMethodLink::PyModuleMethodLink( const char * moduleName,
		const char * methodName, PyCFunctionWithKeywords method,
		const char * docString ) :
	Script::InitTimeJob( 0 ),
	moduleName_( moduleName ),
	methodName_( methodName )
{
	mdReal_.ml_name = const_cast< char * >( methodName_ );
	mdReal_.ml_meth = (PyCFunction)method;
	mdReal_.ml_flags = METH_VARARGS | METH_KEYWORDS;
	mdReal_.ml_doc = const_cast< char * >( docString );

	mdStop_.ml_name = NULL;
	mdStop_.ml_meth = NULL;
	mdStop_.ml_flags = 0;
	mdStop_.ml_doc = NULL;
}


/**
 *	Destructor
 */
PyModuleMethodLink::~PyModuleMethodLink()
{
}


/**
 *	This method adds this factory method to the module
 */
void PyModuleMethodLink::init()
{
	Py_InitModule( const_cast<char *>(moduleName_), &mdReal_ );
}


// -----------------------------------------------------------------------------
// Section: PyModuleAttrLink
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
PyModuleAttrLink::PyModuleAttrLink( const char * moduleName,
		const char * objectName, PyObject * pObject ) :
	Script::InitTimeJob( 0 ),
	moduleName_( moduleName ),
	objectName_( objectName ),
	pObject_( pObject )
{
	MF_ASSERT( moduleName_ );
	MF_ASSERT( objectName_ );
}


/**
 *	This method adds the object to the module.
 */
void PyModuleAttrLink::init()
{
	PyObject_SetAttrString(
		PyImport_AddModule( const_cast<char *>( moduleName_ ) ),
		const_cast<char *>( objectName_ ),
		pObject_ );
	Py_DECREF( pObject_ );
}



// -----------------------------------------------------------------------------
// Section: PyModuleResultLink
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
PyModuleResultLink::PyModuleResultLink( const char * moduleName,
		const char * objectName, PyObject * (*pFunction)() ) :
	Script::InitTimeJob( 1 ),
	moduleName_( moduleName ),
	objectName_( objectName ),
	pFunction_( pFunction )
{
	MF_ASSERT( moduleName_ );
	MF_ASSERT( objectName_ );
}


/**
 *	This method adds the object to the module.
 */
void PyModuleResultLink::init()
{
	PyObject * pObject = (*pFunction_)();
	if (pObject == NULL)
	{
		ERROR_MSG( "Error initialising object '%s' in module '%s'\n",
			objectName_, moduleName_ );
		PyErr_PrintEx(0);
		PyErr_Clear();
	}
	else
	{
		PyObject_SetAttrString(
			PyImport_AddModule( const_cast<char *>( moduleName_ ) ),
			const_cast<char *>( objectName_ ),
			pObject );
		Py_DECREF( pObject );
	}
}



// -----------------------------------------------------------------------------
// Section: PyFactoryMethodLink
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
PyFactoryMethodLink::PyFactoryMethodLink( const char * moduleName,
		const char * methodName, PyTypePlus * pType ) :
	Script::InitTimeJob( 0 ),
	moduleName_( moduleName ),
	methodName_( methodName ),
	pType_( pType )
{
}

/**
 *	Destructor
 */
PyFactoryMethodLink::~PyFactoryMethodLink()
{
}


/**
 *	This method adds this factory method to the module
 */
void PyFactoryMethodLink::init()
{
	// get the module
	PyObject * pModule = PyImport_AddModule( const_cast<char *>(moduleName_) );
	MF_ASSERT( pModule != NULL );

	// make sure the type is ready or can be made so
	int isReady = PyType_Ready( pType_ );
	MF_ASSERT( isReady == 0 );

	// add the type to the module
	PyObject_SetAttrString( pModule, const_cast<char *>(methodName_),
		(PyObject*)pType_ );

	// set the name of the module in the type dict
	PyObject * pModuleName = Script::getData( moduleName_ );
	PyDict_SetItemString( pType_->tp_dict, "__module__", pModuleName );
	Py_DECREF( pModuleName );

	/*
	// Unfortunately that doesn't work - we have to change the name
	// in tp_name :( ... and even if that can be made to work, then
	// tp_name has to match the name in the module so we'd have to
	// change it anyway ... so we may as well leave typeobject.c alone.
	int modNameLen = strlen( moduleName_ );
	int typNameLen = strlen( methodName_ );
	char * combinedName = new char[modNameLen+1+typNameLen+1];
	memcpy( combinedName, moduleName_, modNameLen );
	combinedName[modNameLen] = '.';
	memcpy( combinedName + modNameLen+1, methodName_, typNameLen );
	combinedName[modNameLen+1+typNameLen] = 0;
	pType_->tp_name = combinedName;
	*/

	// for now have changed typeobject to look in __dict__ for name too
	// but still unsure ... might have been better to have full name in
	// tp_name anyway ... we can do that when the next upgrade breaks it :)
	PyObject * pMethodName = Script::getData( methodName_ );
	PyDict_SetItemString( pType_->tp_dict, "__name__", pMethodName );
	Py_DECREF( pMethodName );

	// and we're done
}



// -----------------------------------------------------------------------------
// Section: Pickling helper functions
// -----------------------------------------------------------------------------

/**
 *	This function builds the result tuple that is expected from __reduce__
 *	style methods. It is used by the PY_PICKLING_METHOD_DECLARE macro.
 */
PyObject * Script::buildReduceResult( const char * consName,
	PyObject * pConsArgs )
{
	if (pConsArgs == NULL) return NULL;	// error already set

	static PyObject * s_pBWPicklingModule = PyImport_AddModule( "_BWp" );

	PyObject * pConsFunc =
		PyObject_GetAttrString( s_pBWPicklingModule, (char*)consName );
	if (pConsFunc == NULL)
	{
		Py_DECREF( pConsArgs );
		return NULL;					// error already set
	}

	PyObject * pRes = PyTuple_New( 2 );
	PyTuple_SET_ITEM( pRes, 0, pConsFunc );
	PyTuple_SET_ITEM( pRes, 1, pConsArgs );
	return pRes;
}


// -----------------------------------------------------------------------------
// Section: InitTimeJob
// -----------------------------------------------------------------------------

typedef std::vector< Script::InitTimeJob * >	InitTimeJobsVector;
typedef std::map< int, InitTimeJobsVector >		InitTimeJobsMap;
static InitTimeJobsMap * s_initTimeJobsMap = NULL;
static bool s_initTimeJobsInitted = false;

/**
 *	Constructor
 */
Script::InitTimeJob::InitTimeJob( int rung )
{
	if (s_initTimeJobsInitted)
	{
		CRITICAL_MSG( "Script::InitTimeJob (rung %d) "
			"constructed after script init time!\n", rung );
		return;
	}

	if (s_initTimeJobsMap == NULL)
	{
		s_initTimeJobsMap = new InitTimeJobsMap();
	}
	(*s_initTimeJobsMap)[rung].push_back( this );
}


/**
 *	Destructor
 */
Script::InitTimeJob::~InitTimeJob()
{
	/*
	if (!s_initTimeJobsInitted)
	{
		ERROR_MSG( "Script::InitTimeJob "
			"destructed before script init time!\n" );
	}
	*/
}


/**
 *	This function runs init time jobs
 */
void runInitTimeJobs()
{
	if (s_initTimeJobsInitted)
	{
		ERROR_MSG( "Script::runInitTimeJobs called more than once\n" );
		return;
	}

	s_initTimeJobsInitted = true;

	if (s_initTimeJobsMap != NULL)
	{
		for (InitTimeJobsMap::iterator mit = s_initTimeJobsMap->begin();
			mit != s_initTimeJobsMap->end();
			mit++)
		{
			for (InitTimeJobsVector::iterator vit = mit->second.begin();
				vit != mit->second.end();
				vit++)
			{
				(*vit)->init();
			}
		}

		delete s_initTimeJobsMap;
		s_initTimeJobsMap = NULL;
	}
}


// -----------------------------------------------------------------------------
// Section: FiniTimeJob
// -----------------------------------------------------------------------------

typedef std::vector< Script::FiniTimeJob * >	FiniTimeJobsVector;
typedef std::map< int, FiniTimeJobsVector >		FiniTimeJobsMap;
static FiniTimeJobsMap * s_finiTimeJobsMap = NULL;
static bool s_finiTimeJobsRun = false;

/**
 *	Constructor
 */
Script::FiniTimeJob::FiniTimeJob( int rung )
{
	if (s_finiTimeJobsRun)
	{
		CRITICAL_MSG( "Script::FiniTimeJob (rung %d) "
			"constructed after script finalised time!\n", rung );
		return;
	}

	if (s_finiTimeJobsMap == NULL)
	{
		s_finiTimeJobsMap = new FiniTimeJobsMap();
	}
	(*s_finiTimeJobsMap)[rung].push_back( this );
}


/**
 *	Destructor
 */
Script::FiniTimeJob::~FiniTimeJob()
{
}


/**
 *	This function runs fini time jobs
 */
void runFiniTimeJobs()
{
	if (s_finiTimeJobsRun)
	{
		ERROR_MSG( "Script::runFiniTimeJobs called more than once\n" );
		return;
	}

	s_finiTimeJobsRun = true;

	if (s_finiTimeJobsMap != NULL)
	{
		for (FiniTimeJobsMap::iterator mit = s_finiTimeJobsMap->begin();
			mit != s_finiTimeJobsMap->end();
			mit++)
		{
			for (FiniTimeJobsVector::iterator vit = mit->second.begin();
				vit != mit->second.end();
				vit++)
			{
				(*vit)->fini();
			}
		}

		delete s_finiTimeJobsMap;
		s_finiTimeJobsMap = NULL;
	}
}


// -----------------------------------------------------------------------------
// Section: setData and getData utility functions
// -----------------------------------------------------------------------------

/**
 *	This function tries to interpret its argument as a boolean,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, bool & rBool,
	const char * varName )
{
	if (PyInt_Check( pObject ))
	{
		rBool = PyInt_AsLong( pObject ) != 0;
		return 0;
	}

	if (PyString_Check( pObject ))
	{
		char * pStr = PyString_AsString( pObject );
		if (!_stricmp( pStr, "true" ))
		{
			rBool = true;
			return 0;
		}
		else if(!_stricmp( pStr, "false" ))
		{
			rBool = false;
			return 0;
		}
	}

	PyErr_Format( PyExc_TypeError, "%s must be set to a bool", varName );
	return -1;
}


/**
 *	This function tries to interpret its argument as an integer,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, int & rInt,
	const char * varName )
{
	if (PyInt_Check( pObject ))
	{
		rInt = PyInt_AsLong( pObject );
		return 0;
	}

	if (PyFloat_Check( pObject ))
	{
		rInt = (int)PyFloat_AsDouble( pObject );
		return 0;
	}

	if (PyLong_Check( pObject ))
	{
		long asLong = PyLong_AsLong( pObject );
		rInt = int( asLong );

		if (!PyErr_Occurred())
		{
			if (rInt == asLong)
			{
				return 0;
			}
		}
		else
		{
			PyErr_Clear();
		}
	}

	PyErr_Format( PyExc_TypeError, "%s must be set to an int", varName );
	return -1;

}


/**
 *	This function tries to interpret its argument as an integer,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, int64 & rInt,
	const char * varName )
{
	if (PyLong_Check( pObject ))
	{
		rInt = PyLong_AsLongLong( pObject );
		if (!PyErr_Occurred()) return 0;
	}

	if (PyInt_Check( pObject ))
	{
		rInt = PyInt_AsLong( pObject );
		return 0;
	}

	if (PyFloat_Check( pObject ))
	{
		rInt = (int64)PyFloat_AsDouble( pObject );
		return 0;
	}

	PyErr_Format( PyExc_TypeError, "%s must be set to a long", varName );
	return -1;
}

/**
 *	This function tries to interpret its argument as an unsigned
 *	64-bit integer, setting it if it is, and generating an exception
 *	otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, uint64 & rUint,
	const char * varName )
{
	if (PyLong_Check( pObject ))
	{
		rUint = PyLong_AsUnsignedLongLong( pObject );
		if (!PyErr_Occurred()) return 0;
	}

	if (PyInt_Check( pObject ))
	{
		long intValue = PyInt_AsLong( pObject );
		if (intValue >= 0)
		{
			rUint = (uint64)intValue;
			return 0;
		}
		else
		{
			PyErr_Format( PyExc_ValueError, "Cannot set %s of type unsigned long to %d",
						varName, int(intValue) );
			return -1;
		}
	}

	if (PyFloat_Check( pObject ))
	{
		rUint = (uint64)PyFloat_AsDouble( pObject );
		return 0;
	}

	PyErr_Format( PyExc_TypeError,
			"%s must be set to a unsigned long", varName );
	return -1;
}


/**
 *	This function tries to interpret its argument as an unsigned integer,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, uint & rUint,
	const char * varName )
{
	if (PyInt_Check( pObject ))
	{
		long longValue = PyInt_AsLong( pObject );
		rUint = longValue;

		if ((longValue >= 0) && (static_cast< long >( rUint ) == longValue))
		{
			return 0;
		}
	}

	if (PyFloat_Check( pObject ))
	{
		rUint = (int)PyFloat_AsDouble( pObject );
		return 0;
	}

	if (PyLong_Check( pObject ))
	{
		unsigned long asUnsignedLong = PyLong_AsUnsignedLong( pObject );
		rUint = uint( asUnsignedLong );
		if (!PyErr_Occurred() &&
				(rUint == asUnsignedLong))
		{
			return 0;
		}
		PyErr_Clear();

		long asLong = PyLong_AsLong( pObject );
		rUint = uint( asLong );
		if (!PyErr_Occurred() &&
				(asLong >= 0) &&
				(asLong == static_cast< long >( rUint ) ))
		{
			return 0;
		}
	}

	PyErr_Format( PyExc_TypeError, "%s must be set to an uint", varName );
	return -1;

}


/**
 *	This function tries to interpret its argument as a float,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, float & rFloat,
	const char * varName )
{
	double	d;
	int ret = Script::setData( pObject, d, varName );
	if (ret == 0) rFloat = float(d);
	return ret;
}


/**
 *	This function tries to interpret its argument as a double,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, double & rDouble,
	const char * varName )
{
	if (PyFloat_Check( pObject ))
	{
		rDouble = PyFloat_AsDouble( pObject );
		return 0;
	}

	if (PyInt_Check( pObject ))
	{
		rDouble = PyInt_AsLong( pObject );
		return 0;
	}

	if (PyLong_Check( pObject ))
	{
		rDouble = PyLong_AsUnsignedLong( pObject );
		if (!PyErr_Occurred()) return 0;
	}

	PyErr_Format( PyExc_TypeError, "%s must be set to a float", varName );
	return -1;

}


/**
 *	This function tries to interpret its argument as a Vector2,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Vector2 & rVector,
	const char * varName )
{
	if (PyVector<Vector2>::Check( pObject ))
	{
		rVector = ((PyVector<Vector2>*)pObject)->getVector();
		return 0;
	}
	PyErr_Clear();

	float	a,	b;

	if (PyArg_ParseTuple( pObject, "ff", &a, &b ))
	{
		rVector[0] = a;
		rVector[1] = b;
		return 0;
	}

	PyErr_Format( PyExc_TypeError,
		"%s must be set to a Vector2 or tuple of 2 floats", varName );
	return -1;
}


/**
 *	This function tries to interpret its argument as a Vector3,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Vector3 & rVector,
	const char * varName )
{
	if (PyVector<Vector3>::Check( pObject ))
	{
		rVector = ((PyVector<Vector3>*)pObject)->getVector();
		return 0;
	}
	PyErr_Clear();

	if (PyArg_ParseTuple( pObject, "fff", &rVector.x, &rVector.y, &rVector.z ))
	{
		return 0;
	}

	PyErr_Format( PyExc_TypeError,
		"%s must be set to a Vector3 or a tuple of 3 floats", varName );
	return -1;
}

/**
 *	This function tries to interpret its argument as a Vector4,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Vector4 & rVector,
	const char * varName )
{
	if (PyVector<Vector4>::Check( pObject ))
	{
		rVector = ((PyVector<Vector4>*)pObject)->getVector();
		return 0;
	}
	PyErr_Clear();

	float	a,	b,	c,	d;

	if (PyArg_ParseTuple( pObject, "ffff", &a, &b, &c, &d ))
	{
		rVector[0] = a;
		rVector[1] = b;
		rVector[2] = c;
		rVector[3] = d;
		return 0;
	}

	PyErr_Format( PyExc_TypeError,
		"%s must be set to a Vector4 or tuple of 4 floats", varName );
	return -1;
}

/**
 *	This function tries to interpret its argument as a Matrix,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Matrix & rMatrix,
	const char * varName )
{
	if (MatrixProvider::Check( pObject ))
	{
		((MatrixProvider*) pObject)->matrix(rMatrix);
		return 0;
	}

	PyErr_Format( PyExc_TypeError, "%s must be a MatrixProvider", varName );
	return -1;
}

/**
 *	This function tries to interpret its argument as a PyObject,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	It handles reference counting properly, but it does not check
 *	the type of the input object.
 *
 *	None is translated into NULL.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, PyObject * & rPyObject,
	const char * /*varName*/ )
{
	PyObject * inputObject = rPyObject;

	rPyObject = (pObject != Py_None) ? pObject : NULL;

	Py_XINCREF( rPyObject );

	if (inputObject)
	{
		WARNING_MSG( "Script::setData( pObject , rPyObject ): "
			"rPyObject is not NULL and is DECREFed and replaced by pObject\n" );
	}
	Py_XDECREF( inputObject );

	return 0;
}


/**
 *	This function tries to interpret its argument as a SmartPointer<PyObject>,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@see setData of PyObject * &
 */
int Script::setData( PyObject * pObject, SmartPointer<PyObject> & rPyObject,
	const char * /*varName*/ )
{
	PyObject * pSet = (pObject != Py_None) ? pObject : NULL;

	if (rPyObject.getObject() != pSet) rPyObject = pSet;

	return 0;
}


/**
 *	This function tries to interpret its argument as a Capabilities set,
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Capabilities & rCaps,
	const char * varName )
{
	bool good = true;

	Capabilities	wantCaps;

	// type check
	if (!PyList_Check( pObject ))
	{
		good = false;
	}

	// accumulate new caps
	int ncaps = PyList_Size( pObject );
	for (int i=0; i<ncaps && good; i++)
	{
		PyObject * argElt = PyList_GetItem( pObject, i );	// borrowed
		if (PyInt_Check( argElt ))
		{
			wantCaps.add( PyInt_AsLong( argElt ) );
		}
		else
		{
			good = false;
		}
	}

	if (!good)
	{
		PyErr_Format( PyExc_TypeError,
			"%s must be set to a list of ints", varName );
		return -1;
	}

	rCaps = wantCaps;
	return 0;
}


/**
 *	This function tries to interpret its argument as a string
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, std::string & rString,
	const char * varName )
{
	PyObjectPtr pUTF8String;

	if (PyUnicode_Check( pObject ))
	{
		pUTF8String = PyObjectPtr( PyUnicode_AsUTF8String( pObject ),
			PyObjectPtr::STEAL_REFERENCE );
		pObject = pUTF8String.get();
		if (pObject == NULL)
		{
			return -1;
		}
	}

	if (!PyString_Check( pObject ))
	{
		PyErr_Format( PyExc_TypeError, "%s must be set to a string.", varName );
		return -1;
	}

	char *ptr_cs;
	Py_ssize_t len_cs;
	PyString_AsStringAndSize( pObject, &ptr_cs, &len_cs );
	rString = std::string( ptr_cs, len_cs );
	return 0;
}


/**
 *	This function tries to interpret its argument as a wide string
 *	setting it if it is, and generating an exception otherwise.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, std::wstring & rString,
	const char * varName )
{
	if (PyString_Check( pObject ) || PyUnicode_Check( pObject ))
	{
		SmartPointer<PyObject> pUO( PyObject_Unicode( pObject ), true );

		if (pUO)
		{
			PyUnicodeObject* unicodeObj = reinterpret_cast<PyUnicodeObject*>(pUO.getObject());

			Py_ssize_t ulen = PyUnicode_GET_DATA_SIZE( unicodeObj ) / sizeof(Py_UNICODE);
			if (ulen >= 0)
			{
				// In theory this is bad, because we're assuming that 
				// sizeof(Py_UNICODE) == sizeof(wchar_t), and that ulen maps to
				// of characters that PyUnicode_AsWideChar will write into the 
				// destination buffer. In practice this is true, but for good measure
				// I'm going to stick in a compile-time assert.
				BW_STATIC_ASSERT( sizeof(Py_UNICODE) == sizeof(wchar_t), SizeOfPyUnicodeIsNotSizeOfWchar_t );
				rString.resize(ulen);

				Py_ssize_t nChars = 
					PyUnicode_AsWideChar( unicodeObj, &rString[0], ulen );

				if ( nChars != -1 )
				{
					return 0;
				}
			}
		}
	}

	PyErr_Format( PyExc_TypeError,
			"%s must be set to a wide string.", varName );
	return -1;

}


/**
 *	This function tries to interpret its argument as a string
 *	setting it if it is, and generating an exception otherwise.
 *
 *	The char * reference is assumed to point to an array on the heap,
 *	which is re-allocated to fit the size of the new string. If this
 *	isn't what you want you should probably be using std::string.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, char * & rString,
	const char * varName )
{
	std::string ss;
	int ret = Script::setData( pObject, ss, varName );
	if (ret == 0)
	{
		if (rString != NULL) delete [] rString;

		int sl = ss.length();
		rString = new char[ sl+1 ];
		memcpy( rString, ss.data(), sl );
		rString[ sl ] = 0;
	}
	return ret;
}


/**
 *	This function tries to interpret its argument as a Mercury address.
 *
 *	@return 0 for success, -1 for error (like pySetAttribute)
 */
int Script::setData( PyObject * pObject, Mercury::Address & rAddr,
					const char * varName )
{
	if (PyTuple_Check( pObject ) &&
		PyTuple_Size( pObject ) == 2 &&
		PyInt_Check( PyTuple_GET_ITEM( pObject, 0 ) ) &&
		PyInt_Check( PyTuple_GET_ITEM( pObject, 1 ) ))
	{
		rAddr.ip   = PyInt_AsLong( PyTuple_GET_ITEM( pObject, 0 ) );
		rAddr.port = uint16( PyInt_AsLong( PyTuple_GET_ITEM( pObject, 1 ) ) );
		return 0;
	}
	else
	{
		PyErr_Format( PyExc_TypeError,
			"%s must be a tuple of two ints", varName );
		return -1;
	}
}


// -----------------------------------------------------------------------------
// Section: Script::getData
// -----------------------------------------------------------------------------

/**
 * This function makes a PyObject from a bool
 */
PyObject * Script::getData( const bool data )
{
	return PyBool_FromLong( data );
}


/**
 * This function makes a PyObject from an int
 */
PyObject * Script::getData( const int data )
{
	return PyInt_FromLong( data );
}


/**
 * This function makes a PyObject from an unsigned int
 */
PyObject * Script::getData( const uint data )
{
	if (int(data) < 0)
		return PyLong_FromUnsignedLong( data );
	else
		return PyInt_FromLong( data );
}


/**
 * This function makes a PyObject form an int64
 */
PyObject * Script::getData( const int64 data )
{
	return PyLong_FromLongLong( data );
}


/**
 * This function makes a PyObject form an uint64
 */
PyObject * Script::getData( const uint64 data )
{
	return PyLong_FromUnsignedLongLong( data );
}


/**
 * This function makes a PyObject from a float
 */
PyObject * Script::getData( const float data )
{
	return PyFloat_FromDouble( data );
}


/**
 * This function makes a PyObject from a double
 */
PyObject * Script::getData( const double data )
{
	return PyFloat_FromDouble( data );
}


/**
 * This function makes a PyObject from a Vector2
 */
PyObject * Script::getData( const Vector2 & data )
{
	return new PyVectorCopy< Vector2 >( data );
}


/**
 * This function makes a PyObject from a Vector3
 */
PyObject * Script::getData( const Vector3 & data )
{
	return new PyVectorCopy< Vector3 >( data );
}


/**
 * This function makes a PyObject from a Vector4
 */
PyObject * Script::getData( const Vector4 & data )
{
	return new PyVectorCopy< Vector4 >( data );
}


/**
 * This function makes a PyObject from a Direction3D
 */
PyObject * Script::getData( const Direction3D & data )
{
	return getData( data.asVector3() );
}


/**
 * This function makes a read-only PyObject from a Vector2
 */
PyObject * Script::getReadOnlyData( const Vector2 & data )
{
	return new PyVectorCopy< Vector2 >( data, /*isReadOnly:*/ true );
}


/**
 * This function makes a PyObject from a Vector3
 */
PyObject * Script::getReadOnlyData( const Vector3 & data )
{
	return new PyVectorCopy< Vector3 >( data, /*isReadOnly:*/ true );
}


/**
 * This function makes a PyObject from a Vector4
 */
PyObject * Script::getReadOnlyData( const Vector4 & data )
{
	return new PyVectorCopy< Vector4 >( data, /*isReadOnly:*/ true );
}


/**
 * This function makes a PyVector2 that is a reference to a Vector2 member.
 */
PyObject * Script::getDataRef( PyObject * pOwner, Vector2 * pData )
{
	return new PyVectorRef< Vector2 >( pOwner, pData );
}


/**
 * This function makes a PyVector3 that is a reference to a Vector2 member.
 */
PyObject * Script::getDataRef( PyObject * pOwner, Vector3 * pData )
{
	return new PyVectorRef< Vector3 >( pOwner, pData );
}


/**
 * This function makes a PyVector4 that is a reference to a Vector2 member.
 */
PyObject * Script::getDataRef( PyObject * pOwner, Vector4 * pData )
{
	return new PyVectorRef< Vector4 >( pOwner, pData );
}

/**
 * This function makes a PyObject from a Matrix
 */
PyObject * Script::getData( const Matrix & data )
{
	PyMatrix * pyM = new PyMatrix();
	pyM->set( data );

	return pyM;
}

/**
 * This function makes a PyObject from a PyObject,
 *	and it handles reference counting properly.
 *
 *	NULL is translated into None
 */
PyObject * Script::getData( const PyObject * data )
{
	PyObject * ret = (data != NULL) ? const_cast<PyObject*>( data ) : Py_None;
	Py_INCREF( ret );
	return ret;
}


/**
 * This function makes a PyObject from a ConstSmartPointer<PyObject>,
 *	and it handles reference counting properly.
 *
 *	@see getData for const PyObject *
 */
PyObject * Script::getData( ConstSmartPointer<PyObject> data )
{
	PyObject * ret = (data ?
		const_cast<PyObject*>( data.getObject() ) : Py_None);
	Py_INCREF( ret );
	return ret;
}


/**
 * This function makes a PyObject from a Capabilities set
 */
PyObject * Script::getData( const Capabilities & data )
{
	PyObject * ret = PyList_New( 0 );
	for (uint i=0; i <= Capabilities::s_maxCap_; i++)
	{
		if (data.has( i ))
		{
			PyObject * pBit = PyInt_FromLong( i );
			PyList_Append( ret, pBit );
			Py_DECREF( pBit );
		}
	}

	return ret;
}


/**
 *	This function makes a PyObject from a string.
 */
PyObject * Script::getData( const std::string & data )
{
	PyObject * pRet = PyString_FromStringAndSize(
		const_cast<char *>( data.data() ), data.size() );

	return pRet;
}


/**
 *	This function makes a PyObject from a wide string.
 */
PyObject * Script::getData( const std::wstring & data )
{
	PyObject * pRet = PyUnicode_FromWideChar(
		const_cast<wchar_t *>( data.c_str() ), data.size() );

	return pRet;
}


/**
 *	This function makes a PyObject from a const char *.
 */
PyObject * Script::getData( const char * data )
{
	PyObject * pRet = PyString_FromString( const_cast<char *>( data ) );

	return pRet;
}


/**
 *	This function makes a PyObject from a Mercury address.
 */
PyObject * Script::getData( const Mercury::Address & addr )
{
	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SET_ITEM( pTuple, 0, Script::getData( addr.ip ) );
	PyTuple_SET_ITEM( pTuple, 1, Script::getData( addr.port ) );
	return pTuple;
}


// -----------------------------------------------------------------------------
// Section: Helpers
// -----------------------------------------------------------------------------

PyObject * Script::argCountError( const char * fn, int optas, int allas, ... )
{
	std::string argTypes;

	va_list val;
	va_start( val, allas );
	for (int i = 0; i < allas; i++)
	{
		if (i > 0) argTypes.append( ", " );

		char * argType = va_arg( val, char * );
		int argTypeLen = strlen( argType );
		if (argTypeLen > 13 && !strncmp( argType, "SmartPointer<", 13 ))
		{
			argTypes.append( argType+13, argTypeLen-13-1 );
		}
		else if (argTypeLen > 3 && !strncmp( argType+argTypeLen-3, "Ptr", 3 ))
		{
			argTypes.append( argType, argTypeLen-3 );
		}
		else
		{
			while (argTypeLen>0 && argType[argTypeLen-1] == '*')
				argTypeLen--;
			argTypes.append( argType, argTypeLen );
		}
	}
	va_end( val );

	if (allas == 0)
	{
		PyErr_Format( PyExc_TypeError, "%s() brooks no arguments.", fn );
	}
	else if (optas == allas)
	{
		const char * plural = (allas != 1) ? "s" : "";
		PyErr_Format( PyExc_TypeError,
			"%s() expects %d argument%s of type%s %s",
			fn, allas, plural, plural, argTypes.c_str() );
	}
	else
	{
		PyErr_Format( PyExc_TypeError,
			"%s() expects between %d and %d arguments of types %s",
			fn, optas, allas, argTypes.c_str() );
	}

	return NULL;
}

#if BWCLIENT_AS_PYTHON_MODULE

const std::string Script::getMainScriptPath()
{
	std::string result;
	PyObject *path = PySys_GetObject("path");
	if (PyList_Size(path) > 0)
	{
		result = PyString_AS_STRING(PyList_GetItem(path, 0));
	}
	return result;
}

#endif // BWCLIENT_AS_PYTHON_MODULE


namespace Script
{
template <> const char * zeroValueName<int>()
{
	return "0";
}

template <> const char * zeroValueName<float>()
{
	return "0.0";
}
};

// script.cpp
