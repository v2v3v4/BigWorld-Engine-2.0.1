/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REAL_CALLER_HPP
#define REAL_CALLER_HPP

#include "cstdmf/smartpointer.hpp"
#include "pyscript/pyobject_plus.hpp"

class Entity;
class MethodDescription;

typedef SmartPointer< Entity >                   EntityPtr;

/**
 *	This class implements a simple helper Python type. Objects of this type are
 *	used to represent methods that the client can call on the server.
 */
class RealCaller : public PyObjectPlus
{
	Py_Header( RealCaller, PyObjectPlus )

public:
	RealCaller( Entity * pEntity,
			const MethodDescription * pMethodDescription,
			PyTypePlus * pType = &RealCaller::s_type_ ) :
		PyObjectPlus( pType ),
		pEntity_( pEntity ),
		pMethodDescription_( pMethodDescription )
	{
	}

	PY_METHOD_DECLARE( pyCall )

private:
	EntityPtr pEntity_;
	const MethodDescription * pMethodDescription_;
};

#endif // REAL_CALLER_HPP
