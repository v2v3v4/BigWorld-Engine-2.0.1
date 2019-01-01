/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONDEMNED_CHANNELS_HPP
#define CONDEMNED_CHANNELS_HPP

#include "network/interfaces.hpp"
#include "misc.hpp"

#include <list>
#include <map>

namespace Mercury
{

class Channel;

/**
 *	This class is used handle channels that will be deleted once they have no
 *	unacked packets.
 */
class CondemnedChannels : public TimerHandler
{
public:
	CondemnedChannels() : timerHandle_() {}
	~CondemnedChannels();

	void add( Channel * pChannel );

	Channel * find( ChannelID channelID ) const;

	bool deleteFinishedChannels();

	int numCriticalChannels() const;

private:
	static const int AGE_LIMIT = 60;

	virtual void handleTimeout( TimerHandle, void * );
	bool shouldDelete( Channel * pChannel, uint64 now );

	typedef std::list< Channel * > NonIndexedChannels;
	typedef std::map< ChannelID, Channel * > IndexedChannels;

	NonIndexedChannels	nonIndexedChannels_;
	IndexedChannels		indexedChannels_;

	TimerHandle timerHandle_;
};

} // namespace Mercury

#endif // CONDEMNED_CHANNELS_HPP
