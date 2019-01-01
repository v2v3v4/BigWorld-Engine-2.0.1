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
#include "resource_statistics.hpp"
#include "cstdmf/resource_counters.hpp"
#include <sstream>


// Defines
#define GRANULARITY_MODE_COUNT 3

/**
 *	Initalise the singleton instance.
 */
ResourceStatistics ResourceStatistics::s_instance_;


/**
 *	Default constructor
 */
ResourceStatistics::ResourceStatistics() :
	granularityMode_(ResourceCounters::MEMORY)
{
	BW_GUARD;
}

	
/**
 *	Returns the ResourceStatistics singleton instance
 *
 *	returns	Singleton instance
 */
ResourceStatistics& ResourceStatistics::instance()
{
	BW_GUARD;
	return s_instance_;
}


/**
 *	Displays resource usage statistics in the passed console.
 *
 *	param console	The console the statistics will be displayed in.
 */
void ResourceStatistics::displayResourceStatistics(XConsole & console)
{
	BW_GUARD;
	static const size_t STR_SIZE = 64;
	char statString[STR_SIZE];
	statString[STR_SIZE - 1] = 0;

	// Console title
	console.setCursor(0, 0);
	console.print("Resource Usage Console\n\n");

	// Display last file written
	if (!lastFileWritten_.empty())
		console.print("Last file written: " + lastFileWritten_ + "\n\n");
	else
		console.print("Press the 'c' key to save to a .CSV file\n\n");

	// Display granularity
	console.print("Granularity (SPACE) - ");
	printGranularity(console, 22);
	
	// Display resource usage statistics
	printUsageStatistics(console);
}


/**
 *	This dumps the current resource statistics to a CSV file.
 */
void ResourceStatistics::dumpToCSV(XConsole & console)
{
	BW_GUARD;
	// Find a free filename:
	size_t cnt = 0;
	std::string filename;
	while (true)
	{
		std::ostringstream out;
		out << "resource_stats_" << (int)cnt++ << ".csv";
		filename = out.str();
		FILE *file = bw_fopen(filename.c_str(), "rb");
		bool newFile = file == NULL;
		if (file != NULL)
			fclose(file);
		if (newFile)
			break;
	}

	ResourceCounters::instance().toCSV(filename);
	lastFileWritten_ = filename;
}


/**
 *	Cycles through the granularity modes.
 */
void ResourceStatistics::cycleGranularity()
{
	BW_GUARD;
	granularityMode_ = static_cast<ResourceCounters::GranularityMode>((granularityMode_ + 1) % GRANULARITY_MODE_COUNT);
}


/**
 *	Prints the current granularity to the passed console using the passed hanging.
 *
 *	param console	The output console
 *	param hanging	The hanging to use when outputting
 */
void ResourceStatistics::printGranularity(XConsole & console, uint hanging)
{
	BW_GUARD;
	static std::string spaces = "                                            ";
	
	switch (granularityMode_)
	{
	case ResourceCounters::MEMORY:
		{
			console.print("One pool\n");
			console.print(spaces.substr(0, hanging));
			console.print("1 - Memory\n\n\n");
		} break;
	case ResourceCounters::SYSTEM_VIDEO_MISC:
		{
			console.print("Three pools\n");
			console.print(spaces.substr(0, hanging));
			console.print("1 - System Memory\n");
			console.print(spaces.substr(0, hanging));
			console.print("2 - Video Memory\n");
			console.print(spaces.substr(0, hanging));
			console.print("3 - Misc. Memory\n\n\n");
		} break;
	case ResourceCounters::SYSTEM_DEFAULT_MANAGED_MISC:
		{
			console.print("Four pools\n");
			console.print(spaces.substr(0, hanging));
			console.print("1 - System Memory\n");
			console.print(spaces.substr(0, hanging));
			console.print("2 - Default Video Memory\n");
			console.print(spaces.substr(0, hanging));
			console.print("3 - Managed Video Memory\n");
			console.print(spaces.substr(0, hanging));
			console.print("4 - Misc. Memory\n\n\n");
		} break;
	default:
		{
			ERROR_MSG("ResourceStatistics::getGranularity:"
					"Unknown granularity type - '%d'\n",
					granularityMode_ );
		}
	}
}


/**
 *	Prints the current granularity to the passed console using the passed hanging.
 *
 *	param console	The output console
 */
void ResourceStatistics::printUsageStatistics(XConsole & console)
{
	BW_GUARD;
	console.print(ResourceCounters::instance().getUsageDescription(granularityMode_));
}