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

#include "base_chunk_space.hpp"

#include "chunk.hpp"
#include "chunk_obstacle.hpp"
#include "chunk_space.hpp"
#include "geometry_mapping.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "math/boundbox.hpp"
#include "physics2/hulltree.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )
PROFILER_DECLARE( BaseChunkSpace_Column_delDynamicObstacle, "BaseChunkSpace Column delDynamicObstacle" );
PROFILER_DECLARE( BaseChunkSpace_Column_shutIfSeen, "BaseChunkSpace Column shutIfSeen" );
PROFILER_DECLARE( BaseChunkSpace_removeFromBlurred, "BaseChunkSpace removeFromBlurred" );

// -----------------------------------------------------------------------------
// Section: HullChunk
// -----------------------------------------------------------------------------

/**
 *	This class is the element that is put into hull trees to
 *	identify a chunk.
 */
class HullChunk : public HullContents
{
public:
	HullChunk( Chunk * pChunk ) : pChunk_( pChunk ) {}

	~HullChunk() { pChunk_->smudge(); }

	Chunk	 * pChunk_;
};


// -----------------------------------------------------------------------------
// Section: Static data
// -----------------------------------------------------------------------------

int BaseChunkSpace::s_obstacleTreeDepth = 5;

// -----------------------------------------------------------------------------
// Section: BaseChunkSpace
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
BaseChunkSpace::BaseChunkSpace( ChunkSpaceID id ) :
	id_( id ),
	minGridX_( 0 ),
	maxGridX_( 0 ),
	minGridY_( 0 ),
	maxGridY_( 0 )
{
}


/**
 *	Destructor.
 */
BaseChunkSpace::~BaseChunkSpace()
{
	BW_GUARD;
	fini();
}


/**
 *	This method must clears the current chunks from the space.
 */
void BaseChunkSpace::fini()
{
	BW_GUARD;
	// This looks a little odd, so needs some explaination - We get the last
	// chunk in the currentChunk's first vector and delete it.  This will in
	// turn call delChunk which removes the chunk from the vector, and if the 
	// vector is empty gets rid of the vector from currentChunks_.
	while (!currentChunks_.empty())
	{
		ChunkMap::iterator it = currentChunks_.begin();
		delete it->second.back();
	}
	currentChunks_.clear();
	MF_ASSERT_DEV( currentChunks_.empty() );
}


/**
 *	This method adds an (unloaded) chunk to our collection.
 *
 *	Run in main thread.
 */
void BaseChunkSpace::addChunk( Chunk * pChunk )
{
	BW_GUARD;
	currentChunks_[ pChunk->identifier() ].push_back( pChunk );
	if ( pChunk->isOutsideChunk() )
	{
		std::pair< int, int > coord( pChunk->x(), pChunk->z() );
		gridChunks_[coord].push_back( pChunk );
	}

	pChunk->appointAsAuthoritative();
}


/**
 *	This method finds the chunk with the same identifier as the input chunk. If
 *	it can't be found, it adds the chunk to our collection, otherwise, it
 *	deletes the input chunk and returns the found one.
 *
 *	Run in main thread.
 */
Chunk * BaseChunkSpace::findOrAddChunk( Chunk * pChunk )
{
	BW_GUARD;
	ChunkMap::iterator found =
		currentChunks_.find( pChunk->identifier() );
	if (found != currentChunks_.end())
	{
		for (uint i = 0; i < found->second.size(); i++)
		{
			if (pChunk->mapping() == found->second[i]->mapping())
			{
				if (pChunk != found->second[i])
					delete pChunk;
				return found->second[i];
			}
		}
	}

	this->addChunk( pChunk );
	return pChunk;
}


/**
 *	Remove an unloaded chunk. Use only when it is no longer being referenced.
 *
 *	Run in main thread.
 */
void BaseChunkSpace::delChunk( Chunk * pChunk )
{
	BW_GUARD;
	if ( pChunk->isOutsideChunk() )
	{
		std::pair< int, int > coord( pChunk->x(), pChunk->z() );
		std::vector< Chunk* > & mappings = gridChunks_[coord];

		for (uint i = 0; i < mappings.size(); i++)
		{
			if ( mappings[i] == pChunk )
			{
				mappings.erase( mappings.begin() + i );
				break;
			}
		}
		if (mappings.empty())
			gridChunks_.erase( coord );
	}

	// simply remove it from our stringmap
	ChunkMap::iterator found = currentChunks_.find( pChunk->identifier() );
	if (found != currentChunks_.end())
	{
		for (uint i = 0; i < found->second.size(); i++)
		{
			if (found->second[i] == pChunk)
			{
				found->second.erase( found->second.begin() + i );
				break;
			}
		}
		if (found->second.empty())
			currentChunks_.erase( found );
	}
}


