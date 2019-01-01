/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCE_MANAGER_STATS_HPP
#define RESOURCE_MANAGER_STATS_HPP

#include <d3d9types.h>
#include "moo/moo_dx.hpp"
#include "moo/device_callback.hpp"
#include "xconsole.hpp"

const int NUM_FRAMES = 1;

/**
 * TODO: to be documented.
 */
class ResourceManagerStats : public Moo::DeviceCallback
{
public:
	static ResourceManagerStats& instance();
	static void fini();
	void displayStatistics( XConsole & console );
	void logStatistics( std::ostream& f_logFile );
	bool enabled() const
	{
		return (available_ && enabled_);
	}
	void createUnmanagedObjects();
	void deleteUnmanagedObjects();
private:
	static ResourceManagerStats* s_instance_;
	ResourceManagerStats();
	~ResourceManagerStats();
	const std::string& resourceString( uint32 i );
	void getData();
	D3DRESOURCESTATS results_[NUM_FRAMES][D3DRTYPECOUNT];
	DX::Query*	pEventQuery_;
	bool		available_;
	bool		enabled_;
	int			frame_;
};

#endif
