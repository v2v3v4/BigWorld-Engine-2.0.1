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

#include "Python.h"

#include "remote_entity_method.hpp"

#include "mailbox_base.hpp"
#include "method_description.hpp"

#include "cstdmf/memory_stream.hpp"

DECLARE_DEBUG_COMPONENT2( "entitydef", 0 )

// -----------------------------------------------------------------------------
// Section: RemoteEntityMethod
// -----------------------------------------------------------------------------

PY_TYPEOBJECT_WITH_CALL( RemoteEntityMethod )

PY_BEGIN_METHODS( RemoteEntityMethod )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( RemoteEntityMethod )
PY_END_ATTRIBUTES()


/**
 *	This method is called when a script wants to call this method
 *	on a remote script handler (entity/base).
 */
PyObject * RemoteEntityMethod::pyCall( PyObject * args )
{
	if (!pMethodDescription_->areValidArgs( true, args, true ))
	{
		return NULL;
	}

	BinaryOStream * pBOS = pMailBox_->getStream( *pMethodDescription_ );

	if (pBOS == NULL)
	{
		WARNING_MSG( "EntityMailBox::RemoteEntityMethod: "
				"Could not get stream to call %s (no attached client?)\n",
				pMethodDescription_->name().c_str() );
		Py_Return;
	}

#if ENABLE_WATCHERS
	uint32 startingSize = pBOS->size();
#endif

	pMethodDescription_->addToStream( true, args, *pBOS );

#if ENABLE_WATCHERS
	EntityMailBoxRef ref = PyEntityMailBox::reduceToRef( pMailBox_.getObject() );
	switch(ref.component())
	{
	case EntityMailBoxRef::CELL:
	case EntityMailBoxRef::BASE_VIA_CELL:
	case EntityMailBoxRef::CLIENT_VIA_CELL:
		pMethodDescription_->stats().countSentToGhosts( pBOS->size() - startingSize );
		break;
	case EntityMailBoxRef::BASE:
	case EntityMailBoxRef::CELL_VIA_BASE:
	case EntityMailBoxRef::CLIENT_VIA_BASE:
		pMethodDescription_->stats().countSentToBase( pBOS->size() - startingSize );
		break;
	case EntityMailBoxRef::CLIENT:
		pMethodDescription_->stats().countSentToOwnClient( pBOS->size() - startingSize );
		break;
	}
#endif // ENABLE_WATCHERS

	pMailBox_->sendStream();

	Py_RETURN_NONE;
}


// remote_entity_method.cpp
