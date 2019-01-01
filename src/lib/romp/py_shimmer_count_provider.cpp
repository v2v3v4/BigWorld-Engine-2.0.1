/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "py_shimmer_count_provider.hpp"

DECLARE_DEBUG_COMPONENT2( "2DComponents", 0 )

// -----------------------------------------------------------------------------
// Section: PyShimmerCountProvider
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyShimmerCountProvider )
PY_BEGIN_METHODS( PyShimmerCountProvider )
PY_END_METHODS()
PY_BEGIN_ATTRIBUTES( PyShimmerCountProvider )
PY_END_ATTRIBUTES()

//PY_SCRIPT_CONVERTERS( PyShimmerCountProvider )

/*~ function BigWorld.PyShimmerCountProvider
 *	@components{ client, tools }
 *
 *	This function creates a PyShimmerCountProvider.  It records
 *	the number of shimmer objects currently visible.  It can be
 *	used to optimise Post-Processing effects that use the shimmer
 *	channel (alpha of the back buffer).
 *
 *	@return				a new PyShimmerCountProvider.
 */
PY_FACTORY( PyShimmerCountProvider, BigWorld )


/**
 *	Get an attribute for python
 */
PyObject * PyShimmerCountProvider::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return Vector4Provider::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int PyShimmerCountProvider::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return Vector4Provider::pySetAttribute( attr, value );
}


/**
 *	Factory method
 */
PyObject * PyShimmerCountProvider::pyNew( PyObject * args )
{
	return new PyShimmerCountProvider;
}


// py_shimmer_count_provider.cpp
