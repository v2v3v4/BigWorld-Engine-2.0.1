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

#include "chunk_item.hpp"
#include "chunk.hpp"
#include "chunk_space.hpp"

#include "cstdmf/guard.hpp"

#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#include <umbraobject.hpp>
#endif

uint32	ChunkItemBase::s_instanceCount_		= 0;
uint32	ChunkItemBase::s_instanceCountPeak_	= 0;

#ifndef _RELEASE

/**
 *	Constructor
 */
ChunkItemBase::ChunkItemBase( WantFlags wantFlags ) :
	__count( 0 ), 
	drawMark_(0),
	wantFlags_( wantFlags ), 
	pChunk_( NULL )
#if UMBRA_ENABLE
	,
	pUmbraDrawItem_( NULL )
#endif
{
	s_instanceCount_++;
	if ( s_instanceCount_ > s_instanceCountPeak_ )
		s_instanceCountPeak_ = s_instanceCount_;
}

/**
 *	Copy constructor
 */
ChunkItemBase::ChunkItemBase( const ChunkItemBase & oth ) :
	__count( 0 ), 
	drawMark_(0),
	wantFlags_( oth.wantFlags_ ), 
	pChunk_( oth.pChunk_ )
#if UMBRA_ENABLE
	,
	pUmbraDrawItem_( NULL )
#endif
{
	s_instanceCount_++;
	if ( s_instanceCount_ > s_instanceCountPeak_ )
		s_instanceCountPeak_ = s_instanceCount_;
}

#endif // !_RELEASE

PROFILER_DECLARE( ChunkItemBase_destruct, "ChunkItemBase_destruct" );
/**
 *	Destructor
 */
ChunkItemBase::~ChunkItemBase()
{
	// Note, we explicitly dereference pointers so ensuing destruction can be
	// profiled.
	PROFILER_SCOPED( ChunkItemBase_destruct );

#if UMBRA_ENABLE
	// Delete any umbra draw item
	delete pUmbraDrawItem_;
#endif

	s_instanceCount_--;
}


/**
 *	Utility method to implement 'lend' given a (world space) bounding box
 */
void ChunkItemBase::lendByBoundingBox( Chunk * pLender,
	const BoundingBox & worldbb )
{
	BW_GUARD;
	int allInOwnChunk = pChunk_->isOutsideChunk() ? 0 : -1;
	// assume it's not all within its own chunk if the item
	// is in an outside chunk (i.e. if bb test passes then that's
	// good enough to loan for us)

	// go through every bound portal
	Chunk::piterator pend = pLender->pend();
	for (Chunk::piterator pit = pLender->pbegin(); pit != pend; pit++)
	{
		if (!pit->hasChunk()) continue;

		Chunk * pConsider = pit->pChunk;

		// if it's not in that chunk's bounding box
		// then it definitely doesn't want it
		if (!worldbb.intersects( pConsider->boundingBox() )) continue;

		// if that's an outside chunk and the item is completely
		// within its own chunk then it also doesn't want it
		if (pConsider->isOutsideChunk())
		{
			// don't bother checking this for inside chunks since they're
			// not allowed to have interior chunks (i.e. bb is good enough)
			if (allInOwnChunk < 0)
			{
				// should really check if it's not completely within the union
				// of all interior chunks, but checking just its own is an OK
				// approximation ... if we had the hull tree at this stage then
				// we could do a different test using findChunkFromPoint, but we
				// don't, and it would miss some cases too, so this will do
				Vector3 bbpts[2] = { worldbb.minBounds(), worldbb.maxBounds() };
				Vector3 tpoint;	// this simple algorithm obviously only works
				int i;			// if our own chunk has no interior chunks
				for (i = 0; i < 8; i++)
				{
					tpoint.x = bbpts[(i>>0)&1].x;
					tpoint.y = bbpts[(i>>1)&1].y;
					tpoint.z = bbpts[(i>>2)&1].z;
					if (!pChunk_->contains( tpoint )) break;
				}
				allInOwnChunk = (i == 8);
				// if we are all in our own chunk (and we are in an inside
				// chunk, which is the only way we get here), then we can't
				// be in this chunk too... and furthermore we can't be any
				// any other chunks at all, so we can just stop here
				if (allInOwnChunk) break;
			}
			// if it's all within its own chunk then it can't be in this one
			//if (allInOwnChunk) continue;
			// ... but since we only calculate allInOwnChunk if our chunk is
			// an inside chunk, and if it were true we would have stopped
			// the loop already, then there's no point checking it again here,
			// in fact allInOwnChunk can only be zero here.
			MF_ASSERT_DEV( !allInOwnChunk );
			// could make the code a bit neater but this way is more logical
		}

		// ok so that chunk does really want this item then
		if (pConsider->addLoanItem( static_cast<ChunkItem*>(this) ))
		{
			pConsider->updateBoundingBoxes( static_cast<ChunkItem*>( this ) );
		}
	}
}


