/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/*
 * This file provides the core initialisation of the BWLog (bwlog.so)
 * Python module.
 */

#include "Python.h"

#include "py_bwlog.hpp"
#include "py_query_result.hpp"


PyMODINIT_FUNC
initbwlog()
{
	if (PyType_Ready( &PyBWLog::s_type_ ) < 0)
	{
		return;
	}

	const char *MODULE_NAME = "bwlog";
	PyObject *m = Py_InitModule3( (char *)MODULE_NAME, NULL,
									"Interface to BWLog files");

	if (m == NULL)
	{
		return;
	}

#	define INSERT_CLASS( CLASS, NAME )								\
	Py_INCREF( &CLASS::s_type_ );									\
	PyModule_AddObject( m, NAME, (PyObject*)&CLASS::s_type_ );		\

	INSERT_CLASS( PyBWLog, "BWLog" );

	INSERT_CLASS( PyQueryResult, "Result" );

	// Add in all the flags the Python will need
#	define INSERT_BWLOG_CONSTANT( NAME )							\
	PyModule_AddIntConstant( m, #NAME, (int)NAME );

	INSERT_BWLOG_CONSTANT( SHOW_DATE );
	INSERT_BWLOG_CONSTANT( SHOW_TIME );
	INSERT_BWLOG_CONSTANT( SHOW_HOST );
	INSERT_BWLOG_CONSTANT( SHOW_USER );
	INSERT_BWLOG_CONSTANT( SHOW_PID );
	INSERT_BWLOG_CONSTANT( SHOW_APPID );
	INSERT_BWLOG_CONSTANT( SHOW_PROCS );
	INSERT_BWLOG_CONSTANT( SHOW_SEVERITY );
	INSERT_BWLOG_CONSTANT( SHOW_MESSAGE );
	INSERT_BWLOG_CONSTANT( SHOW_ALL );

	INSERT_BWLOG_CONSTANT( DONT_INTERPOLATE );
	INSERT_BWLOG_CONSTANT( PRE_INTERPOLATE );
	INSERT_BWLOG_CONSTANT( POST_INTERPOLATE );

	INSERT_BWLOG_CONSTANT( LOG_BEGIN );
	INSERT_BWLOG_CONSTANT( LOG_END );

#	define INSERT_BWLOG_QUERY_CONSTANT( NAME )						\
	PyModule_AddIntConstant( m, #NAME, (int)QUERY_##NAME );

	INSERT_BWLOG_QUERY_CONSTANT( FORWARDS );
	INSERT_BWLOG_QUERY_CONSTANT( BACKWARDS );

#	define INSERT_CONSTANT( NAME )							\
	PyModule_AddIntConstant( m, #NAME, NAME );

	INSERT_CONSTANT( MESSAGE_LOGGER_MSG );
	INSERT_CONSTANT( MESSAGE_LOGGER_REGISTER );
	INSERT_CONSTANT( MESSAGE_LOGGER_PROCESS_BIRTH );
	INSERT_CONSTANT( MESSAGE_LOGGER_PROCESS_DEATH );

	// Add in what this version of MessageLogger is called
	PyModule_AddStringConstant( m, "VERSION_NAME", "message_logger" );


	/* Add the severity levels to the python module */
	PyObject *severityLevels = PyDict_New();
	PyModule_AddObject( m, "SEVERITY_LEVELS", severityLevels );

#	define INSERT_SEVERITY( LEVEL )									\
	PyDict_SetItemString( severityLevels, messagePrefix( LEVEL ),	\
		PyInt_FromLong( LEVEL ) );

	INSERT_SEVERITY( MESSAGE_PRIORITY_TRACE );
	INSERT_SEVERITY( MESSAGE_PRIORITY_DEBUG );
	INSERT_SEVERITY( MESSAGE_PRIORITY_INFO );
	INSERT_SEVERITY( MESSAGE_PRIORITY_NOTICE );
	INSERT_SEVERITY( MESSAGE_PRIORITY_WARNING );
	INSERT_SEVERITY( MESSAGE_PRIORITY_ERROR );
	INSERT_SEVERITY( MESSAGE_PRIORITY_CRITICAL );
	INSERT_SEVERITY( MESSAGE_PRIORITY_HACK );
	INSERT_SEVERITY( MESSAGE_PRIORITY_SCRIPT );

	// Add in the minimal set of component names that can be registered
	PyObject *minNames = PyList_New( 0 );
#	define INSERT_COMPONENT_NAME( NAME )				\
	{													\
		PyObject *str = PyString_FromString( NAME );	\
		PyList_Append( minNames, str );					\
		Py_DECREF( str );								\
	}

	INSERT_COMPONENT_NAME( "CellApp" );
	INSERT_COMPONENT_NAME( "BaseApp" );
	INSERT_COMPONENT_NAME( "LoginApp" );
	INSERT_COMPONENT_NAME( "DBMgr" );
	INSERT_COMPONENT_NAME( "CellAppMgr" );
	INSERT_COMPONENT_NAME( "BaseAppMgr" );

	if (PyModule_AddObject( m, "BASE_COMPONENT_NAMES", minNames ) == -1)
	{
		ERROR_MSG( "initbwlog: Unable to add base ComponentNames to module\n" );
	}
}
