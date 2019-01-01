/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_SHIMMER_COUNT_PROVIDER_HPP
#define PY_SHIMMER_COUNT_PROVIDER_HPP

#pragma warning( disable:4786 )

#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"
#include "moo/visual_channels.hpp"


/*~ class BigWorld.PyShimmerCountProvider
 *	@components{ client, tools }
 *
 *	This class is a special-case V4Provider that exposes the
 *	current number of visible shimmer objects.  It is used
 *	to optimise the Heat Shimmer post-process effect.
 */
class PyShimmerCountProvider : public Vector4Provider
{
	Py_Header( PyShimmerCountProvider, Vector4Provider )

public:
	PyShimmerCountProvider( PyTypePlus * pType = &s_type_ ) :
		Vector4Provider( false, pType )
	{}

	virtual void output( Vector4 & val )
	{
		float n = (float)Moo::ShimmerChannel::nItems();
		val.set( n,n,n,n );
	}

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE()
};

#endif // PY_SHIMMER_COUNT_PROVIDER_HPP
