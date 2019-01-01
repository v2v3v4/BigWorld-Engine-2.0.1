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
#include "visual_channels.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

PROFILER_DECLARE( SortedChannel_drawSorted, "SortedChannel DrawSorted" );
PROFILER_DECLARE( SortedChannel_draw, "SortedChannel Draw" );
	
bool VisualChannel::enabled_ = true;

// -----------------------------------------------------------------------------
// Section: VisualChannel
// -----------------------------------------------------------------------------


/**
 *	This method adds a visual channel to the channel map.
 *	@param name the name of the channel - if you have the annotation channel in
 *	an effect this is the name you use.
 *	@param pChannel the channel to register
 */
void VisualChannel::add( const std::string& name, VisualChannelPtr pChannel )
{
	BW_GUARD;
	if (pChannel != NULL)
	{
		visualChannels_[name] = pChannel;
	}
}

/**
 *	This method removes a channel from the channel map.
 *	@param name the name of the channel to remove
 */
void VisualChannel::remove( const std::string& name )
{
	BW_GUARD;
	VisualChannels::iterator it = visualChannels_.find( name );
	if (it != visualChannels_.end())
	{
		if (it->second && it->second->refCount() > 1)
			WARNING_MSG("VisualChannel not removed properly : %s\n", name.c_str());

		it->second = NULL;
		visualChannels_.erase( it );
	}
}

/**
 *	This method gets a channel from the channel map
 *	@param name the name of the channel to get
 *	@return the channel or NULL if no such channel.
 */
VisualChannelPtr VisualChannel::get( const std::string& name )
{
	BW_GUARD;
	VisualChannels::iterator it = visualChannels_.find( name );
	if (it != visualChannels_.end())
		return it->second;
	return NULL;
}

/**
 *	This method inits the standard channels sorted, internalSorted, 
 *	shimmer and sortedShimmer
 */
void VisualChannel::initChannels()
{
	BW_GUARD;
	add( "sorted", new SortedChannel );
	add( "internalSorted", new InternalSortedChannel );
	add( "distortion", new DistortionChannel );
	add( "shimmer", new ShimmerChannel );
	add( "sortedShimmer", new SortedShimmerChannel );

	// Allocate the default list
	SortedChannel::push();	
}

/**
 *	This method cleans up the channels.
 */
void VisualChannel::finiChannels()
{
	BW_GUARD;
	for
	(
		VisualChannels::iterator it = visualChannels_.begin();
		it != visualChannels_.end();
		++it
	)
	{
		if (it->second && it->second->refCount() > 1)
			WARNING_MSG("VisualChannel not removed properly : %s\n", it->first.c_str());

		it->second = NULL;
	}
	visualChannels_.clear();
}

/**
 *	This method clears all cached channels
 */
void VisualChannel::clearChannelItems()
{
	BW_GUARD;
	VisualChannels::iterator it = visualChannels_.begin();
	while (it != visualChannels_.end())
	{
		(it++)->second->clear();
	}
}

VisualChannel::VisualChannels VisualChannel::visualChannels_;

/**
 *	This method inits the visual draw item
 *
 *	@param pVSS VertextSnapshot containing the vertices to use for drawing.
 *	@param pPrimitives the primitives
 *	@param pStateRecorder the recorded states
 *	@param primitiveGroup the primitive group index
 *	@param distance the distance at which to sort this draw item
 *	@param pStaticVertexColours the static vertex colours for this draw item.
 *	@param sortInternal wheter or not to sort this draw items triangles agains each other
 */

void VisualDrawItem::init( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		SmartPointer<StateRecorder> pStateRecorder, uint32 primitiveGroup, 
		float distance, StaticVertexColoursPtr pStaticVertexColours, 
		bool sortInternal )
{
	BW_GUARD;
	pVSS_ = pVSS;
	pPrimitives_ = pPrimitives;
	pStateRecorder_ = pStateRecorder;
	primitiveGroup_ = primitiveGroup;
	distance_ = distance;
	sortInternal_ = sortInternal;
	pStaticVertexColours_ = pStaticVertexColours;
}


