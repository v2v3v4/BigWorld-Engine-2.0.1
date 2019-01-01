/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MONITORED_CHANNELS_HPP
#define MONITORED_CHANNELS_HPP

#include "interfaces.hpp"

#include <list>

namespace Mercury
{

class Channel;
class EventDispatcher;

/**
 *	This class is used to store monitored channels.  It provides base
 *	functionality used by IrregularChannels and KeepAliveChannels.
 */
class MonitoredChannels : public TimerHandler
{
public:
	typedef std::list< Channel * > Channels;
	typedef Channels::iterator iterator;

	MonitoredChannels() : period_( 0.f ), timerHandle_() {}
	virtual ~MonitoredChannels();

	void addIfNecessary( Channel & channel );
	void delIfNecessary( Channel & channel );
	void setPeriod( float seconds, EventDispatcher & dispatcher );

	void stopMonitoring( EventDispatcher & dispatcher );

	iterator end()	{ return channels_.end(); }

protected:
	/**
	 *  This method must return a reference to the Channel's iterator that
	 *  stores its location in this collection.
	 */
	virtual iterator & channelIter( Channel & channel ) const = 0;

	/**
	 *  This method must return the default check period for this collection,
	 *  and will be used in the first call to addIfNecessary if no period has
	 *  been set previously.
	 */
	virtual float defaultPeriod() const = 0;

	Channels channels_;
	float period_;
	TimerHandle timerHandle_;
};

} // namespace Mercury

#endif // MONITORED_CHANNELS_HPP
