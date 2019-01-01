/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "access_monitor.hpp"

DECLARE_DEBUG_COMPONENT2( "AccessMonitor", 0 );


void AccessMonitor::record( const std::string &fileName )
{
	if ( active_ )
	{
		DEBUG_MSG( "AccessMonitor::(%s)\n", fileName.c_str() );
	}
}

AccessMonitor &AccessMonitor::instance()
{
	static AccessMonitor singletonInstance_;

	return singletonInstance_;
}

#ifndef CODE_INLINE
#include "access_monitor.ipp"
#endif

/* access_monitor.cpp */
