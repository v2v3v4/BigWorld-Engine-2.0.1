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

#include "resource_counters.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <d3d9types.h>

namespace 
{
	/**
	 *	This nicely formats a number.
	 *
	 *	@param number	The number to format.
	 *	@param width	The width of the resultant string.  This may be 
	 *					expanded if this number is too small and the number 
	 *					large.
	 *	@param fill		The character to use for spacing.
	 *	@param thousands	The character to use for a separators between
	 *					thousand units.  If this is NULL then no separator is
	 *					is used.
	 */
	std::string 
	formatNumber
	(
		uint32			number, 
		uint32			width, 
		char			fill			= ' ', 
		char			thousands		= ','
	)
	{
		uint32 n = number;
		uint32 numchars = 0;
		do { n /= 10; ++numchars; } while (n != 0);

		if (thousands != '\0')
			numchars += (numchars - 1)/3;

		size_t len = std::max(width, numchars);
		std::string result(len, fill);

		size_t pos = len - 1;
		size_t ths = 3;
		do
		{
			result[pos] = '0' + (char)(number%10);
			number /= 10;
			--pos;
			--ths;
			if (thousands != '\0' && ths == 0 && number != 0)
			{
				result[pos] = thousands;
				ths = 3;
				--pos;
			}
		}
		while (number != 0);

		return result;
	}
}


/**
 *	Initialise static member variables
 */
ResourceCounters 	ResourceCounters::s_instance_;
bool				ResourceCounters::s_bValid_ = false;


/**
 *	Initialises a ResourceEntry with zero values.
 */
ResourceCounters::Entry::Entry():
	size_(0),
	peakSize_(0),
	instances_(0),
	peakInstances_(0),
	numberAdds_(0),
	numberSubs_(0),
	sum_(0)
{
}


/**
 *	Returns the resource counters singleton instance.
 *
 *	@returns ResourceCounters	Singleton instance
 */
/*static*/ ResourceCounters &ResourceCounters::instance()
{
	return s_instance_;
}


/**
 *	Adds an amount to the passed resource.
 *
 *  @param descriptionPool	The resource and pool to add too.
 *  @param amount			The amount to add to the given resource.
 *  @returns				The current amount allocated to the given resource.
 */
size_t ResourceCounters::add(const DescriptionPool& descriptionPool, size_t amount)
{
	SimpleMutexHolder smh( accessMutex_ );

	Entry &entry = resourceCounts_[descriptionPool];
	
	if (amount != 0)
	{	
		++entry.numberAdds_;
		
		++entry.instances_;
		entry.instances_ > entry.peakInstances_ ? entry.peakInstances_ = entry.instances_ : 0;
		
		entry.size_ += amount;
		entry.size_ > entry.peakSize_ ? entry.peakSize_ = entry.size_ : 0;
		entry.sum_ += amount;
	}
	
	return entry.size_;
}


/**
 *	Subtracts an amount from the passed resource.
 *
 *  @param descriptionPool	The resource and pool to subtract from.
 *  @param amount			The amount to subtract from the given resource.
 *  @returns				The current amount allocated to the given resource.
 */
size_t ResourceCounters::subtract(const DescriptionPool& descriptionPool, size_t amount)
{
	SimpleMutexHolder smh( accessMutex_ );

	Entry &entry = resourceCounts_[descriptionPool];
	
	if (amount != 0)
	{
		++entry.numberSubs_;

		// Shouldn't get negative instances
		if (entry.instances_ > 0)
			--entry.instances_;

		// Shouldn't get negative memory allocated
		if (amount <= entry.size_)
			entry.size_ -= amount;
	}
	
	return entry.size_;
}


bool ResourceCounters::printPoolContents( uint pool )
{
	ResourceCountMap::const_iterator it = this->resourceCounts_.begin();
	DEBUG_MSG("Resource pool contents: (pool id %d)\n", pool);
	for (; it != this->resourceCounts_.end(); it++)
	{
		if ((*it).first.second == pool)
		{
			if ((*it).second.instances_ > 0)
			{
				WARNING_MSG("\tResource (%d): %s\n",
								(*it).second.instances_,
								(*it).first.first.c_str());
			}
		}
	}
	return true;
}

