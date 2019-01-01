/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blocking_db_reply_handler.hpp"

#include "cstdmf/binary_stream.hpp"

#include "network/nub_exception.hpp"

/**
 *	Handles an incoming reply to a lookup request.
 *
 *	@param srcAddr		the source address of the reply
 *	@param header		the message header
 *	@param data			the data stream for the reply
 *	@param args			the opaque data token associated with the initial
 *						request
 */
void BlockingDBLookUpHandler::onMessage(
		const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * /*args*/ )
{
	if (data.remainingLength() == 0)
	{
		//TRACE_MSG( "Entity exists but is not checked out\n" );
		// entity exists but is not checked out
		result_ = BlockingDBLookUpHandler::NOT_CHECKED_OUT;
	}
	else if (data.remainingLength() == sizeof( mailbox_ ))
	{
		// we found it!
		data >> mailbox_;
		//TRACE_MSG( "Got a mailbox: %s/%d\n",
		//	(char*)mailbox_.addr, mailbox_.id );
		result_ = BlockingDBLookUpHandler::OK;
	}
	else if (data.remainingLength() == sizeof( int ))
	{
		//TRACE_MSG( "could not lookup entity\n" );
		int err;
		data >> err;

		if (err != -1)
		{
			WARNING_MSG( "Got back an integer value that was not -1: 0x%x\n",
				err );
			result_ = BlockingDBLookUpHandler::GENERAL_ERROR;
			return;
		}

		result_ = BlockingDBLookUpHandler::DOES_NOT_EXIST;
	}
	else
	{
		// bad data size
		ERROR_MSG( "BlockingDBLookUpHandler::onMessage: "
				"got bad data size=%d\n",
			data.remainingLength() );
		result_ = BlockingDBLookUpHandler::GENERAL_ERROR;
	}
}


/**
 *	Handles an exception while waiting for the corresponding request's reply.
 *
 *	@param ne		the nub exception
 *	@param args		the opaque token argument associated with the original
 *					request
 */
void BlockingDBLookUpHandler::onException(
		const Mercury::NubException & ne,
		void* /*args*/ )
{
	if (ne.reason() == Mercury::REASON_TIMER_EXPIRED)
	{
		result_ = BlockingDBLookUpHandler::TIMEOUT;
	}
	else
	{
		result_ = BlockingDBLookUpHandler::GENERAL_ERROR;
	}
}


// blocking_db_reply_handler.cpp
