/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGINAPP_HPP
#define LOGINAPP_HPP

#include "connection/log_on_params.hpp"
#include "connection/login_reply_record.hpp"

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/singleton.hpp"

#include "network/channel_owner.hpp"
#include "network/event_dispatcher.hpp"
#include "network/netmask.hpp"
#include "network/public_key_cipher.hpp"
#include "network/interfaces.hpp"

#include "resmgr/datasection.hpp"
#include "server/server_app.hpp"
#include "server/stream_helper.hpp"

#include "math/ema.hpp"

#include "login_int_interface.hpp"

#include <memory>
#include <vector>

class LoginAppConfig;
class StreamEncoder;

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

typedef Mercury::ChannelOwner DBMgr;

typedef std::vector< int >	LoginAppPortList;

/**
 *	This class implements the main singleton object in the login application.
 */
class LoginApp : public ServerApp,
	public Singleton< LoginApp >
{
public:
	SERVER_APP_HEADER( LoginApp, loginApp )

	typedef LoginAppConfig Config;

	LoginApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );
	~LoginApp();

	// external methods

	virtual void login( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	virtual void probe( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	// internal methods
	void controlledShutDown( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	const NetMask& 	netMask() const			{ return netMask_; }
	uint32	externalIPFor( uint32 ip ) const;

public:
	void sendFailure( const Mercury::Address & addr,
		Mercury::ReplyID replyID, int status, const char * msg = NULL,
		LogOnParamsPtr pParams = NULL );

	void sendAndCacheSuccess( const Mercury::Address & addr,
			Mercury::ReplyID replyID, const LoginReplyRecord & replyRecord,
			LogOnParamsPtr pParams );

	Mercury::NetworkInterface &	intInterface()		{ return interface_; }
	Mercury::NetworkInterface &	extInterface()		{ return extInterface_; }

	DBMgr & dbMgr()					{ return *dbMgr_.pChannelOwner(); }
	const DBMgr & dbMgr() const		{ return *dbMgr_.pChannelOwner(); }

	bool isDBReady() const
	{
		return this->dbMgr().channel().isEstablished();
	}

	void controlledShutDown();

	uint8 systemOverloaded() const
	{ return systemOverloaded_; }

	void systemOverloaded( uint8 status )
	{
		systemOverloaded_ = status;
		systemOverloadedTime_ = timestamp();
	}


private:
	// From ServerApp
	virtual bool init( int argc, char * argv[] );
	virtual bool run();
	virtual void onSignalled( int sigNum );

	/**
	 *	This class is used to store a recent, successful login. It is used to
	 *	handle the case where the reply to the client is dropped.
	 */
	class CachedLogin
	{
	public:
		// We set creationTime_ to 0 to indicate that the login is pending.
		CachedLogin() : creationTime_( 0 ) {}

		bool isTooOld() const;
		bool isPending() const;

		void pParams( LogOnParamsPtr pParams ) { pParams_ = pParams; }
		LogOnParamsPtr pParams() { return pParams_; }

		void replyRecord( const LoginReplyRecord & record );
		const LoginReplyRecord & replyRecord() const { return replyRecord_; }

		/// This method re-initialises the cache object to indicate that it is
		/// pending.
		void reset() { creationTime_ = 0; }

	private:
		uint64 creationTime_;
		LogOnParamsPtr pParams_;
		LoginReplyRecord replyRecord_;
	};

	bool handleResentPendingAttempt( const Mercury::Address & addr,
		Mercury::ReplyID replyID );
	bool handleResentCachedAttempt( const Mercury::Address & addr,
		LogOnParamsPtr pParams, Mercury::ReplyID replyID );

	void sendSuccess( const Mercury::Address & addr,
		Mercury::ReplyID replyID, const LoginReplyRecord & replyRecord,
		const std::string & encryptionKey );

	std::auto_ptr< StreamEncoder > 	pLogOnParamsEncoder_;
	Mercury::NetworkInterface		extInterface_;

	NetMask 			netMask_;
	uint32		 		externalIP_;

	typedef std::map< uint32, uint32 > ExternalIPs;
	ExternalIPs	 		externalIPs_;

	uint8				systemOverloaded_;
	uint64				systemOverloadedTime_;

	typedef std::map< Mercury::Address, CachedLogin > CachedLoginMap;
	CachedLoginMap 		cachedLoginMap_;

	AnonymousChannelClient dbMgr_;

	// Rate Limiting state

	// the time of the start of the last time block
	uint64 				lastRateLimitCheckTime_;
	// the number of logins left for this time block
	uint				numAllowedLoginsLeft_;

	static LoginApp * pInstance_;

	static const uint32 UPDATE_STATS_PERIOD;

	/**
	 *	This class represents login statistics. These statistics are exposed to
	 *	watchers.
	 */
	class LoginStats: public TimerHandler
	{
	public:
		/**
		 *	Constructor.
		 */
		LoginStats();

		/**
		 *	Overridden from TimerHandler.
		 */
		virtual void handleTimeout( TimerHandle handle, void * arg )
		{
			this->update();
		}

		// Incrementing accessors

		/**
		 *	Increment the count for rate-limited logins.
		 */
		void incRateLimited() 	{ ++all_.value(); ++rateLimited_.value(); }

		/**
		 *	Increment the count for failed logins.
		 */
		void incFails() 		{ ++all_.value(); ++fails_.value(); }

		/**
		 *	Increment the count for repeated logins (duplicate logins that came
		 *	in from the client while the original was pending.
		 */
		void incPending() 		{ ++all_.value(); ++pending_.value(); }

		/**
		 *	Increment the count for successful logins.
		 */
		void incSuccesses() 	{ ++all_.value(); ++successes_.value(); }

		// Average accessors

		/**
		 *	Return the failed logins per second average.
		 */
		float fails() const 		{ return fails_.average(); }

		/**
		 *	Return the rate-limited logins per second average.
		 */
		float rateLimited() const 	{ return rateLimited_.average(); }

		/**
		 *	Return the repeated logins (due to already pending login) per
		 *	second average.
		 */
		float pending() const 		{ return pending_.average(); }

		/**
		 *	Return the successful logins per second average.
		 */
		float successes() const 	{ return successes_.average(); }

		/**
		 *	Return the logins per second average.
		 */
		float all() const 			{ return all_.average(); }

		/**
		 *	This method updates the averages to the accumulated values.
		 */
		void update()
		{
			fails_.sample();
			rateLimited_.sample();
			successes_.sample();
			pending_.sample();
			all_.sample();
		}

	private:
		/// Failed logins.
		AccumulatingEMA< uint32 > fails_;
		/// Rate-limited logins.
		AccumulatingEMA< uint32 > rateLimited_;
		/// Repeated logins that matched a pending login.
		AccumulatingEMA< uint32 > pending_;
		/// Successful logins.
		AccumulatingEMA< uint32 > successes_;
		/// All logins.
		AccumulatingEMA< uint32 > all_;

		/// The bias for all the exponential averages.
		static const float BIAS;
	};

	LoginStats loginStats_;
	TimerHandle statsTimer_;
};

#endif // LOGINAPP_HPP
