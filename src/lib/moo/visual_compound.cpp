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
#include "visual_compound.hpp"
#include "resmgr/primitive_file.hpp"
#include "moo/effect_lighting_setter.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/primitive_file_structs.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/vertex_declaration.hpp"
#include "moo/visual_channels.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/profiler.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

PROFILER_DECLARE( VisualCompound_draw, "VisualCompound Draw" );
PROFILER_DECLARE( VisualCompound_update, "VisualCompound update" );
PROFILER_DECLARE( VisualCompound_dirty, "VisualCompound dirty" );
PROFILER_DECLARE( VisualCompound_Batch_del, "VisualCompound Batch del" );


namespace Moo
{

// -----------------------------------------------------------------------------
// Section: VisualCompound
// -----------------------------------------------------------------------------
bool VisualCompound::disable_ = false;

static uint32 statCookie = -1;
static uint32 nDrawCalls = 0;
static uint32 nVisibleBatches = 0;
static uint32 nVisibleObjects = 0;
static uint32 s_enableLighting = 1;
static bool s_enableDrawPrim = true;
static bool firstTime = true;


/**
 *	Constructor.
 */
VisualCompound::VisualCompound()
: nSourceVerts_( 0 ),
  nSourceIndices_( 0 ),
  pDecl_( NULL ),
  bb_( BoundingBox::s_insideOut_ ),
  sourceBB_( BoundingBox::s_insideOut_ ),
  drawCookie_( -1 ),
  updateCookie_( -1 ),
  valid_( true ),
  taskAdded_( false )
{
	BW_GUARD;
	if (firstTime)
	{
		firstTime = false;
		MF_WATCH( "Chunks/compound draw calls", nDrawCalls, Watcher::WT_READ_ONLY );
		MF_WATCH( "Chunks/compound visible batches", nVisibleBatches, Watcher::WT_READ_ONLY );
		MF_WATCH( "Chunks/compound visible objects", nVisibleObjects, Watcher::WT_READ_ONLY );

		MF_WATCH(
			"Chunks/compound enable lighting",
			s_enableLighting,
			Watcher::WT_READ_WRITE,
			"Switch on/off localised lighting for visual compounds." );

		MF_WATCH( "Render/Performance/DrawPrim VisualCompound", s_enableDrawPrim,
			Watcher::WT_READ_WRITE,
			"Allow VisualCompound to call drawIndexedPrimitive()." );
	}
}


/**
 *	Destructor.
 */
VisualCompound::~VisualCompound()
{
	BW_GUARD;
	SimpleMutexHolder mh(mutex_);
	while (lightingSetters_.size())
	{
		delete lightingSetters_.back();
		lightingSetters_.pop_back();
	}
	materials_.clear();
	this->invalidate();
}


namespace
{
	BinaryPtr getPrimitivePart( const std::string& resName, const std::string& primitivesFileName )
	{
		BW_GUARD;
		BinaryPtr pSection;
		if (resName.find_first_of( '/' ) == std::string::npos)
		{
			std::string partName = primitivesFileName + resName;
			pSection = BWResource::instance().rootSection()->readBinary( partName );
		}
		else
		{
			// find out where the data should really be stored
			std::string fileName, partName;
			splitOldPrimitiveName( resName, fileName, partName );

			// read it in from this file
			pSection = fetchOldPrimitivePart( fileName, partName );
		}
		return pSection;
	}
};


/**
 *	This method inits the source resource for this visual compound.
 *	The input visual should be a static visual, with no nodes, the
 *	visual compound does not load nodes.
 *
 *	@param resourceID	The resource id of the visual resource to base this compound on.
 *
 *	@return true if successful, false otherwise.
 */
bool VisualCompound::init( const std::string& resourceID )
{
	BW_GUARD;
	bool ret = false;

	resourceName_ = resourceID;

	DataSectionPtr pVisualSection = BWResource::openSection( resourceID );
	if (pVisualSection)
	{
		// Open the geometry section, the resource is assumed to be a static
		// visual with one or more primitive groups
		DataSectionPtr pGeometry = pVisualSection->openSection( "renderSet/geometry" );
		if (pGeometry)
		{
			std::string baseName = resourceID.substr(
				0, resourceID.find_last_of( '.' ) );
			std::string primitivesFileName = baseName + ".primitives/";

			//if there is uv2, then don't use it.
			// build the stream name
			std::string stream = pGeometry->readString( "vertices" );
			uint dot = stream.find_last_of('.');
			if (dot == std::string::npos)
			{
				stream = "";
			}
			else
			{
				stream = stream.substr(0, dot+1);
			}
			// try the uv2 stream.
			BinaryPtr uvStream = getPrimitivePart( stream + "uv2", primitivesFileName );
			// try the colour stream.
			BinaryPtr colourStream = getPrimitivePart( stream + "colour", primitivesFileName );
			if (!uvStream && !colourStream)
			{

				// Get vertices resource
				BinaryPtr pVerts = getPrimitivePart( pGeometry->readString( "vertices" ),
					primitivesFileName);

				// Get primitives resource
				BinaryPtr pPrims = getPrimitivePart( pGeometry->readString( "primitive" ),
					primitivesFileName);

				if (pPrims && pVerts)
				{
					// load the assets
					if (loadVertices( pVerts ) &&
						loadIndices( pPrims ) &&
						loadMaterials( pGeometry ))
					{
						// get bounding box
						sourceBB_.setBounds(
							pVisualSection->readVector3( "boundingBox/min" ),
							pVisualSection->readVector3( "boundingBox/max" ) );
						ret = true;
					}
				}
			}
		}
	}
	
	// Get vertex declaration
	pDecl_ = VertexDeclaration::get( "xyznuvtb" );

	return ret;
}


/*
 *	This method loads the vertices from the vertex resource. It will only load
 *	rigid vertices.
 */
bool VisualCompound::loadVertices( BinaryPtr pVerticesSection )
{
	BW_GUARD;
	bool ret = false;

	// Get the vertex header
	const VertexHeader* pVH = reinterpret_cast<const VertexHeader*>( pVerticesSection->data() );
	// If the vertices are xyznuv format load them.
	if (std::string( pVH->vertexFormat_ ) == std::string( "xyznuv" ))
	{
		const VertexXYZNUV* pVerts = reinterpret_cast<const VertexXYZNUV*>( pVH + 1 );

		sourceVerts_.assign( pVerts, pVerts + pVH->nVertices_ );
		ret = true;
	}
	// If the vertices are xyznuvtb format load them.
	else if (std::string( pVH->vertexFormat_ ) == std::string( "xyznuvtb" ))
	{
		const VertexXYZNUVTB* pVerts = reinterpret_cast<const VertexXYZNUVTB*>( pVH + 1 );
		sourceVerts_.assign( pVerts, pVerts + pVH->nVertices_ );
		ret = true;
	}

	return ret;
}


/*
 *	This method loads the indices and the primitive groups from the primitives resource.
 */
bool VisualCompound::loadIndices( BinaryPtr pIndicesSection )
{
	BW_GUARD;
	bool ret = false;

	// Get the index header
	const IndexHeader* pIH = 
		reinterpret_cast<const IndexHeader*>( pIndicesSection->data() );

	// The max index count per primitive group for us to consider using the visual 
	// compound on an object.
	const uint32 MAX_INDEX_COUNT = 3000;

	// Only supports index lists, no strips.
	if( (std::string( pIH->indexFormat_ ) == std::string( "list" ) ||
		std::string( pIH->indexFormat_ ) == std::string( "list32" )  && Moo::rc().maxVertexIndex()>0xFFFF)
		&&
		(pIH->nIndices_ / pIH->nTriangleGroups_) < MAX_INDEX_COUNT  )
	{
		// Make a local copy of the indices
		if( std::string( pIH->indexFormat_ ) == std::string( "list" ) )
			sourceIndices_.assign( ( pIH + 1 ), pIH->nIndices_, D3DFMT_INDEX16 );
		else
			sourceIndices_.assign( ( pIH + 1 ), pIH->nIndices_, D3DFMT_INDEX32 );

		// Get the primitive groups
		const PrimitiveGroup* pPG = 
			reinterpret_cast< const PrimitiveGroup* >(
				(unsigned char*)( pIH + 1 ) + pIH->nIndices_ * sourceIndices_.entrySize() );

		nSourceVerts_ = 0;
		nSourceIndices_ = 0;

        // Go through the primitive groups and remap the primitves to be zero based
		// from the start of its vertices.
		for (int i = 0; i < pIH->nTriangleGroups_; i++)
		{
			nSourceIndices_ += pPG[i].nPrimitives_ * 3;
			sourcePrimitiveGroups_.push_back( pPG[i] );
			
			PrimitiveGroup& pg = sourcePrimitiveGroups_.back();
			uint32 top = 0;
			uint32 bottom = -1;
			for (int i = 0; i < (pg.nPrimitives_ * 3); i++)
			{
				uint32 val = sourceIndices_[i + pg.startIndex_];
				top = max(top, val);
				bottom = min(bottom, val);
			}
			pg.nVertices_ = top - bottom + 1;
			pg.startVertex_ = bottom;
			for (int i = 0; i < (pg.nPrimitives_ * 3); i++)
			{
				sourceIndices_.set( i + pg.startIndex_,
					sourceIndices_[ i + pg.startIndex_ ] - pg.startVertex_ );
			}
			nSourceVerts_ += pg.nVertices_;
		}
		ret = true;
	}

	return ret;
}


/*
 * This method loads the materials used by the primitive groups.
 */
bool VisualCompound::loadMaterials( DataSectionPtr pGeometrySection )
{
	BW_GUARD;
	bool ret = false;

    // Get the primitive group data sections
	std::vector<DataSectionPtr> pgSections;
	pGeometrySection->openSections( "primitiveGroup", pgSections );

	if (pgSections.size())
	{
		// Iterate over the primitive group sections and create their materials.
		for (uint i = 0; i < pgSections.size(); i++)
		{
			EffectMaterialPtr pMaterial = NULL;
			DataSectionPtr pMaterialSection = pgSections[i]->openSection( "material" );
			if (pMaterialSection)
			{
				pMaterial = new EffectMaterial;
				if (pMaterial->load( pMaterialSection ))
				{
					ret = true;
				}
				else
				{
					pMaterial = NULL;
				}
			}

			materials_.push_back( pMaterial );
			lightingSetters_.push_back( new EffectLightingSetter( pMaterial ? pMaterial->pEffect() : NULL ) );
		}
	}
	return ret;
}


/**
 *	This method draws the visual compound
 *	@return true if successful
 */
bool VisualCompound::draw( EffectMaterialPtr pMaterialOverride )
{
	BW_GUARD;
	SimpleMutexHolder holder(mutex_);
	bool ret = false;

	uint32 drawCookie = rc().frameTimestamp();

	// Update statistics
	if (statCookie != drawCookie)
	{
		statCookie = drawCookie;
		nDrawCalls = 0;
		nVisibleBatches = 0;
		nVisibleObjects = 0;
	}

	// Check if the materials are valid
	for (uint32 i = 0; i < materials_.size(); i++)
	{
		if ( materials_[i] &&
			materials_[i]->channel())
		{
			valid_ = false;
			return ret;
		}
	}

	// Check if we need to do any work in the background thread
	// This is just a precaution as the background thread might have failed
	if (dirtyBatches_.size() &&
		!taskAdded_)
	{
		taskAdded_ = true;
		BgTaskManager::instance().addBackgroundTask( this );
	}

	// Check if there is anything to draw.
	if (drawCookie != drawCookie_)
	{
		// If we are not drawing anything, at least try to preload
		// some resources
		for (uint32 batch = 0; batch < renderBatches_.size(); batch++)
		{
			if (!renderBatches_[batch]->preloaded())
			{
				renderBatches_[batch]->preload();
				break;
			}
		}
		return true;
	}

	// Set up the effect visual context.
	EffectVisualContext::instance().pRenderSet( NULL );
	EffectVisualContext::instance().staticLighting( false );
	EffectVisualContext::instance().isOutside( true );
	
	// Set the world matrix to be identity
	rc().push();
	rc().world( Matrix::identity );

	// Set up the diffuse and specular light containers

	LightContainerPtr pRCLC = rc().lightContainer();
	LightContainerPtr pRCSLC = rc().specularLightContainer();

	static LightContainerPtr pLC = new LightContainer;
	static LightContainerPtr pSLC = new LightContainer;

	pLC->init( pRCLC, bb_, true, false );
	pSLC->init( pRCSLC, bb_, true );

	rc().lightContainer( pLC );
	rc().specularLightContainer( pSLC );

    // Set our vertex and primitive sources
	rc().setVertexDeclaration( pDecl_->declaration() );

	// We only allow a certain amount of new resources the graphics card per frame.
	// If there is nothing queued up to render that has not been preloaded, we 
	// allow one batch that has yet to be seen to be transferred to the
	// hardware
	Batch* preloadBatch = NULL;
	uint32 preloadCount = 1;

	// Set to true if the draw cookies are reset in the last material pass
	// This flag will remain false if the last material fails to be set.
	bool drawCookiesReset = false;

	// Iterate over the materials
	EffectMaterialPtr pMat = pMaterialOverride;
	for (uint32 i = 0; i < materials_.size(); i++)
	{
		// Get the vertex and index counts for the current primitive group
		uint32 singlePrimCount = sourcePrimitiveGroups_[i].nPrimitives_;
		uint32 singleVertCount = sourcePrimitiveGroups_[i].nVertices_;
		uint32 singleIndexCount = singlePrimCount * 3;
		uint32 x8PrimCount = singlePrimCount * 8;
		uint32 x8VertCount = singleVertCount * 8;
		uint32 x8IndexCount = singleIndexCount * 8;

		// If no material override is defined use the current material
		if (!pMaterialOverride)
			pMat = materials_[i];
		if (pMat && pMat->begin())
		{
			lightingSetters_[i]->begin();
			for (uint32 j = 0; j < pMat->nPasses(); j++)
			{
				if (pMat->beginPass( j ))
				{
					// Set the last pass flag if we are rendering using the last pass
					// of the last material
					bool lastPass = ((i + 1) == materials_.size() &&
						(j + 1) == pMat->nPasses());
					
					// The draw cookies get reset in the last pass
					drawCookiesReset = lastPass;

					// Iterate over our render batches
					for (uint32 batch = 0; batch < renderBatches_.size(); batch++)
					{
						Batch* pBatch = renderBatches_[batch];
						
						// Check if we actually want to render this batch
						if (pBatch->drawCookie() == drawCookie)
						{
							// If this batch has not been preloaded, preload it if
							// we are allowed to, otherwise skip it, even if it is
							// visible.
							if (!pBatch->preloaded())
							{
								if (preloadCount > 0)
								{
									pBatch->preload();
									preloadCount--;
									preloadBatch = NULL;
								}
								else
								{
									pBatch->drawCookie( drawCookie - 1);
									continue;
								}
							}


							// set the buffers
							pBatch->vertexBuffer().set( 0, 0, sizeof( VertexXYZNUVTBPC ) );
							pBatch->indexBuffer().set();

							if (s_enableLighting)
							{
								lightingSetters_[i]->apply(
									pBatch->pLightContainer(),
									pBatch->pSpecLContainer() );
							}

							// Grab the first primitive group, we are only interested in the start
							// of the vertices and indices, so clean out the nPrims and nverts
							PrimitiveGroup masterPG = pBatch->primitiveGroups()[i];
							masterPG.nPrimitives_ = 0;
							masterPG.nVertices_ = 0;

							// This flag is here so that we know if we currently have something
							// in our primitive group that we want to draw but have not yet drawn.
							bool drawing = false;


							// We iterate over the sequences in the batch, the batch sequences 
							// are a bitfield that says which entries in the geometry are
							// visible.
							Batch::Sequences::const_iterator seqit = pBatch->sequences().begin();
							Batch::Sequences::const_iterator seqend = pBatch->sequences().end();

							uint32 remainder = pBatch->transformHolders().size();
							while (seqit != seqend)
							{
								if ((*seqit) == 0xff)
								{
									// If all the bits are set in the current sequence add eight
									// entries to the primtive group
									drawing = true;
									masterPG.nPrimitives_ += x8PrimCount;
									masterPG.nVertices_ += x8VertCount;
									nVisibleObjects += 8;
								}
								else if ((*seqit) == 0)
								{
									// if none of the bits are set in the sequence, draw the current
									// queued primitives if we are currently drawing or add 8 entries
									// to the start of the primitives if we are not currently drawing
									if (drawing)
									{
										if ( s_enableDrawPrim )
										{
											Moo::rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, masterPG.startVertex_, 
												masterPG.nVertices_, masterPG.startIndex_, masterPG.nPrimitives_ );
										}
										drawing = false;

										nDrawCalls++;
										masterPG.startIndex_ += masterPG.nPrimitives_ * 3 + x8IndexCount;
										masterPG.startVertex_ += masterPG.nVertices_ + x8VertCount;
										masterPG.nPrimitives_ = 0;
										masterPG.nVertices_ = 0;
									}
									else
									{
										masterPG.startIndex_ += x8IndexCount;
										masterPG.startVertex_ += x8VertCount;
									}
								}
								else
								{
									// if we are handling individual sequence entries iterate over them and draw
									// as appropriate
									uint32 mask = 1;
									uint32 seqObj = std::min(uint32(8), remainder);
									for (uint32 subSeq = 0; subSeq < seqObj; subSeq++)
									{
										if ((*seqit) & mask)
										{
											drawing = true;
											masterPG.nPrimitives_ += singlePrimCount;
											masterPG.nVertices_ += singleVertCount;
											nVisibleObjects++;

										}
										else
										{
											if (drawing)
											{
												if ( s_enableDrawPrim )
												{
													Moo::rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, masterPG.startVertex_, 
														masterPG.nVertices_, masterPG.startIndex_, masterPG.nPrimitives_ );
												}
												drawing = false;

												nDrawCalls++;
												masterPG.startIndex_ += masterPG.nPrimitives_ * 3 + singleIndexCount;
												masterPG.startVertex_ += masterPG.nVertices_ + singleVertCount;
												masterPG.nPrimitives_ = 0;
												masterPG.nVertices_ = 0;
											}
											else
											{
												masterPG.startIndex_ += singleIndexCount;
												masterPG.startVertex_ += singleVertCount;
											}
										}
										mask <<= 1;
									}
								}
								remainder -= 8;
								++seqit;
							}

							nVisibleBatches++;

							// If this is the last render of this batch this time around set the 
							// draw cookie to an invalid value so that subsequent renders this frame
							// will not render this unless it gets batched again.
							if (lastPass)
								pBatch->drawCookie(drawCookie - 1);

							// if we have primitives in our group that are not yet drawn, draw them.
							if (drawing)
							{
								if ( s_enableDrawPrim )
								{
									Moo::rc().drawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, masterPG.startVertex_, 
										masterPG.nVertices_, masterPG.startIndex_, masterPG.nPrimitives_ );
								}
								nDrawCalls++;
							}
						}
						else
						{
							if (preloadCount > 0 && 
								preloadBatch == NULL &&
								!pBatch->preloaded())
							{
								preloadBatch = pBatch;
							}
						}
						ret = true;
					}
					pMat->endPass();
				}
			}
			pMat->end();
		}
	}

	// Reset all batch drawcookies if they have not already been reset
	if (!drawCookiesReset)
	{
		for (uint32 batch = 0; batch < renderBatches_.size(); batch++)
		{
			Batch* pBatch = renderBatches_[batch];
			pBatch->drawCookie( drawCookie - 1 );
		}
	}


	// If we have a batch that needs preloading, preload it
	if (preloadBatch)
	{
		preloadBatch->preload();
	}

	drawCookie_ = drawCookie - 1;

	// Set our states back to normal
	rc().pop();
	rc().lightContainer( pRCLC );
	rc().specularLightContainer( pRCSLC );

	return ret;
}


