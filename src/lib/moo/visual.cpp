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

#pragma warning( disable : 4503 )

#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/diary.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/primitive_file.hpp"

#include "render_context.hpp"
#include "directional_light.hpp"
#include "omni_light.hpp"
#include "spot_light.hpp"
#include "texture_manager.hpp"
#include "primitive_manager.hpp"
#include "vertices_manager.hpp"
#include "animation_manager.hpp"
#include "math/blend_transform.hpp"
#include "vertex_formats.hpp"
#include "effect_material.hpp"
#include "visual.hpp"
#include "effect_constant_value.hpp"
#include "effect_visual_context.hpp"
#include "visual_manager.hpp"
#include "visual_channels.hpp"
#include "visual_common.hpp"
#include "fog_helper.hpp" 
#include "vertex_streams.hpp"
#include "resource_load_context.hpp"

#include "physics2/worldtri.hpp"
#include "physics2/bsp.hpp"

#ifndef CODE_INLINE
#include "visual.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )


namespace Moo
{

PROFILER_DECLARE( Visual_draw, "Visual Draw" );

// File statics
namespace 
{
	typedef StringHashMap< BSPProxyPtr > BSPMap;

	BSPMap		s_premadeBSPs;
	SimpleMutex s_premadeBSPsLock;

	// Create an object that registers a watcher to read the size of the
	// premade BSP cache.
	class WatchOwner
	{
	public:
		uint32 premadeBSPCount() const { return s_premadeBSPs.size(); }
	
		WatchOwner()
		{
			MF_WATCH("Render/Visual/premadeBSP_Count", *this,
				&WatchOwner::premadeBSPCount, "The size of the premade BSP Cache");
		}
	};

	WatchOwner wo;
}


// Class statics
uint32								Visual::drawCount_			= 0;
VectorNoDestructor< class Visual* > Visual::s_batches_;
bool								Visual::disableBatching_	= false;

/**
 *	BSPProxy constructor.
 */
BSPProxy::BSPProxy( BSPTree * pTree ) :
	pTree_( pTree )
{
}

/**
 *	BSPProxy destructor.
 */
BSPProxy::~BSPProxy()
{
	BW_GUARD;
	delete pTree_;
	pTree_ = NULL;
}

/**
 *	This class stores per primitivegroup batch data for the batching.
 */
class PrimitiveGroupBatcher
{
public:
	PrimitiveGroupBatcher(Visual::PrimitiveGroup* pGroup) :
		pGroup_(pGroup)
	{
	}

	/*
	 *	This method batches per primitivegroup instance information
	 *	for delayed rendering.
	 */
	void batch(StaticVertexColoursPtr staticVertexColours)
	{
		BW_GUARD;
		static DogWatch s_pgBatcher( "PrimGB" );
		s_pgBatcher.start();
		batchInstances_.push_back( BatchInstance(staticVertexColours) );

		// record the per instance data for this batch
		if ( pGroup_->material_->pEffect() )
		{
			batchInstances_.back().recordMaterial( 
					pGroup_->material_->pEffect() );
		}

		s_pgBatcher.stop();
	}

	/*
	 *	This method draws the instances of this primitive group that have been
	 *	batched this frame.
	 */
	void draw(Visual::Geometry* pGeometry)
	{
		BW_GUARD;
		
		// The vertex format is already set up as being non-static
		bool lastStatic = false;

		Moo::EffectMaterialPtr pMat = pGroup_->material_;
		if (pMat->begin())
		{
			for (uint32 i = 0; i < pMat->nPasses(); i++)
			{
				pMat->beginPass( i );

				// Iterate over the batch instances, set the per instance data,
				// commit the material changes and draw.
				VectorNoDestructor<BatchInstance>::iterator it = batchInstances_.begin();
				VectorNoDestructor<BatchInstance>::iterator end = batchInstances_.end();
				while (it != end)
				{
					// Set the appropriate vertex format if we are rendering
					// with or without static vertex colours
					bool useStaticLighting = it->staticVertexColours_.exists() && it->staticVertexColours_->readyToRender();

					if (!lastStatic && useStaticLighting)
					{
						lastStatic = true;
						pGeometry->vertices_->setVertices( Moo::rc().mixedVertexProcessing(), true );
					}
					else if (lastStatic && !useStaticLighting)
					{
						lastStatic = false;
						pGeometry->vertices_->setVertices( Moo::rc().mixedVertexProcessing(), false );
					}

					// Set up our static vertex colour stream
					if (useStaticLighting)
					{
						it->staticVertexColours_->set();
					}

					it->apply();
					static DogWatch s_commitChanges( "CommitChanges" );
					s_commitChanges.start();
					pMat->commitChanges();
					s_commitChanges.stop();
					static DogWatch s_draw( "Draw" );
					s_draw.start();
					pGeometry->primitives_->drawPrimitiveGroup( pGroup_->groupIndex_ );
					s_draw.stop();

					// Unset the static vertex colours
					if (useStaticLighting)
					{
						it->staticVertexColours_->unset();
					}

					++it;
				}
				pMat->endPass();
			}
			pMat->end();
			batchInstances_.clear();
		}
	}


	/**
	 *	This class is the container of the per instance batching data.
	 */
	class BatchInstance
	{
	public:
		BatchInstance(StaticVertexColoursPtr staticVertexColours) :
			staticVertexColours_( staticVertexColours )
		{
		}
		void clear()
		{
			BW_GUARD;
			recordedEffectConstants_.clear();
			staticVertexColours_ = NULL;
		}

		// This method records the auto constants used by this material.
		void recordMaterial( ManagedEffectPtr pEffect )
		{
			BW_GUARD;
			static DogWatch s_recordAutos( "RecordAutos" );
			s_recordAutos.start();
			pEffect->recordAutoConstants( recordedEffectConstants_ );
			s_recordAutos.stop();
		}

