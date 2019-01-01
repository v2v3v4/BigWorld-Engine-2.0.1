/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef IRREGULAR_CHANNELS_HPP
#define IRREGULAR_CHANNELS_HPP

#include "monitored_channels.hpp"

namespace Mercury
{

/**
 *	This class is used to store irregular channels and manages their resends.
 */
class IrregularChannels : public MonitoredChannels
{
public:
	void addIfNecessary( Channel & channel );

protected:
	iterator & channelIter( Channel & channel ) const;
	float defaultPeriod() const;

private:
	virtual void handleTimeout( TimerHandle, void * );
};

} // namespace Mercury

#endif // IRREGULAR_CHANNELS_HPP
