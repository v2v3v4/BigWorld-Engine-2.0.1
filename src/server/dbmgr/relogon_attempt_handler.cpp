/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "relogon_attempt_handler.hpp"

#include "database.hpp"

#include "baseapp/baseapp_int_interface.hpp"

#include "network/event_dispatcher.hpp"
#include "network/nub_exception.hpp"


/**
 *	Constructor.
 */
RelogonAttemptHandler::RelogonAttemptHandler( EntityTypeID entityTypeID,
		DatabaseID dbID,
		const Mercury::Address & replyAddr,
		bool offChannel,
		Mercury::ReplyID replyID,
		LogOnParamsPtr pParams,
		const Mercury::Address & addrForProxy ) :
	hasAborted_( false ),
	ekey_( entityTypeID, dbID ),
	replyAddr_( replyAddr ),
	offChannel_( offChannel ),
	replyID_( replyID ),
	pParams_( pParams ),
	addrForProxy_( addrForProxy ),
	replyBundle_()
{
	INFO_MSG( "RelogonAttemptHandler: Starting relogon attempt for "
				"DBID: %"FMT_DBID", Type: %d\n",
			dbID, entityTypeID );
	Database::instance().onStartRelogonAttempt( entityTypeID, dbID, this );
}


/**
 *	Destructor.
 */
RelogonAttemptHandler::~RelogonAttemptHandler()
{
	waitForDestroyTimer_.cancel();

	if (!hasAborted_)
	{
		this->abort();
	}
}


/*
 *	Mercury::ReplyMessageHandler override.
 */
void RelogonAttemptHandler::handleMessage(
	const Mercury::Address & source,
	Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data,
	void * arg )
{
	uint8 result;
	data >> result;

	if (hasAborted_)
	{
		DEBUG_MSG( "RelogonAttemptHandler: "
			"Ignoring BaseApp reply, re-logon attempt has been aborted.\n" );

		// Delete ourselves as we have been aborted.
		delete this;
		return;
	}

	switch (result)
	{
	case BaseAppIntInterface::LOG_ON_ATTEMPT_TOOK_CONTROL:
	{
		INFO_MSG( "RelogonAttemptHandler: It's taken over.\n" );
		Mercury::Address proxyAddr;
		data >> proxyAddr;

		EntityMailBoxRef baseRef;
		data >> baseRef;

		replyBundle_.startReply( replyID_ );

		// Assume success.
		replyBundle_ << (uint8)LogOnStatus::LOGGED_ON;
		replyBundle_ << proxyAddr;
		replyBundle_.transfer( data, data.remainingLength() );

		if (!offChannel_)
		{
			Database::getChannel( replyAddr_ ).send(
				&replyBundle_ );
		}
		else
		{
			Database::instance().interface().send( replyAddr_,
				replyBundle_ );
		}

		delete this;

		break;
	}

	case BaseAppIntInterface::LOG_ON_ATTEMPT_REJECTED:
	{
		INFO_MSG( "RelogonAttemptHandler: "
				"Re-login not allowed for %s.\n",
			pParams_->username().c_str() );

		Database::instance().sendFailure( replyID_, replyAddr_,
			false, /* offChannel */
			LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN,
			"Relogin denied." );

		delete this;

		break;
	}

	case BaseAppIntInterface::LOG_ON_ATTEMPT_WAIT_FOR_DESTROY:
	{
		INFO_MSG( "RelogonAttemptHandler: "
			"Waiting for existing entity to be destroyed.\n" );

		// Wait 5 seconds for an entity to be destroyed before
		// giving up.
		const int waitForDestroyTimeout = 5000000;
		waitForDestroyTimer_ =
			Database::instance().mainDispatcher().addTimer(
				waitForDestroyTimeout, this, NULL );

		// Don't delete ourselves just yet, wait for the timeout
		// to occur.
		break;
	}

	default:
		CRITICAL_MSG( "RelogonAttemptHandler: Invalid result %d\n",
				int(result) );
		delete this;
		break;
	}
}


/*
 *	Mercury::ReplyMessageHandler override.
 */
void RelogonAttemptHandler::handleException(
	const Mercury::NubException & exception,
	void * arg )
{
	ERROR_MSG( "RelogonAttemptHandler::handleException: "
				"Unexpected channel exception.\n" );
	this->terminateRelogonAttempt(
			Mercury::reasonToString( exception.reason() ));
}


void RelogonAttemptHandler::handleShuttingDown(
	const Mercury::NubException & exception, void * arg )
{
	INFO_MSG( "RelogonAttemptHandler::handleShuttingDown: Ignoring\n" );
	delete this;
}


/*
 *	TimerHandler override.
 */
void RelogonAttemptHandler::handleTimeout( TimerHandle handle, void * pUser )
{
	// If we timeout after receiving a LOG_ON_ATTEMPT_WAIT_FOR_DESTROY
	// then it's time to give up and destroy ourselves.
	this->terminateRelogonAttempt(
			"Timeout waiting for previous login to disconnect." );
}


/**
 *	This method sends a notification to a client connection that a re-login
 *	attempt has failed.
 */
void RelogonAttemptHandler::terminateRelogonAttempt(
	const char * clientMessage )
{
	if (!hasAborted_)
	{
		this->abort();
		ERROR_MSG( "RelogonAttemptHandler: %s.\n", clientMessage );

		Database::instance().sendFailure( replyID_, replyAddr_,
				offChannel_,
				LogOnStatus::LOGIN_REJECTED_BASEAPP_TIMEOUT,
				clientMessage );
	}

	delete this;
}


/**
 *	This method is called when the entity that we're trying to re-logon
 *	to suddenly logs off.
 */
void RelogonAttemptHandler::onEntityLogOff()
{
	if (hasAborted_)
	{
		return;
	}

	this->abort();

	// Log on normally
	Database::instance().logOn( replyAddr_, replyID_, pParams_,
		addrForProxy_, offChannel_ );
}


/**
 *	This method flags this handler as aborted. It should not do any more
 *	processing but may still need to wait for callbacks.
 */
void RelogonAttemptHandler::abort()
{
	hasAborted_ = true;
	Database::instance().onCompleteRelogonAttempt(
			ekey_.typeID, ekey_.dbID );
}

// relogon_attempt_handler.cpp