/*
 *	This method updates the draw cookie, the draw cookie is used to 
 *	identify objects in compounds that need to be drawn this frame.
 */
void VisualCompound::updateDrawCookie()
{
	drawCookie_ = rc().frameTimestamp();
}


namespace 
{

void createVertexBuffer( uint32 nVerts, Moo::VertexBuffer& pVertexBuffer )
{
	BW_GUARD;
	DWORD usageFlag = D3DUSAGE_WRITEONLY | (rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0);
	Moo::VertexBuffer pVB;
	HRESULT hr = pVB.create( nVerts * sizeof( VertexXYZNUVTBPC ), 
		usageFlag, 0, D3DPOOL_MANAGED, "VisualCompound/VertexBuffer" );
	if (SUCCEEDED( hr ))
	{
		pVertexBuffer = pVB;
	}
	else
		pVertexBuffer.release();
}

VertexXYZNUVTBPC* lockVertexBuffer( Moo::VertexBuffer& pVertexBuffer )
{
	BW_GUARD;
	VertexXYZNUVTBPC* pVerts = NULL;
	HRESULT hr = pVertexBuffer.lock( 0, 0, (void**)(&pVerts), 0 );
	if (FAILED( hr ))
	{
		pVerts = NULL;
	}
	return pVerts;
}

VertexXYZNUVTBPC transformVertex( const VertexXYZNUVTBPC& v, const Matrix& transform,
						 const Matrix& vecTransform )
{
	VertexXYZNUVTBPC out;
	out.pos_ = transform.applyPoint( v.pos_ );
	out.normal_ = vecTransform.applyVector( v.normal_ );
	out.normal_.normalise();
	out.uv_ = v.uv_;
	out.tangent_ = vecTransform.applyVector( v.tangent_ );
	out.tangent_.normalise();
	out.binormal_ = vecTransform.applyVector( v.binormal_ );
	out.binormal_.normalise();
	return out;
}

void copyVertices( VertexXYZNUVTBPC* pDest, const VertexXYZNUVTBPC* pSource, uint32 count, 
	const Matrix& transform )
{
	BW_GUARD;
	Matrix vecTrans;
	vecTrans.invert( transform );
	XPMatrixTranspose( &vecTrans, &vecTrans );
	for (uint32 i = 0; i < count; i++)
	{
		*(pDest++) = transformVertex( *(pSource++), transform, vecTrans );
	}
}

}


