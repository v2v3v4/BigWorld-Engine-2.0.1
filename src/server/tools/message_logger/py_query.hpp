/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYQUERY_HPP
#define PYQUERY_HPP

#include "cstdmf/smartpointer.hpp"
#include "pyscript/pyobject_plus.hpp"

class PyQuery;
typedef SmartPointer< PyQuery > PyQueryPtr;

#include "log_string_interpolator.hpp"
#include "py_bwlog.hpp"
#include "py_query_result.hpp"
#include "query_range.hpp"
#include "user_log_reader.hpp"

#include "pyscript/script.hpp" // Only required for PyObjectPtr

/**
 * Generator-style object for iterating over query results.
 */
class PyQuery : public PyObjectPlus
{
	Py_InstanceHeader( PyQuery );

public:
	//PyQuery( BWLog *pLog, QueryParams *pParams, UserLog *pUserLog );
	PyQuery( PyBWLogPtr pBWLog, QueryParamsPtr pParams,
		UserLogReaderPtr pUserLog );

	/* Python API */
	PyObject * pyGetAttribute( const char *attr );

	PY_METHOD_DECLARE( py_get );
	PY_METHOD_DECLARE( py_inReverse );
	PY_METHOD_DECLARE( py_getProgress );
	PY_METHOD_DECLARE( py_resume );
	PY_METHOD_DECLARE( py_tell );
	PY_METHOD_DECLARE( py_seek );
	PY_METHOD_DECLARE( py_step );
	PY_METHOD_DECLARE( py_setTimeout );


	/* Non-Python API */
	PyObject * next();

private:
	// This object should only be deleted by Py_DECREF
	//~PyQuery();

	PyQueryResult * getResultForEntry( const LogEntry &entry, bool filter );

	PyObject * getContextLines();

	PyObject * updateContextLines( PyQueryResult *pResult, 
		int numLinesOfContext );

	bool interpolate( const LogStringInterpolator *handler,
		QueryRangePtr pRange, std::string &dest );

protected:
	QueryParamsPtr pParams_;
	QueryRangePtr pRange_;

	PyBWLogPtr pBWLog_;

	UserLogReaderPtr pUserLogReader_;

	PyQueryResultPtr pContextResult_;

	QueryRange::iterator contextPoint_;
	QueryRange::iterator contextCurr_;
	QueryRange::iterator mark_;

	bool separatorReturned_;

	PyObjectPtr pCallback_;

	float timeout_;
	int timeoutGranularity_;
};

#endif // PYQUERY_HPP
