/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGINAPP_CHECK_STATUS_REPLY_HANDLER_HPP
#define LOGINAPP_CHECK_STATUS_REPLY_HANDLER_HPP

#include "network/basictypes.hpp"
#include "network/interfaces.hpp"

/**
 *	This method handles the checkStatus request's reply.
 */
class LoginAppCheckStatusReplyHandler : public Mercury::ReplyMessageHandler
{
public:
	LoginAppCheckStatusReplyHandler( const Mercury::Address & srcAddr,
			Mercury::ReplyID replyID );

private:
	virtual void handleMessage( const Mercury::Address & /*srcAddr*/,
			Mercury::UnpackedMessageHeader & /*header*/,
			BinaryIStream & data, void * /*arg*/ );

	virtual void handleException( const Mercury::NubException & /*ne*/,
		void * /*arg*/ );
	virtual void handleShuttingDown( const Mercury::NubException & /*ne*/,
		void * /*arg*/ );

	Mercury::Address srcAddr_;
	Mercury::ReplyID replyID_;
};

#endif // LOGINAPP_CHECK_STATUS_REPLY_HANDLER_HPP
