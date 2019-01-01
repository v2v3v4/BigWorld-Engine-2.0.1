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
#include "worldeditor/world/items/editor_chunk_tree.hpp"
#include "worldeditor/project/project_module.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/world/item_info_db.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/misc/cvswrapper.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_light.hpp"
#include "appmgr/options.hpp"
#include "appmgr/module_manager.hpp"
#include "common/tools_common.hpp"
#include "speedtree/speedtree_config.hpp"
#include "speedtree/speedtree_collision.hpp"
#include "speedtree/billboard_optimiser.hpp"
#include "speedtree/speedtree_renderer.hpp"
#include "romp/fog_controller.hpp"
#include "romp/static_light_values.hpp"
#include "romp/static_light_fashion.hpp"
#include "model/super_model.hpp"
#include "physics2/bsp.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/resource_cache.hpp"
#include "cstdmf/debug.hpp"


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )


static AutoConfigString s_notFoundModel( "system/notFoundModel" );	


namespace
{
	// Watcher helper class for turning on and off light debugging
	class LightDebugging
	{
	public:
		static bool enabled()
		{
			BW_GUARD;

			return instance().enabled_;
		}
	private:
		LightDebugging() : 
		   enabled_( false )
		{
			MF_WATCH( "SpeedTree/Debug Light Positions", enabled_, Watcher::WT_READ_WRITE,
				"Display debug lines from the selected tree to the lights affecting it" );
		}
		static LightDebugging& instance()
		{
			static LightDebugging s_inst;
			return s_inst;
		}
		bool enabled_;
	};

}

// -----------------------------------------------------------------------------
// Section: EditorChunkTree
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EditorChunkTree::EditorChunkTree() :
	hasPostLoaded_( false ),
	castsShadow_( true ),
	bspBB_( BoundingBox::s_insideOut_ )
{
	BW_GUARD;

	// load missing model (static)
	missingTreeModel_ = Model::get( s_notFoundModel.value() );
	ResourceCache::instance().addResource( missingTreeModel_ );
}


/**
 *	Destructor.
 */
EditorChunkTree::~EditorChunkTree()
{
}


/**
 *	overridden edShouldDraw method
 */
bool EditorChunkTree::edShouldDraw()
{
	BW_GUARD;

	if( !ChunkTree::edShouldDraw() )
		return false;
	
	return OptionsScenery::visible();
}

/**
 *	overridden draw method
 */