		// Apply the recorded per instance data.
		void apply()
		{
			BW_GUARD;
			std::vector< RecordedEffectConstantPtr >::iterator it = recordedEffectConstants_.begin();
			std::vector< RecordedEffectConstantPtr >::iterator end = recordedEffectConstants_.end();
            while (it != end)
			{
				(*it)->apply();
				it++;
			}
		}
		StaticVertexColoursPtr staticVertexColours_;
	private:
		std::vector< RecordedEffectConstantPtr > recordedEffectConstants_;
	};
private:
	VectorNoDestructor<BatchInstance> batchInstances_;
	Visual::PrimitiveGroup* pGroup_;
};


/**
 *	This class stores the geometry portion of the batches.
 */
class GeometryBatcher
{
public:
	GeometryBatcher( Visual::Geometry* pGeometry, Visual::RenderSet* pRenderSet )
	: pGeometry_( pGeometry ),
	  pRenderSet_( pRenderSet )
	{
		BW_GUARD;
		Visual::PrimitiveGroupVector::iterator it = pGeometry_->primitiveGroups_.begin();
		Visual::PrimitiveGroupVector::iterator end = pGeometry_->primitiveGroups_.end();

		while (it != end)
		{
			primitiveGroupBatches_.push_back( PrimitiveGroupBatcher( &*it ) );
			it++;
		}
	}

	/*
	 *	This method batches the primitive groups that use this geometry.
	 *	@param staticVertexColours the static vertex colours for this batch
	 */
	void batch( StaticVertexColoursPtr staticVertexColours )
	{
		BW_GUARD;
		EffectVisualContextSetter setter( pRenderSet_ );
		std::vector<PrimitiveGroupBatcher>::iterator it = primitiveGroupBatches_.begin();
		while (it != primitiveGroupBatches_.end())
		{
			(it++)->batch( staticVertexColours );
		}
	}

	/*
	 *	This method draws the batched primiitve groups that use this geometry.
	 */
	bool draw()
	{
		BW_GUARD;
		if (FAILED(pGeometry_->vertices_->setVertices( rc().mixedVertexProcessing() )))
		{
			ERROR_MSG( "Moo::GeometryBatcher::draw - Unable to set vertices\n" );
			return false;
		}
		if (FAILED(pGeometry_->primitives_->setPrimitives()))
		{
			ERROR_MSG( "Moo::GeometryBatcher::draw - Unable to set primitives\n" );
			return false;
		}

		EffectVisualContextSetter setter( pRenderSet_ );
		std::vector<PrimitiveGroupBatcher>::iterator it = primitiveGroupBatches_.begin();
		while (it != primitiveGroupBatches_.end())
		{
			(it++)->draw(pGeometry_);
		}
		return true;
	}

private:
	Visual::Geometry* pGeometry_;
	Visual::RenderSet* pRenderSet_;
	std::vector<PrimitiveGroupBatcher> primitiveGroupBatches_;
};

/**
 *	This class stores the top level of the batched visual.
 *	The batched visual stores per instance information for
 *	a visual that has had its batch method called.
 */
class VisualBatcher
{
public:
	VisualBatcher() :
	  batchedThisFrame_( -1 )
	{}

	/*
	 *	This method adds a geometry description to the visual batcher.
	 *	@param pGeometry pointer to the geometry description.
	 *	@param pRenderSet the renderset that owns the geometry section
	 */
	void addGeometry( Visual::Geometry* pGeometry, Visual::RenderSet* pRenderSet )
	{
		BW_GUARD;
		geometryBatches_.push_back( GeometryBatcher( pGeometry, pRenderSet ) );
	}


	/*
	 *	This tells the visualbatcher to batch this visual for later rendering.
	 *	@param staticVertexColours the static vertex colours for this batch
	 */
	void batch( StaticVertexColoursPtr staticVertexColours )
	{
		BW_GUARD;
		for (uint32 i = 0; i < geometryBatches_.size(); i++)
		{
			geometryBatches_[i].batch( staticVertexColours );
		}
	}

	/*
	 *	This method draws the batched instances of this visual.
	 */
	bool draw()
	{
		BW_GUARD;
		for (uint32 i = 0; i < geometryBatches_.size(); i++)
		{
			geometryBatches_[i].draw();
		}
		return true;
	}

