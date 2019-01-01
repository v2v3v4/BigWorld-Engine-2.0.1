/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SENDING_STATS_HPP
#define SENDING_STATS_HPP

#include "cstdmf/profile.hpp"
#include "cstdmf/timer_handler.hpp"

#include "math/stat_with_rates_of_change.hpp"

namespace Mercury
{

class EventDispatcher;

/**
 *	This class maintains the statistics associated with network sending.
 */
class SendingStats : public TimerHandler
{
public:
	SendingStats();

	void init( EventDispatcher & dispatcher );
	void fini();

	double bitsPerSecond() const;
	double packetsPerSecond() const;
	double messagesPerSecond() const;

#if ENABLE_WATCHERS
	ProfileVal & mercuryTimer()	{ return mercuryTimer_; }
	ProfileVal & systemTimer()	{ return systemTimer_; }

	static WatcherPtr pWatcher();
#endif // ENABLE_WATCHERS

private:
	virtual void handleTimeout( TimerHandle handle, void * arg );

	TimerHandle	timerHandle_;

	// TODO: Get rid of this friend
	friend class NetworkInterface;

	typedef IntrusiveStatWithRatesOfChange< unsigned int > Stat;

	Stat::Container * pStats_;

	Stat numBytesSent_;
	Stat numBytesResent_;
	Stat numPacketsSent_;
	Stat numPacketsResent_;
	Stat numPiggybacks_;
	Stat numPacketsSentOffChannel_;
	Stat numBundlesSent_;
	Stat numMessagesSent_;
	Stat numReliableMessagesSent_;
	Stat numFailedPacketSend_;
	Stat numFailedBundleSend_;

#if ENABLE_WATCHERS
	ProfileVal	mercuryTimer_;
	ProfileVal	systemTimer_;
#endif // ENABLE_WATCHERS

};

}

#endif // SENDING_STATS_HPP
