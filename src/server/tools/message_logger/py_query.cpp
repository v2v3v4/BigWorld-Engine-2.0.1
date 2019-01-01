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

#include "py_query.hpp"

#include "query_params.hpp"


namespace // anonymous
{

/**
 *	tp_iter function for PyQuery objects.
 */
PyObject *query_iter( PyObject *pQuery )
{
	Py_INCREF( pQuery );
	return pQuery;
}

/**
 *	tp_iternext function for PyQuery objects.
 */
PyObject *query_iternext( PyObject *pIter )
{
	PyQuery *pQuery = static_cast< PyQuery * >( pIter );
	return pQuery->next();
}

} // namespace (anonymous)

PY_TYPEOBJECT_WITH_ITER( PyQuery, query_iter, query_iternext );

PY_BEGIN_METHODS( PyQuery )

	PY_METHOD( get )
	PY_METHOD( inReverse )
	PY_METHOD( getProgress )
	PY_METHOD( resume )
	PY_METHOD( tell )
	PY_METHOD( seek )
	PY_METHOD( step )
	PY_METHOD( setTimeout )

PY_END_METHODS();

PY_BEGIN_ATTRIBUTES( PyQuery );

PY_END_ATTRIBUTES();



// -----------------------------------------------------------------------------
// Section: Python API
// -----------------------------------------------------------------------------


PyObject * PyQuery::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 * Fetch at most the next 'n' search results. Passing 0 as the argument means
 * fetch all possible results.
 */
PyObject * PyQuery::py_get( PyObject *args )
{
	int n = 0;

	if (!PyArg_ParseTuple( args, "|i", &n ))
	{
		return NULL;
	}

	PyObject *list = PyList_New( 0 );
	for (int i=0; n == 0 || i < n; i++)
	{
		PyObject *result = this->next();

		if (result == NULL)
		{
			PyErr_Clear();
			return list;
		}
		else
		{
			PyList_Append( list, result );
		}
	}

	return list;
}


/**
 * Returns true if this query will return results in reverse order
 */
PyObject * PyQuery::py_inReverse( PyObject *args )
{

	if (pParams_->getDirection() == QUERY_BACKWARDS)
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}


/**
 * Returns a tuple containing the number of entries seen as well as the
 * total number of entries in the provided range.
 */
PyObject * PyQuery::py_getProgress( PyObject *args )
{

	if (!pRange_->areBoundsGood())
	{
		return Py_BuildValue( "(ii)", 0, 0 );
	}

	return Py_BuildValue( "(ii)",
			pRange_->getNumEntriesVisited(),
			pRange_->getTotalEntries() );
}


/**
 * Resume a query that was started earlier, recalculating the endpoint of the
 * search and updating any mappings that might have changed.
 */
PyObject * PyQuery::py_resume( PyObject *args )
{
	pBWLog_->refreshFileMaps();
	pUserLogReader_->reloadFiles();
	pRange_->resume();

	// TODO : Commit check the block that has now moved into pRange->resume

	Py_RETURN_NONE;
}


/**
 * If you pass 'True' as an argument to this method, it will return the address
 * of the end of the query.  Without an argument, the expected behaviour of
 * returning the current position of the query is exhibited.
 */
PyObject * PyQuery::py_tell( PyObject *args )
{

	char tellEnd = 0;
	if (!PyArg_ParseTuple( args, "|b", &tellEnd ))
	{
		return NULL;
	}

	// TODO: the following funcationality is a candidate to move into
	//       QueryRange
	QueryRangeIterator it = tellEnd ? pRange_->iter_end() : pRange_->iter_curr();

	if (it.isGood())
	{
		const UserSegmentReader *pSegment =
			pUserLogReader_->getUserSegment( it.getSegmentNumber() );
		if (pSegment == NULL)
		{
			PyErr_Format( PyExc_ValueError, "No UserSegment found for "
				"segment %d", it.getSegmentNumber() );
			return NULL;
		}

		return Py_BuildValue( "(sii)", pSegment->getSuffix().c_str(),
			it.getEntryNumber(), it.getMetaOffset() );
	}

	Py_RETURN_NONE;
}


PyObject * PyQuery::py_seek( PyObject *args )
{
	const char *suffix;
	int segmentNum;
	int entryNum;
	int metaOffset;
	int postIncrement = 0;

	if (!PyArg_ParseTuple( args, "(sii)|i",
			&suffix, &entryNum, &metaOffset, &postIncrement ))
	{
		return NULL;
	}

	segmentNum = pUserLogReader_->getSegmentIndexFromSuffix( suffix );
	if (segmentNum == -1)
	{
		PyErr_Format( PyExc_KeyError, "Unknown segment suffix '%s'", suffix );
		return NULL;
	}

	if (pRange_->seek( segmentNum, entryNum, metaOffset, postIncrement ))
	{
		Py_RETURN_NONE;
	}

	PyErr_Format( PyExc_RuntimeError,
		"(%d,%d) is not within the current extents of this query %s.",
		segmentNum, entryNum, pRange_->asString().c_str() );
	return NULL;
}


