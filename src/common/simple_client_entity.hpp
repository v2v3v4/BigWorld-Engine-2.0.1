/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SIMPLE_CLIENT_ENTITY_HPP
#define SIMPLE_CLIENT_ENTITY_HPP

#include "entitydef/entity_description.hpp"

/**
 *	This namespace contains functions for dealing with simple client entities
 *	implemented by a Python object, with methods and properties in the
 *	natural way.
 */
namespace SimpleClientEntity
{
	bool propertyEvent( PyObjectPtr pEntity, const EntityDescription & edesc,
		int messageID, BinaryIStream & data, bool callSetForTopLevel );

	bool methodEvent( PyObjectPtr pEntity, const EntityDescription & edesc,
		int messageID, BinaryIStream & data );

};

#endif // SIMPLE_CLIENT_ENTITY_HPP
