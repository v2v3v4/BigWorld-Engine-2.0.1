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

#include "py_bwlog.hpp"

#include "constants.hpp"
#include "mlutil.hpp"
#include "query_params.hpp"
#include "py_query.hpp"
#include "py_user_log.hpp"

#include "cstdmf/debug.hpp"
#include "network/logger_message_forwarder.hpp"

PY_TYPEOBJECT( PyBWLog )

PY_BEGIN_METHODS( PyBWLog )

 	PY_METHOD( getComponentNames )
	PY_METHOD( getFormatStrings )
	PY_METHOD( getStrings )
	PY_METHOD( getHostnames )
 	PY_METHOD( getUsers )
	PY_METHOD( getUserLog )
 	PY_METHOD( fetch )

PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyBWLog )

	PY_ATTRIBUTE( root )

PY_END_ATTRIBUTES()

PY_FACTORY( PyBWLog, "BWLog" );


//=====================================
// HostnamePopulator
//=====================================


class HostnamePopulator : public HostnameVisitor
{
public:
	HostnamePopulator();
	~HostnamePopulator();

	bool onHost( uint32 addr, const std::string &hostname );

	PyObject *getDictionary();
private:
	void cleanupOnError();

	// Note: we don't want to delete pDict in a destructor as it will be
	// used to return with the result set. The only case it will be destroyed
	// is if an error occurs during population. This is handled in
	// cleanupOnError
	PyObject *pDict_;
};


HostnamePopulator::HostnamePopulator() :
	pDict_( PyDict_New() )
{ }


HostnamePopulator::~HostnamePopulator()
{
	Py_XDECREF( pDict_ );
}


PyObject * HostnamePopulator::getDictionary()
{
	Py_XINCREF( pDict_ );
	return pDict_;
}


void HostnamePopulator::cleanupOnError()
{
	Py_DECREF( pDict_ );
	pDict_ = NULL;
}


