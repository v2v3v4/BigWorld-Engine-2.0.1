/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_VOLATILE_INFO_HPP
#define PY_VOLATILE_INFO_HPP

#include "Python.h"

#include "volatile_info.hpp"

namespace Script
{
	int setData( PyObject * pObject, VolatileInfo & rInfo,
			const char * varName = "" );
	PyObject * getData( const VolatileInfo & info );
}

namespace PyVolatileInfo
{

	bool priorityFromPyObject( PyObject * pObject, float & priority );
	PyObject * pyObjectFromPriority( float priority );
}

#endif // PY_VOLATILE_INFO_HPP