/**
 *	Returns the current allocation for the given resource.
 *
 *  @param descriptionPool	The resource to get.
 *  @returns				The current allocation for the given resource.
 */
size_t ResourceCounters::operator[](const DescriptionPool& descriptionPool) const
{
	ResourceCounters *myself = const_cast<ResourceCounters *>(this);
	if (myself->resourceCounts_.find(descriptionPool) == myself->resourceCounts_.end())
	{
		return 0;
	}
	else
	{
		return myself->resourceCounts_[descriptionPool].size_;
	}
}


/**
 *	This allows iteration over the resources being counted.
 *
 *  @returns				An iterator at the beginning of the resources
 *							being counted.
 */
ResourceCounters::Iterator ResourceCounters::begin() const
{
	return resourceCounts_.begin();
}


/**
 *	This allows iteration over the resources being counted.
 *
 *  @returns				An iterator at the end of the resources
 *							being counted.
 */
ResourceCounters::Iterator ResourceCounters::end() const
{
	return resourceCounts_.end();
}


/**
 *	Returns a map of the descriptions while ignoring pool.
 *
 *  @returns				A map of descriptions.
 */
std::map<int, std::string> ResourceCounters::getDescriptionsMap()
{
	uint count = 0;
	std::map<int, std::string> descriptions;
	Iterator it = resourceCounts_.begin();

	// Loop through the resources
	while (it != resourceCounts_.end())
	{
		std::map<int, std::string>::iterator it2 = descriptions.begin();
		bool found = false;

		// Search for the description in the descriptions map
		while (it2 != descriptions.end())
		{
			if (it2->second == it->first.first)
			{
				found = true;
				break;
			}

			it2++;
		}

		// Check if the description is already in the description map
		if (!found)
			descriptions[count++] = it->first.first;

		it++;
	}

	return descriptions;
}


/**
 *	Returns a string description of the current memory usage.
 *
 *	@param granularityMode	The granularity mode to be used to categorise the resource usage.
 *  @returns				String describing the current memory usage.
 */
std::string ResourceCounters::getUsageDescription(GranularityMode granularityMode)
{
	// Create an output file stream
	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	// Print the file header to the stream
	addHeader(ss);

	// Get descriptions map
	std::map<int, std::string> descriptionsMap = getDescriptionsMap();

	// Add total usage
	addTotalUsage(ss, descriptionsMap, SYSTEM_MASK | DEFAULT_MASK | MANAGED_MASK | MISC_MASK);	

	// Create the appropriate subtrees based on granularity
	switch (granularityMode)
	{
	case MEMORY:
		{
			addMemorySubtree(ss, descriptionsMap, SYSTEM_MASK | DEFAULT_MASK | MANAGED_MASK | MISC_MASK);
		} break;
	case SYSTEM_VIDEO_MISC:
		{
			addTotalUsage(ss, descriptionsMap, SYSTEM_MASK);
			addMemorySubtree(ss, descriptionsMap, SYSTEM_MASK);
			ss << std::endl;
			addTotalUsage(ss, descriptionsMap, DEFAULT_MASK | MANAGED_MASK);
			addMemorySubtree(ss, descriptionsMap, DEFAULT_MASK | MANAGED_MASK);
			ss << std::endl;
			addTotalUsage(ss, descriptionsMap, MISC_MASK);
			addMemorySubtree(ss, descriptionsMap, MISC_MASK);
		} break;
	case SYSTEM_DEFAULT_MANAGED_MISC:
		{
			addTotalUsage(ss, descriptionsMap, SYSTEM_MASK);
			addMemorySubtree(ss, descriptionsMap, SYSTEM_MASK);
			ss << std::endl;
			addTotalUsage(ss, descriptionsMap, DEFAULT_MASK);
			addMemorySubtree(ss, descriptionsMap, DEFAULT_MASK);
			ss << std::endl;
			addTotalUsage(ss, descriptionsMap, MANAGED_MASK);
			addMemorySubtree(ss, descriptionsMap, MANAGED_MASK);
			ss << std::endl;
			addTotalUsage(ss, descriptionsMap, MISC_MASK);
			addMemorySubtree(ss, descriptionsMap, MISC_MASK);
		} break;
	default:
		{
			ERROR_MSG(	"ResourceCounters::getUsageDescription : "
						"Unknown granularity type - '%d'\n",
						granularityMode );
		}
	}

	return ss.str();
}


