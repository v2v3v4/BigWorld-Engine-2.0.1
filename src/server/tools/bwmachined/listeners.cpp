/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "listeners.hpp"
#include "bwmachined.hpp"
#include <syslog.h>

Listeners::Listeners( BWMachined &machined ) : machined_( machined ) {}

void Listeners::handleNotify( const ProcessMessage &pm, in_addr addr )
{
	char address[6];
	memcpy( address, &addr, sizeof( addr ) );
	memcpy( address + sizeof( addr ), &pm.port_, sizeof( pm.port_ ) );

	Members::iterator iter = members_.begin();

	while (iter != members_.end())
	{
		ListenerMessage &lm = iter->lm_;

		if (lm.category_ == pm.category_ &&
			(lm.uid_ == lm.ANY_UID || lm.uid_ == pm.uid_) &&
			(lm.name_ == pm.name_ || lm.name_.size() == 0))
		{
			int msglen = lm.preAddr_.size() + sizeof( address ) +
				lm.postAddr_.size();
			char *data = new char[ msglen ];
			int preSize = lm.preAddr_.size();
			int postSize = lm.postAddr_.size();

			memcpy( data, lm.preAddr_.c_str(), preSize );
			memcpy( data + preSize, address, sizeof( address ) );
			memcpy( data + preSize + sizeof( address ), lm.postAddr_.c_str(),
				postSize );

			// and send to the appropriate port locally
			machined_.ep().sendto( data, msglen, lm.port_, iter->addr_ );
			delete [] data;
		}

		++iter;
	}
}

void Listeners::checkListeners()
{
	for (Members::iterator it = members_.begin(); it != members_.end(); it++)
	{
		if (!checkProcess( it->lm_.pid_ ))
		{
			syslog( LOG_INFO, "Dropping dead listener (for %s's) with pid %d",
				it->lm_.name_.c_str(), it->lm_.pid_ );
			members_.erase( it-- );
		}
	}
}
