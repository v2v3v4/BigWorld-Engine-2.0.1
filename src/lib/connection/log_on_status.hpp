/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_ON_STATUS_HPP
#define LOG_ON_STATUS_HPP

class LogOnStatus
{
public:

	/// This enumeration contains the possible results of a logon attempt.  If
	/// you update this mapping, you need to make corresponding changes to
	/// bigworld/src/client/connection_control.cpp.
	enum Status
	{
		// client status values
		NOT_SET,
		LOGGED_ON,
		CONNECTION_FAILED,
		DNS_LOOKUP_FAILED,
		UNKNOWN_ERROR,
		CANCELLED,
		ALREADY_ONLINE_LOCALLY,
		PUBLIC_KEY_LOOKUP_FAILED,
		LAST_CLIENT_SIDE_VALUE = 63,

		// server status values
		LOGIN_MALFORMED_REQUEST,
		LOGIN_BAD_PROTOCOL_VERSION,

		LOGIN_REJECTED_NO_SUCH_USER,
		LOGIN_REJECTED_INVALID_PASSWORD,
		LOGIN_REJECTED_ALREADY_LOGGED_IN,
		LOGIN_REJECTED_BAD_DIGEST,
		LOGIN_REJECTED_DB_GENERAL_FAILURE,
		LOGIN_REJECTED_DB_NOT_READY,
		LOGIN_REJECTED_ILLEGAL_CHARACTERS,
		LOGIN_REJECTED_SERVER_NOT_READY,
		LOGIN_REJECTED_UPDATER_NOT_READY,	// No longer used
		LOGIN_REJECTED_NO_BASEAPPS,
		LOGIN_REJECTED_BASEAPP_OVERLOAD,
		LOGIN_REJECTED_CELLAPP_OVERLOAD,
		LOGIN_REJECTED_BASEAPP_TIMEOUT,
		LOGIN_REJECTED_BASEAPPMGR_TIMEOUT,
		LOGIN_REJECTED_DBMGR_OVERLOAD,
		LOGIN_REJECTED_LOGINS_NOT_ALLOWED,
		LOGIN_REJECTED_RATE_LIMITED,

		LOGIN_CUSTOM_DEFINED_ERROR = 254,
		LAST_SERVER_SIDE_VALUE = 255
	};

	/// This is the constructor.
	LogOnStatus(Status status = NOT_SET) : status_(status)
	{
	}

	/// This method returns true if the logon succeeded.
	bool succeeded() const {return status_ == LOGGED_ON;}

	/// This method returns true if the logon failed.
	bool fatal() const
	{
		return
			status_ == CONNECTION_FAILED ||
			status_ == CANCELLED ||
			status_ == UNKNOWN_ERROR;
	}

	/// This method returns true if the logon was successful, or is still
	/// pending.
	bool okay() const
	{
		return
			status_ == NOT_SET ||
			status_ == LOGGED_ON;
	}

	/// Assignment operator.
	void operator = (int status) { status_ = status; }

	/// This operator returns the status as an integer.
	operator int() const { return status_; }

	Status value() const { return Status(status_); }

private:
	int status_;
};

#endif // LOG_ON_STATUS_HPP