VisualCompound::Batch* VisualCompound::getNextDirtyBatch()
{
	BW_GUARD;
	SimpleMutexHolder mh( mutex_ );
	Batch* pDirty = NULL;
	if (dirtyBatches_.size())
	{
		pDirty = dirtyBatches_.back();
		dirtyBatches_.pop_back();
	}

	return pDirty;
}


/*
 *	This method updates the dirty batches of the visual compound
 */
bool VisualCompound::update()
{
	BW_GUARD_PROFILER( VisualCompound_update );
	char buf[256];
	bw_snprintf( buf, sizeof(buf), "VisualCompound Update (%s) (%d)",
		resourceName_.c_str(), batchMap_.size() );

	DiaryScribe deUpdate(Diary::instance(), buf);

	bool ret = true;
	Batch* pBatch = NULL;
	// Grab the next dirty batch until there are no more dirty batches
	while ((ret == true) && (pBatch = getNextDirtyBatch()))
	{
		// Grab the compound mutex while we get the transforms
		mutex_.grab();
		
		// If the batch has no transform holders, erase it
		if (pBatch->transformHolders().size() == 0)
		{
			BatchMap::iterator it = batchMap_.begin();

			while (it != batchMap_.end())
			{
				if (it->second == pBatch)
				{
					delete it->second;
					ret = true;
					batchMap_.erase( it );
					break;
				}
				++it;
			}
			mutex_.give();
			continue;
		}

		// Grab the transforms for this batch
		uint32 nTransforms = pBatch->transformHolders().size();
		std::vector<Matrix> transforms;
		transforms.reserve( nTransforms );

		for (uint32 i = 0; i < nTransforms; i++)
		{
			transforms.push_back(pBatch->transformHolders()[i]->transform());
		}

		mutex_.give();

		// Calculate number of verts and indices
		uint32 nVerts = nSourceVerts_ * nTransforms;
		uint32 nIndices = nSourceIndices_ * nTransforms;

		// Create the vertex and index buffers
		Moo::VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		createVertexBuffer( nVerts, vertexBuffer );
		D3DFORMAT format;
		if(Moo::rc().maxVertexIndex()>0xFFFF )
			format = D3DFMT_INDEX32;
		else
			format = D3DFMT_INDEX16;

		indexBuffer.create( nIndices, format,
			D3DUSAGE_WRITEONLY | (rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0),
			D3DPOOL_MANAGED, "VisualCompound/IndexBuffer" );

		bool created = false;

		// Init the primitive groups
		std::vector<PrimitiveGroup> pgs;
		pgs.resize(sourcePrimitiveGroups_.size());

		// This is the bounding box for the batch
		BoundingBox batchBounds = BoundingBox::s_insideOut_;

		if (vertexBuffer.valid() && indexBuffer.valid())
		{
			VertexXYZNUVTBPC* pVData = lockVertexBuffer( vertexBuffer );
			IndicesReference indicesReference = indexBuffer.lock();
			if (pVData && indicesReference.valid())
			{
				// Iterate over the materials and create the primitive groups
				// for each material
				uint32 vertexBase = 0;
				uint32 indexBase = 0;
				for (uint32 i = 0; i < materials_.size(); i++)
				{
					PrimitiveGroup& pg = pgs[i];
					const PrimitiveGroup& spg = sourcePrimitiveGroups_[i];
					pg.startVertex_ = vertexBase;
					pg.startIndex_ = indexBase;
					pg.nVertices_ = 0;
					pg.nPrimitives_ = 0;

					// Iterate over the transforms and make copies of the objects
					for (uint32 j = 0; j < nTransforms; j++)
					{
						pg.nVertices_ += spg.nVertices_;
						pg.nPrimitives_ += spg.nPrimitives_;
						indicesReference.copy( sourceIndices_, spg.nPrimitives_ * 3,
							indexBase, spg.startIndex_, vertexBase );
						copyVertices( pVData + vertexBase, &sourceVerts_[spg.startVertex_],
							spg.nVertices_, transforms[j] );

						indexBase += spg.nPrimitives_ * 3;
						vertexBase += spg.nVertices_;

						BoundingBox bb = sourceBB_;
						bb.transformBy( transforms[j] );
						batchBounds.addBounds( bb );
					}
					created = true;
				}
			}
			if (pVData)
				vertexBuffer.unlock();
			if (indicesReference.valid())
				indexBuffer.unlock();

			SimpleMutexHolder mh(mutex_);

			// If the batch has been added back on the dirty list, we have to update it again
			if (std::find(dirtyBatches_.begin(), dirtyBatches_.end(), pBatch ) == dirtyBatches_.end())
			{
				// Only update if we have successfully created the resources
				if (created)
				{
					pBatch->vertexBuffer( vertexBuffer );
					pBatch->indexBuffer( indexBuffer );
					pBatch->primitiveGroups( pgs );
					pBatch->boundingBox( batchBounds );
					renderBatches_.push_back( pBatch );
				}
				else
				{
					// Something went wrong while creating the batch, try again.
					dirty(pBatch);
					ret = false;
				}
			}
			
			// Update the boundingbox for the whole visual compound
			bb_ = BoundingBox::s_insideOut_;
			for (uint32 i = 0; i < renderBatches_.size(); i++)
			{
				bb_.addBounds(renderBatches_[i]->boundingBox());
			}

		}
		else
		{
			dirty(pBatch);
			ret = false;
		}
	}

	return ret;
}