bool HostnamePopulator::onHost( uint32 addr, const std::string &hostname )
{
	if (pDict_ == NULL)
	{
		return false;
	}

	char *hostAddr = inet_ntoa( (in_addr&)addr );
	PyObjectPtr pyHostname( PyString_FromString( hostname.c_str() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pyHostname == NULL)
	{
		this->cleanupOnError();
		return false;
	}

	if (PyDict_SetItemString( pDict_, hostAddr, pyHostname.get() ) == -1)
	{
		this->cleanupOnError();
		return false;
	}

	return true;
}




//=====================================
// LogComponentsPopulator
//=====================================


class LogComponentsPopulator : public LogComponentsVisitor
{
public:
	LogComponentsPopulator();
	~LogComponentsPopulator();

	bool onComponent( const std::string &componentName );

	PyObject *getList();
private:
	void cleanupOnError();

	// Note: we don't want to delete pDict in a destructor as it will be
	// used to return with the result set. The only case it will be destroyed
	// is if an error occurs during population. This is handled in
	// cleanupOnError
	PyObject *pList_;
};


LogComponentsPopulator::LogComponentsPopulator() :
	pList_( PyList_New( 0 ) )
{ }


LogComponentsPopulator::~LogComponentsPopulator()
{
	Py_XDECREF( pList_ );
}


PyObject * LogComponentsPopulator::getList()
{
	Py_XINCREF( pList_ );
	return pList_;
}


void LogComponentsPopulator::cleanupOnError()
{
	Py_DECREF( pList_ );
	pList_ = NULL;
}


bool LogComponentsPopulator::onComponent( const std::string &componentName )
{
	if (pList_ == NULL)
	{
		return false;
	}

	PyObjectPtr pyString( PyString_FromString( componentName.c_str() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pyString == NULL)
	{
		this->cleanupOnError();
		return false;
	}

	if (PyList_Append( pList_, pyString.get() ) == -1 )
	{
		this->cleanupOnError();
		return false;
	}

	return true;
}




//=====================================
// Python API
//=====================================


PyObject * PyBWLog::pyNew( PyObject *args )
{
	// Enable syslog output of error messages from a python script
	DebugMsgHelper::shouldWriteToSyslog( true );

	const char *pLogDir = NULL;
	// We don't support writing via Python
	if (!PyArg_ParseTuple( args, "s", &pLogDir ))
	{
		// No need for a PyErr_Format here.. will be set by ParseTuple
		return NULL;
	}

	// Create the log object
	PyBWLogPtr pLog( new PyBWLog(), PyBWLogPtr::STEAL_REFERENCE );

	bool status = true;
	status = pLog->init( pLogDir );

	if (!status)
	{
		PyErr_Format( PyExc_IOError,
			"Log init failed for log dir '%s'.",
			(pLogDir != NULL) ? pLogDir : pLog->getLogDirectory() );
		return NULL;
	}

	// Increment the reference count so it will be owned by the interpreter
	// when we leave this scope
	Py_INCREF( pLog.getObject() );
	return pLog.getObject();
}


PyObject * PyBWLog::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


PyObject * PyBWLog::py_getComponentNames( PyObject *args )
{
	LogComponentsPopulator components;

	pLogReader_->getComponentNames( components );

	return components.getList();
}


PyObject * PyBWLog::py_getHostnames( PyObject *args )
{
	HostnamePopulator hosts;

	pLogReader_->getHostnames( hosts );

	return hosts.getDictionary();
}


PyObject * PyBWLog::py_getStrings( PyObject *args )
{
	WARNING_MSG( "PyBWLog::py_getStrings: This method is deprecated. "
		"Use getFormatStrings instead.\n" );
	return this->py_getFormatStrings( args );
}


PyObject * PyBWLog::py_getFormatStrings( PyObject *args )
{
	FormatStringList formatStrings = pLogReader_->getFormatStrings();

	PyObject *pList = PyList_New( formatStrings.size() );
	int i = 0;

	FormatStringList::iterator it = formatStrings.begin();
	while (it != formatStrings.end())
	{
		PyObject *pyFormatString = PyString_FromString( it->c_str() );
		PyList_SET_ITEM( pList, i, pyFormatString );
		++it;
		++i;
	}

	PyList_Sort( pList );

	return pList;
}


PyObject * PyBWLog::py_getUsers( PyObject *args )
{
	PyObject *pDict = PyDict_New();

	const UsernamesMap &usernames = pLogReader_->getUsernames();

	UsernamesMap::const_iterator it = usernames.begin();
	while (it != usernames.end())
	{
		PyObject *pyUid = PyInt_FromLong( it->first );
		const char *username = it->second.c_str();

		PyDict_SetItemString( pDict, username, pyUid );
		++it;
	}

	return pDict;
}


PyObject * PyBWLog::py_getUserLog( PyObject *args )
{
	int uid;
	if (!PyArg_ParseTuple( args, "i", &uid ))
	{
		// NB: not setting PyErr_Format here, PyArg_ will set something
		//     appropriate for us.
		return NULL;
	}

	PyUserLogPtr pyUserLog = this->getUserLog( uid );
	if (pyUserLog == NULL)
	{
		PyErr_Format( PyExc_KeyError,
			"No entries for UID %d in this log", uid );
		return NULL;
	}

	PyObject *ret = pyUserLog.getObject();
	Py_INCREF( ret );
	return ret;
}


PyObject * PyBWLog::py_fetch( PyObject *args, PyObject *kwargs )
{
	QueryParamsPtr pParams = this->getQueryParamsFromPyArgs( args, kwargs );

	if (pParams == NULL)
	{
		// PyErr set in getQueryParamsFromPyArgs via init()
		return NULL;
	}

	uint16 uid = pParams->getUID();
	PyUserLogPtr pUserLog = this->getUserLog( uid );

	if (pUserLog == NULL)
	{
		PyErr_Format( PyExc_RuntimeError,
			"PyBWLog::py_fetch: No user log for UID %d\n", uid );
		return NULL;
	}

	return this->fetch( pUserLog, pParams );
}



//=====================================
// Non-Python API
//=====================================

PyBWLog::PyBWLog() :
	PyObjectPlus( &PyBWLog::s_type_ ),
	pLogReader_( NULL )
{ }


PyBWLog::~PyBWLog()
{
	if (pLogReader_ != NULL)
	{
		delete pLogReader_;
	}
}


bool PyBWLog::init( const char *logDir )
{
	if (pLogReader_ != NULL)
	{
		ERROR_MSG( "PyBWLog::init: Log reader already exists, unable to "
				"re-initialise.\n" );
		return false;
	}

	pLogReader_ = new BWLogReader();
	return pLogReader_->init( logDir );
}


QueryParamsPtr PyBWLog::getQueryParamsFromPyArgs( PyObject *args,
	PyObject *kwargs )
{
	QueryParamsPtr pParams( new QueryParams(), QueryParamsPtr::NEW_REFERENCE );

	if (!pParams->init( args, kwargs, pLogReader_ ) || !pParams->isGood())
	{
		return NULL;
	}

	return pParams;
}


PyObject * PyBWLog::fetch( PyUserLogPtr pUserLog, QueryParamsPtr pParams )
{
	if (pUserLog->getUID() != pParams->getUID())
	{
		PyErr_Format( PyExc_RuntimeError, "UserLog UID %d does not match "
			"QueryParams UID %d\n", pUserLog->getUID(), pParams->getUID() );
		return NULL;
	}

	return new PyQuery( this, pParams, pUserLog->getUserLog() );
}


const char *PyBWLog::getLogDirectory() const
{
	const char *pLogDir = NULL;

	if (pLogReader_ != NULL)
	{
		pLogDir = pLogReader_->getLogDirectory();
	}

	return pLogDir;
}


const char *PyBWLog::getHostByAddr( uint32 addr ) const
{
	return pLogReader_->getHostByAddr( addr );
}


const char *PyBWLog::getComponentByID( int typeID ) const
{
	return pLogReader_->getComponentByID( typeID );
}


/**
 * Allows access to our contained BWLogReader object.
 *
 * It should only be used to temporarily gain access to the reader to resolve
 * log state and then discard. DO NOT STORE THIS POINTER!
 *
 * This is a borrowed reference.
 */
BWLogReader * PyBWLog::getBWLogReader()
{
	return pLogReader_;
}


/**
 * This method provides a simple pass through to the wrapped BWLogReader.
 *
 * @returns true on successful file map refresh, false on error.
 */
bool PyBWLog::refreshFileMaps()
{
	return pLogReader_->refreshFileMaps();
}


const LogStringInterpolator *PyBWLog::getHandlerForLogEntry(
	const LogEntry &entry )
{
	return pLogReader_->getHandlerForLogEntry( entry );
}


PyUserLogPtr PyBWLog::getUserLog( uint16 uid )
{
	UserLogReaderPtr pUserLogReader = pLogReader_->getUserLog( uid );
	if (pUserLogReader == NULL)
	{
		return NULL;
	}

	PyUserLogPtr pUserLog( new PyUserLog( pUserLogReader, this ),
										PyUserLogPtr::STEAL_REFERENCE );

	return pUserLog;
}

// pybwlog.cpp
