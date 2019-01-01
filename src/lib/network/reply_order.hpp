/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REPLY_ORDER_HPP
#define REPLY_ORDER_HPP

#include "misc.hpp"

#include <vector>

namespace Mercury
{

/**
 * 	@internal
 * 	This class represents a request that requires a reply.
 * 	It is used internally by Mercury.
 */
class ReplyOrder
{
public:
	/// The user reply handler.
	ReplyMessageHandler * handler;

	/// User argument passed to the handler.
	void * arg;

	/// Timeout in microseconds.
	int microseconds;

	/// Pointer to the reply ID for this request, written in 
	/// NetworkInterface::send().
	ReplyID * pReplyID;
};

/// This vector stores all the requests for this bundle.
typedef std::vector< ReplyOrder >	ReplyOrders;

} // namespace Mercury

#endif // REPLY_ORDER_HPP
