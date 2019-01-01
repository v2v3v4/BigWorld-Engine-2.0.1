/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCE_COUNTER_HPP
#define RESOURCE_COUNTER_HPP

#include "config.hpp"
#include "stdmf.hpp"
#include "concurrency.hpp"

#include <map>
#include <utility>
#include <string>

/**
 *	The ResourceCounters class is a simple class that tracks the memory usage of a component
 *	based on a description string (the component allocating/deallocating the memory) and a
 *	memory pool enumeration (the memory pool to be used, i.e. system memory, managed memory,
 *	or default memory). The main two worker methods of this class are add and subtract. These
 *	methods track memory allocations and deallocations of a component using the description-pool.
 *	The ResourceCounters class maintains a map of the components that are being memory tracked.
 *	Rather than calling the methods of ResourceCounters directly, the exposed MACROS below should
 *	be used.
 */
class ResourceCounters
{
public:
	typedef std::pair<std::string, uint>			DescriptionPool;
	typedef unsigned int							Handle;

	enum GranularityMode {MEMORY, SYSTEM_VIDEO_MISC, SYSTEM_DEFAULT_MANAGED_MISC};
	enum MemoryPool {DEFAULT = 0, MANAGED = 1, SYSTEM = 2};
	enum MemoryPoolMask {DEFAULT_MASK = 1, MANAGED_MASK = 2, SYSTEM_MASK = 4, MISC_MASK = 8};

	static ResourceCounters& instance();
	~ResourceCounters() { s_bValid_ = false; }

	size_t add(const DescriptionPool& descriptionPool, size_t amount);
	size_t subtract(const DescriptionPool& descriptionPool, size_t amount);

	std::string getUsageDescription(GranularityMode granularityMode);

	bool toCSV(const std::string &filename);

	Handle newHandle(const std::string& description);
	std::string description(Handle handle) const;

	static bool isValid() { return s_bValid_; }

	bool printPoolContents(uint pool);

private:
	struct Entry
	{
		size_t			size_;
		size_t			peakSize_;
		size_t			instances_;
		size_t			peakInstances_;
		size_t			numberAdds_;
		size_t			numberSubs_;
		uint64			sum_;

		Entry();
	};


	typedef std::map<DescriptionPool, Entry>		ResourceCountMap;

	static ResourceCounters		s_instance_;
	static bool					s_bValid_;
	ResourceCountMap			resourceCounts_;

	typedef ResourceCountMap::const_iterator		Iterator;

	ResourceCounters();

	size_t operator[](const DescriptionPool& descriptionPool) const;

	Iterator begin() const;
	Iterator end() const;

	std::map<int, std::string> getDescriptionsMap();

	void addHeader(std::ostream& out);
	void addTotalUsage(std::stringstream& ss, std::map<int, std::string>& descriptions, uint pool);
	void addMemorySubtree(std::stringstream& ss, std::map<int, std::string>& descriptions, uint pool);

	std::string setIndentation(uint pool, uint depth);
	std::string parseDescription(std::string desc, uint depth);

	typedef std::map<Handle, std::string>				ResourceHandles;
	typedef ResourceHandles::const_iterator				ResourceHandlesConstIter;

	ResourceHandles	resourceHandles_;
	Handle			lastHandle_;
	SimpleMutex		accessMutex_;
};


#if ENABLE_RESOURCE_COUNTERS

#define RESOURCE_COUNTER_ADD(DESCRIPTION_POOL, AMOUNT)						  \
	if (ResourceCounters::instance().isValid())								  \
		ResourceCounters::instance().add(DESCRIPTION_POOL, (size_t)(AMOUNT)); 

#define RESOURCE_COUNTER_SUB(DESCRIPTION_POOL, AMOUNT)						  \
	if (ResourceCounters::instance().isValid())								  \
		ResourceCounters::instance().subtract(DESCRIPTION_POOL, (size_t)(AMOUNT)); 

#define RESOURCE_COUNTER_NEWHANDLE(DESCRIPTION)								  \
	((ResourceCounters::instance().isValid())								  \
		? ResourceCounters::instance().newHandle(DESCRIPTION)				  \
		: (unsigned int)-1);

#else

#define RESOURCE_COUNTER_ADD(DESCRIPTION_POOL, AMOUNT)
#define RESOURCE_COUNTER_SUB(DESCRIPTION_POOL, AMOUNT)
#define RESOURCE_COUNTER_NEWHANDLE(DESCRIPTION)

#endif // ENABLE_RESOURCE_COUNTERS


#endif // RESOURCE_COUNTER_HPP