/*
 *	Device notification, need to delete all dx objects
 */
void VisualCompound::deleteManagedObjects()
{
	BW_GUARD;
	SimpleMutexHolder holder(mutex_);

	BatchMap::iterator it = batchMap_.begin();

	while (it != batchMap_.end())
	{
		dirty(it->second);
		++it;
	}
}

namespace
{
	typedef SmartPointer<VisualCompound> VisualCompoundPtr;
	typedef std::map<std::string,VisualCompoundPtr> CompoundMap;
	CompoundMap s_compounds;
	SimpleMutex s_compoundsMutex;
	SimpleMutex s_delMutex;

	typedef std::vector<std::string> InvalidVector;
	InvalidVector s_invalidCompounds;

	typedef std::vector<VisualCompoundPtr> CompoundVector;

}


/**
 *	This static method returns a visual compound for a specific visual
 *	If the compound is not already created a new one will be created.
 *	@param visualName the name of the visual to get the compound for
 *	@return the visual compound or NULL if invalid
 */
VisualCompound* VisualCompound::get( const std::string& visualName )
{
	BW_GUARD;
	// Try to find the visualcompound in the compound list
	s_compoundsMutex.grab();
	CompoundMap::iterator it = s_compounds.find( visualName );
	VisualCompound* pCompound = NULL;
	if (it != s_compounds.end())
	{
		pCompound = it->second.getObject();
	}
	else
	{
		DiaryScribe dsCompound( Diary::instance(), std::string( "VisualCompound get " ) + visualName );
		// If the visualName is in the invalid list, don't try to
		// create a compound for it
		if (std::find(s_invalidCompounds.begin(), s_invalidCompounds.end(), visualName) ==
			s_invalidCompounds.end())
		{
			// If one is not created yet, create one.
			// We give back the mutex while we create the new compound
			// so as not to hold up the rendering thread
			s_compoundsMutex.give();
			pCompound = new VisualCompound;
			bool success = pCompound->init( visualName );
			s_compoundsMutex.grab();
			if (success)
			{
				s_compounds[visualName] = pCompound;
			}
			else
			{
				pCompound = NULL;
				s_invalidCompounds.push_back(visualName);
			}
		}
	}
	s_compoundsMutex.give();
	return pCompound;
}


