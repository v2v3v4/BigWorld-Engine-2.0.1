/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_SECTION_CACHE_HPP
#define DATA_SECTION_CACHE_HPP

#include "datasection.hpp"
#include "cstdmf/concurrency.hpp"

#include <map>
#include <string>

/**
 *	A cache for DataSection objects. It stores the absolute path for each
 *	entry that is cached, and maps to a smart pointer. When the cache exceeds
 *	a certain amount, entries are removed from the cache on a LRU basis.
 */	

class DataSectionCache
{
public:
	~DataSectionCache();
	
	static DataSectionCache* instance();

	DataSectionCache* setSize( int maxBytes );

	static void fini();

	/// Add an entry to the cache
	void				add( const std::string & name,
		DataSectionPtr dataSection );
	
	/// Search the cache for an entry
	DataSectionPtr		find( const std::string & name );
	
	/// Remove an entry from the cache (release our reference count to it)
	void				remove( const std::string & name );

	// Clear the whole cache
	void				clear();

	/// Dump the state of the cache, for debugging
	void 				dumpCacheState();

private:
	DataSectionCache();

	/// CacheNode is used to provide a doubly-linked cache chain.
	struct CacheNode
	{
		std::string		path_;
		DataSectionPtr	dataSection_;
		int				bytes_;
		CacheNode*		prev_;
		CacheNode*		next_;
	};
	
	typedef std::map<std::string, CacheNode*> DataSectionMap;
	
	DataSectionMap		map_;
	static int			s_maxBytes_;
	static int			s_currentBytes_;
	CacheNode*			cacheHead_;
	CacheNode*			cacheTail_;
	static int			s_hits_;
	static int			s_misses_;
	static bool			s_firstTime_;

	SimpleMutex			accessControl_;

	static DataSectionCache*	s_instance;

private:

	// Helper functions
	void purgeLRU();
	void moveToHead( CacheNode * pNode );
	void unlinkNode( CacheNode * pNode );
	
	// Prevent copying
	DataSectionCache(const DataSectionCache &);
	DataSectionCache& operator=(const DataSectionCache &);
};

#endif // DATA_SECTION_CACHE_HPP
