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
#include "chunk_exit_portal.hpp"

#include "resmgr/xml_section.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/guard.hpp"


#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#endif

PROFILER_DECLARE( ChunkExitPortal_tick, "ChunkExitPortal Tick" );

ChunkExitPortal::Vector ChunkExitPortal::seenExitPortals_;


/**
 *	Constructor.
 */
ChunkExitPortal::ChunkExitPortal( struct ChunkBoundary::Portal& p ):
	  ChunkItem( WANTS_DRAW ),
#ifdef EDITOR_ENABLED
	pOwnSection_( new XMLSection( "portal" ) ),
#endif
	  portal_(p)
 {
 }	


/**
 *	This method loads a ChunkExitPortal from a data section. However,
 *	this is not meant to happen, so instead we assert (as it would
 *	be a code error, for example WorldEditor saving out ChunkExitPortals
 *	to .chunk files by accident.)
 *
 *	There is no reason why ChunkExitPortals couldn't be, in future,
 *	saved to .chunk files for specific reasons - in that case, just
 *	remove the assert and implement the load functionality.
 *
 *	@param	pSection	DataSection containing information for us.
 *	@return	always returns false.
 */
bool ChunkExitPortal::load( DataSectionPtr pSection )
{
	BW_GUARD;
	MF_ASSERT_DEV( "No!  Don't load these from file.. they should only be created"
				" by chunks, when an exit portal is bound." && false );
	return false;
}

#if UMBRA_ENABLE
/**
 *	This method creates the umbra object for the ChunkExitPortal
 *	It needs to be called after the portal has been bound
 */
void ChunkExitPortal::createUmbraObject( )
{
	BW_GUARD_PROFILER( ChunkExitPortal_tick );

	if (!pUmbraDrawItem_)
	{	
		// create an array to hold all portal vertices.
		const uint32		 numVerts = portal_.points.size(); 
		std::vector<Vector3> vertices( numVerts );		

		// get them all
		for( uint32 i = 0; i < numVerts; i++ )
		{
			portal_.objectSpacePoint(i, vertices[i] );
		}

		// Add the portal object to the umbra cell
		Matrix m = pChunk_->transform();

		UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
		pUmbraChunkItem->init( this, &vertices[0], numVerts, m, pChunk_->getUmbraCell() );
		pUmbraDrawItem_ = pUmbraChunkItem;
	}
}
#endif


#ifdef EDITOR_ENABLED
void ChunkExitPortal::edBounds( BoundingBox & bbRet ) const
{
	BW_GUARD;
	const std::vector<Vector2> & points = portal_.points;
	bbRet = BoundingBox::s_insideOut_;

	// first find the average in portal space again
	Vector2 avg( 0.f, 0.f );
	for (uint i = 0; i < points.size(); i++)
		avg += points[i];
	avg /= float( points.size() );

	// now build up the bounding box (also in portal space)
	for (uint i = 0; i < points.size(); i++)
	{
		const Vector2 & apt = points[i];
		bbRet.addBounds( Vector3( apt.x - avg.x, apt.y - avg.y, 0.f ) );
	}

	// and add a bit of depth
	bbRet.addBounds( Vector3( 0.f, 0.f, 0.2f ) );
}


/**
 *	Need a section name for the selection filter.
 *
 *	@return	A DataSectionPtr created solely for the use in SelectionFilter.
 */
DataSectionPtr ChunkExitPortal::pOwnSect()
{
	return pOwnSection_;
}


/**
 *	Need a section name for the selection filter.
 *
 *	@return	A DataSectionPtr created solely for the use in SelectionFilter.
 */
const DataSectionPtr ChunkExitPortal::pOwnSect() const
{
	return pOwnSection_;
}
#endif


/**
 *	This method implements the ChunkItem draw interface.  It simply adds
 *	ourselves to the seenExitPortals list for later use by anybody (for
 *	example the rain uses it to draw rain from the inside.)
 */
void ChunkExitPortal::draw()
{
	BW_GUARD;
	ChunkExitPortal::Vector::iterator it = seenExitPortals_.begin();
	ChunkExitPortal::Vector::iterator end = seenExitPortals_.end();
	while (it != end)
		if (*it++ == this)
			return;
	seenExitPortals_.push_back( this );	
}


/**
 *	This method returns the list of seen exit portals for the
 *	current frame.
 *
 *	@return ChunkExitPortal::Vector& exit portals seen this frame.
 */
ChunkExitPortal::Vector& ChunkExitPortal::seenExitPortals()
{	
	return seenExitPortals_;
}

// chunk_exit_portal.cpp