void EditorChunkTree::draw()
{
	BW_GUARD;

	if( !edShouldDraw() )
		return;

	if (!hasPostLoaded_)
	{
		edPostLoad();
		hasPostLoaded_ = true;
	}

	int drawBspFlag = WorldManager::instance().drawBSP();
	bool projectModule = ProjectModule::currentInstance() == ModuleManager::instance().currentModule();
	bool drawBsp = drawBspFlag != 0 && !projectModule;
	std::string bspName = this->ChunkTree::loadFailed() ? s_notFoundModel : desc_;

	// Load up the bsp tree if needed
	if ((drawBsp || WorldManager::instance().drawSelection() ) && tree_.get())
	{
		if (!bspCreated( bspName ))
		{
			// no vertices loaded yet, create some
			const BSPTree * tree = this->bspTree();

			if (tree)
			{
				Moo::Colour colour((float)rand() / (float)RAND_MAX,
									(float)rand() / (float)RAND_MAX,
									(float)rand() / (float)RAND_MAX,
									1.f);
				std::vector<Moo::VertexXYZL> verts;

				Moo::BSPTreeHelper::createVertexList( *tree, verts, colour);

				addBsp( verts, bspName );
			}
		}
	}

	if ((drawBsp || WorldManager::instance().drawSelection() ))
	{
		if (bspCreated( bspName ))
		{
			//set the transforms
			Matrix transform;
			transform.multiply(edTransform(), chunk()->transform());
			Moo::rc().device()->SetTransform( D3DTS_WORLD, &transform );
			Moo::rc().device()->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
			Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );

			Moo::rc().setPixelShader( NULL );
			Moo::rc().setVertexShader( NULL );
			Moo::rc().setFVF( Moo::VertexXYZL::fvf() );
			Moo::rc().setRenderState( D3DRS_ALPHATESTENABLE, FALSE );
			Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
			Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, TRUE );
			Moo::rc().setRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
			Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
			Moo::rc().fogEnabled( false );

			if (WorldManager::instance().drawSelection())
			{
				Moo::rc().setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
				Moo::rc().setTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
				Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
				Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );

				WorldManager::instance().registerDrawSelectionItem( this );
			}
			else
			{
				Moo::rc().setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
				Moo::rc().setTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
				Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
			}
			Moo::rc().setTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
			Moo::rc().setTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

			this->drawBsp( bspName );
		}
	}
	else
	{
		if (!this->ChunkTree::loadFailed())
		{
			bool drawRed = OptionsMisc::readOnlyVisible() &&
				!EditorChunkCache::instance( *pChunk_ ).edIsWriteable();
			bool drawFrozen = this->edFrozen() && OptionsMisc::frozenVisible();
		
			bool drawColoured = drawRed || drawFrozen;

			if (!projectModule)
			{
				if( drawRed )
				{
					WorldManager::instance().setReadOnlyFog();
				}
				else if ( drawFrozen )
				{
					WorldManager::instance().setFrozenFog();
				}

				this->ChunkTree::tree_->allowBatching( !drawColoured );
			}
			else
			{
				this->ChunkTree::tree_->allowBatching( true );
				FogController::instance().commitFogToDevice();
			}


			if (LightDebugging::enabled() && WorldManager::instance().isItemSelected(this))
				speedtree::SpeedTreeRenderer::enableLightLines(true);

			this->ChunkTree::draw();

			speedtree::SpeedTreeRenderer::enableLightLines(false);
			
			if( drawColoured && !projectModule )
			{
				FogController::instance().commitFogToDevice();
			}
		}
		else 
		{
			// draw missing model
			if (missingTreeModel_)
			{
				Moo::rc().push();
				Moo::rc().preMultiply(edTransform());
				missingTreeModel_->dress();
				missingTreeModel_->draw( true );
				Moo::rc().pop();
			}
		}			
	}
}

struct ErrorCallback
{
	static void printError(
		const std::string & fileName,
		const std::string & errorMsg)
	{
		BW_GUARD;

		std::string msg = fileName + ":" + errorMsg;
		WorldManager::instance().addError(
			chunk, self, "%s", msg.substr(0, 255).c_str());
	}
	
	static Chunk           * chunk;
	static EditorChunkTree * self;
};

Chunk * ErrorCallback::chunk = NULL;
EditorChunkTree * ErrorCallback::self = NULL;

/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkTree::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;

	ErrorCallback::chunk = pChunk;
	ErrorCallback::self  = this;
	speedtree::setErrorCallback(&ErrorCallback::printError);

	edCommonLoad( pSection );

	castsShadow_ = pSection->readBool( "editorOnly/castsShadow", true );

	this->pOwnSect_ = pSection;	
	if (this->ChunkTree::load(pSection, pChunk))
	{
		this->desc_ = BWResource::getFilename(this->ChunkTree::filename());
		this->desc_ = BWResource::removeExtension(this->desc_);
		this->hasPostLoaded_ = false;	
	}
	else
	{
		this->desc_ = BWResource::getFilename(pSection->readString("spt"));
		this->desc_ = BWResource::removeExtension(this->desc_);

		// set BSP for missing model
		if (missingTreeModel_)
		{
			const BSPTree * bsp = missingTreeModel_->decompose();
			this->BaseChunkTree::setBoundingBox( missingTreeModel_->boundingBox() );
			this->BaseChunkTree::setBSPTree( bsp );

			if (!bspCreated( s_notFoundModel ))
			{
				static const Moo::Colour colour(0.0f, 1.0f, 0.0f, 1.f);
				std::vector<Moo::VertexXYZL> verts;

				Moo::BSPTreeHelper::createVertexList( *bsp, verts, colour);

				addBsp( verts, s_notFoundModel );
			}
		}
		
		std::string msg = LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/ERROR_LOADING_TREE",
			this->ChunkTree::lastError() );
		WorldManager::instance().addError(
			pChunk, this, "%s",  
			msg.substr(0, 255).c_str());
	
		this->hasPostLoaded_ = true;
	}
	return true;
}