/**
 *	This method clears out the chunk space of all its chunks and any other
 *	data that it may have accumulated.
 */
void BaseChunkSpace::clear()
{
	BW_GUARD;
	/*
	while (!currentChunks_.empty())
	{
		delete (currentChunks_.end() - 1)->second.back();
	}
	currentChunks_.clear();
	*/


	// all chunks should have been destroyed in ChunkSpace's clear method
	// when their mappings were deleted ...
	//MF_ASSERT_DEV( currentChunks_.empty() );

	// ... except for loading ones
#if 0
	// TODO: Figure out why the assert below triggers, and fix it!
	for (ChunkMap::iterator it = currentChunks_.begin();
		it != currentChunks_.end();
		it++)
	{
		for (uint i = 0; i < it->second.size(); i++)
		{
			MF_ASSERT_DEV( it->second[i]->loading() );
		}
	}
#endif

	this->clearSpaceData();
}


/**
 *	This method clears out the space data associated with this space.
 */
void BaseChunkSpace::clearSpaceData()
{
	// all we have to do is clear out our saved data entries
	dataEntries_.clear();
}

/**
 *	This method returns whether or not the given grid co-ordinate is within the
 *	gross bounds of this space.
 */
bool BaseChunkSpace::inBounds( int gridX, int gridY ) const
{
	return minGridX_ <= gridX && gridX <= maxGridX_ &&
			minGridY_ <= gridY && gridY <= maxGridY_;
}


float g_gridResolution = GRID_RESOLUTION;

// -----------------------------------------------------------------------------
// Section: BaseChunkSpace collision related
// -----------------------------------------------------------------------------

// const float ObTreeDesc::RANGE = GRID_RESOLUTION;

QTRange calculateQTRange( const ChunkObstacle & input, const Vector2 & origin,
						 float range, int depth )
{
	const float WIDTH = float(1 << depth);

	BoundingBox bb = input.bb_;
	bb.transformBy( input.transform_ );

	QTRange rv;

	rv.left_	= int(WIDTH * (bb.minBounds().x - origin.x)/range);
	rv.right_	= int(WIDTH * (bb.maxBounds().x - origin.x)/range);
	rv.bottom_	= int(WIDTH * (bb.minBounds().z - origin.y)/range);
	rv.top_		= int(WIDTH * (bb.maxBounds().z - origin.y)/range);

#if 0
	// TODO: For large objects, we could scale them up a bit so that they don't
	// create as many leaf nodes. This saves memory.
	int width = (rv.right_ - rv.left_);
	if (width > 8)
	{
		width >>= 3;

		uint32 mask = ~1;
		while (width)
		{
			mask <<= 1;
			width >>= 1;
		}

		int oldLeft = rv.left_;
		int oldRight = rv.right_;

		rv.left_ &= mask;
		rv.right_ |= ~mask;
	}

	width = (rv.top_ - rv.bottom_);
	if (width > 8)
	{
		width >>= 3;

		uint32 mask = ~1;
		while (width)
		{
			mask <<= 1;
			width >>= 1;
		}

		int oldBottom = rv.bottom_;
		int oldTop = rv.top_;

		rv.bottom_ &= mask;
		rv.top_ |= ~mask;
	}
#endif

	return rv;
}

bool containsTestPoint( const ChunkObstacle & /*input*/,
		const Vector3 & /*point*/ )
{
	// TODO: Implement this.
	return true;
}


// -----------------------------------------------------------------------------
// Section: BaseChunkSpace::Column
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
BaseChunkSpace::Column::Column( int x, int z ) :
	pChunkTree_( NULL ),
	pObstacleTree_( new ObstacleTree( gridToPoint( x ), gridToPoint( z ),
			BaseChunkSpace::obstacleTreeDepth(),
			GRID_RESOLUTION ) ),
	heldObstacles_(),
	pOutsideChunk_( NULL ),
	stale_( false ),
	shutTo_( NULL )
{
}


