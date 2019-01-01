/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYBWLOG_HPP
#define PYBWLOG_HPP

#include "cstdmf/smartpointer.hpp"

class PyBWLog;
typedef SmartPointer< PyBWLog > PyBWLogPtr;

#include "Python.h"

#include "bwlog_reader.hpp"
#include "py_user_log.hpp"

#include "pyscript/script.hpp" // required for PyFactoryMethodLink
#include "pyscript/pyobject_plus.hpp"


/**
 * This class provides the Python wrapper for the BWLogReader class.
 */
class PyBWLog : public PyObjectPlus
{
	Py_InstanceHeader( PyBWLog )

public:

	/* Python API */
	PyObject* pyGetAttribute( const char *attr );
	PY_METHOD_DECLARE( py_getComponentNames );
	PY_METHOD_DECLARE( py_getFormatStrings );
	PY_METHOD_DECLARE( py_getStrings );
	PY_METHOD_DECLARE( py_getHostnames );
	PY_METHOD_DECLARE( py_getUsers );
	PY_METHOD_DECLARE( py_getUserLog );
	PY_KEYWORD_METHOD_DECLARE( py_fetch );

	// TODO: deprecated interface. remove the 'root' attribute in 2.0, name
	//       it something more appropriate.
	PY_RO_ATTRIBUTE_DECLARE( getLogDirectory(), root );
	PY_FACTORY_DECLARE();

	/* Non-Python API */
	PyBWLog();

	bool init( const char *logDir );

	QueryParamsPtr getQueryParamsFromPyArgs( PyObject *args, PyObject *kwargs );
	PyObject *fetch( PyUserLogPtr pUserLog, QueryParamsPtr pParams );

	const char *getLogDirectory() const;
	const char *getHostByAddr( uint32 addr ) const;
	const char *getComponentByID( int typeID ) const;

	BWLogReader *getBWLogReader();

	const LogStringInterpolator *getHandlerForLogEntry( const LogEntry &entry );

	bool refreshFileMaps();

private:
	// Deriving from PyObjectPlus, use Py_DECREF rather than delete
	~PyBWLog();

	PyUserLogPtr getUserLog( uint16 uid );

	BWLogReader *pLogReader_;
};

#endif // PYBWLOG_HPP
