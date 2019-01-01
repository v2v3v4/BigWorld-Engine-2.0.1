/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blocking_db_logon_handler.hpp"

#include "cstdmf/binary_stream.hpp"

/**
 *	Handles a reply message corresponding to a previously issued logon request.
 *
 *	@param srcAddr		the source address of the reply
 *	@param header		the reply message header
 *	@param data			the reply data stream
 *	@param args			the token opaque argument associated with the original
 *						request
 */
void BlockingDBLogonHandler::onMessage(
		const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * /*args*/ )
{
	uint8 statusByte;
	data >> statusByte;

	status_ = (LogOnStatus::Status) statusByte;

	if (status_ == LogOnStatus::LOGGED_ON)
	{
		if (data.remainingLength() >= int(sizeof( Mercury::Address ) ))
		{
			data >> baseAppAddr_;
			// session key is returned if this entity is a proxy
			SessionKey sessionKey;
			if (data.remainingLength() == sizeof( SessionKey ))
			{
				data >> sessionKey;
			}
		}
		else
		{
			ERROR_MSG( "Database returned unexpected message size.\n" );
		}
	}
	else
	{
		INFO_MSG( "Could not log on: %s (%d)\n",
			this->statusToString(), status_ );
		if (data.remainingLength())
		{
			// text error message (?)
			data >> errString_;
			//TRACE_MSG( "error (%d): %s\n",
			//	remainingSize, errString_.c_str() );
		}
	}
}


/**
 *	Returns a string that describes the given login status value.
 *
 *	@param status	the log on status code
 *	@return a C-string describing the status code
 */
const char * BlockingDBLogonHandler::statusToString() const
{
	switch (status_)
	{
		case LogOnStatus::NOT_SET:
			return "Log on status not set";
		case LogOnStatus::LOGGED_ON:
			return "Logged on";
		case LogOnStatus::CONNECTION_FAILED:
			return "Connection failed";
		case LogOnStatus::DNS_LOOKUP_FAILED:
			return "DNS lookup failed";
		case LogOnStatus::UNKNOWN_ERROR:
			return "Unknown error";
		case LogOnStatus::CANCELLED:
			return "Cancelled";
		case LogOnStatus::ALREADY_ONLINE_LOCALLY:
			return "Already online locally";
		case LogOnStatus::PUBLIC_KEY_LOOKUP_FAILED:
			return "Public key lookup failed";

		case LogOnStatus::LOGIN_MALFORMED_REQUEST:
			return "Malformed request";
		case LogOnStatus::LOGIN_BAD_PROTOCOL_VERSION:
			return "Bad protocol version";

		case LogOnStatus::LOGIN_REJECTED_NO_SUCH_USER:
			return "No such user";
		case LogOnStatus::LOGIN_REJECTED_INVALID_PASSWORD:
			return "Invalid password";
		case LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN:
			return "Already logged in";
		case LogOnStatus::LOGIN_REJECTED_BAD_DIGEST:
			return "Bad digest";
		case LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE:
			return "DB general failure";
		case LogOnStatus::LOGIN_REJECTED_DB_NOT_READY:
			return "DB not ready";
		case LogOnStatus::LOGIN_REJECTED_ILLEGAL_CHARACTERS:
			return "Illegal characters";
		case LogOnStatus::LOGIN_REJECTED_SERVER_NOT_READY:
			return "Server not ready";
		case LogOnStatus::LOGIN_REJECTED_NO_BASEAPPS:
			return "No BaseApps";
		case LogOnStatus::LOGIN_REJECTED_BASEAPP_OVERLOAD:
			return "BaseApps overloaded";
		case LogOnStatus::LOGIN_REJECTED_CELLAPP_OVERLOAD:
			return "CellApps overloaded";
		case LogOnStatus::LOGIN_REJECTED_BASEAPP_TIMEOUT:
			return "BaseApp timeout";
		case LogOnStatus::LOGIN_REJECTED_BASEAPPMGR_TIMEOUT:
			return "BaseAppMgr timeout";
		case LogOnStatus::LOGIN_REJECTED_DBMGR_OVERLOAD:
			return "DBMgr overloaded";

		case LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR:
			return "Custom defined error";
		default:
			return "(unknown logon status)";
	}
}

// blocking_db_logon_handler.cpp