void EditorChunkTree::edPostLoad()
{
}

/**
 * We just loaded up with srcItems lighting data, create some new stuff of our
 * own
 */
void EditorChunkTree::edPostClone( EditorChunkItem* srcItem )
{
	BW_GUARD;

	StaticLighting::markChunk( pChunk_ );

	BoundingBox bb = BoundingBox::s_insideOut_;
	edBounds( bb );
	bb.transformBy( edTransform() );
	bb.transformBy( chunk()->transform() );
	WorldManager::instance().markTerrainShadowsDirty( bb );

	EditorChunkItem::edPostClone( srcItem );
	EditorChunkBspHolder::postClone();
}

void EditorChunkTree::edPostCreate()
{
	BW_GUARD;

	StaticLighting::markChunk( pChunk_ );

	BoundingBox bb = BoundingBox::s_insideOut_;
	edBounds( bb );
	bb.transformBy( edTransform() );
	bb.transformBy( chunk()->transform() );
	WorldManager::instance().markTerrainShadowsDirty( bb );

	EditorChunkItem::edPostCreate();
}

/**
 *	This method does extra stuff when this item is tossed between chunks.
 *
 *	It updates its datasection in that chunk.
 */
void EditorChunkTree::toss( Chunk * pChunk )
{
	BW_GUARD;

	if (pChunk_ != NULL && pOwnSect_)
	{
		EditorChunkCache::instance( *pChunk_ ).
			pChunkSection()->delChild( pOwnSect_ );
		pOwnSect_ = NULL;
	}

	this->ChunkTree::toss( pChunk );

	if (pChunk_ != NULL && !pOwnSect_)
	{
		pOwnSect_ = EditorChunkCache::instance( *pChunk_ ).
			pChunkSection()->newSection( "speedtree" );
		this->edSave( pOwnSect_ );
	}

	// Update the database
	ItemInfoDB::instance().toss( this, pChunk != NULL );
}


/**
 *	Save to the given section
 */
bool EditorChunkTree::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	pSection->writeString( "spt", this->ChunkTree::filename() );
	pSection->writeInt( "seed", this->ChunkTree::seed() );
	pSection->writeMatrix34( "transform", this->transform() );

	pSection->writeBool( "reflectionVisible", reflectionVisible_ );

	pSection->writeBool( "editorOnly/castsShadow", castsShadow_ );

	return true;
}

/**
 *	Called when our containing chunk is saved
 */
void EditorChunkTree::edChunkSave()
{
}

/**
 *	Called when our containing chunk is saved; save the lighting info
 */
void EditorChunkTree::edChunkSaveCData(DataSectionPtr cData)
{
}

/**
 *	This method sets this item's transform for the editor
 *	It takes care of moving it into the right chunk and recreating the
 *	collision scene and all that
 */
bool EditorChunkTree::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	// it's permanent, so find out where we belong now
	BoundingBox lbb( Vector3(0.f,0.f,0.f), Vector3(1.f,1.f,1.f) );
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyPoint(
		(lbb.minBounds() + lbb.maxBounds()) * 0.5f ) );
	if (pNewChunk == NULL) return false;

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		this->setTransform( m );
		this->syncInit();
		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;


	// Calc the old world space BB, so we can update the terrain shadows
	BoundingBox oldBB = BoundingBox::s_insideOut_;
	edBounds( oldBB );
	oldBB.transformBy( edTransform() );
	oldBB.transformBy( chunk()->transform() );

	// ok, accept the transform change then
	//transform_ = m;
	Matrix transform = this->transform();
	transform.multiply( m, pOldChunk->transform() );
	transform.postMultiply( pNewChunk->transformInverse() );
	this->setTransform( transform );

	// Calc the new world space BB, so we can update the terrain shadows
	BoundingBox newBB = BoundingBox::s_insideOut_;
	edBounds( newBB );
	newBB.transformBy( edTransform() );
	newBB.transformBy( pNewChunk->transform() );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	WorldManager::instance().markTerrainShadowsDirty( oldBB );
	WorldManager::instance().markTerrainShadowsDirty( newBB );

	edMove( pOldChunk, pNewChunk );

	// Recalculate static lighting in the old and new chunks
	StaticLighting::markChunk( pNewChunk );
	StaticLighting::markChunk( pOldChunk );

	if (pOldChunk != pNewChunk )
	{
		edSave( pOwnSect() );
	}
	this->syncInit();
	return true;
}