/**
 * This method implements functionality similar to that of 
 * fseek( fp, +/-1, SEEK_CUR ), however for log queries.
 *
 * Seeking forwards can be done as many times as you want, but you can only
 * seek backwards once, much like ungetc().  Note that passing FORWARDS for the
 * argument means search forwards with respect to the query's direction, as
 * opposed to towards the end of the log.  Same goes for BACKWARDS.
 */
PyObject * PyQuery::py_step( PyObject *args )
{
	int offset;

	if (!PyArg_ParseTuple( args, "i", &offset ))
	{
		return NULL;
	}

	if (offset == QUERY_BACKWARDS)
	{
		pRange_->step_back();
		Py_RETURN_NONE;
	}
	else if (offset == QUERY_FORWARDS)
	{
		pRange_->step_forward();
		Py_RETURN_NONE;
	}

	PyErr_Format( PyExc_ValueError,
		"You must pass either QUERY_FORWARDS or QUERY_BACKWARDS to step()" );
	return NULL;
}


/**
 * This method sets a timeout callback that will be called inside Query::next()
 * at a specified interval. If the callback raises an exception, this will
 * cause PyQuery::next() to abort prematurely. The primary use of this
 * mechanism is to limit the runtime of sparse queries.
 */
PyObject * PyQuery::py_setTimeout( PyObject *args )
{
	PyObject *pFunc = NULL;
	float timeout = 0;
	int granularity = 1000;

	if (!PyArg_ParseTuple( args, "fO|i", &timeout, &pFunc, &granularity ))
	{
		return NULL;
	}

	if (!PyCallable_Check( pFunc ))
	{
		PyErr_Format( PyExc_TypeError, "Callback argument is not callable" );
		return NULL;
	}

	// Insert new callback
	timeout_ = timeout;
	pCallback_ = pFunc;
	timeoutGranularity_ = granularity;

	Py_RETURN_NONE;
}



// ------------------------------------------------------------------------------
// Section: Non-Python API
// ------------------------------------------------------------------------------

/**
 * Constructor.
 *
 * @param pBWLog 			The log object.
 * @param pParams			The query parameters.
 * @param pUserLogReader	The log reader.
 */
PyQuery::PyQuery( PyBWLogPtr pBWLog, QueryParamsPtr pParams,
		UserLogReaderPtr pUserLogReader ) :
	PyObjectPlus( &s_type_ ),
	pParams_( pParams ),
	pRange_( new QueryRange( pParams_, pUserLogReader ),
				QueryRangePtr::NEW_REFERENCE ),
	pBWLog_( pBWLog ),
	pUserLogReader_( pUserLogReader ),
	pContextResult_( NULL ),
	contextPoint_( pRange_.get() ),
	contextCurr_(  pRange_.get() ),
	mark_(         pRange_.get() ),
	separatorReturned_( false ),
	pCallback_( NULL ),
	timeout_( 0 ),
	timeoutGranularity_( 0 )
{ }


/**
 * This method returns the PyQueryResult* for the provided entry, or NULL if
 * filter is true and the entry doesn't pass the filter criteria (raises an
 * appropriate exception).
 */
PyQueryResult * PyQuery::getResultForEntry( const LogEntry &entry, bool filter )
{
	// Get the fmt string to go with it
	const LogStringInterpolator *pHandler =
		pBWLog_->getHandlerForLogEntry( entry );

	if (pHandler == NULL)
	{
		PyErr_Format( PyExc_LookupError,
			"PyQuery::getResultForEntry: Unknown string offset: %d",
			(int)entry.stringOffset_ );
		return NULL;
	}

	// Get the Component to go with it
	const LoggingComponent *pComponent =
		pUserLogReader_->getComponentByID( entry.componentID_ );
	if (pComponent == NULL)
	{
		PyErr_Format( PyExc_LookupError,
			"PyQuery::getResultForEntry: Unknown component id: %d",
			entry.componentID_ );
		return NULL;
	}

	std::string matchText = pHandler->fmt();

	if (pParams_->isPreInterpolate())
	{
		if (!this->interpolate( pHandler, pRange_, matchText ))
		{
			PyErr_Format( PyExc_LookupError,
					"Invalid interpolation data for '%s'", matchText.c_str() );
			return NULL;
		}
	}

	// Candidate for cleanup, move all of this to a 'handleFiltering' method
	// Filter
	if (filter)
	{
		if (!pParams_->validateAddress( pComponent->getAddress().ip ))
		{
			return NULL;
		}

		if (!pParams_->validatePID( pComponent->msg_.pid_ ))
		{
			return NULL;
		}

		if (pComponent->appid_ &&
			!pParams_->validateAppID( pComponent->appid_ ))
		{
			return NULL;
		}

		if (!pParams_->validateProcessType( pComponent->typeid_ ))
		{
			return NULL;
		}

		if (!pParams_->validateMessagePriority( entry.messagePriority_))
		{
			return NULL;
		}

		if (!pParams_->validateIncludeRegex( matchText.c_str() ))
		{
			return NULL;
		}

		if (!pParams_->validateExcludeRegex( matchText.c_str() ))
		{
			return NULL;
		}
	}

	if (pParams_->isPostInterpolate())
	{
		if (!this->interpolate( pHandler, pRange_, matchText ))
		{
			PyErr_Format( PyExc_LookupError,
					"Invalid interpolation data for '%s'", matchText.c_str() );
			return NULL;
		}
	}

	return new PyQueryResult( entry, pBWLog_, pUserLogReader_,
								pComponent, matchText );
}