/**
 *	Constructor. Registers with chunk's static factory registry
 */
ChunkItemFactory::ChunkItemFactory(
		const std::string & section,
		int priority,
		Creator creator ) :
	priority_( priority ),
	creator_( creator )
{
	BW_GUARD;
	Chunk::registerFactory( section, *this );
}


/**
 *	This virtual method calls the creator function that was passed in,
 *	as long as it's not NULL. It is called by a Chunk when it encounters
 *	the section name.
 *
 *	@return true if succeeded
 */
ChunkItemFactory::Result ChunkItemFactory::create( Chunk * pChunk,
	DataSectionPtr pSection ) const
{
	BW_GUARD;
	if (creator_ == NULL)
	{	
		std::string errorStr = "No item factory found for section ";
		if ( pSection )
		{
			errorStr += "'" + pSection->sectionName() + "'";
		}
		else
		{
			errorStr += "<unknown>";
		}
		return ChunkItemFactory::Result( NULL, errorStr );
	}
	return (*creator_)( pChunk, pSection );
}

#ifndef MF_SERVER
/**
 *	This method is the specialised toss method for ChunkItem
 *	It makes sure the item is added to the spaces tick list
 *	if it wants to be ticked
 *	@param pChunk the chunk to toss the item into
 */
void ChunkItem::toss(Chunk* pChunk)	
{ 
	// If this item wants tick make sure it is in the appropriate
	// spaces tick list
	if (this->wantsTick())
	{
		// Grab the current space
		ChunkSpace* pCurrentSpace = NULL;
		if (pChunk_)
		{
			pCurrentSpace = pChunk_->space();
		}

		ChunkSpace* pNewSpace = NULL;
		if (pChunk)
		{
			pNewSpace = pChunk->space();
		}

		// If the spaces are different remove the item from the
		// current space tick list and insert it into the new space
		if (pCurrentSpace != pNewSpace)
		{
			if (pCurrentSpace)
			{
				pCurrentSpace->delTickItem( this );
			}
			if (pNewSpace)
			{
				pNewSpace->addTickItem( this );
			}
		}
	}

#if UMBRA_ENABLE
	if (pUmbraDrawItem_ != NULL)
	{
		Umbra::Cell* pCell = NULL;
		if (pChunk)
		{
			pCell = pChunk->getUmbraCell();
		}
		pUmbraDrawItem_->pUmbraObject()->object()->setCell(pCell);
	}
#endif

	// Call the base class toss
	this->SpecialChunkItem::toss(pChunk);
}
#endif // MF_SERVER

/**
 *	This method adds a chunk as a borrower of this item
 *	what this means is that the ChunkItem overlaps this chunk
 *	as well, which means we need to render as part of this chunk
 *	if it exists in a different umbra cell to our own cell
 *	@param pChunk the chunk that is borrowing us
 */
