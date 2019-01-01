/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATUS_CHECK_WATCHER_HPP
#define STATUS_CHECK_WATCHER_HPP

#include "cstdmf/watcher.hpp"
#include "network/interfaces.hpp"

/**
 *	This watcher is used to check the current status of the system.
 */
class StatusCheckWatcher : public CallableWatcher
{
public:
	StatusCheckWatcher();

protected:
	virtual bool setFromStream( void * base,
			const char * path,
			WatcherPathRequestV2 & pathRequest );

private:
	class ReplyHandler : public Mercury::ShutdownSafeReplyMessageHandler
	{
	public:
		ReplyHandler( WatcherPathRequestV2 & pathRequest );

	private:
		virtual void handleMessage( const Mercury::Address & source,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			void * arg );

		virtual void handleException( const Mercury::NubException & ne,
			void * arg );

	void sendResult( bool status, const std::string & output );

		WatcherPathRequestV2 & pathRequest_;
	};

};

#endif // STATUS_CHECK_WATCHER_HPP
