/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REPLY_HANDLERS_HPP
#define REPLY_HANDLERS_HPP

#include "network/basictypes.hpp"
#include "network/interfaces.hpp"
#include "network/misc.hpp"

#include "server/common.hpp"

#include <vector>


/**
 *	This class is used to handle reply messages from BaseApps when a
 *	createBase message has been sent. This sends the base creation reply back
 *	to the DBMgr.
 */
class CreateBaseReplyHandler : public Mercury::ReplyMessageHandler
{
public:
	CreateBaseReplyHandler( const Mercury::Address & srcAddr,
		 	Mercury::ReplyID replyID,
			const Mercury::Address & externalAddr );

private:
	void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, void * arg );

	void handleException( const Mercury::NubException & ne, void * arg );
	void handleShuttingDown( const Mercury::NubException & ne, void * arg );

	Mercury::Address 	srcAddr_;
	Mercury::ReplyID	replyID_;
	Mercury::Address 	externalAddr_;
};


/**
 *	This class is used to handle reply messages and forward it on.
 */
class ForwardingReplyHandler : public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	ForwardingReplyHandler( const Mercury::Address & srcAddr,
			Mercury::ReplyID replyID );

private:
	void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, void * arg );

	void handleException( const Mercury::NubException & ne, void * arg );

	virtual void prependData( Mercury::Bundle & bundle,
								BinaryIStream & data ) {};

	Mercury::Address 	srcAddr_;
	Mercury::ReplyID	replyID_;
};


/**
 *	This class is used to handle the controlled shutdown stage that can be sent
 *	to all BaseApps at the same time.
 */
class SyncControlledShutDownHandler :
	public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	SyncControlledShutDownHandler( ShutDownStage stage, int count );

private:
	virtual void handleMessage( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * );

	virtual void handleException( const Mercury::NubException & ne, void * );

	void finalise();
	void decCount();

	ShutDownStage stage_;
	int count_;
};


/**
 *	This class is used to handle the controlled shutdown stage that is sent to
 *	all BaseApps sequentially.
 */
class AsyncControlledShutDownHandler :
	public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	AsyncControlledShutDownHandler( ShutDownStage stage,
			std::vector< Mercury::Address > & addrs );
	virtual ~AsyncControlledShutDownHandler() {}

private:
	virtual void handleMessage( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * );

	virtual void handleException( const Mercury::NubException & ne, void * );

	void sendNext();

	ShutDownStage stage_;
	std::vector< Mercury::Address > addrs_;
	int numSent_;
};


/**
 *	This class is used to handle replies from the CellAppMgr to the checkStatus
 *	request.
 */
class CheckStatusReplyHandler : public ForwardingReplyHandler
{
public:
	CheckStatusReplyHandler( const Mercury::Address & srcAddr,
			Mercury::ReplyID replyID );

	virtual void prependData( Mercury::Bundle & bundle,
		   BinaryIStream & data );
};

#endif // REPLY_HANDLERS_HPP