/**
 *	This method clears the draw item.
 */
void VisualDrawItem::fini()
{
	BW_GUARD;
	pVSS_->fini();
	pVSS_ = NULL;
	pPrimitives_ = NULL;
	pStateRecorder_ = NULL;
	pStaticVertexColours_ = NULL;
}

static bool useBaseVertexIndex = true;
static bool s_enableDrawPrim = true;

/**
 *	This method draws the draw item.
 */
void VisualDrawItem::draw()
{
	BW_GUARD;
	static bool firstTime = true;
	if (firstTime)
	{
		MF_WATCH( "Render/Use BaseVertexIndex", useBaseVertexIndex, Watcher::WT_READ_WRITE,
			"Use baseVertexIndex to offset the stream index for rendering with Visual channels, "
			"rather than modifying the source indices explicitly" );

		MF_WATCH( "Render/Performance/DrawPrim VisualDrawItem", s_enableDrawPrim,
			Watcher::WT_READ_WRITE,
			"Allow VisualDrawItem to call drawIndexedPrimitive()." );

		firstTime = false;
	}

	if (sortInternal_)
	{
		drawSorted();
	}
	else
	{
		drawUnsorted();
	}
}

/**
 * This structure is used to store the distance to a particular triangle in a
 * Primitive::PrimGroup structure.
 */
struct TriangleDistance
{
	TriangleDistance( float d1, float d2, float d3, int index )
	{
		dist_ = max( d1, max( d2, d3 ));
		index_ = index;
	}
	bool operator < (const TriangleDistance& d )
	{
		return this->dist_ < d.dist_;
	}

	float dist_;
	int index_;
};

bool operator < ( const TriangleDistance& d1, const TriangleDistance& d2 )
{
	return d1.dist_ < d2.dist_;
}

/*
 *	This method sorts the draw item's triangles and then draws them
 */
