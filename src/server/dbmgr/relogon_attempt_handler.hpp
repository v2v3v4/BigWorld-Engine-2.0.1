/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RELOGON_ATTEMPT_HANDLER_HPP
#define RELOGON_ATTEMPT_HANDLER_HPP

#include "cstdmf/timer_handler.hpp"
#include "connection/log_on_params.hpp"
#include "dbmgr_lib/idatabase.hpp"
#include "network/bundle.hpp"
#include "network/interfaces.hpp"

/**
 *	This class is used to receive the reply from a createEntity call to
 *	BaseAppMgr during a re-logon operation.
 */
class RelogonAttemptHandler : public Mercury::ReplyMessageHandler,
							public TimerHandler
{
public:
	RelogonAttemptHandler( EntityTypeID entityTypeID,
			DatabaseID dbID,
			const Mercury::Address & replyAddr,
			bool offChannel,
			Mercury::ReplyID replyID,
			LogOnParamsPtr pParams,
			const Mercury::Address & addrForProxy );

	virtual ~RelogonAttemptHandler();

	// Mercury::ReplyMessageHandler methods
	virtual void handleMessage( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg );

	virtual void handleException( const Mercury::NubException & exception,
		void * arg );

	virtual void handleShuttingDown( const Mercury::NubException & exception,
		void * arg );

	void onEntityLogOff();

	// TimerHandler methods
	virtual void handleTimeout( TimerHandle handle, void * pUser );

private:
	void terminateRelogonAttempt( const char *clientMessage );

	void abort();

	bool 					hasAborted_;
	EntityDBKey				ekey_;
	Mercury::Address 		replyAddr_;
	bool 					offChannel_;
	Mercury::ReplyID 		replyID_;
	LogOnParamsPtr 			pParams_;
	Mercury::Address 		addrForProxy_;
	Mercury::Bundle			replyBundle_;

	TimerHandle				waitForDestroyTimer_;
};

#endif // RELOGON_ATTEMPT_HANDLER_HPP