/**
 *	Destructor.
 */
BaseChunkSpace::Column::~Column()
{
	BW_GUARD;
	for (HeldObstacles::iterator iter = heldObstacles_.begin();
		iter != heldObstacles_.end();
		iter++)
	{
		const ChunkObstacle * pObstacle = *iter;

		if (pObstacle->pChunk() != NULL)
		{
			pObstacle->pChunk()->smudge();
		}
		pObstacle->decRef();
	}

	// Could make this a member
	delete pObstacleTree_;
	delete pChunkTree_;

	for (uint i = 0; i < holdings_.size(); i++)
	{
		delete holdings_[i];
	}
}


/**
 *
 */
void BaseChunkSpace::Column::addChunk( HullBorder & border, Chunk * pChunk )
{
	BW_GUARD;
	// see if this column has already seen this chunk
	if (shutTo_ == pChunk) return;

	HullContents * stuff = new HullChunk( pChunk );

	if (pChunk->isOutsideChunk())
	{
		if (pOutsideChunk_ != NULL)
		{
			ERROR_MSG( "Column::addChunk: Replacing outside chunk %s with %s!\n",
				pOutsideChunk_->identifier().c_str(), pChunk->identifier().c_str() );

			// take whichever has the most static chunk items
			// TODO: Allow multiple outside chunk items!
			if (pChunk->sizeStaticItems() <= pOutsideChunk_->sizeStaticItems())
				pChunk = pOutsideChunk_;
		}
		pOutsideChunk_ = pChunk;
	}
	else
	{
		if (pChunkTree_ == NULL) pChunkTree_ = new HullTree();
		pChunkTree_->add( border, stuff );

		// if this is an inside chunk being added to a column with an
		// outside chunk already, shake up the outside chunk and see
		// if its foreign items now belong in this newly added chunk.
		if (pOutsideChunk_ != NULL)
		{
			pOutsideChunk_->jogForeignItems();
		}
	}

	holdings_.push_back( stuff );

	// note: we don't have to worry about replacing outside chunks,
	// because we still take a holding in it, which is released when
	// we are destroyed like all the others are.
}


/**
 *	This method adds a hull to the obstacle tree of this column.
 *
 *	Note that the same obstacle should share the same ChunkSpace::Obstacle
 *	object if it is added to multiple columns, so that the collision
 *	routines can tell they're the same and not test them twice.
 *
 *	Also note that the hull passed in here should already be in world
 *	co-ordinates - this method will not apply the transform in the
 *	obstacle object to it.
 */
void BaseChunkSpace::Column::addObstacle( const ChunkObstacle & obstacle )
{
	BW_GUARD;
	// see if this column has already seen this chunk's obstacles
	if (shutTo_ == obstacle.pChunk()) return;

	obstacle.incRef();
	heldObstacles_.push_back( &obstacle );
	pObstacleTree_->add( obstacle );
}

/**
 *	This method adds a dynamic obstacle to our obstacle tree
 */
void BaseChunkSpace::Column::addDynamicObstacle( const ChunkObstacle & obstacle)
{
	BW_GUARD;
	// if we're refocussing this chunk then ignore it
	if (shutTo_ == obstacle.pChunk()) return;

	// keep a pointer to it
	obstacle.incRef();
	heldObstacles_.push_back( &obstacle );

	// just add it to the root node of our obstacle tree
	ObstacleTree::Node::Elements & elements = const_cast<
		ObstacleTree::Node::Elements&>( pObstacleTree_->pRoot()->elements() );
	elements.push_back( &obstacle );
}

/**
 *	This method removes an existing dynamic obstacle from our obstacle tree
 */
void BaseChunkSpace::Column::delDynamicObstacle( const ChunkObstacle & obstacle)
{
	BW_GUARD_PROFILER( BaseChunkSpace_Column_delDynamicObstacle );
	// find it in our held obstacles (should be near the end)
	HeldObstacles::reverse_iterator iter = std::find(
		heldObstacles_.rbegin(), heldObstacles_.rend(), &obstacle );
	if (iter == heldObstacles_.rend()) return;

	// remove it
	*iter = heldObstacles_.back();
	heldObstacles_.pop_back();

	// now find it in the root node of our obstacle tree
	ObstacleTree::Node::Elements & elements = const_cast<
		ObstacleTree::Node::Elements&>( pObstacleTree_->pRoot()->elements() );
	ObstacleTree::Node::Elements::reverse_iterator found2 = std::find(
		elements.rbegin(), elements.rend(), &obstacle );

	IF_NOT_MF_ASSERT_DEV( found2 != elements.rend() )
	{
		return;
	}

	// and remove it from there too
	*found2 = elements.back();
	elements.pop_back();

	// decrement the reference count and we are done!
	obstacle.decRef();
}