void VisualDrawItem::drawSorted()
{
	BW_GUARD_PROFILER( SortedChannel_drawSorted );

	static VectorNoDestructor<float> vertexDistances;

	// Grab the primitive group
	const PrimitiveGroup& pg = pPrimitives_->primitiveGroup( primitiveGroup_ );

	// Resize the vertex distances to fit all our vertices
	vertexDistances.resize( pg.nVertices_ );

	static VectorNoDestructor<TriangleDistance> triangleDistances;

	// Get the depth of the vertices in the vertex snapshot
	if (pVSS_->getVertexDepths( pg.startVertex_, pg.nVertices_, &vertexDistances.front() ))
	{
		triangleDistances.clear();
		uint32 index = pg.startIndex_;
		uint32 end = pg.startIndex_ + (pg.nPrimitives_ * 3);
	
		// Store the distance of the triangles in this primitive group
		uint32 currentTriangleIndex = 0;
		while (index != end)
		{
			float dist1 = vertexDistances[pPrimitives_->indices()[index] - pg.startVertex_];
			++index;
			float dist2 = vertexDistances[pPrimitives_->indices()[index] - pg.startVertex_];
			++index;
			float dist3 = vertexDistances[pPrimitives_->indices()[index] - pg.startVertex_];
			++index;
			triangleDistances.push_back( TriangleDistance( dist1, dist2, dist3, currentTriangleIndex ) );
			currentTriangleIndex += 3;
		}

		// No point continuing if there are no triangles to render
		if (triangleDistances.size() == 0) return;

		// Sort triangle distances back to front (using reverse iterator as +z is into the screen)
		std::sort( triangleDistances.rbegin(), triangleDistances.rend() );
	}

	bool useStaticLighting = pStaticVertexColours_.exists() && pStaticVertexColours_->readyToRender();

	// Set the vertices and store the vertex offset
	uint32 vertexOffset = pVSS_->setVertices( pg.startVertex_, 
		pg.nVertices_, useStaticLighting );

	// Get the difference in vertex offset from the primitive group to the actual offset
	// of the vertex in the (potentially) dynamic vb.
	int32 offsetDiff = int32(vertexOffset) - int32(pg.startVertex_);

	// If we have static vertex colours set them on stream 1
	if (useStaticLighting)
		pStaticVertexColours_->set();

	const uint32 MAX_16_BIT_INDEX = 0xffff; 
 		 
 	uint32 maxIndex = pg.startVertex_ + pg.nVertices_ + offsetDiff; 
 		 
 	D3DFORMAT format = D3DFMT_INDEX32; 
 	if (maxIndex <= MAX_16_BIT_INDEX) 
 	{ 
 		format = D3DFMT_INDEX16; 
 	} 

	if (maxIndex < Moo::rc().maxVertexIndex()) 
 	{ 
 		 
 		// Lock the indices. 
		DynamicIndexBufferBase& dib = rc().dynamicIndexBufferInterface().get( format ); 
    	Moo::IndicesReference ind = dib.lock2( triangleDistances.size() * 3 );
		if (ind.valid())
		{
			// Grab the indices from the primitives object 
			const Moo::IndicesHolder& indices = pPrimitives_->indices(); 

 			// Iterate over our sorted triangles and copy the indices into the new index 
 			// buffer for rendering back to front. Also fix up the indices so that we do not 
 			// have to give a negative BaseVertexIndex 
 			VectorNoDestructor<TriangleDistance>::iterator sit = triangleDistances.begin(); 
 			VectorNoDestructor<TriangleDistance>::iterator send = triangleDistances.end(); 
	 			 
 			int offset = 0; 
 			if (useBaseVertexIndex) 
 			{ 
 				while (sit != send) 
 				{ 
 					uint32 triangleIndex = (sit++)->index_; 
 					triangleIndex += pg.startIndex_; 
 					ind.set( offset++, indices[triangleIndex++] ); 
 					ind.set( offset++, indices[triangleIndex++] ); 
 					ind.set( offset++, indices[triangleIndex++] ); 
 				} 
 			} 
 			else 
 			{ 
 				while (sit != send) 
 				{ 
 					uint32 triangleIndex = (sit++)->index_; 
 					triangleIndex += pg.startIndex_; 
 					ind.set( offset++, indices[triangleIndex++] + offsetDiff ); 
 					ind.set( offset++, indices[triangleIndex++] + offsetDiff ); 
 					ind.set( offset++, indices[triangleIndex++] + offsetDiff ); 
 				} 
 				offsetDiff = 0; 
 			} 
 			 
			// Unlock the indices and set them on the device 
			dib.unlock(); 
		}
 		else if (!useBaseVertexIndex)
			offsetDiff=0;	
 		
 		if ( SUCCEEDED(dib.indexBuffer().set()) )
		{
			uint32 firstIndex = dib.lockIndex();
 			 
 			// Set the recorded states on the device 
 			pStateRecorder_->setStates(); 
 			 
 			if (s_overrideZWrite_) 
 			{ 
 				rc().pushRenderState( D3DRS_ZWRITEENABLE ); 
 				rc().setRenderState( D3DRS_ZWRITEENABLE, FALSE ); 
 			} 
 			 
	 		// Draw the sorted primitives 
			if ( s_enableDrawPrim )
			{
				rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, offsetDiff, pg.startVertex_, 
					pg.nVertices_, firstIndex, pg.nPrimitives_ );
			}
 			if (s_overrideZWrite_) 
				rc().popRenderState();
		}

		// Reset stream 1 if static vertex colours are being used 
		if (useStaticLighting) 
			pStaticVertexColours_->unset();
	}
	else
	{
		 ERROR_MSG( "VisualDrawItem::drawSorted: Unable to render item as the " 
 			"max index used by the item (%d) is more than the device maximum (%d)\n", 
 			maxIndex, Moo::rc().maxVertexIndex()); 
	}
}