	uint32 batchedThisFrame_;
private:
	std::vector<GeometryBatcher> geometryBatches_;

};

// -----------------------------------------------------------------------------
// Section: Methods used by VisualLoader
// The following methods were created because they are needed by VisualLoader.
// Most of them are inline here because VisualLoader is only instantiated here.
// -----------------------------------------------------------------------------

static BSPProxyPtr NULL_BSPPROXYPTR( NULL ); 

// Finds the material with the specified identifier.
inline const EffectMaterialPtr Visual::Materials::find(
		const std::string& identifier ) const
{
	BW_GUARD;
	for ( const_iterator ppMat = this->begin(); ppMat != this->end(); ++ppMat )
	{
		if (identifier == (*ppMat)->identifier())
			return *ppMat;
	}

	return NULL;
}

// Adds a BSP into our BSP cache.
// Needed by VisualLoader
inline void Visual::BSPCache::add( const std::string& resourceID, BSPProxyPtr& pBSP )
{
	BW_GUARD;
	s_premadeBSPsLock.grab();
	s_premadeBSPs.insert(std::make_pair( resourceID, pBSP ));
	s_premadeBSPsLock.give();
}

// Finds a BSP into our BSP cache.
// Needed by VisualLoader
inline BSPProxyPtr& Visual::BSPCache::find( const std::string& resourceID )
{
	BW_GUARD;
	s_premadeBSPsLock.grab();
	BSPMap::iterator bspit = s_premadeBSPs.find( resourceID );
	BSPProxyPtr& pFoundBSP = (bspit != s_premadeBSPs.end()) ?
		bspit->second : NULL_BSPPROXYPTR;
	s_premadeBSPsLock.give();
	return pFoundBSP;
}

// Returns singleton instance of BSPCache. Needed by VisualLoader.
inline Visual::BSPCache& Visual::BSPCache::instance()
{
	static BSPCache bspCache;
	return bspCache;
}

// Loads BSP from  _bsp.visual file.
inline BSPProxyPtr Visual::loadBSPVisual( const std::string& resourceID )
{
	BW_GUARD;
	VisualPtr bspVisual = VisualManager::instance()->get( resourceID );
	return (bspVisual && bspVisual->pBSP_) ? bspVisual->pBSP_ : NULL_BSPPROXYPTR;
}


/**
 * Constructor for the visual.
 *
 * @param resourceID the identifier of the visuals resource.
 * @param validateNodes	Boolean indicating whether to validate that indices
 *						used mesh creation refer to valid vertices.
 * @param loadingFlags	Flags for partial loading (geometry-only for flora for example).
 */
Visual::Visual( const std::string& resourceID, bool validateNodes /*= true */, uint32 loadingFlags /*= LOAD_ALL*/ ) :	
	isInVisualManager_( true ),
	pBSP_( NULL ),
	validateNodes_( validateNodes ),
	pBatcher_( NULL ),
	isOK_( true ), 
	resourceID_(resourceID)
{
	BW_GUARD;

	DiaryScribe dsVisual( Diary::instance(), std::string( "Visual " ) + resourceID );
	uint32 nPGroups = 0;

	std::string baseName =
			resourceID.substr( 0, resourceID.find_last_of( '.' ) );
	VisualLoader< Visual > loader( baseName );

	// open the resource
	DataSectionPtr root = loader.getRootDataSection();
	if (!root)
	{
		ERROR_MSG( "Couldn't open visual %s\n", resourceID.c_str() );
		isOK_ = false;
		return;
	}

	Moo::ScopedResourceLoadContext resLoadCtx( BWResource::getFilename( resourceID ) );

	// Load BSP Tree
	BSPMaterialIDs 	bspMaterialIDs;
	BSPTreePtr 		pFoundBSP;
	bool bspRequiresMapping = false;
	
	if (loadingFlags == LOAD_ALL)
	{
		bspRequiresMapping = loader.loadBSPTree( pFoundBSP, bspMaterialIDs );
	}

	// Load our primitives
	const std::string primitivesFileName = loader.getPrimitivesResID() + '/';

	// load the hierarchy
	DataSectionPtr nodeSection = root->openSection( "node" );
	if (nodeSection)
	{
		rootNode_ = new Node;
		rootNode_->loadRecursive( nodeSection );
	}
	else
	{
		// If there are no nodes in the resource create one.
		rootNode_ = new Node;
		rootNode_->identifier( "root" );		
	}

	// open the renderesets
	std::vector< DataSectionPtr > renderSets;
	root->openSections( "renderSet", renderSets );

	if (isOK_ && (renderSets.size() == 0))
	{
		ERROR_MSG( "Moo::Visual: No rendersets in %s\n", resourceID.c_str() );
		isOK_ = false;
	}

	// iterate through the rendersets and create them
	std::vector< DataSectionPtr >::iterator rsit = renderSets.begin();
	std::vector< DataSectionPtr >::iterator rsend = renderSets.end();
	while (rsit != rsend)
	{
		DataSectionPtr renderSetSection = *rsit;
		++rsit;
		RenderSet renderSet;

		// Read worldspace flag
		renderSet.treatAsWorldSpaceObject_ = renderSetSection->readBool( "treatAsWorldSpaceObject" );
		std::vector< std::string > nodes;

		// Read the node identifiers of all nodes that affect this renderset.
		renderSetSection->readStrings( "node", nodes );

		// Are there any nodes?
		if (nodes.size())
		{
			// Iterate through the node identifiers.
			std::vector< std::string >::iterator it = nodes.begin();
			std::vector< std::string >::iterator end = nodes.end();
			while (it != end)
			{
				// Find the current node.
				NodePtr node = rootNode_->find( *it );
				if (isOK_ && (!node))
				{
					ERROR_MSG( "Couldn't find node %s in %s\n", (*it).c_str(), resourceID.c_str() );
					isOK_ = false;
				}

				// Add the node to the rendersets list of nodes
				renderSet.transformNodes_.push_back( node );				

				++it;
			}
		}
		else
		{
			// The renderset is not affected by any nodes, so we will force it to be affected by the rootNode.
			renderSet.transformNodes_.push_back( rootNode_ );			
		}

		// Calculate the first node's static world transform
		//  (for populateWorldTriangles)
		NodePtr pMainNode = renderSet.transformNodes_.front();
		renderSet.firstNodeStaticWorldTransform_ = pMainNode->transform();

		while (pMainNode != rootNode_)
		{
			pMainNode = pMainNode->parent();
			renderSet.firstNodeStaticWorldTransform_.postMultiply(
				pMainNode->transform() );
		}

		// Open the geometry sections
		std::vector< DataSectionPtr > geometries;
		renderSetSection->openSections( "geometry", geometries );

		if (isOK_ && (geometries.size() == 0))
		{
			ERROR_MSG( "No geometry in renderset in %s\n", resourceID.c_str() );
			isOK_ = false;
		}		

		std::vector< DataSectionPtr >::iterator geit = geometries.begin();
		std::vector< DataSectionPtr >::iterator geend = geometries.end();

		// iterate through the geometry sections
		while (geit != geend)
		{
			DataSectionPtr geometrySection = *geit;
			++geit;
			Geometry geometry;

			// Get a reference to the vertices used by this geometry
			std::string verticesName = geometrySection->readString( "vertices" );
			if (verticesName.find_first_of( '/' ) >= verticesName.size())
				verticesName = primitivesFileName + verticesName;

			//NOTE: this streamName is not used yet, it's holding it's spot for when 
			// it could be used in any refactoring of our vertex data.
			//std::string streamName = geometrySection->readString( "stream", "" );
			//if (streamName.find_first_of( '/' ) >= streamName.size())
			//	streamName = primitivesFileName + streamName;

			int numNodesValidate = 0;
			// only validate number of nodes if we have to.
			if ( validateNodes_ )
				numNodesValidate = renderSet.transformNodes_.size();
			geometry.vertices_ = VerticesManager::instance()->get( verticesName, numNodesValidate );

			if (isOK_ && (geometry.vertices_->nVertices() == 0))
			{
				ERROR_MSG( "No vertex information in \"%s\".\nUnable to load \"%s\".\n",
					verticesName.c_str(), resourceID.c_str() );
				isOK_ = false;
			}

			// Get a reference to the indices used by this geometry
			std::string indicesName = geometrySection->readString( "primitive" );
			if (indicesName.find_first_of( '/' ) >= indicesName.size())
				indicesName = primitivesFileName + indicesName;
			geometry.primitives_ = PrimitiveManager::instance()->get( indicesName );
			geometry.primitives_->calcGroupOrigins( geometry.vertices_ );

			if (isOK_ && (geometry.primitives_->indices().size() == 0))
			{
				ERROR_MSG( "No indices information in \"%s\".\nUnable to load \"%s\".\n",
					indicesName.c_str(), resourceID.c_str() );
				isOK_ = false;
			}

			// Open the primitive group descriptions
			std::vector< DataSectionPtr > primitiveGroups;
			geometrySection->openSections( "primitiveGroup", primitiveGroups );

			std::vector< DataSectionPtr >::iterator prit = primitiveGroups.begin();
			std::vector< DataSectionPtr >::iterator prend = primitiveGroups.end();

			bool bumped=false;
			// Iterate through the primitive group descriptions
			while (prit != prend)
			{
				nPGroups++;
				DataSectionPtr primitiveGroupSection = *prit;
				++prit;
				// Read the primitve group data.
				PrimitiveGroup primitiveGroup;
				primitiveGroup.groupIndex_ = primitiveGroupSection->asInt();

				if (loadingFlags == LOAD_ALL)
				{
					EffectMaterialPtr pMat = new EffectMaterial();
					ownMaterials_.push_back( pMat );
					primitiveGroup.material_ = pMat;
					pMat->load( primitiveGroupSection->openSection( "material" ) );
					if (pMat->readyToUse())
					{
						bumped = bumped || pMat->bumpMapped();
					}
				}
				geometry.primitiveGroups_.push_back( primitiveGroup );
			}

			if (isOK_ && (loadingFlags == LOAD_ALL) && bumped && !geometry.vertices_->bumpedFormat())
			{
				//warning: using bumped shader on non-bumped data.
				WARNING_MSG( "Geometry using a bumped material without having bumped information generated,"
							" re-save the model in ModelEditor: \"%s\"\n", resourceID.c_str() );
			}

			renderSet.geometry_.push_back( geometry );
		}

		renderSets_.push_back( renderSet );
	}

	// Load the bounding box
	loader.setBoundingBox( bb_ );

	// Add the BSP's bounds if one was loaded up seperately
	if ((loadingFlags == LOAD_ALL) && pFoundBSP && !bspRequiresMapping )
		bb_.addBounds( pFoundBSP->pTree()->getBB() );

	// Load portal info
	std::vector< DataSectionPtr > portals;

	root->openSections( "portal", portals );

	std::vector< DataSectionPtr >::iterator it = portals.begin();
	std::vector< DataSectionPtr >::iterator end = portals.end();

	while (it != end)
	{
		std::vector< ::Vector3 > portalPoints;
		(*it)->readVector3s( "point", portalPoints );
		if ( portalPoints.size() )
		{
			PortalData pd;

			int pdflags = 0;	// ordered as in chunk_boundary.hpp
			pdflags |= int((*it)->asString() == "heaven") << 1;
			pdflags |= int((*it)->asString() == "earth") << 2;
			pdflags |= int((*it)->asString() == "invasive") << 3;
			pd.flags( pdflags );

			for( uint32 i = 0; i < portalPoints.size(); i++ )
			{
				pd.push_back( Vector3( portalPoints[ i ] ) );
			}

			portals_.push_back( pd );
		}
		it++;
	}

	if ((loadingFlags == LOAD_ALL) && pFoundBSP)
	{
		pBSP_ = pFoundBSP;

		if (bspRequiresMapping)
		{
			loader.remapBSPMaterialFlags( *pBSP_->pTree(), ownMaterials_, bspMaterialIDs );
		}
	}

	this->useExistingNodes( NodeCatalogue::instance() );	
}

// Destruct the visual
Visual::~Visual()
{
	BW_GUARD;
	// get rid of the bsp
	s_premadeBSPsLock.grab();
	BSPMap::iterator it = s_premadeBSPs.find( resourceID_ );
	if (it != s_premadeBSPs.end())
		s_premadeBSPs.erase(it);	
	s_premadeBSPsLock.give();	

	if (isInVisualManager_)
	{
		// let the visual manager know we're gone
		isInVisualManager_ = false;
		VisualManager::instance()->del( this );
	}

	// dispose any materials we allocated
	ownMaterials_.clear();

	if (pBatcher_)
		delete pBatcher_;
}

Visual::DrawOverride* Visual::s_pDrawOverride = NULL;

#ifdef EDITOR_ENABLED

bool Visual::updateBSP()
{
	BW_GUARD;
	std::string baseName = resourceID_.substr(
		0, resourceID_.find_last_of( '.' ) );

	VisualLoader< Visual > loader( baseName );
	BSPTreePtr 		pFoundBSP;
	BSPMaterialIDs	materialIDs;
	bool bspRequiresMapping = loader.loadBSPTreeNoCache( pFoundBSP, materialIDs );

	if (!pFoundBSP) return false;

	BSPCache::instance().add( resourceID_, pFoundBSP );
	pBSP_ = pFoundBSP;

	if (bspRequiresMapping)
		loader.remapBSPMaterialFlags( *pBSP_->pTree(), ownMaterials_, materialIDs );

	return true;
}

#endif // EDITOR_ENABLED

/*
 *	This class is a helper class for the visual draw methods to set
 *	up common properties before drawing.
 */
VisualHelper::VisualHelper()
{
	BW_GUARD;
	pLC_ = new LightContainer;
	pSLC_ = new LightContainer;
}

/*
 *	This method works out wether this visual should draw or not.
 *	@param ignoreBoundingBox if true the bounding box is assumed to be in the view volume.
 *	@param bb the bounding box of the visual
 *	@return true if the visual should be drawn.
 */
bool VisualHelper::shouldDraw( bool ignoreBoundingBox, const BoundingBox& bb )
{
	BW_GUARD;
	bool shouldDraw = true;

	worldViewProjection_.multiply( rc().world(), rc().viewProjection() );
	if (!ignoreBoundingBox)
	{
		bb.calculateOutcode( worldViewProjection_ );
		shouldDraw = !bb.combinedOutcode();
	}
	return shouldDraw;
}

/*
*	This method sets up the lighting and the common renderstates used by visuals.
*	@param dynamicOnly wether to set up only dynamic lights
*	@param bb the bounding box for the visual
*	@param rendering whether this is an actual rendering pass or not
*/
void VisualHelper::start( bool dynamicOnly, const BoundingBox& bb, bool rendering )
{
	BW_GUARD;
	// Transform the bounding box to world space
    wsBB_ = bb;
	wsBB_.transformBy( rc().world() );

	// Set up the light containers
	pRCLC_ = rc().lightContainer();
	pLC_->init( pRCLC_, wsBB_, true, dynamicOnly );
	rc().lightContainer( pLC_ );

	pRCSLC_ = rc().specularLightContainer();
	pSLC_->init( pRCSLC_, wsBB_, true );
	rc().specularLightContainer( pSLC_ );

	// If we are rendering, set up common render states
	if (rendering)
	{
		FogHelper::setFog( 1, 0, D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR ); 
	}
}


/*
*	This method resets lighting and common renderstates.
*	@param rendering whether this is an actual rendering pass or not
*/
void VisualHelper::fini( bool rendering )
{
	BW_GUARD;
	// reset the light containers
	rc().lightContainer( pRCLC_ );
	rc().specularLightContainer( pRCSLC_ );

	// if we were rendering, reset common render states
	if (rendering)
	{
        FogHelper::setFogEnable( false ); 
 		 
 		FogHelper::setFogStart( rc().fogNear() ); 
 		FogHelper::setFogEnd( rc().fogFar() ); 
	}
}


/**
 *	This method draws the visual.
 *	@param ignoreBoundingBox if this value is true, this
		method will not cull the visual based on its bounding box.
 *	@param useDefaultPose set this value to false if you have
 *		manually updated the node catalogue for this visual.
 *	@return S_OK if succeeded
 */
HRESULT Visual::draw( bool ignoreBoundingBox, bool useDefaultPose )
{
	BW_GUARD_PROFILER( Visual_draw );

	if (!isOK_) return S_FALSE;

	if (s_pDrawOverride)
		return s_pDrawOverride->draw( this, ignoreBoundingBox );

	static DogWatch visual( "Visual" );
	visual.start();

	static VisualHelper helper;

	if (helper.shouldDraw(ignoreBoundingBox, bb_))
	{
		drawCount_++;

		// If the world transforms in our node catalogue haven't been updated,
		// then do so by traversing our original hierarchy and copying the
		// results into the global node catalogue.
		if (useDefaultPose)
		{
			rootNode_->traverse();
			rootNode_->loadIntoCatalogue();				
		}

		bool useStaticLighting = staticVertexColours_.exists() && staticVertexColours_->readyToRender();
		helper.start( useStaticLighting, bb_ );

		EffectVisualContext::instance().initConstants();
		EffectVisualContext::instance().staticLighting( useStaticLighting );

		if (useStaticLighting)
			staticVertexColours_->set();

		RenderSetVector::iterator rsit = renderSets_.begin();
		RenderSetVector::iterator rsend = renderSets_.end();

		// Iterate through our rendersets.
		while (rsit != rsend)
		{
			RenderSet& renderSet = *rsit;
			++rsit;

			EffectVisualContextSetter setter( &renderSet );

			GeometryVector::iterator git = renderSet.geometry_.begin();
			GeometryVector::iterator gend = renderSet.geometry_.end();

			// Iterate through the geometry
			while (git != gend)
			{
				Geometry& geometry = *git;
				++git;

				// Set our vertices.
				if (FAILED( geometry.vertices_->setVertices(rc().mixedVertexProcessing(), useStaticLighting )))
				{
					ERROR_MSG( "Moo::Visual: Couldn't set vertices for %s\n", resourceID_.c_str() );
					continue;
				}

				geometry.vertices_->clearSoftwareSkinner( );

				// Set our indices.
				if (FAILED( geometry.primitives_->setPrimitives() ))
				{
					ERROR_MSG( "Moo::Visual: Couldn't set primitives for %s\n", resourceID_.c_str() );
					continue;
				}

				// This is used to track what kind of vertices are set for the 
				// primitive group, this is so that we can avoid costly vertex set
				// operations (i.e. for morph vertices) for each material that uses
				// the same geometry
				bool transformedVertsSet = false;

				PrimitiveGroupVector::iterator pgit = geometry.primitiveGroups_.begin();
				PrimitiveGroupVector::iterator pgend = geometry.primitiveGroups_.end();

				// Iterate through our primitive groups.
				while (pgit != pgend)
				{
					PrimitiveGroup& primitiveGroup = *pgit;
					++pgit;
					EffectMaterialPtr pMat = primitiveGroup.material_;
					if (!pMat->readyToUse())
						continue;

					if (!pMat->channel())
					{
						if (!pMat->skinned() && renderSet.treatAsWorldSpaceObject_)
						{
							if (!transformedVertsSet)
							{
							geometry.vertices_->setTransformedVertices( pMat->bumpMapped(),
								renderSet.transformNodes_ );

								transformedVertsSet = true;
							}

						}
						else if (transformedVertsSet)
						{
							geometry.vertices_->setVertices(rc().mixedVertexProcessing(), useStaticLighting);
							transformedVertsSet = false;
						}

						if (pMat->begin())
						{
							for (uint32 i = 0; i < pMat->nPasses(); i++)
							{
								pMat->beginPass( i );
								geometry.primitives_->drawPrimitiveGroup( primitiveGroup.groupIndex_ );
								pMat->endPass();
							}
							pMat->end();
						}
					}
					else
					{

						VertexSnapshotPtr pVSS = geometry.vertices_->getSnapshot
							(renderSet.transformNodes_, pMat->skinned(), pMat->bumpMapped());
						float distance =
							rc().view().applyPoint( helper.worldSpaceBB().centre() + 
							geometry.primitives_->origin( primitiveGroup.groupIndex_ ) ).z;
						pMat->channel()->addItem( pVSS, geometry.primitives_, pMat,
							primitiveGroup.groupIndex_, distance, staticVertexColours_ );
					}
				}
				geometry.vertices_->clearSoftwareSkinner( );
			}
		}
		if (useStaticLighting)
			staticVertexColours_->unset();

		// Reset the streams.
		VertexBuffer::reset( Moo::UV2Stream::STREAM_NUMBER );
		VertexBuffer::reset( Moo::ColourStream::STREAM_NUMBER );

		helper.fini();
	}

	staticVertexColours_ = NULL;

	visual.stop();

	return S_OK;
}

/**
 *	This method batches the visual for delayed rendering.
 *	@param ignoreBoundingBox if this value is true, this
		method will not cull the visual based on its bounding box.
 *	@param useDefaultPose set this value to false if you have
 *		manually updated the node catalogue for this visual.
 *	@return S_OK if succeeded
 */
HRESULT Visual::batch( bool ignoreBoundingBox, bool useDefaultPose )
{
	BW_GUARD;
	// If there is a drawoverride set, ignore the batching and
	// draw using the override in stead.
	if (s_pDrawOverride)
		return s_pDrawOverride->draw( this, ignoreBoundingBox );

	/*
	 *	Set up watchers for the batching.
	 */
	static bool firstTime = true;
	static bool useBatcher = true;
	if (firstTime)
	{
		firstTime = false;
		MF_WATCH( "Visual/Use batcher", useBatcher, Watcher::WT_READ_WRITE,
			"Use batching system for rendering visuals with the batch flag set" );
	}

	// If batching is disabled or the watcher tells us not to use
	// batching, draw in stead.
	if (!useBatcher || disableBatching_)
		return this->draw( ignoreBoundingBox, useDefaultPose );

	// If no batcher object is present, create a new one.
	if (!pBatcher_)
	{
		pBatcher_ = new VisualBatcher();

		// iterate over the visual rendersets and collect them for batching.
		RenderSetVector::iterator rsit = renderSets_.begin();
		RenderSetVector::iterator rsend = renderSets_.end();
		while (rsit != rsend)
		{
			RenderSet& renderSet = *rsit;
			++rsit;

			GeometryVector::iterator git = renderSet.geometry_.begin();
			GeometryVector::iterator gend = renderSet.geometry_.end();

			// Iterate over the geometry sections and collect them for batching.
			while (git != gend)
			{
				Geometry& geometry = *git;
				++git;
				pBatcher_->addGeometry( &geometry, &renderSet );
			}
		}
	}

	static DogWatch batch( "Batch" );
	batch.start();

	static VisualHelper helper;

	// check if we should draw this visual this frame.
	if (helper.shouldDraw(ignoreBoundingBox, bb_))
	{
		// Add this batcher to the batchers list for this frame.
		if (!rc().frameDrawn(pBatcher_->batchedThisFrame_))
		{
			s_batches_.push_back( this );
		}

		drawCount_++;

		// If the world transforms in our node catalogue haven't been updated,
		// then do so by traversing our original hierarchy and copying the
		// results into the global node catalogue.
		if (useDefaultPose)
		{
			rootNode_->traverse();
			rootNode_->loadIntoCatalogue();
		}

		// Set up the common properties for batching
		bool useStaticLighting = staticVertexColours_.exists() && staticVertexColours_->readyToRender();
		helper.start( useStaticLighting, bb_, false );

		// Init the effectvisual context
		EffectVisualContext::instance().initConstants();
		EffectVisualContext::instance().staticLighting( useStaticLighting );

		// Do the actual batching
		pBatcher_->batch( staticVertexColours_ );

		// We are done
		helper.fini( false );
	}

	staticVertexColours_ = NULL;

	batch.stop();

	return S_OK;
}

HRESULT Visual::drawBatch()
{
	BW_GUARD;
	if (pBatcher_)
	{
		static DogWatch batchDraw( "Drawbatches" );
		batchDraw.start();
		static VisualHelper helper;
		helper.shouldDraw( true, bb_);
		helper.start( false, bb_, true );
		EffectVisualContext::instance().initConstants();
		EffectVisualContext::instance().staticLighting( false );

		pBatcher_->draw();
		helper.fini();
		batchDraw.stop();
	}
	return S_OK;
}

HRESULT Visual::drawBatches()
{
	BW_GUARD;
	VectorNoDestructor< Visual* >::iterator it = s_batches_.begin();
	VectorNoDestructor< Visual* >::iterator end = s_batches_.end();
	while (it != end)
	{
		(*it)->drawBatch();
		// invalidate the batch token, just in case we re-batch this frame.
		(*it)->pBatcher_->batchedThisFrame_--;
		it++;
	}
	s_batches_.clear();
	return S_OK;
}

/**
 *	This method tells the visual that its nodes will already have been
 *	traversed when it gets drawn, and that the transforms it should use
 *	for drawing them will not be stored in its own copy of the nodes,
 *	but rather in those ones provided by the given catalogue.
 *	The identifiers of the nodes currently used by each RenderSet
 *	is used to look up which node they should be sourced from in
 *	the provided node catalogue.
 *
 *	@note: After this call, the model's node hierarchy is still available
 *	through the rootNode accessor, however the transforms in those nodes
 *	will no longer be used to draw the visual - only the nodes' names will
 *	be the same.
 */
void Visual::useExistingNodes( NodeCatalogue& nodeCatalogue )
{
	BW_GUARD;
	NodeCatalogue::grab();	

	RenderSetVector::iterator rsit = renderSets_.begin();
	RenderSetVector::iterator rsend = renderSets_.end();

	while (rsit != rsend)
	{
		RenderSet& renderSet = *rsit;
		++rsit;

		for (NodePtrVector::iterator nit = renderSet.transformNodes_.begin();
			nit != renderSet.transformNodes_.end();
			++nit)
		{
			*nit = nodeCatalogue[ (*nit)->identifier() ];
			MF_ASSERT_DEV( *nit );
		}
	}

	NodeCatalogue::give();
}

/**
 *	Add an animation to the visuals animation list.
 *
 *  @param animResourceID the resource identifier of the animation to load.
 *
 *  @return pointer to the animation that was added
 *
 */
AnimationPtr Visual::addAnimation( const std::string& animResourceID )
{
	BW_GUARD;
	AnimationPtr anim = AnimationManager::instance().get( animResourceID, rootNode_ );
	if (anim)
	{
		animations_.push_back( anim );
	}
	return anim;
}


/**
 *	Find an animation in the visuals animation list.
 *
 *	@param identifier the animation identifier
 *
 *	@return a pointer to the animation
 *
 */
AnimationPtr Visual::findAnimation( const std::string& identifier ) const
{
	BW_GUARD;
	AnimationVector::const_iterator it = animations_.begin();
	AnimationVector::const_iterator end = animations_.end();
	while (it != end)
	{
		if ((*it)->identifier() == identifier)
		{
			return *it;
		}

		if ((*it)->internalIdentifier() == identifier)
		{
			return *it;
		}
		++it;
	}
	return NULL;
}

/**
 *	This method returns the number of triangles used by this geometry.
 */
uint32 Visual::Geometry::nTriangles() const
{
	BW_GUARD;
	uint32 nTriangles = 0;

	for( uint32 i = 0; i < primitives_->nPrimGroups(); i++ )
	{
		const Moo::PrimitiveGroup& pg = primitives_->primitiveGroup( i );
		nTriangles += pg.nPrimitives_;
	}
	return nTriangles;
}

/**
 *	This method overrides all materials that have the given materialIdentifier
 *	with the given material.
 */
void Visual::overrideMaterial( const std::string& materialIdentifier, const EffectMaterial& mat )
{
	BW_GUARD;
	RenderSetVector::iterator rsit = renderSets_.begin();
	RenderSetVector::iterator rsend = renderSets_.end();

	// Iterate through our rendersets.
	while (rsit != rsend)
	{
		RenderSet& renderSet = *rsit;
		++rsit;

		GeometryVector::iterator git = renderSet.geometry_.begin();
		GeometryVector::iterator gend = renderSet.geometry_.end();

		// Iterate through our geometries
		while (git != gend)
		{
			Geometry& geometry = *git;
			git++;

			PrimitiveGroupVector::iterator pgit = geometry.primitiveGroups_.begin();
			PrimitiveGroupVector::iterator pgend = geometry.primitiveGroups_.end();

			// Iterate through our primitive groups.
			while (pgit != pgend)
			{
				PrimitiveGroup& primitiveGroup = *pgit;
				++pgit;


				if (primitiveGroup.material_->identifier() == materialIdentifier)
                {
					*primitiveGroup.material_ = mat;
                }
			}
		}
	}

}


/**
 *	This method gathers all the uses of the given material identifier in
 *	this visual, and puts pointers to their pointers into the given vector.
 *	It also optionally sets a pointer to the initial material of the given
 *	identifier into the ppOriginal argument (when non-NULL)
 *	It returns the number of material pointers found (the size of the vector)
 *
 *	If materialIdentifier is a wildcard "*", this function returns all materials.
 */
int Visual::gatherMaterials( const std::string & materialIdentifier,
	std::vector< PrimitiveGroup * > & primGroups, ConstEffectMaterialPtr * ppOriginal )
{
	BW_GUARD;
	int	mindex = 0;
	primGroups.clear();

	bool everyMaterial = (materialIdentifier == "*");

	RenderSetVector::iterator rsit = renderSets_.begin();
	RenderSetVector::iterator rsend = renderSets_.end();

	// Iterate through our rendersets.
	while (rsit != rsend)
	{
		RenderSet& renderSet = *rsit;
		++rsit;

		GeometryVector::iterator git = renderSet.geometry_.begin();
		GeometryVector::iterator gend = renderSet.geometry_.end();

		// Iterate through our geometries
		while (git != gend)
		{
			Geometry& geometry = *git;
			git++;

			PrimitiveGroupVector::iterator pgit = geometry.primitiveGroups_.begin();
			PrimitiveGroupVector::iterator pgend = geometry.primitiveGroups_.end();

			// Iterate through our primitive groups.
			while (pgit != pgend)
			{
				PrimitiveGroup& primitiveGroup = *pgit;
				++pgit;

				if (everyMaterial ||
					ownMaterials_[mindex]->identifier() == materialIdentifier)
				{
					primGroups.push_back( &primitiveGroup );
					if (ppOriginal)
					{
						*ppOriginal = ownMaterials_[mindex];
						ppOriginal = NULL;
					}
				}

				mindex++;
			}
		}
	}

	return primGroups.size();
}


/**
 *	This method collates a list of all the visual's original material ptrs.
 *
 *	@param materials	a vector of material pointers to fill.
 *
 *	@return int	The number of materials found
 */
int	Visual::collateOriginalMaterials(
		std::vector< EffectMaterialPtr > & materials )
{
	BW_GUARD;
	materials.clear();

	std::vector<EffectMaterialPtr>::iterator it = ownMaterials_.begin();
	std::vector<EffectMaterialPtr>::iterator end = ownMaterials_.end();

	while ( it != end )
	{
		materials.push_back( *it++ );
	}

	return ( materials.size() );
}

namespace
{
	template< int skipSize >
	void memcpy_strided( void* dest, void* src, size_t elements, size_t elementSize )
	{
		BW_GUARD;
		uint8* d_ptr = static_cast<uint8*>( dest );
		uint8* s_ptr = static_cast<uint8*>( src );
		uint8* end = d_ptr + (elementSize * elements);

		while ( d_ptr != end )
		{
			memcpy( d_ptr, s_ptr, elementSize );

			d_ptr += elementSize;
			s_ptr += (elementSize + skipSize);
		}
	}

