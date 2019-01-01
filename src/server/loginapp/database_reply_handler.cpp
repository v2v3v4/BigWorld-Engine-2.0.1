/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "database_reply_handler.hpp"

#include "loginapp.hpp"

#include "connection/log_on_status.hpp"

#include "cstdmf/debug.hpp"

#include "network/nub_exception.hpp"

/**
 *	Constructor.
 */
DatabaseReplyHandler::DatabaseReplyHandler(
		LoginApp & loginApp,
		const Mercury::Address & clientAddr,
		Mercury::ReplyID replyID,
		LogOnParamsPtr pParams ) :
	loginApp_( loginApp ),
	clientAddr_( clientAddr ),
	replyID_( replyID ),
	pParams_( pParams )
{
	DEBUG_MSG( "DBReplyHandler created at %f\n",
		float(double(timestamp())/stampsPerSecondD()) );
}


/**
 *	This method is called when a message comes back from the system.
 *	It deletes itself at the end.
 */
void DatabaseReplyHandler::handleMessage(
	const Mercury::Address & /*source*/,
	Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data,
	void * /*arg*/ )
{
	uint8 status;
	data >> status;

	if (status != LogOnStatus::LOGGED_ON)
	{
		if (data.remainingLength() > 0)
		{
			std::string msg;
			data >> msg;
			loginApp_.sendFailure( clientAddr_, replyID_, status,
				msg.c_str(), pParams_ );
		}
		else
		{
			loginApp_.sendFailure( clientAddr_, replyID_, status,
				"Database returned an unelaborated error. Check DBMgr log.",
				pParams_ );
		}

		LoginApp & app = loginApp_;
		if ((app.systemOverloaded() == 0 &&
			 status == LogOnStatus::LOGIN_REJECTED_BASEAPP_OVERLOAD) ||
				status == LogOnStatus::LOGIN_REJECTED_CELLAPP_OVERLOAD ||
				status == LogOnStatus::LOGIN_REJECTED_DBMGR_OVERLOAD)
		{
			DEBUG_MSG( "DatabaseReplyHandler::handleMessage(%s): "
					"failure due to overload (status=%x)\n",
				clientAddr_.c_str(), status );
			app.systemOverloaded( status );
		}
		delete this;
		return;
	}

	if (data.remainingLength() < int(sizeof( LoginReplyRecord )))
	{
		ERROR_MSG( "DatabaseReplyHandler::handleMessage: "
						"Login failed. Expected %"PRIzu" bytes got %d\n",
				sizeof( LoginReplyRecord ), data.remainingLength() );

		if (data.remainingLength() == sizeof(LoginReplyRecord) - sizeof(int))
		{
			ERROR_MSG( "DatabaseReplyHandler::handleMessage: "
					"This can occur if a login is attempted to an entity type "
					"that is not a Proxy.\n" );

			loginApp_.sendFailure( clientAddr_, replyID_,
				LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR,
				"Database returned a non-proxy entity type.",
				pParams_ );
		}
		else
		{
			loginApp_.sendFailure( clientAddr_, replyID_,
				LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE,
				"Database returned an unknown error.",
				pParams_ );
		}

		delete this;
		return;
	}

	LoginReplyRecord lrr;
	data >> lrr;

	// If the client has an external address, send them the firewall
	// address instead!

	if (!loginApp_.netMask().containsAddress( clientAddr_.ip ))
	{
		INFO_MSG( "DatabaseReplyHandler::handleMessage: "
				"Redirecting external client %s to firewall.\n",
			clientAddr_.c_str() );
		lrr.serverAddr.ip = loginApp_.externalIPFor( lrr.serverAddr.ip );
	}

	loginApp_.sendAndCacheSuccess( clientAddr_,
			replyID_, lrr, pParams_ );

	delete this;
}


/**
 *	This method is called when no message comes back from the system,
 *	or some other error occurs. It deletes itself at the end.
 */
void DatabaseReplyHandler::handleException(
	const Mercury::NubException & ne,
	void * /*arg*/ )
{
	loginApp_.sendFailure( clientAddr_, replyID_,
		LogOnStatus::LOGIN_REJECTED_DBMGR_OVERLOAD, "No reply from DBMgr.",
		pParams_ );

	WARNING_MSG( "DatabaseReplyHandler: got an exception (%s)\n",
			Mercury::reasonToString( ne.reason() ) );
	DEBUG_MSG( "DBReplyHandler timed out at %f\n",
			float(double(timestamp())/stampsPerSecondD()) );

	delete this;
}


void DatabaseReplyHandler::handleShuttingDown( const Mercury::NubException & ne,
		void * )
{
	INFO_MSG( "DatabaseReplyHandler::handleShuttingDown: Ignoring %s\n",
		clientAddr_.c_str() );
	delete this;
}


// database_reply_handler.cpp