void ChunkItemBase::addBorrower( Chunk* pChunk )
{
	borrowers_.insert( pChunk );

#if UMBRA_ENABLE
	if (pUmbraDrawItem_)
	{
		Umbra::Cell* pBorrowerCell = pChunk->getUmbraCell();
		if (pBorrowerCell != pChunk_->getUmbraCell())
		{
			
			UmbraLenders::iterator lit = umbraLenders_.find( pBorrowerCell );
			if (lit == umbraLenders_.end())
			{
				umbraLenders_[pBorrowerCell] = createLender( pBorrowerCell );
			}
		}
	}
#endif // UMBRA_ENABLE
}


/**
 *	This method removes a chunk as a borrower of this item
 *	@param pChunk the chunk that no longer wants to borrow us
 *	@see ChunkItemBase::addBorrower
 */
void ChunkItemBase::delBorrower( Chunk* pChunk )
{
	BW_GUARD;
	// If the chunk is one of our borrowers, continue
	Borrowers::iterator bit = borrowers_.find( pChunk );
	if (bit != borrowers_.end())
	{
		borrowers_.erase( bit );

#if UMBRA_ENABLE
		// Iterate over all our borrowers to see if any of them are in the same
		// cell as the one we just removed
		for (bit = borrowers_.begin(); bit != borrowers_.end(); bit++)
		{
			if ((*bit)->getUmbraCell() == pChunk->getUmbraCell())
			{
				break;
			}
		}

		// If we didn't reach the end of the list, another borrower chunk exists
		// in the same cell as the one we just removed and we keep the lender
		// object for this chunk
		if (bit == borrowers_.end())
		{
			UmbraLenders::iterator lit = umbraLenders_.find( pChunk->getUmbraCell() );
			if (lit != umbraLenders_.end())
				umbraLenders_.erase( lit );
		}
#endif // UMBRA_ENABLE
	}
}


/**
 *	This method removes a chunk as a borrower of this item
 *	@param pChunk the chunk that no longer wants to borrow us
 *	@see ChunkItemBase::addBorrower
 */
void ChunkItemBase::clearBorrowers()
{
	BW_GUARD;

	borrowers_.clear();

#if UMBRA_ENABLE
	umbraLenders_.clear();
#endif // UMBRA_ENABLE
}


#if UMBRA_ENABLE
/**
 *	This method creates a lender for this chunk
 *	@param pCell the umbra cell to lend the chunk item to
 *	@return the umbra object proxy for the lender
 */
UmbraObjectProxyPtr ChunkItemBase::createLender(Umbra::Cell* pCell)
{
	UmbraObjectProxyPtr pLenderObject;

	Matrix itemTransform;

	UmbraObjectProxyPtr pChunkObject = pUmbraDrawItem_->pUmbraObject();
	pChunkObject->object()->getObjectToCellMatrix((Umbra::Matrix4x4&)itemTransform);

	// Set up the umbra object
	pLenderObject = UmbraObjectProxy::get( pChunkObject->pModelProxy() );
	pLenderObject->object()->setUserPointer( pChunkObject->object()->getUserPointer() );
	pLenderObject->object()->setCell( pCell );
	pLenderObject->object()->setObjectToCellMatrix( (Umbra::Matrix4x4&)itemTransform );

	return pLenderObject;
}

/**
 *	This method updates all our umbra lender objects, this needs to be done
 *	if the Umbra draw item is recreated
 */
void ChunkItemBase::updateUmbraLenders()
{
	// Take a copy of all our borrowers
	Borrowers oldBorrowers = borrowers_;

	// Clear all the borrowers and lenders
	borrowers_.clear();
	umbraLenders_.clear();

	// Re-add all the borrowers to recreate the lenders
	Borrowers::iterator bit = oldBorrowers.begin();
	for (; bit != oldBorrowers.end(); bit++)
	{
		this->addBorrower( *bit );
	}
}
#endif // UMBRA_ENABLE


// chunk_item.cpp
