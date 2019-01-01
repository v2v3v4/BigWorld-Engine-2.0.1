/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "py_query_result.hpp"

#include "log_entry.hpp"
#include "logging_component.hpp"
#include "query_result.hpp"
#include "user_log.hpp"


PY_TYPEOBJECT( PyQueryResult );

PY_BEGIN_METHODS( PyQueryResult )

	PY_METHOD( format );

PY_END_METHODS();

PY_BEGIN_ATTRIBUTES( PyQueryResult )

	PY_ATTRIBUTE( time )
	PY_ATTRIBUTE( host )
	PY_ATTRIBUTE( pid )
	PY_ATTRIBUTE( appid )
	PY_ATTRIBUTE( username )
	PY_ATTRIBUTE( component )
	PY_ATTRIBUTE( severity )
	PY_ATTRIBUTE( message )
	PY_ATTRIBUTE( stringOffset )

PY_END_ATTRIBUTES();



//=====================================
// Python API
//=====================================


PyObject * PyQueryResult::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


int PyQueryResult::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


PyObject * PyQueryResult::py_format( PyObject *args )
{
	if (pQueryResult_ == NULL)
	{
		PyErr_Format( PyExc_ValueError, "Query result has not been properly "
			"initialised." );
		return NULL;
	}

	unsigned flags = SHOW_ALL;
	if (!PyArg_ParseTuple( args, "|I", &flags ))
	{
		return NULL;
	}

	int len;

	const char *line = pQueryResult_->format( flags, &len );
 	return PyString_FromStringAndSize( line, len );
}



//=====================================
// Non-Python API
//=====================================


PyQueryResult::PyQueryResult() :
	PyObjectPlus( &s_type_ ),
	pQueryResult_( new QueryResult() )
{ }


PyQueryResult::PyQueryResult( const LogEntry &entry, PyBWLogPtr pBWLog,
	UserLogReaderPtr pUserLog, const LoggingComponent *pComponent,
	const std::string &message ) :

	PyObjectPlus( &s_type_ ),
	pQueryResult_( new QueryResult( entry, pBWLog->getBWLogReader(),
					pUserLog.getObject(), pComponent, message ) )
{ }


PyQueryResult::~PyQueryResult()
{
	if (pQueryResult_)
	{
		delete pQueryResult_;
	}
}


std::string PyQueryResult::getMessage() const
{
	return pQueryResult_->getMessage();
}


uint32 PyQueryResult::getStringOffset () const
{
	return pQueryResult_->getStringOffset();
}


double PyQueryResult::getTime() const
{
	return pQueryResult_->getTime();
}


const char * PyQueryResult::getHost() const
{
	return pQueryResult_->getHost();
}


int PyQueryResult::getPID() const
{
	return pQueryResult_->getPID();
}


int PyQueryResult::getAppID() const
{
	return pQueryResult_->getAppID();
}


const char * PyQueryResult::getUsername() const
{
	return pQueryResult_->getUsername();
}


const char * PyQueryResult::getComponent() const
{
	return pQueryResult_->getComponent();
}


int PyQueryResult::getSeverity() const
{
	return pQueryResult_->getSeverity();
}