/**
 *	This method adds an instance of a visual at the set transform as part of a batch
 *	@param visualName the name of the visual to add
 *	@param transform the transform of the visual
 *	@param batchCookie the batch identifier of this visual
 *	@return a TransformHolder that has the relevant information for the instance
 */
TransformHolder* VisualCompound::add( const std::string& visualName, const Matrix& transform, uint32 batchCookie )
{
	BW_GUARD;
	VisualCompound* pCompound = get( visualName );
	TransformHolder* pTH = NULL;
	
	if (pCompound && pCompound->valid())
	{
		pTH = pCompound->addTransform( transform, batchCookie );
	}
	return pTH;
}


/**
 *	This method adds a transformholder to a visual compound
 *	@param transform the transform of the visual
 *	@param batchCookie the batch identifier of this visual
 *	@return a TransformHolder that has the relevant information for the instance
 */
TransformHolder* VisualCompound::addTransform( const Matrix& transform, uint32 batchCookie )
{
	BW_GUARD;
	SimpleMutexHolder holder(mutex_);

	Batch* pBatch = NULL;
	BatchMap::iterator it = batchMap_.find(batchCookie);
	if (it == batchMap_.end())
	{
		pBatch = new Batch( this );
		batchMap_.insert( std::make_pair( batchCookie, pBatch ) );
		dirtyBatches_.push_back(pBatch);
	}
	else
	{
		pBatch = it->second;
	}

	dirty( pBatch );

	return pBatch->add( transform );
}

