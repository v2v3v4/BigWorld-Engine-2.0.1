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

#include "cstdmf/concurrency.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/memory_trace.hpp"
#include "visual_manager.hpp"

#ifndef CODE_INLINE
#include "visual_manager.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

VisualManager::VisualManager() :
	fullHouse_( false )
{
}

VisualManager::~VisualManager()
{
	BW_GUARD;
	if (visuals_.size())
	{
		WARNING_MSG( "VisualManager::~VisualManager - not all visuals freed.\n" ); 
		VisualMap::iterator it = visuals_.begin();
		while (it != visuals_.end())
		{
			INFO_MSG( "-- Visual not deleted: %s\n", it->first.c_str() );
			it++;
		}
	}
}


VisualManager* VisualManager::instance()
{
	return pInstance_;
}


/**
 *	Get the given visual resource.
 */
VisualPtr VisualManager::get( const std::string& resourceID, bool loadIfMissing )
{
	BW_GUARD;
	// first check in the map
	VisualPtr p = this->find( resourceID );
	if (p) return p;
	// load it if not yet loaded
	if (BWResource::instance().openSection( resourceID ))
	{
		if (fullHouse_)
		{
			CRITICAL_MSG( "VisualManager::getVisualResource: "
				"Loading the visual '%s' into a full house!\n",
				resourceID.c_str() );
		}

		MEM_TRACE_BEGIN( resourceID )
		p = new Visual( resourceID );
		MEM_TRACE_END()

		this->add( &*p, resourceID );
	}
	return p;
}


/**
 *	Set whether or not we have a full house
 *	(and thus cannot load any new visuals)
 */
void VisualManager::fullHouse( bool noMoreEntries )
{
	fullHouse_ = noMoreEntries;
}


/**
 *	Add this visual to the map of those already loaded
 */
void VisualManager::add( Visual * pVisual, const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( visualsLock_ );

	VisualMap::iterator it = visuals_.insert(
		std::make_pair( resourceID, pVisual ) ).first;
}

void VisualManager::del(Visual* pVisual)
{
	BW_GUARD;
	if (pInstance_)
		pInstance_->delInternal( pVisual );
}

/**
 *	Remove this visual from the map of those already loaded
 */
void VisualManager::delInternal( Visual * pVisual )
{
	BW_GUARD;
	SimpleMutexHolder smh( visualsLock_ );

	for (VisualMap::iterator it = visuals_.begin(); it != visuals_.end(); it++)
	{
		if (it->second == pVisual)
		{
			//DEBUG_MSG( "VisualManager::del: %s\n", it->first.c_str() );
			visuals_.erase( it );
			return;
		}
	}

	ERROR_MSG( "VisualManager::del: "
		"Could not find visual at 0x%08X to delete it\n", pVisual );
}


/**
 *	Find this visual in the map of those already loaded
 */
VisualPtr VisualManager::find( const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( visualsLock_ );

	VisualMap::iterator it = visuals_.find( resourceID );
	if (it == visuals_.end()) return NULL;

	//return VisualPtr( it->second, VisualPtr::FALLIBLE );

	// Visual is not a SafeReferenceCount object, because we assume
	// that it is shielded from multithreading effects by Models.
	// Since this is not entirely true (particle systems, etc.) it
	// is worthwhile taking at least some precautions here...
	if (it->second->refCount() == 0) return NULL;
	return VisualPtr( it->second );
}

VisualManager* VisualManager::pInstance_ = NULL;

void VisualManager::init()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pInstance_ == NULL )
	{
		return;
	}

	pInstance_ = new VisualManager;
}

void VisualManager::fini()
{
	BW_GUARD;
	MF_ASSERT_DEV( pInstance_ );
	delete pInstance_;
	pInstance_ = NULL;
}


}

// visual_manager.cpp
