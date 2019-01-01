/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "login_app_check_status_reply_handler.hpp"

#include "database.hpp"

#include "network/bundle.hpp"
#include "network/channel_sender.hpp"

/**
 *	Constructor.
 */
LoginAppCheckStatusReplyHandler::LoginAppCheckStatusReplyHandler(
		const Mercury::Address & srcAddr,
		Mercury::ReplyID replyID ) :
	srcAddr_( srcAddr ),
	replyID_ ( replyID )
{
}


/**
 *	This method handles the reply message.
 */
void LoginAppCheckStatusReplyHandler::handleMessage(
		const Mercury::Address & /*srcAddr*/,
		Mercury::UnpackedMessageHeader & /*header*/,
		BinaryIStream & data, void * /*arg*/ )
{
	Mercury::ChannelSender sender( Database::getChannel( srcAddr_ ) );
	Mercury::Bundle & bundle = sender.bundle();

	bundle.startReply( replyID_ );

	bool isOkay;
	int32 numBaseApps;
	int32 numCellApps;

	data >> isOkay >> numBaseApps >> numCellApps;

	bundle << uint8( isOkay && (numBaseApps > 0) && (numCellApps > 0));

	bundle.transfer( data, data.remainingLength() );

	if (numBaseApps <= 0)
	{
		bundle << "No BaseApps";
	}

	if (numBaseApps <= 0)
	{
		bundle << "No CellApps";
	}

	delete this;
}


/**
 *	This method handles a failed request.
 */
void LoginAppCheckStatusReplyHandler::handleException(
		const Mercury::NubException & /*ne*/, void * /*arg*/ )
{
	Mercury::ChannelSender sender( Database::getChannel( srcAddr_ ) );
	Mercury::Bundle & bundle = sender.bundle();

	bundle.startReply( replyID_ );
	bundle << uint8( false );
	bundle << "No reply from BaseAppMgr";

	delete this;
}


void LoginAppCheckStatusReplyHandler::handleShuttingDown(
		const Mercury::NubException & /*ne*/, void * /*arg*/ )
{
	INFO_MSG( "LoginAppCheckStatusReplyHandler::handleShuttingDown: "
			"Ignoring\n" );
	delete this;
}

// login_app_check_status_reply_handler.cpp
