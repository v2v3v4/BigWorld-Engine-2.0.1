/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "py_user_log.hpp"

#include "log_string_interpolator.hpp"
#include "py_query_result.hpp"

PY_TYPEOBJECT( PyUserLog )

PY_BEGIN_METHODS( PyUserLog )

	PY_METHOD( getComponents )
	PY_METHOD( getEntry )
	PY_METHOD( getSegments )
	PY_METHOD( fetch )

PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyUserLog )

	PY_ATTRIBUTE( uid )
	PY_ATTRIBUTE( username )

PY_END_ATTRIBUTES()



//=====================================
// UserComponentPopulator
//=====================================

class UserComponentPopulator : public UserComponentVisitor
{
public:
	UserComponentPopulator();
	~UserComponentPopulator();

	bool onComponent( const LoggingComponent &component );

	PyObject * getList();
private:
	void cleanupOnError();

	// Note: we don't want to delete pList in a destructor as it will be
	// used to return with the result set. The only case it will be destroyed
	// is if an error occurs during population. This is handled in
	// cleanupOnError
	PyObject *pList_;
};


UserComponentPopulator::UserComponentPopulator() :
	pList_( PyList_New( 0 ) )
{ }


UserComponentPopulator::~UserComponentPopulator()
{
	Py_XDECREF( pList_ );
}


PyObject * UserComponentPopulator::getList()
{
	Py_XINCREF( pList_ );
	return pList_;
}


void UserComponentPopulator::cleanupOnError()
{
	Py_DECREF( pList_ );
	pList_ = NULL;
}