/**
 *	This method finds a chunk at the given point in this column, excluding
 *	the given chunk.
 */
Chunk * BaseChunkSpace::Column::findChunkExcluding( const Vector3 & point,
	Chunk * pNot )
{
	BW_GUARD;
	Chunk * pChunk = (pOutsideChunk_ != pNot) ? pOutsideChunk_ : NULL;

	if (pChunkTree_ != NULL)
	{
		const HullChunk * result = static_cast< const HullChunk * >(
			pChunkTree_->testPoint( point ) );

		while (result != NULL)
		{
			if( (pChunk == NULL || pChunk == pOutsideChunk_ ||
				pChunk->volume() > result->pChunk_->volume() ) &&
				result->pChunk_->boundingBox().intersects( point ) )
			{
				if (result->pChunk_ != pNot) pChunk = result->pChunk_;
			}

			result = static_cast<const HullChunk*>( result->pNext_ );
		}
	}

	return pChunk;
}


/**
 *	This method finds a chunk at the given point in this column.
 */
Chunk * BaseChunkSpace::Column::findChunk( const Vector3 & point )
{
	BW_GUARD;
	Chunk * pChunk = pOutsideChunk_;

	if (pChunkTree_ != NULL)
	{
		const HullChunk * result = static_cast< const HullChunk * >(
			pChunkTree_->testPoint( point ) );

		while (result != NULL)
		{
			if (((pChunk == pOutsideChunk_) ||
						(pChunk->volume() > result->pChunk_->volume())) &&
					result->pChunk_->boundingBox().intersects( point ))
			{
				pChunk = result->pChunk_;
			}

			result = static_cast<const HullChunk*>( result->pNext_ );
		}
	}

	return pChunk;
}


/**
 *	This method returns the size of this column and its hull trees.
 */
long BaseChunkSpace::Column::size() const
{
	long sz = sizeof( Column );
	if (pChunkTree_ != NULL)
	{
		sz += pChunkTree_->size( 0, false );
	}

	sz += this->obstacles().size();

	sz += holdings_.capacity() * sizeof( HullContents * );
//	sz += seen_.size() * sizeof( Chunk * );

	// add in an estimate of obstacle size
	sz += holdings_.size() * sizeof( HullChunk );

	sz += heldObstacles_.capacity() * sizeof( ChunkObstacle * );

	return sz;
}




/**
 *	Open the column up and record the given chunk as having been seen
 */
void BaseChunkSpace::Column::openAndSee( Chunk * pChunk )
{
	// only add it if we weren't already shut to it;
	//  if we were shut to it then we must have seen it already
	if (shutTo_ != pChunk)
	{
		seen_.push_back( pChunk );
	}
	shutTo_ = NULL;
}


/**
 *	Shut the column to the given chunk, if it's been seen
 */
void BaseChunkSpace::Column::shutIfSeen( Chunk * pChunk )
{	
	BW_GUARD_PROFILER( BaseChunkSpace_Column_shutIfSeen );
	if (std::find( seen_.begin(), seen_.end(), pChunk ) != seen_.end())
	{
		shutTo_ = pChunk;
	}
	else
	{
		shutTo_ = NULL;
	}
}




/**
 *	This method looks up a chunk in our map by its identifier.
 *
 *	Only run in main thread.
 */
Chunk * BaseChunkSpace::findChunk( const std::string & identifier,
	const std::string & mappingName )
{
	BW_GUARD;
	// MF_ASSERT( MainThreadTracker::isCurrentThreadMain() );
	ChunkMap::iterator found = currentChunks_.find( identifier );

	if (found == currentChunks_.end())
	{
		return NULL;
	}

	for (uint i = 0; i < found->second.size(); i++)
	{
		if (mappingName.empty() ||
				found->second[i]->mapping()->name() == mappingName)
		{
			return found->second[i];
		}
	}
	return NULL;
}