/*
 *	This method draws the draw item without sorting the triangles first,
 *	this is an optimisation for additive objects
 */
void VisualDrawItem::drawUnsorted( )
{
	BW_GUARD;
	const PrimitiveGroup& pg = pPrimitives_->primitiveGroup( primitiveGroup_ );
	bool useStaticLighting = pStaticVertexColours_.exists() && pStaticVertexColours_->readyToRender();

	// Set the vertices and store the vertex offset
	uint32 vertexOffset = pVSS_->setVertices( pg.startVertex_, 
		pg.nVertices_, useStaticLighting );

	// Set the recorded states on the device
	pStateRecorder_->setStates();

	if (s_overrideZWrite_)
	{
		rc().pushRenderState( D3DRS_ZWRITEENABLE );
		rc().setRenderState( D3DRS_ZWRITEENABLE, FALSE );
	}

	// If our vertexoffset is the same as the startvertex for the primitive group
	// we do not need to remap the indices.
	if (vertexOffset == pg.startVertex_)
	{
		// If we have static vertex colours set them on stream 1
		if (useStaticLighting)
			pStaticVertexColours_->set();

		// Set the primitives
		if (FAILED(pPrimitives_->setPrimitives()))
		{
			ERROR_MSG( "VisualDrawItem::drawUnsorted -"
						"Unable to set primitives for: %s\n",
						pPrimitives_->resourceID().c_str() );
		}
		else
		{

			// Render the primitivegroup
			pPrimitives_->drawPrimitiveGroup( primitiveGroup_ );

			// Reset stream 1 if static vertex colours are being used
			if (useStaticLighting)
				pStaticVertexColours_->unset();
		}
	}
	else
	{
		// Get the difference in the vertex offset from our pg and the vertices set.
		int32 offsetDiff = int32(vertexOffset) - int32(pg.startVertex_);

		if (useBaseVertexIndex)
		{
			// Set the primitives
			if (FAILED(pPrimitives_->setPrimitives()))
			{
				ERROR_MSG( "VisualDrawItem::drawUnsorted -"
						"Unable to set primitives for: %s\n",
						pPrimitives_->resourceID().c_str() );
			}
			else if ( s_enableDrawPrim ) // Draw the sorted primitives
			{
				rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, offsetDiff, pg.startVertex_, pg.nVertices_, pg.startIndex_, pg.nPrimitives_ );
			}

		}
		else
		{
			// Get the index count (primitives * 3 as we're rendering triangle list)
			int pgIndicesCnt = pg.nPrimitives_ * 3;

			// Get the original indices
			const Moo::IndicesHolder& pgIndices = pPrimitives_->indices();

			if (pgIndices.entrySize() != 2)
				WARNING_MSG( "Performance concern: Drawing a sorted mesh with a very large polygon count" );
		
			DynamicIndexBufferBase& dynamicIndexBuffer = rc().dynamicIndexBufferInterface().get( pgIndices.format() );

			// Lock the index buffer
			Moo::IndicesReference dxIndices = dynamicIndexBuffer.lock2( pgIndicesCnt );
			if ( dxIndices.valid() )
			{
				int it = pg.startIndex_;
				int itEnd = pg.startIndex_ + pgIndicesCnt;

				// Copy original indices and fix them up for difference in vertex offset.
				// We need to do this, as negative BaseVertexIndex does not work on all hardware.
				while ( it != itEnd )
				{
					dxIndices.set( it - pg.startIndex_, pgIndices[ it ] + offsetDiff );
					++it;
				}

				dynamicIndexBuffer.unlock();
				if (SUCCEEDED(dynamicIndexBuffer.indexBuffer().set()))
				{
					uint32 firstIndex = dynamicIndexBuffer.lockIndex();

					// Draw the sorted primitives
					if ( s_enableDrawPrim )
					{
						rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, vertexOffset, pg.nVertices_, firstIndex , pg.nPrimitives_ );
					}
				}
			}
		}
	}
	if (s_overrideZWrite_)
		rc().popRenderState();
}