/**
 *	This method adds a batch to the dirty list and releases
 *	the batches d3d resources
 *	@param pBatch the batch to set as dirty
 */
void VisualCompound::dirty( VisualCompound::Batch* pBatch )
{	
	BW_GUARD_PROFILER( VisualCompound_dirty );
	// Clear the render resources
	pBatch->vertexBuffer().release();
	pBatch->indexBuffer().release();

	// If the batch is in the render batches remove it from there
	BatchVector::iterator rit = std::find( renderBatches_.begin(), renderBatches_.end(), pBatch );
	if (rit != renderBatches_.end())
	{
		renderBatches_.erase( rit );
	}
	
	// If the batch is not in the dirty list, add it to the list
	BatchVector::iterator dit = std::find( dirtyBatches_.begin(), dirtyBatches_.end(), pBatch );
	if (dit == dirtyBatches_.end())
	{
		dirtyBatches_.push_back( pBatch );
	}

	// If a background task is not added for this visual compound, add one
	if (!taskAdded_)
	{
		BgTaskManager::instance().addBackgroundTask( this );
		taskAdded_ = true;
	}
}


/**
 *	This method invalidates the visual compound and removes all its batches
 */
void VisualCompound::invalidate()
{
	BW_GUARD;
	BatchMap::iterator it = batchMap_.begin();
	BatchMap::iterator end = batchMap_.end();

	while (it != end)
	{
		it->second->invalidate();
		delete it->second;
		it->second = NULL;
		it++;
	}
}