/**
 *	Modify the given data entry in this chunk space.
 *
 *	If the instruction is to revoke a previous entry, then return the
 *	key that it used to have (or -1 if not found)
 *	If the instruction is to add a new entry, then return the key passed in,
 *	unless that entry id already exists, in which case return -1.
 *	The short version: -1 = bad :)
 */
uint16 BaseChunkSpace::dataEntry( const SpaceEntryID & entryID, uint16 key,
	const std::string & data )
{
	BW_GUARD;
	// see if it's being added
	if (key != uint16(-1))
	{
		// make sure it is novel
		DataEntryMapKey demk;
		demk.entryID = entryID;
		demk.key = key;
		if (dataEntries_.find( demk ) != dataEntries_.end())
			return uint16(-1);

		// ok it's novel, so add it
		dataEntries_.insert( std::make_pair( demk, data ) );
		return key;
	}
	// ok it's being revoked then
	else
	{
		// make sure we have it (not best way to find I know)
		DataEntryMap::iterator it;
		for (it = dataEntries_.begin(); it != dataEntries_.end(); it++)
		{
			if (it->first.entryID == entryID) break;
		}
		if (it == dataEntries_.end())
			return uint16(-1);

		// ok we have it, so remove it
		key = it->first.key;
		dataEntries_.erase( it );
		return key;
	}
}

/**
 *	Retrieve the given exact entry.
 *	key may be -1 and is just a hint.
 *	Return NULL if not found.
 */
BaseChunkSpace::DataValueReturn BaseChunkSpace::dataRetrieveSpecific(
	const SpaceEntryID & entryID, uint16 key )
{
	BW_GUARD;
	DataEntryMap::iterator it;
	if (key == uint16(-1))
	{
		for (it = dataEntries_.begin(); it != dataEntries_.end(); it++)
		{
			if (it->first.entryID == entryID) break;
		}
		if (it != dataEntries_.end())
			return DataValueReturn( it->first.key, &it->second );
		else
			return std::make_pair( uint16(-1), (const std::string*)NULL );
	}
	else
	{
		DataEntryMapKey demk;
		demk.entryID = entryID;
		demk.key = key;
		it = dataEntries_.find( demk );
		if (it != dataEntries_.end())
			return DataValueReturn( it->first.key, &it->second );
		else
			return DataValueReturn( uint16(-1), (const std::string*)NULL );
	}
}

/**
 *	Retrieve the first data entry for that key in this chunk space.
 */
const std::string * BaseChunkSpace::dataRetrieveFirst( uint16 key )
{
	BW_GUARD;
	DataEntryMap::const_iterator found = this->dataRetrieve( key );

	return (found != dataEntries_.end()) ? &found->second : NULL;
}

/**
 *	Retrieve the first data entry of the given key in this chunk space,
 *	in the form of an iterator which can be used to find them all.
 */
BaseChunkSpace::DataEntryMap::const_iterator BaseChunkSpace::dataRetrieve(
	uint16 key )
{
	BW_GUARD;
	DataEntryMapKey demk;
	demk.entryID.ip = 0;
	demk.entryID.port = 0;
	demk.entryID.salt = 0;
	demk.key = key;

	DataEntryMap::const_iterator found = dataEntries_.lower_bound( demk );
	if (found != dataEntries_.end() && found->first.key == key)
		return found;
	else
		return dataEntries_.end();
}


/**
 *	Stub mappingSettings definition
 */
void BaseChunkSpace::mappingSettings( DataSectionPtr /*pSS*/ )
{
}


/**
 *	This method adds the a chunk to this list of chunks that are
 *	not currently in focus.
 */
void BaseChunkSpace::blurredChunk( Chunk * pChunk )
{
	BW_GUARD;
	// but only if it's bound
	if (pChunk->isBound())
	{
		blurred_.push_back( pChunk );
	}
}


/**
 * returns true if in list and removed, else return false.
 */
bool BaseChunkSpace::removeFromBlurred( Chunk * pChunk )
{	
	BW_GUARD_PROFILER( BaseChunkSpace_removeFromBlurred );
	std::vector<Chunk*>::iterator found =
		std::find( blurred_.begin(), blurred_.end(), pChunk );

	if ( found != blurred_.end() )
	{
		blurred_.erase( found );
		return true;
	}

	return false;
}

// base_chunk_space.cpp
