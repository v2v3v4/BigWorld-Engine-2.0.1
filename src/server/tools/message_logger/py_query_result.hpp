/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYQUERY_RESULT_HPP
#define PYQUERY_RESULT_HPP

#include "cstdmf/smartpointer.hpp"

class PyQueryResult;
typedef SmartPointer< PyQueryResult > PyQueryResultPtr;

#include "Python.h"

#include "py_bwlog.hpp"
#include "query_result.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "cstdmf/stdmf.hpp"

#include <string>


/**
 * This is the python object returned from a Query.
 */
class PyQueryResult : public PyObjectPlus
{
	Py_InstanceHeader( PyQueryResult );

public:
	PyQueryResult();
	PyQueryResult( const LogEntry &entry, PyBWLogPtr pBWLog,
		UserLogReaderPtr pUserLog, const LoggingComponent *component,
		const std::string &message );

private:
	// Let the reference counting take care of deletion
	~PyQueryResult();

	// Python API
	PyObject * pyGetAttribute( const char *attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( getTime(), time );
	PY_RO_ATTRIBUTE_DECLARE( getHost(), host );
	PY_RO_ATTRIBUTE_DECLARE( getPID(), pid );
	PY_RO_ATTRIBUTE_DECLARE( getAppID(), appid );
	PY_RO_ATTRIBUTE_DECLARE( getUsername(), username );
	PY_RO_ATTRIBUTE_DECLARE( getComponent(), component );
	PY_RO_ATTRIBUTE_DECLARE( getSeverity(), severity );
	PY_RO_ATTRIBUTE_DECLARE( getMessage(), message );
	PY_RO_ATTRIBUTE_DECLARE( getStringOffset(), stringOffset );

	PY_METHOD_DECLARE( py_format );


	// Properties

	QueryResult *pQueryResult_;

	// Accessors for the python attributes
	std::string getMessage() const;
	uint32 getStringOffset() const;
	double getTime() const;
	const char * getHost() const;
	int getPID() const;
	int getAppID() const;
	const char * getUsername() const;
	const char * getComponent() const;
	int getSeverity() const;
};

#endif // PYQUERY_RESULT_HPP