/**
 *	This method draws all the current visual compounds
 *	
 */
void VisualCompound::drawAll( EffectMaterialPtr pMaterialOverride )
{
	BW_GUARD;
	DiaryScribe deDraw(Diary::instance(), "VisualCompound Draw" );

	static DogWatch s_dwCompound( "Visual Compound" );
	s_dwCompound.start();

	PROFILER_SCOPED( VisualCompound_draw );


	s_compoundsMutex.grab();

	// List of references to potentially invalid compounds
	static InvalidVector delList;
	delList.reserve(s_compounds.size());

	// Grab all the compounds that we want to update or draw
	static CompoundVector tempCompounds;
	CompoundMap::iterator sit = s_compounds.begin();
	CompoundMap::iterator send = s_compounds.end();

	tempCompounds.reserve(s_compounds.size());

	while (sit != send)
	{
		VisualCompoundPtr pCompound = sit->second;
		if (pCompound.hasObject())
		{
			// If the compound wants draw, add it to the list
			if (pCompound->wantsDraw())
			{
				tempCompounds.push_back(pCompound);
			}
			// This compound might be invalid or empty
			// Keep it's name so we can check after draw
			if (!pCompound->valid() ||
					pCompound->batchMap_.size() == 0)
			{
				delList.push_back( sit->first );
			}
		}
		else
		{
			delList.push_back( sit->first );
		}
		++sit;
	}

	s_compoundsMutex.give();

	CompoundVector::iterator it = tempCompounds.begin();
	CompoundVector::iterator end = tempCompounds.end();

	while (it != end)
	{
		VisualCompoundPtr pCompound = *it;
		pCompound->draw( pMaterialOverride );
		++it;
	}

	// Clear the temporary list so that we do not hold on to
	// references to compounds that are in use.
	tempCompounds.clear();

	// Only allow deleting of the compounds if no one is currently 
	//	adding to the compounds
	if (s_delMutex.grabTry())
	{
		SimpleMutexHolder smh(s_compoundsMutex);
		while (delList.size())
		{
			CompoundMap::iterator it = s_compounds.find( delList.back() );
			VisualCompound* pCompound = it->second.getObject();
			// Double check if the compound is still invalid
			// only delete if the compound is still invalid
			if (!pCompound || !pCompound->valid() ||
				pCompound->batchMap_.size() == 0)
			{
				// If the compound is not valid, add it to the list of invalid
				// compounds so that we don't try to create it again
				if (!pCompound || !pCompound->valid())
					s_invalidCompounds.push_back(delList.back());

				s_compounds.erase( it );
			}
			delList.pop_back();
		}
		s_delMutex.give();
	}

	// tidy up the del list
	delList.clear();
	
	s_dwCompound.stop();
}