/**
 *	Prints the file header to fileName.
 *
 *  @param out			The stream the header is written to.
 */
void ResourceCounters::addHeader(std::ostream& out)
{
	out << "                                    |Usage       |Peak        |Insts |Peak  |Allocs |De-    |Average     " << std::endl;
	out << "Name                                |            |Usage       |      |Insts |       |allocs |            " << std::endl;
	out << "---------------------------------------------------------------------------------------------------------" << std::endl;
}


/**
 *	Adds the total usage for the passed memory pool to the passed stringstream.
 *
 *  @param ss			The stream to write to.
 *  @param descriptions	The map of descriptions.
 *  @param pool			The usage memory pool.
 */
void ResourceCounters::addTotalUsage(	std::stringstream& ss,
										std::map<int, std::string>& descriptions,
										uint pool)
{
	uint size = 0;
	uint peakSize = 0;
	uint instances = 0;
	uint peakInstances = 0;
	uint numberAdds = 0;
	uint numberSubs = 0;
	uint64 sum = 0;

	// First level is total memory usage
	Iterator it = resourceCounts_.begin();

	// Loop through the resources adding up total memory usage statistics
	while (it != resourceCounts_.end())
	{
		Entry const &entry = it->second;

		if (	((pool & SYSTEM_MASK) && it->first.second == D3DPOOL_SYSTEMMEM) ||
				((pool & DEFAULT_MASK) && it->first.second == D3DPOOL_DEFAULT) ||
				((pool & MANAGED_MASK) && it->first.second == D3DPOOL_MANAGED) ||
				(	(pool & MISC_MASK) &&
					(it->first.second != D3DPOOL_SYSTEMMEM) &&
					(it->first.second != D3DPOOL_DEFAULT) &&
					(it->first.second != D3DPOOL_MANAGED)
				)
			)
		{
			size          += entry.size_;
			peakSize      += entry.peakSize_;
			instances     += entry.instances_;
			peakInstances += entry.peakInstances_;
			numberAdds    += entry.numberAdds_;
			numberSubs    += entry.numberSubs_;
			sum           += entry.sum_;
		}

		it++;
	}
	uint64 average = 0;
	if (numberAdds != 0) average = sum/numberAdds;

	std::string desc;
	if ((pool & DEFAULT_MASK) &&
		(pool & MANAGED_MASK) &&
		(pool & SYSTEM_MASK) &&
		(pool & MISC_MASK))
		desc = "Total usage";
	else if (	(pool & DEFAULT_MASK) &&
				(pool & MANAGED_MASK))
		desc = "   Video";
	else if (pool & SYSTEM_MASK)
		desc = "   System";
	else if (pool & DEFAULT_MASK)
		desc = "   Default";
	else if (pool & MANAGED_MASK)
		desc = "   Managed";
	else if (pool & MISC_MASK)
		desc = "   Misc";
	else
		ERROR_MSG("ResourceCounters::addTotalUsage : Unknown memory pool!");

	// Output statistics
	std::ios_base::fmtflags old_options = ss.flags(std::ios_base::left);
	ss << std::setw(34) << std::setfill(' ') << desc << " ";
	ss.flags(old_options);
	
	old_options = ss.flags(std::ios_base::right);

	ss	<< formatNumber(size           , 12) << ' '
		<< formatNumber(peakSize       , 12) << ' '
		<< formatNumber(instances      ,  6) << ' '
		<< formatNumber(peakInstances  ,  6) << ' '
		<< formatNumber(numberAdds     ,  7) << ' '
		<< formatNumber(numberSubs     ,  7) << ' '
		<< formatNumber((uint32)average, 12) << std::endl;

	ss.flags(old_options);
}