bool UserComponentPopulator::onComponent( const LoggingComponent &component )
{
	if (pList_ == NULL)
	{
		return false;
	}

	PyObjectPtr pyEntry( Py_BuildValue( "sii(si)",
			component.msg_.componentName_.c_str(),
			component.msg_.pid_,
			component.appid_,
			component.firstEntry_.getSuffix(),
			component.firstEntry_.getIndex() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pyEntry == NULL)
	{
		this->cleanupOnError();
		return false;
	}

	if (PyList_Append( pList_, pyEntry.get() ) == -1)
	{
		this->cleanupOnError();
		return false;
	}

	return true;
}



//=====================================
// UserSegmentPopulator
//=====================================


class UserSegmentPopulator : public UserSegmentVisitor
{
public:
	UserSegmentPopulator();
	~UserSegmentPopulator();

	bool onSegment( const UserSegmentReader *pSegment );

	PyObject *getList();
private:
	void cleanupOnError();

	// Note: we don't want to delete pList in a destructor as it will be
	// used to return with the result set. The only case it will be destroyed
	// is if an error occurs during population. This is handled in
	// cleanupOnError
	PyObject *pList_;
};


UserSegmentPopulator::UserSegmentPopulator() :
	pList_( PyList_New( 0 ) )
{ }


UserSegmentPopulator::~UserSegmentPopulator()
{
	Py_XINCREF( pList_ );
}


PyObject * UserSegmentPopulator::getList()
{
	Py_XINCREF( pList_ );
	return pList_;
}


void UserSegmentPopulator::cleanupOnError()
{
	Py_DECREF( pList_ );
	pList_ = NULL;
}


bool UserSegmentPopulator::onSegment( const UserSegmentReader *pSegment )
{
	if (pList_ == NULL)
	{
		return false;
	}

	PyObjectPtr pyEntry( Py_BuildValue( "sddiii",
			pSegment->getSuffix().c_str(),
			pSegment->getStartLogTime().asDouble(),
			pSegment->getEndLogTime().asDouble(),
			pSegment->getNumEntries(),
			pSegment->getEntriesLength(),
			pSegment->getArgsLength() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pyEntry == NULL)
	{
		this->cleanupOnError();
		return false;
	}

	if (PyList_Append( pList_, pyEntry.get() ) == -1)
	{
		this->cleanupOnError();
		return false;
	}

	return true;
}



//=====================================
// Python API
//=====================================


PyObject * PyUserLog::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


PyObject * PyUserLog::py_fetch( PyObject *args, PyObject *kwargs )
{
	if (pBWLog_ == NULL)
	{
		PyErr_Format( PyExc_RuntimeError, "UserLog missing reference to a "
			"valid BWLog instance." );
		return NULL;
	}

	QueryParamsPtr pParams = pBWLog_->getQueryParamsFromPyArgs( args, kwargs );
	if (pParams == NULL)
	{
		// Sets PyErr in QueryParams::init
		return NULL;
	}

	// Make sure the QueryParams know where the query is coming from.
	pParams->setUID( this->getUID() );

	return pBWLog_->fetch( this, pParams );
}


/**
 * Returns a list of tuples corresponding to the components in this user's log.
 * Each tuple takes the format (name, pid, appid, firstEntry). The
 * 'firstEntry' member is a 2-tuple of (suffix, index).
 */
PyObject * PyUserLog::py_getComponents( PyObject *args )
{
	UserComponentPopulator components;

	pUserLogReader_->getUserComponents( components );

	return components.getList();
}


// TODO: this method assumes the user has knowledge of the suffix / index
//       structure of the logs. this knowledge should be abstracted away from
//       the user.
/**
 * Returns a PyQueryResult object corresponding to the provided tuple of
 * (suffix, index).
 */
PyObject * PyUserLog::py_getEntry( PyObject *args )
{
	const char *suffix;
	int index;

	if (!PyArg_ParseTuple( args, "(si)", &suffix, &index ))
	{
		return NULL;
	}

	LogEntryAddress entryAddr( suffix, index );
	LogEntry entry;
	UserSegmentReader *pSegment = NULL;

	// We might not be able to get an entry because the segment it lives in may
	// have been rolled.
	if (!pUserLogReader_->getEntryAndSegment( entryAddr, entry, pSegment,
			false ))
	{
		Py_RETURN_NONE;
	}

	// Get the fmt string to go with it
	const LogStringInterpolator *pHandler =
		pBWLog_->getHandlerForLogEntry( entry );

	if (pHandler == NULL)
	{
		PyErr_Format( PyExc_LookupError,
			"PyUserLog::py_getEntry: Unknown string offset: %d",
			(int)entry.stringOffset_ );
		return NULL;
	}

	// Get the Component to go with it
	const LoggingComponent *pComponent =
					pUserLogReader_->getComponentByID( entry.componentID_ );
	if (pComponent == NULL)
	{
		PyErr_Format( PyExc_LookupError,
			"PyUserLog::py_getEntry: Unknown component id: %d",
			entry.componentID_ );
		return NULL;
	}

	// Interpolate the message given the entry details
	std::string message;
	if (!pSegment->interpolateMessage( entry, pHandler, message ))
	{
		PyErr_Format( PyExc_LookupError, "PyUserLog::py_getEntry: "
			"Failed to resolve log message." );
		return NULL;
	}

	return new PyQueryResult( entry, pBWLog_, pUserLogReader_,
								pComponent, message );
}


/**
 * Returns a list of tuples corresponding to the segments in this UserLog.
 * Each tuple takes the format (suffix, starttime, endtime, nEntries, size
 * of entries file, size of args file).
 */
PyObject * PyUserLog::py_getSegments( PyObject *args )
{
	UserSegmentPopulator segments;

	pUserLogReader_->visitAllSegmentsWith( segments );

	return segments.getList();
}



//=====================================
// Non-Python API
//=====================================


PyUserLog::PyUserLog( UserLogReaderPtr pUserLogReader, PyBWLogPtr pBWLog ) :
	PyObjectPlus( &PyUserLog::s_type_ ),
	pUserLogReader_( pUserLogReader ),
	pBWLog_( pBWLog )
{ }


/**
 * This method allows other Python modules to get a handle on the
 * UserLogReader this module is using.
 *
 * It is used primarily by BWLogModule to pass the UserLogReader into
 * QueryModule.
 */
UserLogReaderPtr PyUserLog::getUserLog() const
{
	return pUserLogReader_;
}


uint16 PyUserLog::getUID() const
{
	return pUserLogReader_->getUID();
}


std::string PyUserLog::getUsername() const
{
	return pUserLogReader_->getUsername();
}
