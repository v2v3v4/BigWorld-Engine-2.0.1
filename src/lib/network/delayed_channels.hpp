/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DELAYED_CHANNELS_HPP
#define DELAYED_CHANNELS_HPP

#include "frequent_tasks.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/singleton.hpp"

#include <set>

namespace Mercury
{

class Channel;
class EventDispatcher;

typedef SmartPointer< Channel > ChannelPtr;

/**
 *	This class is responsible for sending delayed channels. These are channels
 *	that have had delayedSend called on them. This is so that multiple messages
 *	can be put on the outgoing bundle instead of sending a bundle for each.
 */
class DelayedChannels : public FrequentTask
{
public:
	void init( EventDispatcher & dispatcher );
	void fini( EventDispatcher & dispatcher );

	void add( Channel & channel );

	void sendIfDelayed( Channel & channel );

private:
	virtual void doTask();

	typedef std::set< ChannelPtr > Channels;
	Channels channels_;
};

} // namespace Mercury

#endif // DELAYED_CHANNELS_HPP