/*
 *	This method is a quick allocator of draw items, the draw items allocated by this method
 *	are only valid for one frame.
 */
VisualDrawItemPtr VisualDrawItem::get()
{
	BW_GUARD;
	static uint32 timeStamp = rc().frameTimestamp();
	if (!rc().frameDrawn( timeStamp ))
		s_nextAlloc_ = 0;

	if (s_nextAlloc_ == s_drawItems_.size())
		s_drawItems_.push_back( new VisualDrawItem );
	return s_drawItems_[s_nextAlloc_++];
}

uint32 VisualDrawItem::s_nextAlloc_;
std::vector< VisualDrawItemPtr > VisualDrawItem::s_drawItems_;
bool VisualDrawItem::s_overrideZWrite_=false;

/**
 *	This method draws a shimmer draw item, a shimmer draw item is rendered
 *	to the shimmer channel.
 */
void ShimmerDrawItem::draw()
{
	BW_GUARD;
	if (sortInternal_)
	{
		drawSorted();
	}
	else
	{
		drawUnsorted();
	}
}

/*
 *	This method is a quick allocator of shimmer draw items, the draw items allocated by 
 *	this method are only valid for one frame.
 */
ShimmerDrawItemPtr ShimmerDrawItem::get()
{
	BW_GUARD;
	static uint32 timeStamp = rc().frameTimestamp();
	if (!rc().frameDrawn( timeStamp ))
		s_nextAlloc_ = 0;

	if (s_nextAlloc_ == s_drawItems_.size())
		s_drawItems_.push_back( new ShimmerDrawItem );
	return s_drawItems_[s_nextAlloc_++];
}

uint32 ShimmerDrawItem::s_nextAlloc_;
std::vector< ShimmerDrawItemPtr > ShimmerDrawItem::s_drawItems_;

/**
 *	This method creates a draw item and adds it to the shimmer channel
 *
 *	@param pVSS VertextSnapshot containing the vertices to use for drawing.
 *	@param pPrimitives the indices
 *	@param pMaterial the material used by this item
 *	@param primitiveGroup the primitiveGroup to draw
 *	@param zDistance the sorting distance of this item
 *	@param pStaticVertexColours the static vertex colours for this draw item.
 */

void ShimmerChannel::addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours )
{
	BW_GUARD;
	if (enabled_)
	{
		pMaterial->begin();
		if (pMaterial->nPasses() > 0)
		{
			SmartPointer<StateRecorder> pRecorder = pMaterial->recordPass( 0 );
			VisualDrawItem* pDrawItem = VisualDrawItem::get();
			pDrawItem->init( pVSS, pPrimitives, pRecorder, primitiveGroup, zDistance );
			addDrawItem( pDrawItem );
			pMaterial->end();
		}
		else
		{
			ERROR_MSG( "SortedChannel::addItem - no passes in material\n" );
		}
	}
}

/**
 *	This method adds a draw item to the shimmer channel.
 *	@param pItem the draw item to add
 */
void ShimmerChannel::addDrawItem( ChannelDrawItem* pItem )
{
	BW_GUARD;
	if (enabled_)
	{
		if (!rc().frameDrawn( s_timeStamp_ ))
			s_drawItems_.clear();
		s_drawItems_.push_back( pItem );
	}
	else
	{
		pItem->fini();
	}
}

ShimmerChannel::DrawItems ShimmerChannel::s_drawItems_;
uint32 ShimmerChannel::s_timeStamp_ = -1;
uint32 ShimmerChannel::s_nItems_ = 0;

