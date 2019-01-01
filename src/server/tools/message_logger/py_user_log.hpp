/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYUSER_LOG_HPP
#define PYUSER_LOG_HPP

#include "cstdmf/smartpointer.hpp"

class PyUserLog;
typedef SmartPointer< PyUserLog > PyUserLogPtr;

#include "Python.h"

#include "py_bwlog.hpp"
#include "user_log_reader.hpp"

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"


class PyUserLog : public PyObjectPlus
{
	Py_InstanceHeader( PyUserLog );

public:
	PyUserLog( UserLogReaderPtr pUserLogReader, PyBWLogPtr pBWLog );

	/* Python API */
	PyObject* pyGetAttribute( const char *attr );
	PY_RO_ATTRIBUTE_DECLARE( getUID(), uid );
	PY_RO_ATTRIBUTE_DECLARE( getUsername(), username );

	PY_KEYWORD_METHOD_DECLARE( py_fetch );
	PY_METHOD_DECLARE( py_getComponents );
	PY_METHOD_DECLARE( py_getEntry );
	PY_METHOD_DECLARE( py_getSegments );


	/* Non-Python API */
	UserLogReaderPtr getUserLog() const;
	uint16 getUID() const;
private:
	// As we're inheriting from PyObjectPlus we should never delete
	// the object, and rely on Py_DECREF to destroy the object
	//~PyUserLog();

	std::string getUsername() const;

	UserLogReaderPtr pUserLogReader_;

	PyBWLogPtr pBWLog_;
};

#endif // PYUSER_LOG_HPP
