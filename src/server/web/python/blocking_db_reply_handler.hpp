/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BLOCKING_DB_LOOKUP_HANDLER_HPP
#define BLOCKING_DB_LOOKUP_HANDLER_HPP

#include "network/basictypes.hpp"
#include "network/blocking_reply_handler.hpp"

class BinaryIStream;

namespace Mercury
{
	class NetworkInterface;
	class NubException;
	class UnpackedMessageHeader;
}

/**
 *	Class that handles replies to lookupEntity requests from the DBMgr. It can
 *	block until it receives a response.
 */
class BlockingDBLookUpHandler : public Mercury::BlockingReplyHandler
{
public:
	/**
	 *	Possible result states from lookups on the database.
	 */
	enum Result
	{
		OK,					/**< mailbox received OK */
		PENDING,			/**< pending response from DBMgr */
		TIMEOUT,			/**< timeout waiting for response */
		DOES_NOT_EXIST,		/**< entity does not exist */
		NOT_CHECKED_OUT, 	/**< DBMgr reports that entity is not checked out */
		GENERAL_ERROR		/**< General communications error,
									maybe DBMgr down */
	};


public:
	/**
	 *	Constructor.
	 *
	 *	@param nub the nub to listen on
	 */
	BlockingDBLookUpHandler( Mercury::NetworkInterface & interface ):
		Mercury::BlockingReplyHandler( interface ),
		result_( PENDING ),
		mailbox_()
	{}


	/**
	 *	Destructor.
	 */
	virtual ~BlockingDBLookUpHandler()
	{}

public: // From BlockingReplyHandler

	virtual void onMessage(
			const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			void * args );

	virtual void onException(
			const Mercury::NubException & ne,
			void * arg );
	/**
	 *	Returns the mailbox object bound to this handler.
	 *	@return a reference to the mailbox
	 */
	const EntityMailBoxRef & mailbox() const { return mailbox_; }


	/**
	 *	Returns the result of the lookup.
	 *	@return the result of the lookup.
	 */
	const Result & result() const { return result_; }

	/**
	 *	Returns a string description of a result.
	 *
	 *	@return a string description of the result of the lookup.
	 */
	static const char* getResultString( Result result )
	{
		switch (result)
		{
#			define ENUM_CASE_STRING(NAME) case NAME: return #NAME

			ENUM_CASE_STRING( OK );
			ENUM_CASE_STRING( PENDING );
			ENUM_CASE_STRING( TIMEOUT );
			ENUM_CASE_STRING( NOT_CHECKED_OUT );
			ENUM_CASE_STRING( GENERAL_ERROR );
			default: return "(UNKNOWN)";
#			undef ENUM_CASE_STRING
		}
	}

private:
	/** The lookup result. */
	Result 				result_;

	/** The mailbox bound to this handler. */
	EntityMailBoxRef  	mailbox_;

};

#endif // BLOCKING_DB_LOOKUP_HANDLER_HPP
