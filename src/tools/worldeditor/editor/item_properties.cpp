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
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_item.hpp"
#include "gizmo/undoredo.hpp"
#include "appmgr/commentary.hpp"
#include "common/editor_views.hpp"


// -----------------------------------------------------------------------------
// Section: ChunkItemMatrixOperation
// -----------------------------------------------------------------------------

class ChunkItemMatrixOperation : public UndoRedo::Operation, public Aligned
{
public:
	ChunkItemMatrixOperation( ChunkItemPtr pItem, Chunk * oldChunk,
			const Matrix & oldPose ) :
		UndoRedo::Operation( int(typeid(ChunkItemMatrixOperation).name()) ),
		pItem_( pItem ),
		oldChunk_( oldChunk ),
		oldPose_( oldPose )
	{
		BW_GUARD;

		addChunk( oldChunk );
		addChunk( pItem_->chunk() );
	}

private:

	virtual void undo()
	{
		BW_GUARD;

		// first add the current state of this block to the undo/redo list
		UndoRedo::instance().add( new ChunkItemMatrixOperation(
			pItem_, pItem_->chunk(), pItem_->edTransform() ) );

		if (pItem_->chunk()) //safety check for vlo references
		{
			// fix up the chunk if necessary
			if ((oldChunk_ != pItem_->chunk()))
			{
				pItem_->chunk()->delStaticItem( pItem_ );
				oldChunk_->addStaticItem( pItem_ );
			}

			// now change the matrix back
			pItem_->edTransform( oldPose_ );
		}
	}

	virtual bool iseq( const UndoRedo::Operation & oth ) const
	{
		BW_GUARD;

		return pItem_ ==
			static_cast<const ChunkItemMatrixOperation&>( oth ).pItem_;
	}

	ChunkItemPtr	pItem_;
	Chunk *			oldChunk_;
	Matrix			oldPose_;
};




// -----------------------------------------------------------------------------
// Section: ChunkItemMatrix
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
ChunkItemMatrix::ChunkItemMatrix( ChunkItemPtr pItem ) :
	pItem_( pItem ),
	origChunk_( NULL ),
	origPose_( Matrix::identity ),
	warned_( false ),
	haveRecorded_( false )
{
	BW_GUARD;

	movementSnaps_ = pItem_->edMovementDeltaSnaps();
}

/**
 *	Destructor
 */
ChunkItemMatrix::~ChunkItemMatrix()
{
}

void ChunkItemMatrix::getMatrix( Matrix & m, bool world )
{
	BW_GUARD;

	m = pItem_->edTransform();
	if (world && pItem_->chunk() != NULL)
	{
		m.postMultiply( pItem_->chunk()->transform() );
	}
}

void ChunkItemMatrix::getMatrixContext( Matrix & m )
{
	BW_GUARD;

	if (pItem_->chunk() != NULL)
	{
		m = pItem_->chunk()->transform();
	}
	else
	{
		m = Matrix::identity;
	}
}

void ChunkItemMatrix::getMatrixContextInverse( Matrix & m )
{
	BW_GUARD;

	if (pItem_->chunk() != NULL)
	{
		m = pItem_->chunk()->transformInverse();
	}
	else
	{
		m = Matrix::identity;
	}
}