PyObject * PyQuery::getContextLines()
{
	// If there's a gap between the start of this context and the last
	// result returned, give back a separator line
	if (!separatorReturned_ && mark_.isGood() && contextCurr_ - mark_ > 1)
	{
		separatorReturned_ = true;
		return new PyQueryResult();
	}

	PyQueryResult * pResult = NULL;

	// If we're positioned over the actual context point, re-use the Result
	// we fetched earlier
	if (contextCurr_ == contextPoint_)
	{
		pResult = pContextResult_.get();
		Py_INCREF( pResult );
	}

	// Otherwise read it from disk like normal
	else
	{
		LogEntry entry;

		if (!pUserLogReader_->getEntryAndQueryRange(
				contextCurr_.getAddress(), entry, pRange_ ))
		{
			PyErr_Format( PyExc_LookupError,
				"Couldn't fetch context entry @ %s",
				contextCurr_.asString().c_str() );
			return NULL;
		}

		pResult = this->getResultForEntry( entry, false );

		if (pResult == NULL)
			return NULL;
	}

	mark_ = contextCurr_;
	++contextCurr_;

	// Terminate context stuff if we've got enough
	if (contextCurr_.getMetaOffset() > 0 ||
		contextCurr_ - contextPoint_ > pParams_->getNumLinesOfContext())
	{
		pContextResult_ = NULL;
		separatorReturned_ = false;
	}

	return pResult;
}


/**
 * This method is used by the iterator helpers.
 */
PyObject * PyQuery::next()
{
	// If we're fetching context, we don't need to search
	if (pContextResult_)
	{
		return this->getContextLines();
	}

	uint64 startTime = timestamp();
	LogEntry entry;

	for (int i=0; pRange_->getNextEntry( entry ); i++)
	{
		// Trigger timeout callback if necessary
		if (pCallback_ != NULL &&
			i % timeoutGranularity_ == 0 &&
			(timestamp() - startTime)/stampsPerSecondD() > timeout_)
		{
			// If the callback raises an exception, terminate the search.
			if (PyObject_CallFunction( pCallback_.getObject(), "O", this ) ==
				NULL)
			{
				return NULL;
			}

			startTime = timestamp();
		}

		PyQueryResult *pResult = this->getResultForEntry( entry, true );

		// Skip to next entry if this one was filtered out
		if (pResult == NULL)
		{
			continue;
		}

		int numLinesOfContext = pParams_->getNumLinesOfContext();

		if (numLinesOfContext != 0)
		{
			return this->updateContextLines( pResult, numLinesOfContext );
		}

		// Remember the position of the most recent result actually returned
		mark_ = pRange_->iter_curr();
		--mark_;

		return pResult;
	}


	PyErr_SetNone( PyExc_StopIteration );
	return NULL;
}


/**
 * This method is used by next() to update the current Query context.
 */
PyObject * PyQuery::updateContextLines( PyQueryResult * pResult, 
		int numLinesOfContext )
{
	// If we need context, set the context fields and re-execute next()
	pContextResult_ = PyQueryResultPtr( pResult,
								PyQueryResultPtr::STEAL_REFERENCE );

	// TODO: some of this functionality could be moved into QueryRange
	//       .. operations on the curr_ iterator..
	contextPoint_ = pRange_->iter_curr();
	--contextPoint_;

	contextCurr_ = contextPoint_;

	// Cycle contextCurr_ backwards if it's ahead of the mark_
	if (!mark_.isGood() || mark_ < contextCurr_)
	{
		for (int j = 0; j < numLinesOfContext; ++j)
		{
			if (mark_.isGood() && contextCurr_ - mark_ <= 1)
			{
				break;
			}
			else
			{
				--contextCurr_;
			}
		}
	}

	// Otherwise cycle it in front of the mark_
	else
	{
		contextCurr_ = mark_;
		++contextCurr_;
	}

	// If the context has advanced past the end of the range, we're done
	if (pRange_->iter_end() < contextCurr_)
	{
		PyErr_SetNone( PyExc_StopIteration );
		return NULL;
	}

	// recurse back into next
	return this->next();
}


/**
 * This method is used to interpolate the given query range's with the 
 * given interpolator into the destination string. 
 */ 
bool PyQuery::interpolate( const LogStringInterpolator *handler,
	QueryRangePtr pRange, std::string &dest )
{
	BinaryIStream * pArgsStream = pRange_->getArgStream();

	if (pArgsStream == NULL)
	{
		return false;
	}

	dest.clear();
	const_cast< LogStringInterpolator *>( handler )->streamToString(
														*pArgsStream, dest );

	return true;
}

// py_query.cpp