void EditorChunkTree::edPreDelete()
{
	BW_GUARD;

	StaticLighting::markChunk( pChunk_ );

	BoundingBox bb = BoundingBox::s_insideOut_;
	edBounds( bb );
	bb.transformBy( edTransform() );
	bb.transformBy( chunk()->transform() );
	WorldManager::instance().markTerrainShadowsDirty( bb );
	EditorChunkItem::edPreDelete();
}


/**
 *	Get the bounding box
 */
void EditorChunkTree::edBounds( BoundingBox& bbRet ) const
{
	BW_GUARD;

	bbRet = this->boundingBox();
}


/**                                                                             
 *  Get the bounding box used for showing the selection                         
 */                                                                             
void EditorChunkTree::edSelectedBox( BoundingBox& bbRet ) const                 
{
	BW_GUARD;

    if ( bspTree() && bspTree()->size() > 0 &&                                  
        Options::getOptionInt( "bspBoundingBox", 1 ) )                          
    {                                                                           
        if ( bspBB_ == BoundingBox::s_insideOut_ )                              
        {                                                                       
            // bsp bounding box uninitialised, so initialise.                   
            const RealWTriangleSet& tris = bspTree()->triangles();              
            for ( RealWTriangleSet::const_iterator i = tris.begin();            
                i != tris.end(); ++i )                                          
            {                                                                   
                bspBB_.addBounds( (*i).v0() );                                  
                bspBB_.addBounds( (*i).v1() );                                  
                bspBB_.addBounds( (*i).v2() );                                  
            }                                                                   
        }                                                                       
        bbRet = bspBB_;                                                         
    }                                                                           
    else                                                                        
    {                                                                           
        this->edBounds( bbRet );                                                
    }                                                                           
}                                                                               


/**
 *	This method returns whether or not this tree should cast a shadow.
 *
 *  @return		Returns whether or not this tree should cast a shadow
 */
bool EditorChunkTree::edAffectShadow() const
{
	return castsShadow_;
}


/**
 *	This method adds this item's properties to the given editor
 */
bool EditorChunkTree::edEdit( GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	MatrixProxy * pMP = new ChunkItemMatrix( this );
	editor.addProperty( new ChunkItemPositionProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/POSITION"), pMP, this ) );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/ROTATION"), pMP ) );
	editor.addProperty( new GenScaleProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/SCALE"), pMP ) );

	// can affect shadow?
	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/CASTS_SHADOW"),
		new AccessorDataProxy< EditorChunkTree, BoolProxy >(
			this, "castsShadow", 
			&EditorChunkTree::getCastsShadow, 
			&EditorChunkTree::setCastsShadow ) ) );

	StaticTextProperty * pProp = new StaticTextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/FILENAME"),
		new ConstantDataProxy< StringProxy >( this->ChunkTree::filename() ) );
	editor.addProperty(pProp);
		
	std::stringstream seed;
	seed << EditorChunkTree::getSeed() <<
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/CHANGE_IN_SPT_CAD");
	editor.addProperty( new StaticTextProperty(
		"seed", new ConstantDataProxy<StringProxy>(seed.str()) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/REFLECTION_VISIBLE"),
		new AccessorDataProxy< EditorChunkTree, BoolProxy >(
			this, "reflectionVisible",
			&EditorChunkTree::getReflectionVis,
			&EditorChunkTree::setReflectionVis ) ) );

	return true;
}


