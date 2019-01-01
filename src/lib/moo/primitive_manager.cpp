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
#include "primitive_manager.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_trace.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

PrimitiveManager::PrimitiveManager()
{
}

PrimitiveManager::~PrimitiveManager()
{
	BW_GUARD;
	while (!primitives_.empty())
	{
		primitives_.erase( primitives_.begin() );
	}
}

PrimitiveManager* PrimitiveManager::instance( )
{
	return pInstance_;
}

void PrimitiveManager::deleteManagedObjects( )
{
	BW_GUARD;
	PrimitiveMap::iterator it = primitives_.begin();
	PrimitiveMap::iterator end = primitives_.end();

	while( it != end )
	{
		it->second->release();
		++it;
	}
}

void PrimitiveManager::createManagedObjects( )
{
	BW_GUARD;
	PrimitiveMap::iterator it = primitives_.begin();
	PrimitiveMap::iterator end = primitives_.end();

	while( it != end )
	{
		it->second->load();
		++it;
	}
}


/**
 *	Get the given primitive resource
 */
PrimitivePtr PrimitiveManager::get( const std::string& resourceID )
{
	BW_GUARD;
	PrimitivePtr res = this->find( resourceID );
	if (res) return res;

	std::string s;

	int pos = resourceID.find_last_of( "." );
	if ( pos >= 0 )
		s = resourceID.substr( pos + 1, resourceID.length() - pos - 1 );

	MEM_TRACE_BEGIN( resourceID )

	res = new Primitive( resourceID );
	res->load();

	this->addInternal( &*res );
	MEM_TRACE_END()

	return res;
}

void PrimitiveManager::del( Primitive * pPrimitive )
{
	BW_GUARD;
	if (pInstance_)
		pInstance_->delInternal( pPrimitive );
}

/**
 *	Add this primitive to the map of those already loaded
 */
void PrimitiveManager::addInternal( Primitive * pPrimitive )
{
	BW_GUARD;
	SimpleMutexHolder smh( primitivesLock_ );

	primitives_.insert( std::make_pair(pPrimitive->resourceID(), pPrimitive ) );
}


/**
 *	Remove this primitive from the map of those already loaded
 */
void PrimitiveManager::delInternal( Primitive * pPrimitive )
{
	BW_GUARD;
	SimpleMutexHolder smh( primitivesLock_ );

	PrimitiveMap::iterator it = primitives_.find( pPrimitive->resourceID() );
	if (it != primitives_.end())
	{
		primitives_.erase( it );
		return;
	}
	else
	{
		for (it = primitives_.begin(); it != primitives_.end(); it++)
		{
			if (it->second == pPrimitive)
			{
				//DEBUG_MSG( "PrimitiveManager::del: %s\n",
				//	pPrimitive->resourceID().c_str() );
				primitives_.erase( it );
				return;
			}
		}
	}

	ERROR_MSG( "PrimitiveManager::del: "
		"Could not find primitive at 0x%08X to delete it\n", pPrimitive );
}


/**
 *	Helper class to find a primitive by name
 */
class FindPrimitive
{
public:
	std::string resourceID_;
	FindPrimitive( const std::string& resourceID )
	: resourceID_( resourceID )
	{
	}
	~FindPrimitive()
	{

	}

	bool operator () ( const Primitive * primitive )
	{
		return primitive->resourceID() == resourceID_;
	}

};


/**
 *	Find this primitive in the map of those already loaded
 */
PrimitivePtr PrimitiveManager::find( const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( primitivesLock_ );

	PrimitiveMap::iterator it = primitives_.find( resourceID );

	if (it != primitives_.end())
	{
		// try incrementing it, but only keep it if it wasn't zero previously
		// if it was zero, then someone is about to call the destructor, and may well
		// be waiting on the primitivesLock_ to remove it from this list.
		return PrimitivePtr( it->second, PrimitivePtr::FALLIBLE );
		// if it is NULL then just return NULL anyway
	}

	return NULL;
}

PrimitiveManager* PrimitiveManager::pInstance_ = NULL;

void PrimitiveManager::init()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( !pInstance_ )
	{
		return;
	}
	pInstance_ = new PrimitiveManager;
}

void PrimitiveManager::fini()
{
	BW_GUARD;
	if (pInstance_)
		delete pInstance_;
	pInstance_ = NULL;
}

}

// primitive_manager.cpp
