/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASEAPP_LOGIN_REQUEST_HPP
#define BASEAPP_LOGIN_REQUEST_HPP

#include "cstdmf/smartpointer.hpp"
#include "retrying_request.hpp"

class BinaryIStream;
class LoginHandler;

namespace Mercury
{
	class Bundle;
	class Channel;
	class NetworkInterface;
}

/**
 *  This class handles the baseapp login stage of the login process.  It is a
 *  bit different to the above classes in that it doesn't retry on itself, it
 *  has a max attempts of 1 but its timeout code creates a new instance of
 *  itself.
 */
class BaseAppLoginRequest : public RetryingRequest
{
public:
	BaseAppLoginRequest( LoginHandler & parent );
	virtual ~BaseAppLoginRequest();

	virtual void handleTimeout( TimerHandle handle, void * arg );
	virtual void addRequestArgs( Mercury::Bundle & bundle );
	virtual void onSuccess( BinaryIStream & data );

	Mercury::NetworkInterface & networkInterface() { return *pInterface_; }
	Mercury::Channel & channel() { return *pChannel_; }

private:
	/// Each instance has its own Channel.  The winning instance transfers its
	/// Channel to the ServerConnection.
	Mercury::Channel * pChannel_;

	/// The attempt number.
	int attempt_;
};

typedef SmartPointer< BaseAppLoginRequest > BaseAppLoginRequestPtr;

#endif // BASEAPP_LOGIN_REQUEST_HPP