/**
 *	This method draws the shimmer channel
 */
void ShimmerChannel::draw()
{
	BW_GUARD;
	bool wasEnabled = enabled_;
	enabled_ = false;
	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
	if (!rc().frameDrawn( s_timeStamp_ ))
		s_drawItems_.clear();

	s_nItems_ = s_drawItems_.size();

	DrawItems::iterator it = s_drawItems_.begin();
	DrawItems::iterator end = s_drawItems_.end();

	while (it != end)
	{
		(*it)->draw();
		(*it++)->fini();
	}
	s_drawItems_.clear();
	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE );
	enabled_ = wasEnabled;
}


/**
 *	This method clears the shimmer channel.
 */
void ShimmerChannel::clear()
{
	BW_GUARD;
	DrawItems::iterator it = s_drawItems_.begin();
	DrawItems::iterator end = s_drawItems_.end();

	while (it != end)
	{
		(*it++)->fini();
	}
	s_drawItems_.clear();
}


/**
 *	This method creates a draw item and adds it to the sorted channel
 *
 *	@param pVSS VertextSnapshot containing the vertices to use for drawing.
 *	@param pPrimitives the indices
 *	@param pMaterial the material used by this item
 *	@param primitiveGroup the primitiveGroup to draw
 *	@param zDistance the sorting distance of this item
 *	@param pStaticVertexColours the static vertex colours for this draw item.
 */
void SortedChannel::addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours )
{
	BW_GUARD;
	if (enabled_)
	{
		pMaterial->begin();
		if (pMaterial->nPasses() > 0)
		{
			SmartPointer<StateRecorder> pRecorder = pMaterial->recordPass( 0 );
			VisualDrawItem* pDrawItem = VisualDrawItem::get();
			pDrawItem->init( pVSS, pPrimitives, pRecorder, primitiveGroup, zDistance, 
				pStaticVertexColours );
			addDrawItem( pDrawItem );
			pMaterial->end();
		}
		else
		{
			ERROR_MSG( "SortedChannel::addItem - no passes in material\n" );
		}
	}
}


/**
 *	This method adds a draw item to the sorted channel.
 *	@param pItem the draw item to add
 */
void SortedChannel::addDrawItem( ChannelDrawItem* pItem )
{
	BW_GUARD;
	if (enabled_)
	{
		if (!rc().frameDrawn( s_timeStamp_ ))
			drawItems().clear();

		drawItems().push_back( pItem );
		pItem->alwaysVisible(s_reflecting_);
	}
	else
	{
		pItem->fini();
	}
}

// Helper function for sorting two draw items
bool lt( const ChannelDrawItemPtr a, const ChannelDrawItemPtr b )
{
	return a->distance() < b->distance();
}

bool s_alreadySorted = false;
/**
 *	This method renders the sorted channel, first it sorts the draw items back to front
 *	then it asks each one to draw itself.
 */
void SortedChannel::draw(bool clear /*=true*/)
{
	BW_GUARD_PROFILER( SortedChannel_draw );

	bool wasEnabled = enabled_;
	enabled_ = false;
	if (!rc().frameDrawn( s_timeStamp_ ) && clear)
		drawItems().clear();

	//Check to see if we can skip the sorting.
	// This is because the same list of objects can be drawn more than once per
	// frame now. It's directly linked to the use of the clear flag....
	if (!s_alreadySorted)
	{
		std::sort( drawItems().rbegin(), drawItems().rend(), lt );
		s_alreadySorted=!clear;
	}
	VectorNoDestructor< ChannelDrawItemPtr >::iterator it = drawItems().begin();
	VectorNoDestructor< ChannelDrawItemPtr >::iterator end = drawItems().end();
	while (it != end)
	{
		if (clear)
		{
			(*it)->draw();
			(*it)->fini();
		}
		else if ((*it)->alwaysVisible())
		{
			VisualDrawItem::overrideZWrite(true);
			(*it)->draw();
			VisualDrawItem::overrideZWrite(false);
		}
		it++;
	}
	if (clear)
	{
		drawItems().clear();
		s_alreadySorted=false;
	}
	enabled_ = wasEnabled;
}

