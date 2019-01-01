/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blocking_reply_handler.hpp"

#include "event_dispatcher.hpp"
#include "network_interface.hpp"

// -----------------------------------------------------------------------------
// Section: BlockingReplyHandler
// -----------------------------------------------------------------------------

namespace Mercury
{

bool BlockingReplyHandler::safeToCall_ = true;

/**
 *	Constructor.
 */
BlockingReplyHandler::BlockingReplyHandler(
		Mercury::NetworkInterface & networkInterface,
		ReplyMessageHandler * pHandler ) :
	interface_( networkInterface ),
	err_( REASON_SUCCESS ),
	timerHandle_(),
	pHandler_( pHandler )
{
}


/**
 * 	This method until a reply is received, or the request times out.
 */
Reason BlockingReplyHandler::waitForReply( Channel * pChannel,
	   int maxWaitMicroSeconds )
{
	// This should not be called after the process has been registered with
	// bwmachined as other process can now find us and send other messages.
	MF_ASSERT_DEV( safeToCall_ );

	EventDispatcher & dispatcher = this->dispatcher();

	bool wasBroken = dispatcher.processingBroken();
	bool isRegularChannel = pChannel && pChannel->isLocalRegular();

	// Since this channel might not be doing any sending while we're waiting for
	// the reply, we need to mark it as irregular temporarily to ensure ACKs are
	// sent until we're done.
	if (isRegularChannel)
	{
		pChannel->isLocalRegular( false );
	}

	if (maxWaitMicroSeconds > 0)
	{
		timerHandle_ = dispatcher.addTimer( maxWaitMicroSeconds, this );
	}

	dispatcher.processUntilBreak();
	timerHandle_.cancel();

	// Restore channel regularity if necessary
	if (isRegularChannel)
	{
		pChannel->isLocalRegular( true );
	}

	dispatcher.breakProcessing( wasBroken );

	return err_;
}


/**
 *	This method handles the max timer going off. If this is called, we have not
 *	received the response in the required time.
 */
void BlockingReplyHandler::handleTimeout( TimerHandle handle, void * arg )
{
	INFO_MSG( "BlockingReplyHandler::handleTimeout: Timer expired\n" );

	interface_.cancelRequestsFor( this, REASON_TIMER_EXPIRED );
}


/**
 * 	This method handles reply messages from Mercury.
 */
void BlockingReplyHandler::handleMessage(
		const Address & addr,
		UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg )
{
	if (pHandler_)
	{
		pHandler_->handleMessage( addr, header, data, arg );
	}
	else
	{
		this->onMessage( addr, header, data, arg );
	}

	err_ = REASON_SUCCESS;

	this->dispatcher().breakProcessing();
}


/**
 * 	This method handles exceptions from Mercury.
 */
void BlockingReplyHandler::handleException(
		const NubException & ne, void * arg )
{
	if (err_ == REASON_SUCCESS)
	{
		err_ = ne.reason();
	}

	this->dispatcher().breakProcessing();

	this->onException( ne, arg );
}


/**
 *	This method returns the dispatcher this handler should use.
 */
EventDispatcher & BlockingReplyHandler::dispatcher()
{
	return interface_.mainDispatcher();
}

} // namespace Mercury

// blocking_reply_handler.cpp
