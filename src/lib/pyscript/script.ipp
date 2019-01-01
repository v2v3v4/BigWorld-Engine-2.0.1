/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// script.ipp


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


namespace Script
{

/**
 *	Static function to call a callable object with error checking.
 *	Note that it decrements the reference counts of both of its arguments.
 *
 *	@return true for success
 *	@see callResult
 */
INLINE bool call( PyObject * pFunction, PyObject * pArgs,
	const char * errorPrefix, bool okIfFunctionNull )
{
	PyObject * res = Script::ask(
		pFunction, pArgs, errorPrefix, okIfFunctionNull, true );
	Py_XDECREF( res );
	return (res != NULL) || (okIfFunctionNull && (pFunction == NULL));
}

}
