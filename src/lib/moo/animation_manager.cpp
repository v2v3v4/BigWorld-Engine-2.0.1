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

#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/timestamp.hpp"
#include "cstdmf/memory_trace.hpp"
#include "cstdmf/concurrency.hpp"

#include "moo_math.hpp"
#include "math/blend_transform.hpp"
#include "animation.hpp"
#include "interpolated_animation_channel.hpp"
#include "discrete_animation_channel.hpp"

#include "animation_manager.hpp"

#ifndef CODE_INLINE
#include "animation_manager.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )


BW_SINGLETON_STORAGE( Moo::AnimationManager )


namespace Moo
{

AnimationManager::AnimationManager() :
	fullHouse_( false )
{
}


/**
 *	This method retrieves the given animation resource, tied to the hierarchy
 *	starting from the input rootNode.
 */
AnimationPtr AnimationManager::get( const std::string& resourceID, NodePtr rootNode )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( rootNode )
	{
		return NULL;
	}

/*	MEMORYSTATUS preStat;
	GlobalMemoryStatus( &preStat );*/

	AnimationPtr res;

	AnimationPtr found = this->find( resourceID );
	if (found)
	{
		MEM_TRACE_BEGIN( resourceID )
		res = new Animation( &*found, rootNode );
		MEM_TRACE_END()
	}

/*	static SIZE_T animationMem = 0;
	MEMORYSTATUS postStat;

	GlobalMemoryStatus( &postStat );
	animationMem += preStat.dwAvailPhys - postStat.dwAvailPhys;
	TRACE_MSG( "AnimationManager: Loaded animation '%s' animations used memory: %d\n",
		resourceID.c_str(), animationMem / 1024 );*/

	return res;
}


/**
 *	This method retrieves the given animation resource, tied to the nodes
 *	recorded in the input catalogue.
 */
AnimationPtr AnimationManager::get( const std::string& resourceID )
{
	BW_GUARD;	
/*	MEMORYSTATUS preStat;
	GlobalMemoryStatus( &preStat );*/

	AnimationPtr res;

	AnimationPtr found = this->find( resourceID );
	if (found.getObject() != NULL)
	{
		MEM_TRACE_BEGIN( resourceID )
		res = new Animation( &*found );
		MEM_TRACE_END()
	}

/*	static SIZE_T animationMem = 0;
	MEMORYSTATUS postStat;

	GlobalMemoryStatus( &postStat );
	animationMem += preStat.dwAvailPhys - postStat.dwAvailPhys;
	TRACE_MSG( "AnimationManager: Loaded animation '%s' animations used memory: %d\n",
		resourceID.c_str(), animationMem / 1024 );*/

	return res;
}


/**
 *	This method removes the given animation from our map, if it is present
 */
void AnimationManager::del( Animation * pAnimation )
{
	BW_GUARD;
	SimpleMutexHolder smh( animationsLock_ );

	for (AnimationMap::iterator it = animations_.begin();
		it != animations_.end();
		it++)
	{
		if (it->second == pAnimation)
		{
			animations_.erase( it );
			break;
		}
	}
}


/**
 *	Set whether or not we have a full house
 *	(and thus cannot load any new animations)
 */
void AnimationManager::fullHouse( bool noMoreEntries )
{
	fullHouse_ = noMoreEntries;
}



/**
 *	This private method returns the animation manager's private copy of the
 *	the input animation resource. The AnimationManager keeps a map of
 *	animations that it has loaded, and if the input name exists in the vector,
 *	it returns that one; otherwise it loads a new one and returns it
 *	(after storing it in the vector for next time).
 */
AnimationPtr AnimationManager::find( const std::string & resourceID )
{
	BW_GUARD;
	AnimationPtr res;

	animationsLock_.grab();
	AnimationMap::iterator it = animations_.find( resourceID );
	if (it != animations_.end())
	{
		//res = AnimationPtr( it->second, AnimationPtr::FALLIBLE );
		// not a SafeReferenceCount object, can't can't do the above
		if (it->second->refCount() != 0)
			res = it->second;
	}
	animationsLock_.give();

	if (!res)
	{
		if (fullHouse_)
		{
			CRITICAL_MSG( "AnimationManager::getAnimationResource: "
				"Loading the animation '%s' into a full house!\n",
				resourceID.c_str() );
		}

		MEM_TRACE_BEGIN( resourceID )
		res = new Animation;

		if (!res->load( resourceID ))
		{
			res = NULL;	// will delete by refcount
		}
		else
		{
			animationsLock_.grab();
			it = animations_.insert( std::make_pair( resourceID, &*res) ).first;
			animationsLock_.give();
		}
		MEM_TRACE_END()
	}

	return res;
}

std::string AnimationManager::resourceID( Animation * pAnim )
{
	BW_GUARD;
	while (pAnim->pMother_.getObject() != NULL)
	{
		pAnim = pAnim->pMother_.getObject();
	}

	SimpleMutexHolder smh( animationsLock_ );

	AnimationMap::iterator it = animations_.begin();
	AnimationMap::iterator end = animations_.end();

	while (it != end)
	{
		if (it->second == pAnim)
			return it->first;
		it++;
	}

	return std::string();
}




}

// animation_manager.cpp