/**
 *	This method clears the sorted channel.
 */
void SortedChannel::clear()
{
	BW_GUARD;
	VectorNoDestructor< ChannelDrawItemPtr >::iterator it = drawItems().begin();
	VectorNoDestructor< ChannelDrawItemPtr >::iterator end = drawItems().end();

	while (it != end)
	{
		(*it++)->fini();
	}

	drawItems().clear();

	s_alreadySorted=false;
}

SortedChannel::DrawItemStack SortedChannel::s_drawItems_;
uint32 SortedChannel::s_timeStamp_ = -1;
bool SortedChannel::s_reflecting_ = true;

void SortedChannel::push()
{
	BW_GUARD;
	s_drawItems_.push_back( DrawItems() );
}

void SortedChannel::pop()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV(s_drawItems_.size() > 1)
	{
		return;
	}
	MF_ASSERT_DEV(drawItems().size() == 0);
	s_drawItems_.pop_back();
}



/**
 *	This method creates a draw item that needs to have its triangles sorted internally and 
 *	adds it to the sorted channel
 *
 *	@param pVSS VertextSnapshot containing the vertices to use for drawing.
 *	@param pPrimitives the indices
 *	@param pMaterial the material used by this item
 *	@param primitiveGroup the primitiveGroup to draw
 *	@param zDistance the sorting distance of this item
 *	@param pStaticVertexColours the static vertex colours for this draw item.
 */
void InternalSortedChannel::addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours )
{
	BW_GUARD;
	if (enabled_)
	{
		pMaterial->begin();
		if (pMaterial->nPasses() > 0)
		{
			SmartPointer<StateRecorder> pRecorder = pMaterial->recordPass( 0 );
			VisualDrawItem* pDrawItem = VisualDrawItem::get();
			pDrawItem->init( pVSS, pPrimitives, pRecorder, primitiveGroup, zDistance, 
				pStaticVertexColours, true );
			addDrawItem( pDrawItem );
			pMaterial->end();
		}
		else
		{
			ERROR_MSG( "SortedChannel::addItem - no passes in material\n" );
		}
	}
}

/**
 *	This method creates a draw item that needs to have its triangles sorted internally be 
 *	drawn to the shimmer channel then adds it to the sorted channel
 *
 *	@param pVSS VertextSnapshot containing the vertices to use for drawing.
 *	@param pPrimitives the indices
 *	@param pMaterial the material used by this item
 *	@param primitiveGroup the primitiveGroup to draw
 *	@param zDistance the sorting distance of this item
 *	@param pStaticVertexColours the static vertex colours for this draw item.
 */
void SortedShimmerChannel::addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours )
{
	BW_GUARD;
	if (enabled_)
	{
		pMaterial->begin();
		if (pMaterial->nPasses() > 0)
		{
			SmartPointer<StateRecorder> pRecorder = pMaterial->recordPass( 0 );
			ShimmerDrawItemPtr pDrawItem = ShimmerDrawItem::get();
			pDrawItem->init( pVSS, pPrimitives, pRecorder, primitiveGroup, zDistance, 
				pStaticVertexColours );
			addDrawItem( pDrawItem.getObject() );
			pMaterial->end();
		}
		else
		{
			ERROR_MSG( "SortedChannel::addItem - no passes in material\n" );
		}
	}
}

/**
 */
void DistortionDrawItem::drawMask()
{
	SmartPointer<StateRecorder> mainPass = pStateRecorder_;
	pStateRecorder_ = maskPass_;
	drawUnsorted();	
	pStateRecorder_ = mainPass;
}

