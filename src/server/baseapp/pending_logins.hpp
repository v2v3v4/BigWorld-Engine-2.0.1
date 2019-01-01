/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PENDING_LOGINS_HPP
#define PENDING_LOGINS_HPP

#include "cstdmf/smartpointer.hpp"
#include "network/basictypes.hpp"

#include <list>
#include <map>

class Proxy;


/**
 *	This class stores information about an entity that has just logged
 *	in via the LoginApp but still needs to confirm this via the BaseApp.
 */
class PendingLogin
{
public:
	// Should never be called but needed by std::map.
	PendingLogin() : pProxy_( NULL ), addrFromLoginApp_( 0, 0 )
	{
	}

	PendingLogin( Proxy * pProxy,
			const Mercury::Address & loginAppAddr ) :
		pProxy_( pProxy ),
		addrFromLoginApp_( loginAppAddr ) {}

	const Mercury::Address & addrFromLoginApp() const
	{
		return addrFromLoginApp_;
	}

	SmartPointer< Proxy > pProxy() const	{ return pProxy_; }

private:
	SmartPointer< Proxy > pProxy_;
	// The address the client used to connect to the loginApp.
	Mercury::Address addrFromLoginApp_;
};


/**
 *	This class is used to store information about entities that have logged
 *	in via the LoginApp but still need to confirm this via the BaseApp.
 */
class PendingLogins
{
public:
	typedef std::map< SessionKey, PendingLogin > Container;
	typedef Container::iterator iterator;

	iterator find( SessionKey key );
	iterator end();
	void erase( iterator iter );

	SessionKey add( Proxy * pProxy, const Mercury::Address & loginAppAddr );

	void tick();

private:
	Container container_;

	/**
	 *	This class is used to keep information in a time-sorted queue. It
	 *	is used to identify proxies that have logged in via LoginApp but
	 *	were never confirmed directly with the BaseApp.
	 */
	class QueueElement
	{
	public:
		QueueElement( GameTime expiryTime,
				EntityID proxyID, SessionKey loginKey ) :
			expiryTime_( expiryTime ),
			proxyID_( proxyID ),
			loginKey_( loginKey )
		{
		}

		bool hasExpired( GameTime time ) const
		{
			return (time == expiryTime_);
		}

		EntityID proxyID() const	{ return proxyID_; }
		SessionKey loginKey() const	{ return loginKey_; }

	private:
		GameTime expiryTime_;
		EntityID proxyID_;
		SessionKey loginKey_;
	};

	typedef std::list< QueueElement > Queue;
	Queue queue_;
};

#endif // PENDING_LOGINS_HPP