/**
 *	Adds the usage subtree for the passed memory pool to the passed stringstream.
 *
 *  @param ss			The stream to write to.
 *  @param descriptions	The map of descriptions.
 *  @param pool			The usage memory pool.
 */
void ResourceCounters::addMemorySubtree(std::stringstream& ss,
										std::map<int, std::string>& descriptions,
										uint pool)
{
	uint depth = 1;
	uint lastDrawnDepth = 1;
	bool hasChildren = false;
	std::string::size_type delimiter = 0;
	std::string::size_type delimiter2 = 0;
	std::string desc = "";
	std::string previousDesc = "";
	uint size = 0;
	uint peakSize = 0;
	uint instances = 0;
	uint peakInstances = 0;
	uint numberAdds = 0;
	uint numberSubs = 0;
	uint64 sum = 0;
	
	// Loop through the descriptions
	std::map<int, std::string>::iterator it = descriptions.begin();
	while (it != descriptions.end())
	{
		delimiter = 0;
		delimiter2 = 0;

		previousDesc = desc;
		desc = it->second;

		if (!hasChildren && depth > 1)
		{
			depth = 1;

			while (true)
			{
				delimiter = desc.find("/");
				delimiter2 = previousDesc.find("/");
					
				if (delimiter != std::string::npos &&
					delimiter2 != std::string::npos)
				{
					// Compare substrings
					if (desc.substr(0, delimiter) == 
						previousDesc.substr(0, delimiter2))
					{
						desc = desc.substr(delimiter+1, desc.size());
						previousDesc = previousDesc.substr(delimiter+1, previousDesc.size());
						depth++;
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		
		delimiter2 = 0;
		hasChildren = false;
		desc = it->second;

		// Parse the depth-th token from the description
		for (uint i = 0; i < depth; i++)
		{
			delimiter = desc.find("/");
			 if (delimiter != std::string::npos)
			{
				desc = desc.substr(delimiter+1, desc.size());
				delimiter2 += delimiter;
			}
			else
			{
				break;
			}
		}

		if (delimiter != std::string::npos)
		{
			desc = it->second.substr(0, delimiter2 + 1);
		}
		else
		{
			desc = it->second;
		}

		size = 0;
		peakSize = 0;
		instances = 0;
		peakInstances = 0;
		numberAdds = 0;
		numberSubs = 0;
		sum = 0;

		// Loop through the resources adding all statistics that match this root
		Iterator res_it = resourceCounts_.begin();

		while (res_it != resourceCounts_.end())
		{
			Entry const &entry = res_it->second;

			if ((res_it->first.first.find(desc) == 0 && delimiter != std::string::npos) ||
				(res_it->first.first.find(desc) == 0 && delimiter == std::string::npos && res_it->first.first.size() == it->second.size()))
			{
				if (res_it->first.first != desc)
					hasChildren = true;

				if (	((pool & SYSTEM_MASK) && res_it->first.second == D3DPOOL_SYSTEMMEM) ||
						((pool & DEFAULT_MASK) && res_it->first.second == D3DPOOL_DEFAULT) ||
						((pool & MANAGED_MASK) && res_it->first.second == D3DPOOL_MANAGED) ||
						(	(pool & MISC_MASK) &&
							(res_it->first.second != D3DPOOL_SYSTEMMEM) &&
							(res_it->first.second != D3DPOOL_DEFAULT) &&
							(res_it->first.second != D3DPOOL_MANAGED)
						)
					)
				{
					size += entry.size_;
					peakSize += entry.peakSize_;
					instances += entry.instances_;
					peakInstances += entry.peakInstances_;
					numberAdds += entry.numberAdds_;
					numberSubs += entry.numberSubs_;
					sum += entry.sum_;
				}
			}

			res_it++;
		}

		//if (size)  // Commented out to prevent the list jumping
		{
			if (lastDrawnDepth > depth)
			{
				ss << std::endl;
			}

			lastDrawnDepth = depth;

			// Output statistics
			std::string name = setIndentation(pool, depth) + parseDescription(desc, depth);
			std::ios_base::fmtflags old_options = ss.flags(std::ios_base::left);
			ss << std::setw(34) << std::setfill(' ') << name << " ";
			ss.flags(old_options);
			
			old_options = ss.flags(std::ios_base::right);

			uint64 average = 0;
			if (numberAdds != 0) average = sum/numberAdds;

			ss	<< formatNumber(size           , 12) << ' '
				<< formatNumber(peakSize       , 12) << ' '
				<< formatNumber(instances      ,  6) << ' '
				<< formatNumber(peakInstances  ,  6) << ' '
				<< formatNumber(numberAdds     ,  7) << ' '
				<< formatNumber(numberSubs     ,  7) << ' '
				<< formatNumber((uint32)average, 12) << std::endl;

			ss.flags(old_options);
		}

		if (hasChildren)
		{
			depth++;
			continue;
		}

		it++;
	}
}


/**
 *	Sets the indentation for the current pool and depth.
 *
 *  @param pool			The usage memory pool.
 *  @param depth		The current depth.
 */
std::string ResourceCounters::setIndentation(uint pool, uint depth)
{
	std::string indent;
	if ((pool & DEFAULT_MASK) &&
		(pool & MANAGED_MASK) &&
		(pool & SYSTEM_MASK) &&
		(pool & MISC_MASK))
		indent = "";
	else
		indent = "   ";
	
	for (uint i = 0; i < depth; i++)
	{
		indent += "   ";
	}

	return indent;
}


/**
 *	Parses the description for the current depth.
 *
 *  @param desc			The current description.
 *  @param depth		The current depth.
 */
std::string ResourceCounters::parseDescription(std::string desc, uint depth)
{
	std::string::size_type delimiter = 0;
	std::string result = desc;

	for (uint i = 0; i < depth; i++)
	{
		delimiter = desc.find("/");
		if (delimiter != std::string::npos)
		{
			result = desc.substr(0, delimiter);
			desc = desc.substr(delimiter+1, desc.size());			
		}
		else
		{
			result = desc;
			break;
		}
	}

	return result;
}


bool ResourceCounters::toCSV(const std::string &filename)
{
	std::ofstream out(filename.c_str());

	out << "Name, Size, Peak size, Instances, Peak Instances, ";
	out << "Number adds, Number subs, Sum" << std::endl;

	for 
	(
		ResourceCountMap::iterator it = resourceCounts_.begin();
		it != resourceCounts_.end();
		++it
	)
	{
		out << it->first.first           << ',';
		out << it->second.size_          << ',';
		out << it->second.peakSize_      << ',';
		out << it->second.instances_     << ',';
		out << it->second.peakInstances_ << ',';
		out << it->second.numberAdds_    << ',';
		out << it->second.numberSubs_    << ',';
		out << it->second.sum_           << std::endl;
	}

	out.close();

	return true;
}


/**
 *	This is the default constructor for the ResourceCounters class.
 */
ResourceCounters::ResourceCounters():
	lastHandle_(0)
{
	resourceHandles_[lastHandle_] = "Unknown memory";

	s_bValid_ = true;
}


/**
 *	Returns a handle for the passed description.
 *
 *  @param description		The description to get a handle for.
 *  @returns				A handle to the passed description.
 */
ResourceCounters::Handle ResourceCounters::newHandle(const std::string& description)
{
	// Iterate through the map searching for the descriptionPool value
	for (uint i = 0; i <= lastHandle_; i++)
	{
		if (resourceHandles_[i] == description)
			return i;
	}

	// This is a new description pool
	Handle result = ++lastHandle_;
	resourceHandles_[result] = description;
	return result;
}


/**
 *	Returns a handle for the passed description.
 *
 *  @param handle			The handle to get a description for.
 *  @returns				The description of the passed handle.
 */
std::string ResourceCounters::description(Handle handle) const
{
	ResourceHandlesConstIter iter = resourceHandles_.find(handle);
	if (iter == resourceHandles_.end())
		return std::string();
	else
		return iter->second;
}