	template<>
	void memcpy_strided<0>( void* dest, void* src, size_t elements, size_t elementSize )
	{
		BW_GUARD;
		memcpy( dest, src, elements * elementSize );
	}

	template< class SrcFormat, class DstFormat >
	bool copyData( PrimitivePtr& prims, VerticesPtr& verts,
					DstFormat*& retPVertices, IndicesHolder& retPIndices,
					uint32 & retNumVertices, uint32 & retNumIndices )
	{
		BW_GUARD;
		Moo::VertexBuffer vb = verts->vertexBuffer();

		if ( vb.valid() )
		{
			uint32 nVertices = verts->nVertices();
			Moo::SimpleVertexLock vl( vb, 0, nVertices*sizeof(SrcFormat), 0 );
			if( vl )
			{
				uint32 nIndices = 0;
				D3DPRIMITIVETYPE pt = prims->primType();
	
	
				for (uint32 i = 0; i < prims->nPrimGroups(); i++)
				{
					nIndices += prims->primitiveGroup( i ).nPrimitives_ * 3;
				}
	
				retPVertices = new DstFormat[ verts->nVertices() ];
				retPIndices.setSize( nIndices, prims->indices().format() );
				retNumVertices = verts->nVertices();
				retNumIndices = nIndices;
	
				memcpy_strided< sizeof(SrcFormat) - sizeof(DstFormat) >( retPVertices, vl, nVertices, sizeof(DstFormat) );
	
				retPIndices.copy( prims->indices(), nIndices );
				return retPIndices.valid();
			}
		}
		return false;
	}

}

/**
 *	This method creates a copy of this visual.
 *	The copy is stored in a newly allocated array of vertices and indices.
 *
 *	@param retPVertices		the address of a pointer to vertices.  This will
 *							be set to point to the new array of vertices.
 *	@param retPIndices		the address of a pointer to indices.  This will
 *							be set to point to the new array of indices.
 *	@param retNumVertices	an integer reference that will be set to the number
 *							of vertices in the new array of vertices
 *	@param retNumIndices	an integer reference that will be set to the number
 *							of indices in the new array of indices
 *	@param material			The primary material associated with the copied
 *							Visual.
 *
 *	@return true if the copy could be made, of false if the method fails.
 */
bool Visual::createCopy( Moo::VertexXYZNUV *& retPVertices, IndicesHolder& retPIndices,
						uint32 & retNumVertices, uint32 & retNumIndices, EffectMaterialPtr & material )
{
	BW_GUARD;
	bool success = false;

	retPVertices = NULL;
	retNumVertices = 0;
	retNumIndices = 0;

	// Iterate through our rendersets.
	RenderSetVector::iterator rsit = renderSets_.begin();
	RenderSetVector::iterator rsend = renderSets_.end();

	while (rsit != rsend)
	{
		RenderSet& renderSet = *rsit;
		++rsit;

		// Iterate through the geometry
		GeometryVector::iterator git = renderSet.geometry_.begin();
		GeometryVector::iterator gend = renderSet.geometry_.end();

		while( git != gend )
		{
			Geometry& geometry = *git;
			++git;

			// Just get the one primitive group
			VerticesPtr verts = geometry.vertices_;
			PrimitivePtr prims = geometry.primitives_;

			material = geometry.primitiveGroups_[0].material_;

			if ( geometry.vertices_->format() == "xyznuv" )
			{
				success = copyData<Moo::VertexXYZNUV, Moo::VertexXYZNUV>( prims, verts,
					retPVertices, retPIndices,
					retNumVertices, retNumIndices );
			}

			if ( geometry.vertices_->format() == "xyznuvtb" )
			{
				success =copyData<Moo::VertexXYZNUVTBPC, Moo::VertexXYZNUV>( prims, verts,
					retPVertices, retPIndices,
					retNumVertices, retNumIndices );
			}
		}
	}

	return success;
}


/**
 *	This method returns the number of vertices in a visual.
 */
uint32 Visual::nVertices() const
{
	BW_GUARD;
	uint32 retNumVertices = 0;

	// Iterate through our rendersets.
	RenderSetVector::const_iterator rsit = renderSets_.begin();
	RenderSetVector::const_iterator rsend = renderSets_.end();

	while (rsit != rsend)
	{
		const RenderSet& renderSet = *rsit;
		++rsit;

		// Iterate through the geometry
		GeometryVector::const_iterator git = renderSet.geometry_.begin();
		GeometryVector::const_iterator gend = renderSet.geometry_.end();

		while( git != gend )
		{
			const Geometry& geometry = *git;
			++git;
			retNumVertices += geometry.vertices_->nVertices();
		}
	}

	return retNumVertices;
}


/**
 *	This method returns the number of triangles in a visual.
 */
uint32 Visual::nTriangles() const
{
	BW_GUARD;
	uint32 num = 0;

	// Iterate through our rendersets.
	RenderSetVector::const_iterator rsit = renderSets_.begin();
	RenderSetVector::const_iterator rsend = renderSets_.end();

	while (rsit != rsend)
	{
		const RenderSet& renderSet = *rsit;
		++rsit;

		// Iterate through the geometry
		GeometryVector::const_iterator git = renderSet.geometry_.begin();
		GeometryVector::const_iterator gend = renderSet.geometry_.end();

		while( git != gend )
		{
			const Geometry& geometry = *git;
			++git;
			num += geometry.nTriangles();
		}
	}

	return num;
}


/**
 *	This method returns vital information about primitive group 0.  That is,
 *	The vertices, primitives and the material.
 *
 *	Currently this method is only used by MeshParticleRenderer's BonedVisualMPT
 *	class.
 */
bool Visual::primGroup0( Moo::VerticesPtr & retVerts, Moo::PrimitivePtr & retPrim, PrimitiveGroup* &retPPrimGroup )
{
	BW_GUARD;
	// get render set #0
	if ( renderSets_.size() > 0 )
	{
		RenderSet& renderSet = renderSets_[0];

		// get geometry #0
		if ( renderSet.geometry_.size() > 0 )
		{
			// Just get the one primitive group
			Geometry& geometry = renderSet.geometry_[0];
			retVerts = geometry.vertices_;
			retPrim = geometry.primitives_;
			retPPrimGroup = &geometry.primitiveGroups_[0];
			return true;
		}
	}

	return false;
}


/**
 *	Draw the visual's primitives and do nothing else.
 *	Assumes : vertex / pixel shader set up
 *			  no nodes involved (i.e. not skinned)
 */
void Visual::justDrawPrimitives()
{
	BW_GUARD;
	RenderSetVector::iterator rsit = renderSets_.begin();
	RenderSetVector::iterator rsend = renderSets_.end();

	// Iterate through our rendersets.
	while (rsit != rsend)
	{
		RenderSet& renderSet = *rsit;
		++rsit;

		GeometryVector::iterator git = renderSet.geometry_.begin();
				GeometryVector::iterator gend = renderSet.geometry_.end();

		// Iterate through the geometry
		while( git != gend )
		{
			Geometry& geometry = *git;
			++git;

			// Set our vertices.
			if (FAILED( geometry.vertices_->setVertices(false) ))
			{
				ERROR_MSG( "Moo::Visual: Couldn't set vertices for %s\n", resourceID_.c_str() );
				continue;
			}

			// Set our indices.
			if (FAILED( geometry.primitives_->setPrimitives() ))
			{
				ERROR_MSG( "Moo::Visual: Couldn't set primitives for %s\n", resourceID_.c_str() );
				continue;
			}

			PrimitiveGroupVector::iterator pgit = geometry.primitiveGroups_.begin();
			PrimitiveGroupVector::iterator pgend = geometry.primitiveGroups_.end();

			// Iterate through our primitive groups.
			while (pgit != pgend)
			{
				PrimitiveGroup& primitiveGroup = *pgit;
				++pgit;

				geometry.primitives_->drawPrimitiveGroup( primitiveGroup.groupIndex_ );
			}
		}
	}
}

namespace BSPTreeHelper
{
	/**
	 * Create array of vertices from given BSP tree.
	 */
	void createVertexList( 
		const ::BSPTree &				source,
		std::vector< Moo::VertexXYZL >& verts, 
		uint32							colour )
	{
		BW_GUARD;
		// Count triangles, ignoring NOCOLLIDE triangles
		uint32 nVerts = 0;
		const  RealWTriangleSet& triangles = source.triangles();

		for( RealWTriangleSet::const_iterator it = triangles.begin();
			it != triangles.end();
			it++)
		{
			if ( !(it->flags() & TRIANGLE_NOCOLLIDE) )
				nVerts += 3;
		}
		
		if ( nVerts > 0 )
		{
			// Resize vector to fit
			verts.resize( nVerts );

			// Copy triangles in
			uint32 i = 0;

			for( RealWTriangleSet::const_iterator it = triangles.begin();
				it != triangles.end();
				it++)
			{
				const WorldTriangle & tri = *it;
				
				if ( !( tri.flags() & TRIANGLE_NOCOLLIDE ) )
				{
 					verts[i].colour_	= colour;
					verts[i++].pos_		= tri.v0();

					verts[i].colour_	= 0xFFFFFFFF;
					verts[i++].pos_		= tri.v1();

					verts[i].colour_	= colour;
					verts[i++].pos_		= tri.v2();
				}
			}

			MF_ASSERT_DEBUG( i == verts.size() );
		}
	}
}

}	// namespace Moo


/*visual.cpp*/
