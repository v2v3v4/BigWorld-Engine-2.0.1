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
#include "data_section_census.hpp"

#include "datasection.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/memory_tracker.hpp"

#include <map>
#include <string>

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

MEMTRACKER_DECLARE( DSCStuff, "DSCStuff", 0 );

// -----------------------------------------------------------------------------
// Section: DataSectionCensus
// -----------------------------------------------------------------------------

typedef std::map< std::string, DataSection *> DSStringMap;
typedef std::map< DataSection *, std::string> DSPointerMap;

/**
 * TODO: to be documented.
 */
struct DSCStuff : public SafeReferenceCount
{
	DSCStuff()
	{
		this->incRef();
	}

	DataSectionPtr find( const std::string & id );
	DataSectionPtr add( const std::string & id, DataSectionPtr pSect );
	void del( DataSection * pSect );
	void clear();

	DSStringMap		strMap;
	DSPointerMap	ptrMap;

	SimpleMutex		accessControl_;
};
static DSCStuff * s_pDefaultStuff = NULL;
static THREADLOCAL(int) s_ppStuffOffset /* = 0*/;
static inline DSCStuff *& pStuff()
	{ return *(DSCStuff**)(s_ppStuffOffset + ((char*)&s_pDefaultStuff)); }
static inline void pStuff( DSCStuff *& rpNewStuff )
	{ s_ppStuffOffset = ((char*)&rpNewStuff) - ((char*)&s_pDefaultStuff); }
static inline DSCStuff * pStuffAlloc()
{
	BW_GUARD;

	DSCStuff *& rpStuff = pStuff();
	MEMTRACKER_BEGIN( DSCStuff );
	if (rpStuff == NULL) rpStuff = new DSCStuff();
	MEMTRACKER_END();
	return rpStuff;
}

/**
 *	This method tells us where our pointer is stored for this thread
 */
void DataSectionCensus::store( void ** word )
{
	// get rid of the old store cleanly
	if (s_ppStuffOffset != 0)	// 'not the default'
	{
		DSCStuff *& rpStuff = pStuff();
		if (rpStuff != NULL)
		{
			bool gone = rpStuff->refCount() == 1;	// very unsafe I know!
			rpStuff->decRef();
			if (gone) rpStuff = NULL;
		}
	}

	// set up the new store
	if (word != NULL)
	{
		pStuff( *(DSCStuff**)word );
		DSCStuff *& rpStuff = pStuff();
		if (rpStuff != NULL) rpStuff->incRef();
	}
	else
	{
		pStuff( s_pDefaultStuff );
	}
}

/**
 *	This method finds the data section matched to the given identifier
 *	if one was created and is still around
 */
DataSectionPtr DataSectionCensus::find( const std::string & id )
{
	BW_GUARD;

	return pStuffAlloc()->find( id );
}
/// helper find method
DataSectionPtr DSCStuff::find( const std::string & id )
{
	BW_GUARD;

	SimpleMutexHolder permission( accessControl_ );

	DSStringMap::iterator found = strMap.find( id );
	if (found != strMap.end())
	{
		DataSectionPtr ret = DataSectionPtr( found->second, ret.FALLIBLE );
		// if the smart pointer creation fails it's because the ref count
		// is zero and someone is blocked on our accessControl_ mutex
		// waiting to delete it from us (so return empty DataSectionPtr).
		return ret;
	}

	return NULL;
}

/**
 *	This method adds a data section to the census. They can't add themselves,
 *	because they do not know (and may not have) their own file name
 */
DataSectionPtr DataSectionCensus::add( const std::string & id, DataSectionPtr pSect )
{
	return pStuffAlloc()->add( id, pSect );
}
/// helper add method
DataSectionPtr DSCStuff::add( const std::string & id, DataSectionPtr pSect )
{
	SimpleMutexHolder permission( accessControl_ );

	// First check to see if we already have this one. This only happens due
	// to a race condition between checking the cache and going and loading
	// then adding it when it isn't there. (Two threads can do this simultaneously)
	DSStringMap::iterator found = strMap.find( id );
	if (found != strMap.end())
	{
		DataSectionPtr ret = DataSectionPtr( found->second, ret.FALLIBLE );
		if (ret)
		{
			INFO_MSG( "Did not add %s to census as it is already there\n",
				id.c_str() );
			return ret;
		}

		// otherwise remove about-to-be-deleted-one from map here,
		// mainly to keep the accounting straight but also for sanity :)
#ifndef _WIN32
		ptrMap.erase( found->second );
		strMap.erase( found );
#else
		DSPointerMap::iterator found2 = ptrMap.find( found->second );
		ptrMap.erase( found2 );
		strMap.erase( found );
#endif
	}

#ifndef _WIN32
	strMap.insert( std::make_pair( id, pSect.getObject() ) );
	ptrMap.insert( std::make_pair( pSect.getObject(), id ) );
	// TODO: Accounting using g++'s stl maps
#else
	DSStringMap::iterator sit = strMap.insert(
		std::make_pair( id, pSect.getObject() ) ).first;
	DSPointerMap::iterator pit = ptrMap.insert(
		std::make_pair( pSect.getObject(), id ) ).first;
#endif

	return pSect;
}


/**
 *	This method removes a data section from the census. This should only
 *	happen when the data section is being destructed.
 */
void DataSectionCensus::del( DataSection * pSect )
{
	pStuffAlloc()->del( pSect );
}
/// helper del method
void DSCStuff::del( DataSection * pSect )
{
	SimpleMutexHolder permission( accessControl_ );

	DSPointerMap::iterator found = ptrMap.find( pSect );
	if (found != ptrMap.end())
	{
#ifndef _WIN32
		strMap.erase( found->second );
		ptrMap.erase( found );
#else
		DSStringMap::iterator found2 = strMap.find( found->second );
		strMap.erase( found2 );
		ptrMap.erase( found );
#endif
	}
}


void DataSectionCensus::fini()
{
	DSCStuff *& rpStuff = pStuff();
	if (rpStuff)
	{
		delete rpStuff;
		rpStuff = NULL;
	}
}

/**
 *	This method clears everything from the census
 */
void DataSectionCensus::clear()
{
	DSCStuff *& rpStuff = pStuff();
	if (rpStuff == NULL) return;
	rpStuff->clear();
}
/// helper clear method
void DSCStuff::clear()
{
	SimpleMutexHolder permission( accessControl_ );

	strMap.clear();
	ptrMap.clear();
}

// data_section_census.cpp
