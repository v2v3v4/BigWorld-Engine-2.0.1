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
#include "memory_trace.hpp"

#ifdef USE_MEMORY_TRACER

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "ROMP", 2 )

extern uint32 memUsed();

ResourceMemoryTrace	ResourceMemoryTrace::s_instance_;

ResourceMemoryTrace::ResourceMemoryTrace()
{
	context_.push( new RMTLeaf() );
}


void ResourceMemoryTrace::begin( const std::string& name )
{
	RMTLeaf* top = context_.top();
	RMTLeaf* entry = new RMTLeaf();

	if ( !top->map_ )
	{
		top->map_ = new std::vector< RMTLeaf::Entry >;
	}

	top->map_->push_back( std::make_pair( name, entry ) );
	context_.push( entry );
	entry->startKB_ = memUsed();
}


void ResourceMemoryTrace::end()
{
	RMTLeaf* top = context_.top();
	top->endKB_ = memUsed();
	context_.pop();
}


bool ResourceMemoryTrace::RMTLeaf::compare( const Entry& one, const Entry& two )
{
	return ( one.second->endKB_-one.second->startKB_ ) < 
			( two.second->endKB_-two.second->startKB_ );
}


void ResourceMemoryTrace::traverse( TreeWalker& w, uint32 currDepth )
{
	if ( currDepth == 0 )
	{
		if ( context_.size() > 1 )
		{
			ERROR_MSG( "ResourceMemoryTrace Error - stack size %d\n", context_.size() );
			while ( context_.size() > 1 )
			{
				RMTLeaf* top = context_.top();
				context_.pop();
				RMTLeaf* current = context_.top();
				std::vector< RMTLeaf::Entry >::iterator it = current->map_->begin();
				std::vector< RMTLeaf::Entry >::iterator end = current->map_->end();
				while ( it != end )
				{
					RMTLeaf::Entry& entry = *it;
					if ( entry.second == top )
					{
						const std::string& str = entry.first;
						ERROR_MSG( "ResourceMemoryTrace - error from ... %s\n", str.c_str() );
					}
					it++;
				}
			}
		}
	}

	RMTLeaf* top = context_.top();

	if ( !top->map_ )
		return;

	std::sort( top->map_->begin(), top->map_->end(), RMTLeaf::compare );
	
	std::vector< RMTLeaf::Entry >::reverse_iterator rit = top->map_->rbegin();
	std::vector< RMTLeaf::Entry >::reverse_iterator rend = top->map_->rend();

	while ( rit != rend )
	{
		RMTLeaf::Entry& entry = *rit;
		RMTLeaf& leaf = *entry.second;

		const std::string& str = entry.first;

		if ( w.atLeaf( entry.first, uint32(std::max(leaf.endKB_ - leaf.startKB_,0L)), currDepth ) && leaf.map_ )
		{
			context_.push(&leaf);
			this->traverse( w, currDepth + 1 );
			context_.pop();
		}

		rit++;
	}
}

#else

namespace
{
    int noLinkWarning   = 0;
}

#endif
