/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DatabaseReplyHandler_HPP
#define DatabaseReplyHandler_HPP

#include "connection/log_on_params.hpp"

#include "network/basictypes.hpp"
#include "network/interfaces.hpp"

class LoginApp;

/**
 *	An instance of this class is used to receive the reply from a call to
 *	the database.
 */
class DatabaseReplyHandler : public Mercury::ReplyMessageHandler
{
public:
	DatabaseReplyHandler(
		LoginApp & loginApp,
		const Mercury::Address & clientAddr,
		const Mercury::ReplyID replyID,
		LogOnParamsPtr pParams );

	virtual ~DatabaseReplyHandler() {}

	virtual void handleMessage( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg );

	virtual void handleException( const Mercury::NubException & ne,
		void * arg );
	virtual void handleShuttingDown( const Mercury::NubException & ne,
		void * arg );

private:
	LoginApp & 			loginApp_;
	Mercury::Address	clientAddr_;
	Mercury::ReplyID	replyID_;
	LogOnParamsPtr		pParams_;
};

#endif // DatabaseReplyHandler_HPP
