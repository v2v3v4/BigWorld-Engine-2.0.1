/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REQUEST_MANAGER_HPP
#define REQUEST_MANAGER_HPP

#include "interfaces.hpp"

#include <map>
#include <vector>

namespace Mercury
{

class EventDispatcher;
class InterfaceTable;
class NetworkInterface;
class ReplyOrder;
class Request;
class RequestManager;

typedef std::vector< ReplyOrder > ReplyOrders;

/**
 *
 */
class RequestManager : public InputMessageHandler
{
public:
	RequestManager( bool isExternal,
		EventDispatcher & dispatcher );
	~RequestManager();

	void cancelRequestsFor( Channel * pChannel );
	void cancelRequestsFor( ReplyMessageHandler * pHandler,
			Reason reason = REASON_CHANNEL_LOST );

	void addReplyOrders( const ReplyOrders & replyOrders, Channel * pChannel );

	void failRequest( Request & request, Reason reason );

private:
	ReplyID getNextReplyID()
	{
		if (nextReplyID_ > REPLY_ID_MAX)
			nextReplyID_ = 1;

		return nextReplyID_++;
	}

	virtual void handleMessage( const Address & source,
		UnpackedMessageHeader & header,
		BinaryIStream & data );

	// -------------------------------------------------------------------------
	// Section: Properties
	// -------------------------------------------------------------------------

	typedef std::map< int, Request * > RequestMap;
	RequestMap requestMap_;

	ReplyID nextReplyID_;

	EventDispatcher & dispatcher_;

	const bool isExternal_; 
};

} // namespace Mercury

#endif // REQUEST_MANAGER_HPP