/*
 *	This method is a quick allocator of DistortionDrawItem, the draw items allocated by 
 *	this method are only valid for one frame.
 */
DistortionDrawItemPtr DistortionDrawItem::get()
{
	BW_GUARD;
	static uint32 timeStamp = rc().frameTimestamp();
	if (!rc().frameDrawn( timeStamp ))
		s_nextAlloc_ = 0;

	if (s_nextAlloc_ == s_drawItems_.size())
		s_drawItems_.push_back( new DistortionDrawItem );
	return s_drawItems_[s_nextAlloc_++];
}

void DistortionDrawItem::fini()
{
	BW_GUARD;
	maskPass_ = NULL;
	VisualDrawItem::fini();
}

uint32 DistortionDrawItem::s_nextAlloc_;
std::vector< DistortionDrawItemPtr > DistortionDrawItem::s_drawItems_;

void DistortionChannel::addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours )
{
	BW_GUARD;
	if (enabled_ && VisualChannel::enabled_)
	{
		if (Moo::rc().reflectionScene())
			return;

		pMaterial->begin();
		if (pMaterial->nPasses() > 0)
		{
			SmartPointer<StateRecorder> pRecorder = pMaterial->recordPass( 0 );
			SmartPointer<StateRecorder> pMaskRecorder = pMaterial->recordPass( 1 );
			DistortionDrawItemPtr pDrawItem = DistortionDrawItem::get();
			pDrawItem->initMask( pMaskRecorder );
			pDrawItem->init( pVSS, pPrimitives, pRecorder, primitiveGroup, zDistance );
			addDrawItem( pDrawItem.getObject() );
			pMaterial->end();
		}
		else
		{
			ERROR_MSG( "DistortionChannel::addItem - no passes in material\n" );
		}
	}
}

/**
 *	This method adds a draw item to the disortion channel.
 *	@param pItem the draw item to add
 */
void DistortionChannel::addDrawItem( DistortionDrawItem* pItem )
{
	BW_GUARD;
	if (enabled_ && VisualChannel::enabled_)
	{
		MF_ASSERT(!Moo::rc().reflectionScene());

		if (!rc().frameDrawn( s_timeStamp_ ))
			s_drawItems_.clear();
		s_drawItems_.push_back( pItem );
	}
	else
	{
		pItem->fini();
	}
}

DistortionChannel::DrawItems DistortionChannel::s_drawItems_;
uint32 DistortionChannel::s_timeStamp_ = -1;

// Helper function for sorting two draw items
bool lt_s( const DistortionDrawItemPtr& a, const DistortionDrawItemPtr& b )
{
	return a->distance() < b->distance();
}

/**
 *	This method draws the distortion channel
 */
void DistortionChannel::draw( bool clear )
{
	BW_GUARD;
	bool wasEnabled = VisualChannel::enabled_;
	VisualChannel::enabled_ = false;
	if (!rc().frameDrawn( s_timeStamp_ ) && clear)
		s_drawItems_.clear();

	std::sort( s_drawItems_.rbegin(), s_drawItems_.rend(), lt_s );
	DrawItems::iterator it = s_drawItems_.begin();
	DrawItems::iterator end = s_drawItems_.end();
	while (it != end)
	{
		if (clear)
		{
			(*it)->draw();
			(*it)->fini();
		}
		else
			(*it)->drawMask();
		it++;
	}
	if (clear)
		s_drawItems_.clear();

	VisualChannel::enabled_ = wasEnabled;
}

/**
  *
  */
void DistortionChannel::clear()
{
	BW_GUARD;
	DrawItems::iterator it = s_drawItems_.begin();
	DrawItems::iterator end = s_drawItems_.end();
	
	while (it != end)
	{
		(*it++)->fini();
	}
	s_drawItems_.clear();
}

// Static for enabling the distortion channel
bool DistortionChannel::enabled_ = true;

} // namespace Moo

// visual_channel.cpp
