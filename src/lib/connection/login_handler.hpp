/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGIN_HANDLER_HPP
#define LOGIN_HANDLER_HPP

#include "baseapp_login_request.hpp"
#include "log_on_params.hpp"
#include "log_on_status.hpp"
#include "login_reply_record.hpp"
#include "retrying_request.hpp"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"

#include "network/basictypes.hpp"

#include <set>
#include <string>
#include <vector>


class BinaryIStream;
class ServerConnection;
namespace Mercury
{
	class NetworkInterface;
}

/**
 *	This class is used to manage the various stages of logging into the server.
 *	This covers logging into both the loginapp and the baseapp.
 */
class LoginHandler : public SafeReferenceCount
{
public:
	LoginHandler( ServerConnection * pServerConnection, 
				  LogOnStatus loginNotSent = LogOnStatus::NOT_SET );
	~LoginHandler();

	void start( const Mercury::Address & loginAppAddr, LogOnParamsPtr pParams );
	void startWithBaseAddr( const Mercury::Address & baseAppAddr, 
							SessionKey loginKey );
	void finish();

	void sendLoginAppLogin();
	void onLoginReply( BinaryIStream & data );

	void sendBaseAppLogin();
	void onBaseAppReply( BaseAppLoginRequestPtr pHandler,
		SessionKey sessionKey );

	void onFailure( Mercury::Reason reason );

	const LoginReplyRecord & replyRecord() const { return replyRecord_; }

	bool done() const						{ return done_; }
	int status() const						{ return status_; }
	LogOnParamsPtr pParams()				{ return pParams_; }

	const std::string & errorMsg() const	{ return errorMsg_; }

	void setError( int status, const std::string & errorMsg )
	{
		status_ = status;
		errorMsg_ = errorMsg;
	}

	ServerConnection * pServerConnection() const { return pServerConnection_; }

	const Mercury::Address & loginAddr() const { return loginAppAddr_; }
	const Mercury::Address & baseAppAddr() const { return baseAppAddr_; }

	void addChildRequest( RetryingRequestPtr pRequest );
	void removeChildRequest( RetryingRequestPtr pRequest );
	void addCondemnedInterface( Mercury::NetworkInterface * pInterface );

	int numBaseAppLoginAttempts() const { return numBaseAppLoginAttempts_; }

private:
	Mercury::Address	loginAppAddr_;
	Mercury::Address	baseAppAddr_;

	LogOnParamsPtr		pParams_;

	ServerConnection* 	pServerConnection_;
	LoginReplyRecord	replyRecord_;
	bool				done_;
	uint8				status_;

	std::string			errorMsg_;

	/// Number of BaseAppLoginRequests that have been created
	int numBaseAppLoginAttempts_;

	/// Currently active RetryingRequests
	typedef std::set< RetryingRequestPtr > ChildRequests;
	ChildRequests childRequests_;

};

typedef SmartPointer< LoginHandler > LoginHandlerPtr;


#endif // LOGIN_HANDLER_HPP
