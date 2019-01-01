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
#include "dirty_chunk_list.hpp"


///////////////////////////////////////////////////////////////////////////////
//	Section: DirtyChunkList  (1 list)
///////////////////////////////////////////////////////////////////////////////

DirtyChunkList::DirtyChunkList( const std::string& type )
	: type_( type )
{
}


bool DirtyChunkList::isDirty( const std::string& chunkName ) const
{
	BW_GUARD;

	return chunks_.find( chunkName ) != chunks_.end();
}


void DirtyChunkList::dirty( Chunk* pChunk )
{
	BW_GUARD;

	chunks_[ pChunk->identifier() ] = pChunk;
}


void DirtyChunkList::clean( const std::string& chunkName )
{
	BW_GUARD;

	chunks_.erase( chunkName );
}


bool DirtyChunkList::any() const
{
	BW_GUARD;

	for (ChunkMap::const_iterator iter = chunks_.begin();
		iter != chunks_.end(); ++iter)
	{
		return true;
	}

	return false;
}


int DirtyChunkList::num() const
{
	return (int)chunks_.size();
}


DirtyChunkList::iterator DirtyChunkList::begin()
{
	return chunks_.begin();
}


DirtyChunkList::iterator DirtyChunkList::end()
{
	return chunks_.end();
}


DirtyChunkList::iterator DirtyChunkList::erase( iterator iter )
{
	return chunks_.erase( iter );
}


DirtyChunkList::const_iterator DirtyChunkList::begin() const
{
	return chunks_.begin();
}


DirtyChunkList::const_iterator DirtyChunkList::end() const
{
	return chunks_.end();
}


bool DirtyChunkList::empty() const
{
	return chunks_.empty();
}


void DirtyChunkList::clear()
{
	BW_GUARD;

	chunks_.clear();
}


///////////////////////////////////////////////////////////////////////////////
//	Section: DirtyChunkLists  (all the  lists)
///////////////////////////////////////////////////////////////////////////////

DirtyChunkList& DirtyChunkLists::operator[]( const std::string& type )
{
	BW_GUARD;

	DirtyLists::iterator iter = dirtyLists_.find( type );

	if (iter != dirtyLists_.end())
	{
		return iter->second;
	}

	return dirtyLists_.insert(
		DirtyLists::value_type( type, DirtyChunkList( type ) ) ).first->second;

}


const DirtyChunkList& DirtyChunkLists::operator[]( const std::string& type ) const
{
	BW_GUARD;

	DirtyLists::const_iterator iter = dirtyLists_.find( type );

	if (iter != dirtyLists_.end())
	{
		return iter->second;
	}

	static DirtyChunkList dummy( "dummy" );
	return dummy;
}


bool DirtyChunkLists::empty() const
{
	BW_GUARD;

	bool result = true;

	for (DirtyLists::const_iterator iter = dirtyLists_.begin();
		iter != dirtyLists_.end(); ++iter)
	{
		if (!iter->second.empty())
		{
			result = false;
			break;
		}
	}

	return result;
}


bool DirtyChunkLists::any() const
{
	BW_GUARD;

	bool result = false;

	for (DirtyLists::const_iterator iter = dirtyLists_.begin();
		iter != dirtyLists_.end(); ++iter)
	{
		if (iter->second.any())
		{
			result = true;
			break;
		}
	}

	return result;
}


bool DirtyChunkLists::isDirty( const std::string& chunkName ) const
{
	BW_GUARD;

	bool result = false;

	for (DirtyLists::const_iterator iter = dirtyLists_.begin();
		iter != dirtyLists_.end(); ++iter)
	{
		if (iter->second.isDirty( chunkName ))
		{
			result = true;
			break;
		}
	}

	return result;
}


void DirtyChunkLists::clean( const std::string& chunkName )
{
	BW_GUARD;

	for (DirtyLists::iterator iter = dirtyLists_.begin();
		iter != dirtyLists_.end(); ++iter)
	{
		iter->second.clean( chunkName );
	}
}


void DirtyChunkLists::clear()
{
	BW_GUARD;

	dirtyLists_.clear();
}