bool ChunkItemMatrix::setMatrix( const Matrix & m )
{
	BW_GUARD;

	Matrix newTransform = m;

	// Snap the transform of the matrix if it's asking for a different
	// translation
	Matrix currentTransform;
	getMatrix( currentTransform, false );
	if (!almostEqual( currentTransform.applyToOrigin(), newTransform.applyToOrigin() ))
	{
		Vector3 t = newTransform.applyToOrigin();

		Vector3 snaps = movementSnaps_;
		if (snaps == Vector3( 0.f, 0.f, 0.f ) && WorldManager::instance().snapsEnabled() )
			snaps = WorldManager::instance().movementSnaps();

		Snap::vector3( t, snaps );

		newTransform.translation( t );
	}

	// check to make sure not too large (i.e. can cross more than 2 chunks)
	BoundingBox bbox;
	pItem_->edBounds(bbox);
	bbox.transformBy( newTransform );
	Vector3 boxVolume = bbox.maxBounds() - bbox.minBounds();
	static const float maxLengthLimit = 100.f;
	static const float minLengthLimit = 0.001f;

	bool result = false;
	if (boxVolume.x < maxLengthLimit && boxVolume.z < maxLengthLimit &&
		boxVolume.x > minLengthLimit && boxVolume.y > minLengthLimit && boxVolume.z > minLengthLimit)
	{
		// always transient
		result = pItem_->edTransform( newTransform, true );
	}
	else if( !warned_ )
	{
		warned_ = true;
		WorldManager::instance().addCommentaryMsg( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/ITEM_PROPERTIES/ITEM_TOO_BIG" ), Commentary::CRITICAL );
	}
	return result;
}

void ChunkItemMatrix::recordState()
{
	BW_GUARD;

	origChunk_ = pItem_->chunk();
	origPose_ = pItem_->edTransform();
	haveRecorded_ = true;
}

bool ChunkItemMatrix::commitState( bool revertToRecord, bool addUndoBarrier )
{
	BW_GUARD;

	if (!haveRecorded_)
		recordState();

	// find out where it is now
	Matrix destPose = pItem_->edTransform();

	// set it back so it starts from the old spot
	pItem_->edTransform( origPose_, true );

	// if we're reverting we stop now
	if (revertToRecord)
		return true;

	// attempt to set the matrix permanently
	bool okToCommit = true;
	if (!pItem_->edTransform( destPose ))
	{
		// set it back if that failed
		pItem_->edTransform( origPose_ );

		okToCommit = false;
	}

	// add the undo operation for it
	UndoRedo::instance().add(
		new ChunkItemMatrixOperation( pItem_, origChunk_, origPose_ ) );
	pItem_->edPostModify();

	// set the barrier with a meaningful name
	if (addUndoBarrier)
	{
		UndoRedo::instance().barrier(
			"Move " + pItem_->edDescription(), false );
		// TODO: Don't always say 'Move ' ...
		//  figure it out from change in matrix
	}

	// check here, so push on an undo for multiselect
	if ( okToCommit )
	{
		if( !revertToRecord )
			warned_ = false;
		return true;
	}

	return false;
}

bool ChunkItemMatrix::hasChanged()
{
	BW_GUARD;

	return (origChunk_ != pItem_->chunk() || origPose_ != pItem_->edTransform());
}

float ChunkItemPositionProperty::length( ChunkItemPtr item )
{
	BW_GUARD;

	BoundingBox bb;
	item->edBounds( bb );
	if( bb.insideOut() )
		return 0.f;
	float length = ( bb.maxBounds() - bb.minBounds() ).length() * 10.f;
	length = Math::clamp( 10.f, length, 200.f );
	return length;
}

/**
 *	This static method in the MatrixProxy base class gets the default
 *	matrix proxy for a given chunk item. It may return NULL.
 */
MatrixProxyPtr MatrixProxy::getChunkItemDefault( ChunkItemPtr pItem )
{
	BW_GUARD;

	return new ChunkItemMatrix( pItem );
}



// -----------------------------------------------------------------------------
// Section: ConstantChunkNameProxy
// -----------------------------------------------------------------------------

namespace Script
{
	PyObject * getData( const Chunk * pChunk );
	// in editor_chunk_portal.cpp
};

std::string chunkPtrToString( Chunk * pChunk )
{
	BW_GUARD;

	PyObject * pPyRet = Script::getData( pChunk );
	MF_ASSERT( pPyRet != NULL );

	std::wstring ret;
	extractUnicode( pPyRet, ret );

	Py_DECREF( pPyRet );
	return bw_wtoutf8( ret );
}


// item_properties.cpp
