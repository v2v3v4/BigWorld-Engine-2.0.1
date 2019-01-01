/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCE_STATISTICS_HPP
#define RESOURCE_STATISTICS_HPP

#include "console.hpp"
#include "cstdmf/resource_counters.hpp"

#include <iostream>
#include <string>

/**
 *	This class displays resource usage statistics to a console.
 *
 *	Most often, this console can be viewed by pressing \<Ctrl\> + F5
 */
class ResourceStatistics : public ResourceUsageConsole::Handler
{
public:
	~ResourceStatistics() {}
	static ResourceStatistics & instance();

	void cycleGranularity();

private:
	ResourceStatistics();
	ResourceStatistics( const ResourceStatistics& );
	ResourceStatistics& operator=( const ResourceStatistics& );

	void displayResourceStatistics( XConsole & console );

	void dumpToCSV(XConsole & console);

	void printGranularity(XConsole & console, uint hanging);
	void printUsageStatistics(XConsole & console);

	// Member variables
private:
	static ResourceStatistics			s_instance_;
	ResourceCounters::GranularityMode	granularityMode_;
	std::string							lastFileWritten_;
};

#endif // RESOURCE_STATISTICS_HPP