/**
 *	This method clears out all the visual compounds
 */
void VisualCompound::fini()
{
	BW_GUARD;
	s_compounds.clear();
}


/**
 *	This method grabs the delete mutex which signals that the visual
 *	compound is not allowed to delete a compound while it is being held
 */
void VisualCompound::grabDelMutex()
{
	s_delMutex.grab();
}

/**
 *	This method tells us if we want to call draw on this visual compound or not.
 */
bool VisualCompound::wantsDraw()
{
	return !Moo::rc().frameDrawn(updateCookie_) || 
		Moo::rc().frameTimestamp() == drawCookie_;
}

/**
 *	This method gives the delete mutex which signals that the visual
 *	compound is not allowed to delete a compound while it is being held
 */
void VisualCompound::giveDelMutex()
{
	s_delMutex.give();
}


/**
 *	This method is the implementations of BackgroundTask::doBackgroundTask
 *	It creates the resources used by the visual compound
 */
void VisualCompound::doBackgroundTask( BgTaskManager & mgr )
{
	BW_GUARD;
	update();
	
	taskAdded_ = false;
}


/**
 *	Destructor
 */
VisualCompound::Batch::~Batch()
{
}

/**
 *	This method clears all the sequences in the batch
 */
void VisualCompound::Batch::clearSequences()
{
	BW_GUARD;
	sequences_.assign( (transformHolders_.size() >> 3) + 1, 0 );
}

/**
 *	This method tells a batch that you want to draw one of its sequences
 *	@param sequence the sequence index to draw
 */
void VisualCompound::Batch::draw( uint32 sequence )
{
	BW_GUARD;
	// If we have not been drawn this frame update the per batch information
	if (drawCookie_ != rc().frameTimestamp())
	{
		this->clearSequences();

		drawCookie_ = rc().frameTimestamp();
		pVisualCompound_->updateDrawCookie();
		
		pLightContainer_->init( Moo::rc().lightContainer(), boundingBox() );
		pSpecLContainer_->init( Moo::rc().specularLightContainer(), boundingBox() );
	}

	// Set the sequence, the sequence is a bitfield
	sequences_[sequence >> 3] |= uint8( 1 << (sequence & 7) );
}

/**
 *	This method creates a transform holder for this batch
 *	@param transform the transform of the object
 *	@return the transform holder object
 */	
TransformHolder* VisualCompound::Batch::add( const Matrix& transform )
{
	BW_GUARD;
	transformHolders_.push_back( new TransformHolder( transform, this, transformHolders_.size() ) );
	return transformHolders_.back();
}


/**
 *	This method dels a transformholder from the batch
 */
void VisualCompound::Batch::del( TransformHolder* transformHolder )
{
	BW_GUARD_PROFILER( VisualCompound_Batch_del );
	SimpleMutexHolder holder(pVisualCompound_->mutex());

	// Remove the transform holder
	TransformHolders::iterator it = 
		std::find( transformHolders_.begin(), transformHolders_.end(), transformHolder );
	if (it != transformHolders_.end())
	{
		transformHolders_.erase( it );
	}
	
	// Set the batch as dirty in the compound
	pVisualCompound_->dirty( this );
}


/**
 *	This method preloads the d3d resources used by a batch
 */
void VisualCompound::Batch::preload()
{
	BW_GUARD;
	vertexBuffer_.preload();
	indexBuffer_.preload();
	preloaded_ = true;
}


/**
 *	This method invalidates all the transformholders used by a batch
 */
void VisualCompound::Batch::invalidate()
{
	BW_GUARD;
	TransformHolders::iterator thit = transformHolders_.begin();
	TransformHolders::iterator thend = transformHolders_.end();
	while (thit != thend)
	{
		(*thit)->pBatch( NULL );
		++thit;
	}
	transformHolders_.clear();
}

/**
 *	This method deletes a transform holder
 */
void TransformHolder::del()
{
	BW_GUARD;
	if (pBatch_)
		pBatch_->del( this );
	delete this;
}

}
