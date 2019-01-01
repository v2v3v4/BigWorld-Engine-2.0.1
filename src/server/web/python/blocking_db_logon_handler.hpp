/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BLOCKING_DB_LOGON_HANDLER_HPP
#define BLOCKING_DB_LOGON_HANDLER_HPP

#include "connection/log_on_status.hpp"

#include "network/basictypes.hpp"
#include "network/blocking_reply_handler.hpp"

#include <string>

class BinaryIStream;
namespace Mercury
{
	class UnpackedMessageHeader;
}

/**
 *	Class that blocks on a DBMgr logon request.
 */
class BlockingDBLogonHandler: public Mercury::BlockingReplyHandler
{
public:
	/**
	 *	Constructor.
	 */
	BlockingDBLogonHandler( Mercury::NetworkInterface& interface ):
		Mercury::BlockingReplyHandler( interface ),
		status_( LogOnStatus::NOT_SET ),
		baseAppAddr_( 0, 0 ),
		errString_()
	{}

	/**
	 *	Destructor.
	 */
	virtual ~BlockingDBLogonHandler() {}

	/**
	 *	Returns the logon status.
	 *
	 *	@return the logon status
	 *	@see LogOnStatus::Status
	 */
	const LogOnStatus::Status &status() const { return status_; }

	/**
	 *	Returns the server address returned from the dbmgr on a logOn request.
	 */
	const Mercury::Address& baseAppAddress() const { return baseAppAddr_; }

	/**
	 *	Returns an error string describing what went wrong with a failed logon
	 *	attempt.
	 */
	const std::string& errString() const { return errString_; }

	const char * statusToString() const;

public: // from BlockingReplyHandler

	virtual void onMessage( const Mercury::Address& srcAddr,
			Mercury::UnpackedMessageHeader &header,
			BinaryIStream& data,
			void* args );

private:
	/** The log on status */
	LogOnStatus::Status 	status_;

	/** The retrieved server address that the client must connect to. */
	Mercury::Address 		baseAppAddr_;

	/** The error string describing a failed logon attempt. */
	std::string 			errString_;
};

#endif // BLOCKING_DB_LOGON_HANDLER_HPP
