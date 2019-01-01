/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACCESS_MONITOR_HPP
#define ACCESS_MONITOR_HPP

// Standard Library Headers.
#include <string>

// Standard MF Library Headers.
#include "cstdmf/debug.hpp"

/**
 *	The class is a singleton that accepts commands to dump file names in
 *	a format that is easily recognisable by a resource monitoring script.
 *	It presents a single point of change for formatting, and its behaviour
 *	can be turned on or off with a single flag - useful for command-line
 *	changes to its behaviour.
 *
 *	By default, it is inactive.
 */
class AccessMonitor
{
public:
	///	@name Constructor.
	//@{
	AccessMonitor();
	//@}

	///	@name Control Methods.
	//@{
	void record( const std::string &fileName );

	void active( bool flag );

	static AccessMonitor &instance();
	//@}

private:
	bool active_;
};


#ifdef CODE_INLINE
#include "access_monitor.ipp"
#endif

#endif

/* access_monitor.cpp */
