/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGINAPP_LOGIN_REQUEST_HPP
#define LOGINAPP_LOGIN_REQUEST_HPP

#include "retrying_request.hpp"

#include "network/misc.hpp"

class BinaryIStream;
class LoginHandler;

namespace Mercury
{
	class Bundle;
} // namespace Mercury


/**
 *  This class handles logging into the LoginApp.
 */
class LoginAppLoginRequest : public RetryingRequest
{
public:
	LoginAppLoginRequest( LoginHandler & parent );

	virtual void addRequestArgs( Mercury::Bundle & bundle );
	virtual void onSuccess( BinaryIStream & data );
	virtual void onFailure( Mercury::Reason reason );
};

#endif // LOGINAPP_LOGIN_REQUEST_HPP
