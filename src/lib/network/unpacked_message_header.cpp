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

#include "unpacked_message_header.hpp"

#include "interface_table.hpp"
#include "network_interface.hpp"

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: UnpackedMessageHeader
// -----------------------------------------------------------------------------

/**
 *	This method returns the name of the message.
 */
const char * UnpackedMessageHeader::msgName() const
{
	return pInterfaceElement ? pInterfaceElement->name() : "";
}

}

// unpacked_message_header.hpp