/**
 *	Find the drop chunk for this item
 */
Chunk * EditorChunkTree::edDropChunk( const Vector3 & lpos )
{
	BW_GUARD;

	Vector3 npos = pChunk_->transform().applyPoint( lpos );

	Chunk * pNewChunk = NULL;

	pNewChunk = pChunk_->space()->findChunkFromPointExact( npos );

	if (pNewChunk == NULL)
	{
		ERROR_MSG( "Cannot move %s to (%f,%f,%f) "
			"because it is not in any loaded chunk!\n",
			this->edDescription().c_str(), npos.x, npos.y, npos.z );
		return NULL;
	}

	return pNewChunk;
}

std::string EditorChunkTree::edDescription()
{
	return desc_;
}


/**
 *	This returns the number of triangles of this tree.
 */
int EditorChunkTree::edNumTriangles() const
{
	BW_GUARD;

#if SPEEDTREE_SUPPORT
	return tree_.get() ? tree_->numTris() : 0;
#else
	return 0;
#endif // SPEEDTREE_SUPPORT
}


/**
 *	This returns the number of primitive groups of this tree.
 */
int EditorChunkTree::edNumPrimitives() const
{
	BW_GUARD;

#if SPEEDTREE_SUPPORT
	return tree_.get() ? tree_->numPrims() : 0;
#else
	return 0;
#endif // SPEEDTREE_SUPPORT
}


/**
 *	This returns the file for the asset of this tree.
 */
std::string EditorChunkTree::edAssetName() const
{
	BW_GUARD;

	return BWResource::getFilename( filename() );
}


/**
 *	This returns the file for the asset of this tree.
 */
std::string EditorChunkTree::edFilePath() const
{
	return filename();
}


Vector3 EditorChunkTree::edMovementDeltaSnaps()
{
	BW_GUARD;

	return EditorChunkItem::edMovementDeltaSnaps();
}

float EditorChunkTree::edAngleSnaps()
{
	BW_GUARD;

	return EditorChunkItem::edAngleSnaps();
}

unsigned long EditorChunkTree::getSeed() const
{
	BW_GUARD;

	return this->ChunkTree::seed();
}

bool EditorChunkTree::setSeed( const unsigned long & seed )
{
	BW_GUARD;

	bool success = this->ChunkTree::seed(seed);
	if (!success)
	{
		std::string msg = LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_TREE/COULD_NOT_CHANGE_TREE_SEED",
			this->ChunkTree::lastError() );
		WorldManager::instance().addCommentaryMsg(msg.substr(0,255));
	}	
	return success;
}

bool EditorChunkTree::getCastsShadow() const
{
	return castsShadow_;
}

bool EditorChunkTree::setCastsShadow( const bool & castsShadow )
{
	BW_GUARD;

	if ( castsShadow_ != castsShadow )
	{
		castsShadow_ = castsShadow;

		MF_ASSERT( pChunk_ != NULL );

		WorldManager::instance().changedChunk( pChunk_ );

		BoundingBox bb = BoundingBox::s_insideOut_;
		edBounds( bb );
		bb.transformBy( edTransform() );
		bb.transformBy( chunk()->transform() );
		WorldManager::instance().markTerrainShadowsDirty( bb );
		if (!pChunk_->isOutsideChunk())
		{
			StaticLighting::markChunk( pChunk_ );
		}
	}	
	return true;
}

Moo::LightContainerPtr EditorChunkTree::edVisualiseLightContainer()
{
	if (!chunk())
		return NULL;

	Moo::LightContainerPtr lc = new Moo::LightContainer;

	BoundingBox bb = this->boundingBox();
	bb.transformBy( this->transform() );
	bb.transformBy( this->chunk()->transform() );

	lc->init( ChunkLightCache::instance( *chunk() ).pAllLights(), bb, false );

	return lc;
}

/// Write the factory statics stuff
/// Write the factory statics stuff
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk)
IMPLEMENT_CHUNK_ITEM( EditorChunkTree, speedtree, 1 )
