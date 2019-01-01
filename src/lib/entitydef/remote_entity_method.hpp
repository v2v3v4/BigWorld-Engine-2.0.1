/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REMOTE_ENTITY_METHOD_HPP
#define REMOTE_ENTITY_METHOD_HPP

#include "cstdmf/smartpointer.hpp"
#include "pyscript/pyobject_plus.hpp"

class MethodDescription;
class PyEntityMailBox;


/**
 *  This class implements a simple helper Python type. Objects of this type are
 *  used to represent methods that the base can call on another script object
 *  (e.g. the cell, the client, or another base).
 */
class RemoteEntityMethod : public PyObjectPlus
{
	Py_Header( RemoteEntityMethod, PyObjectPlus )

	public:
		RemoteEntityMethod( PyEntityMailBox * pMailBox,
				const MethodDescription * pMethodDescription,
				PyTypePlus * pType = &s_type_ ) :
			PyObjectPlus( pType ),
			pMailBox_( pMailBox ),
			pMethodDescription_( pMethodDescription )
		{
		}
		~RemoteEntityMethod() { }

		PY_METHOD_DECLARE( pyCall )

	private:
		SmartPointer< PyEntityMailBox > pMailBox_;
		const MethodDescription * pMethodDescription_;
};


#endif // REMOTE_ENTITY_METHOD_HPP
