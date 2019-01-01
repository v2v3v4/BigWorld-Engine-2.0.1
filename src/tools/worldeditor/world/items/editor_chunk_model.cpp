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
#include "worldeditor/world/items/editor_chunk_model.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/world/item_info_db.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/project/project_module.hpp"
#include "worldeditor/misc/cvswrapper.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_model_obstacle.hpp"
#include "chunk/chunk_light.hpp"
#include "chunk/borrowed_light_combiner.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/string_provider.hpp"
#include "common/material_utility.hpp"
#include "common/material_properties.hpp"
#include "common/material_editor.hpp"
#include "common/dxenum.hpp"
#include "common/tools_common.hpp"
#include "model/matter.hpp"
#include "model/super_model.hpp"
#include "model/super_model_animation.hpp"
#include "model/super_model_dye.hpp"
#include "model/tint.hpp"
#include "romp/static_light_values.hpp"
#include "romp/static_light_fashion.hpp"
#include "romp/fog_controller.hpp"
#include "physics2/material_kinds.hpp"
#include "physics2/bsp.hpp"
#include "moo/visual_manager.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/visual_compound.hpp"
#include "moo/resource_load_context.hpp"
#include "appmgr/options.hpp"
#include "appmgr/module_manager.hpp"
#include "math/colour.hpp"
#include "cstdmf/debug.hpp"
#if UMBRA_ENABLE
#include <umbraModel.hpp>
#include <umbraObject.hpp>
#include "chunk/chunk_umbra.hpp"
#endif
DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )


static AutoConfigString s_notFoundModel( "system/notFoundModel" );	
StringHashMap<int> EditorChunkModel::s_materialKinds_;


std::map<std::string, std::set<EditorChunkModel*> > EditorChunkModel::editorChunkModels_;


void EditorChunkModel::add( EditorChunkModel* model, const std::string& filename )
{
	BW_GUARD;

	editorChunkModels_[ filename ].insert( model );
}


void EditorChunkModel::remove( EditorChunkModel* model )
{
	BW_GUARD;

	for( std::map<std::string, std::set<EditorChunkModel*> >::iterator iter =
		editorChunkModels_.begin(); iter != editorChunkModels_.end(); ++iter )
	{
		std::set<EditorChunkModel*>& modelSet = iter->second;
		std::set<EditorChunkModel*>::iterator siter = modelSet.find( model );
		if (siter != modelSet.end())
		{
			modelSet.erase( siter );
			if (modelSet.size())
			{
				editorChunkModels_.erase( iter );
			}
			return;
		}
	}
}


void EditorChunkModel::reload( const std::string& filename )
{
	BW_GUARD;

	BWResource::instance().purgeAll();
	std::set<EditorChunkModel*> modelSet =
		editorChunkModels_[ BWResource::dissolveFilename( filename ) ];
	std::string sectionName;
	std::vector<DataSectionPtr> sections;
	std::vector<ChunkPtr> chunks;
	std::vector<ModelPtr> models;

	if( !modelSet.empty() )
	{
		EditorChunkModel* ecm = *modelSet.begin();
		sectionName = ecm->sectionName();

		for (int i = 0; i < ecm->pSuperModel_->nModels(); ++i)
		{
			models.push_back( ecm->pSuperModel_->topModel( i ) );
		}
	}

	for( std::set<EditorChunkModel*>::iterator iter = modelSet.begin();
		iter != modelSet.end(); ++iter )
	{
		chunks.push_back( (*iter)->chunk() );
		(*iter)->toss( NULL );
		DataSectionPtr section = new XMLSection( sectionName );
		(*iter)->edSave( section );
		(*iter)->clean();
		sections.push_back( section );
	}

	for (std::vector<ModelPtr>::iterator iter = models.begin();
		iter != models.end(); ++iter)
	{
		(*iter)->reload();
	}

	std::vector<DataSectionPtr>::iterator sec_iter = sections.begin();
	std::vector<ChunkPtr>::iterator chunk_iter = chunks.begin();

	for( std::set<EditorChunkModel*>::iterator iter = modelSet.begin();
		iter != modelSet.end(); ++iter, ++sec_iter, ++chunk_iter )
	{
		// Must make the section a child of the chunk section to set the
		// pOwnSect_ to the correct state.
		DataSectionPtr ownSect;
		if( *chunk_iter )
		{
			ownSect = EditorChunkCache::instance( **chunk_iter ).
				pChunkSection()->newSection( sectionName );
			ownSect->copySections( *sec_iter );
		}
		else
		{
			ownSect = *sec_iter;
		}

		(*iter)->load( ownSect, (*iter)->chunk() );
		(*iter)->toss( *chunk_iter );

		if( !*chunk_iter )
			(*iter)->pOwnSect_ = NULL;

		if( (*iter)->chunk() )
		{
			if( (*iter)->chunk()->isOutsideChunk() )
			{
				WorldManager::instance().markTerrainShadowsDirty( (*iter)->chunk() );
				if ((*iter)->chunk()->space()->staticLightingOutside())
					WorldManager::instance().dirtyLighting( (*iter)->chunk() );
			}
			else
				WorldManager::instance().dirtyLighting( (*iter)->chunk() );
			//TODO : either dirty shadows or dirty lighting means the thumbnail
			//is dirty.  Those methods in WorldEditor should internally then set
			//the thumbnail to dirty.  Should then be able to remove this.
			WorldManager::instance().dirtyThumbnail( (*iter)->chunk() );
		}
	}
}

void EditorChunkModel::clean()
{
	BW_GUARD;

	delete pSuperModel_;
	pSuperModel_ = NULL;
	calculateIsShellModel_ = true;
	cachedIsShellModel_ = false;
	pStaticLightFashion_ = NULL;
	isModelNodeless_ = true;
	firstToss_ = true;
	primGroupCount_ = 0;
	customBsp_ = false;
	standinModel_ = false;
	originalSect_ = NULL;
	outsideOnly_ = false;
	castsShadow_ = true;
	desc_.clear();
	animationNames_.clear();
	dyeTints_.clear();
	tintName_.clear();
	changedMaterials_.clear();

	pAnimation_ = NULL;
	tintMap_.clear();
	materialOverride_.clear();
	fv_.clear();
	label_.clear();
}

// -----------------------------------------------------------------------------
// Section: EditorChunkModel
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EditorChunkModel::EditorChunkModel()
	: isModelNodeless_( true )
	, firstToss_( true )
	, primGroupCount_( 0 )
	, customBsp_( false )
	, standinModel_( false )
	, originalSect_( NULL )
	, outsideOnly_( false )
	, castsShadow_( true )
	, pEditorModel_( NULL )
{
}


/**
 *	Destructor.
 */
EditorChunkModel::~EditorChunkModel()
{
	BW_GUARD;

	delete pEditorModel_;
	remove( this );
}

/**
 *	overridden edShouldDraw method
 */
bool EditorChunkModel::edShouldDraw()
{
	BW_GUARD;

	if( !ChunkModel::edShouldDraw() )
		return false;
	if( isShellModel() )
		return !Chunk::hideIndoorChunks_;

	return OptionsScenery::visible();
}

/**
 *	overridden draw method
 */
void EditorChunkModel::draw()
{
	BW_GUARD;

	if( !edShouldDraw() || (Moo::rc().reflectionScene() && !reflectionVisible_) )
		return;

	if (!hasPostLoaded_)
	{
		edPostLoad();
		hasPostLoaded_ = true;
	}

	if (pSuperModel_ != NULL )
	{
		Moo::rc().push();
		Moo::rc().preMultiply( transform_ );

		bool drawRed = OptionsMisc::readOnlyVisible() &&
					!EditorChunkCache::instance( *chunk() ).edIsWriteable();
		drawRed &= !!OptionsMisc::visible();
		bool drawFrozen = this->edFrozen() && OptionsMisc::frozenVisible();
		bool drawColoured = drawRed || drawFrozen;

		bool projectModule = ProjectModule::currentInstance() == ModuleManager::instance().currentModule();
		if( drawRed && WorldManager::instance().drawSelection())
			return;
		
		if (!projectModule)
		{
			if (drawRed)
			{
				// Set the fog to a constant red colour
				WorldManager::instance().setReadOnlyFog();
			}
			else if (drawFrozen)
			{
				// Set the fog to a constant grey colour
				WorldManager::instance().setFrozenFog();
			}
		}

		// Notify the chunk cache of how many render sets we're going to draw
		WorldManager::instance().addPrimGroupCount( chunk(), primGroupCount_ );

		bool ignoreStaticLighting = false;
		if (pStaticLightFashion_)
			ignoreStaticLighting = OptionsMisc::lighting() != 0;

		int drawBspFlag = WorldManager::instance().drawBSP();
		bool drawBsp = drawBspFlag == 1 && !projectModule;// || (drawBspFlag == 1 && customBsp_);
		if (WorldManager::instance().drawSelection())
		{
			drawBsp = false;
			WorldManager::instance().registerDrawSelectionItem( this );
		}
		// Load up the bsp tree if needed
		if (drawBsp && !bspCreated( bspModelName_ ))
		{
			// no vertices loaded yet, create some
			const BSPTree * tree = pSuperModel_->topModel(0)->decompose();

			if (tree)
			{
				Moo::Colour colour((float)rand() / (float)RAND_MAX,
									(float)rand() / (float)RAND_MAX,
									(float)rand() / (float)RAND_MAX,
									1.f);
				std::vector<Moo::VertexXYZL> verts;

				Moo::BSPTreeHelper::createVertexList( *tree, verts, colour);

				addBsp( verts, bspModelName_ );
			}
		}

		if (drawBsp)
		{
			if (bspCreated( bspModelName_ ))
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

				Moo::rc().setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
				Moo::rc().setTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
				Moo::rc().setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
				Moo::rc().setTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
				Moo::rc().setTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

				this->drawBsp( bspModelName_ );
			}
		}
		else
		{
			bool drawEditorProxy = OptionsEditorProxies::visible();
			static bool s_lastDrawEditorProxy = drawEditorProxy;
			if ((drawEditorProxy != s_lastDrawEditorProxy) && (pEditorModel_))
			{
				this->ChunkModel::toss( pChunk_, drawEditorProxy ? pEditorModel_ : NULL );
				s_lastDrawEditorProxy = drawEditorProxy;
			}

			// Update animation time
			tickAnimation();

			if (ignoreStaticLighting)
			{
				const FashionPtr pFashion = pStaticLightFashion_;
				MF_ASSERT( std::find( fv_.begin(), fv_.end(), pFashion ) != fv_.end() );
				FashionVector nonStaticFV = fv_;
				nonStaticFV.erase( std::find( nonStaticFV.begin(), nonStaticFV.end(), pFashion ) );

				int late = 0;
				for( std::vector<ChunkMaterialPtr>::iterator iter = materialOverride_.begin();
					iter != materialOverride_.end(); ++iter )
				{
					if( changedMaterials_.find( (*iter)->material_->identifier() ) != changedMaterials_.end() )
					{
						nonStaticFV.push_back( *iter );
						++late;
					}
				}

				BorrowedLightCombinerHolder borrowedLightCombiner( borrowedLightCombiner_, borrowers_ );

				pSuperModel_->draw( &nonStaticFV, late );
				if (drawEditorProxy && pEditorModel_)
				{
					pEditorModel_->draw();
				}
			}
			else
			{
				std::vector<ChunkMaterialPtr>::size_type size = materialOverride_.size();

				int late = 0;
				for( std::vector<ChunkMaterialPtr>::iterator iter = materialOverride_.begin();
					iter != materialOverride_.end(); ++iter )
				{
					if( changedMaterials_.find( (*iter)->material_->identifier() ) != changedMaterials_.end() )
					{
						fv_.push_back( *iter );
						++late;
					}
				}

				BorrowedLightCombinerHolder borrowedLightCombiner( borrowedLightCombiner_, borrowers_ );

				pSuperModel_->draw( &fv_, late );

				if (drawEditorProxy && pEditorModel_)
				{
					pEditorModel_->draw();
				}

				fv_.resize( fv_.size() - late );
			}
		}

		if (drawColoured && !projectModule)
		{
			// Reset the fog
			FogController::instance().commitFogToDevice();
		}

		Moo::rc().pop();
	}
}

std::vector<Moo::VisualPtr> EditorChunkModel::extractVisuals()
{
	BW_GUARD;

	std::vector<std::string> models;
	pOwnSect_->readStrings( "resource", models );

	std::vector<Moo::VisualPtr> v;
	v.reserve( models.size() );

	for (uint i = 0; i < models.size(); i++)
	{
		DataSectionPtr modelSection = BWResource::openSection( models[i] );
		if (!modelSection)
		{
			WARNING_MSG( "Couldn't read model %s for ChunkModel\n", models.front().c_str() );
			continue;
		}

		std::string visualName = modelSection->readString( "nodelessVisual" );

		if (visualName.empty())
		{
			visualName = modelSection->readString( "nodefullVisual" );
			if (visualName.empty())
			{
				WARNING_MSG( "ChunkModel %s has a model that has no visual\n", models[i].c_str() );
			}			
			continue;
		}

		Moo::VisualPtr visual = Moo::VisualManager::instance()->get(
			visualName + ".static.visual"
			);

		if (!visual)
			visual = Moo::VisualManager::instance()->get( visualName + ".visual" );

		v.push_back( visual );
	}

	return v;
}

std::vector<std::string> EditorChunkModel::extractVisualNames() const
{
	BW_GUARD;

	std::vector<std::string> models;
	pOwnSect_->readStrings( "resource", models );

	std::vector<std::string> v;
	v.reserve( models.size() );

	for (uint i = 0; i < models.size(); i++)
	{
		DataSectionPtr modelSection = BWResource::openSection( models[i] );
		if (!modelSection)
		{
			WARNING_MSG( "Couldn't read model %s for ChunkModel\n", models.front().c_str() );
			continue;
		}

		std::string visualName = modelSection->readString( "nodelessVisual" );

		if (visualName.empty())
		{
			visualName = modelSection->readString( "nodefullVisual" );
			if (visualName.empty())
			{
				WARNING_MSG( "ChunkModel %s has a model that has no visual\n", models[i].c_str() );
			}
		}

		std::string fullVisualName = visualName + ".static.visual";

		Moo::VisualPtr visual = Moo::VisualManager::instance()->get(
			fullVisualName
			);

		if (!visual)
		{
			fullVisualName = visualName + ".visual";
			visual = Moo::VisualManager::instance()->get( fullVisualName );
		}

		if (visual)
			v.push_back( fullVisualName );
		else
			v.push_back( "" );
	}

	return v;
}

/**
 *	Used by load to get the names of the animations in a model file.
 */
static void addNames(std::vector<std::string>& sections, DataSectionPtr ds,
	const std::string& name)
{
	BW_GUARD;

	std::vector<DataSectionPtr> children;
	ds->openSections( name, children );

	std::set<std::string> names;

	std::vector<std::string>::iterator ni;
	// Build a list of names we already have
	for (ni = sections.begin(); ni != sections.end(); ++ni)
		names.insert( *ni );

	std::vector<DataSectionPtr>::iterator i;
	for (i = children.begin(); i != children.end(); ++i)
	{
		// Only add it if we don't already have a section with the same
		// name
		if (!names.count( (*i)->readString( "name" ) ))
		{
			sections.push_back( (*i)->readString( "name" ) );
		}
	}
}

/**
 *	Used by load to get the names of the dyes and tints in a model file.
 */
static void addDyeTints( std::map<std::string, std::vector<std::string> >& sections, DataSectionPtr ds )
{
	BW_GUARD;

	std::vector<DataSectionPtr> dyes;
	ds->openSections( "dye", dyes );

	std::vector<DataSectionPtr>::iterator i;
	for (i = dyes.begin(); i != dyes.end(); ++i)
	{
		if( sections.find( (*i)->readString( "matter" ) ) == sections.end() )
		{
			std::vector<std::string> names;
			names.push_back( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/DEFAULT_TINT_NAME") );
			addNames( names, (*i), "tint" );
			if( names.size() > 1 )
				sections[ (*i)->readString( "matter" ) ] = names;
		}
	}
}

/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkModel::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;

	pStaticLightFashion_ = NULL;
	isModelNodeless_ = true;
	firstToss_ = true;
	primGroupCount_ = 0;
	customBsp_ = false;
	standinModel_ = false;
	originalSect_ = NULL;
	outsideOnly_ = false;
	castsShadow_ = true;
	desc_.clear();
	animationNames_.clear();
	dyeTints_.clear();
	tintName_.clear();
	changedMaterials_.clear();

	remove( this );
	edCommonLoad( pSection );

	pOwnSect_ = pSection;

	std::vector<std::string> models;
	
	std::string modelName;
	pOwnSect_->readStrings( "resource", models );
	if (models.size())
	{
		modelName = models[0];
		add( this, modelName );
		DataSectionPtr data = BWResource::openSection( modelName );
		if (data) 
		{
			std::string editorModel = data->readString( "editorModel", "" );
			delete pEditorModel_;
			pEditorModel_ = NULL;
			if (editorModel != "")
			{
				std::vector<std::string> editorModels;
				editorModels.push_back( editorModel );
				pEditorModel_ = new SuperModel( editorModels );
			}
		}
	}

	bool ok = this->ChunkModel::load( pSection, pChunk );
	if (!ok)
	{
		originalSect_ = new XMLSection( sectionName() );
		originalSect_->copy( pSection );

		// load in a replecement model
		DataSectionPtr pTemp = new XMLSection( sectionName() );
		pTemp->writeString( "resource", s_notFoundModel );
		pTemp->writeMatrix34( "transform", pSection->readMatrix34( "transform" ) );
		ok = this->ChunkModel::load( pTemp, pChunk );

		bspModelName_ = s_notFoundModel;

		standinModel_ = true;

		// tell the user
		std::string mname = pSection->readString( "resource" );
		WorldManager::instance().addError(pChunk, this, "Model not loaded: %s", mname.c_str());

		// make sure static lighting does not get regerenated
		// (it will look at the absent model for this info)
		isModelNodeless_ = true;

		// don't look for visuals
		hasPostLoaded_ = true;
	}
	else
	{
		bspModelName_ = modelName;

		Moo::ScopedResourceLoadContext resLoadCtx(
						"chunk model " + BWResource::getFilename( modelName ) );

		// for the static lighting
		detectModelType();

		if (pAnimation_)
		{
			animName_ = pSection->readString( "animation/name" );
		}

		tintName_.clear();
		std::vector<DataSectionPtr> dyes;
		pSection->openSections( "dye", dyes );
		for( std::vector<DataSectionPtr>::iterator iter = dyes.begin(); iter != dyes.end(); ++iter )
		{
			std::string dye = (*iter)->readString( "name" );
			std::string tint = (*iter)->readString( "tint" );
			if( tintMap_.find( dye ) != tintMap_.end() )
				tintName_[ dye ] = tint;
		}

		outsideOnly_ = pOwnSect()->readBool( "editorOnly/outsideOnly", outsideOnly_ );
		outsideOnly_ |= this->resourceIsOutsideOnly();

		castsShadow_ = pOwnSect()->readBool( "editorOnly/castsShadow", castsShadow_ );

		// check we're not > 100m in x or z
		BoundingBox bbox;
		edBounds(bbox);
		bbox.transformBy(edTransform());
		Vector3 boxVolume = bbox.maxBounds() - bbox.minBounds();
		static const float lengthLimit = 100.f;
		if (boxVolume.x > lengthLimit || boxVolume.z > lengthLimit)
		{
			std::string mname = pSection->readString( "resource" );
			WorldManager::instance().addError(pChunk, this,
				LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/MODEL_TOO_BIG", mname ).c_str() );
		}

		// Reset the amount of primitive groups in all the visuals
		primGroupCount_ = 0;
		hasPostLoaded_ = false;

		// Build a list of the animations in the model file
		animationNames_.push_back( std::string() );
		DataSectionPtr current = BWResource::openSection(
			pOwnSect()->readString( "resource") ) ;
		while (current)
		{
			addNames( animationNames_, current, "animation" );

			std::string parent = current->readString( "parent" );

			if (parent.empty())
				break;

			current = BWResource::openSection( parent + ".model" );
		}
		std::sort( animationNames_.begin(), animationNames_.end() );
		// cache wide string version for performance
		bw_containerConversion( animationNames_, animationNamesW_, bw_utf8towSW );


		// Build a list of dyes in the model file
		{
			tintMap_.clear();
			DataSectionPtr current = BWResource::openSection(
				pOwnSect()->readString( "resource") ) ;

			if( current )
			{
				addDyeTints( dyeTints_, current );

				// cache wide string version for performance
				for (std::map<std::string, std::vector<std::string> >::iterator
					iter = dyeTints_.begin(); iter != dyeTints_.end(); ++iter)
				{
					std::vector<std::wstring> wsecond;
					bw_containerConversion( iter->second, wsecond, bw_utf8towSW );
					dyeTintsW_[ iter->first ] = wsecond;
				}
			}
		}

		// Build a list of materials in the model file ( if it has any )
		{
			DataSectionPtr model = BWResource::openSection(
				pOwnSect()->readString( "resource") ) ;

			if( model )
			{
				std::set<std::string> existingMaterialOverrides;
				std::vector<ChunkMaterialPtr>::iterator iter = materialOverride_.begin();
				while( iter != materialOverride_.end() )
				{
					existingMaterialOverrides.insert( (*iter)->material_->identifier() );
					changedMaterials_.insert( (*iter)->material_->identifier() );
					++iter;
				}

				Moo::VisualPtr nodefullVisual;
				std::string name = model->readString( "nodefullVisual" );
				if (!name.empty())
				{
					std::string visualName = BWResource::removeExtension( name );
					visualName += ".visual";
					nodefullVisual = Moo::VisualManager::instance()->get( visualName.c_str() );
				}

				Moo::VisualPtr nodelessVisual;
				name = model->readString( "nodelessVisual" );
				if (!name.empty())
				{
					std::string visualName = BWResource::removeExtension( name );
					visualName += ".static.visual";
					nodelessVisual = Moo::VisualManager::instance()->get( visualName.c_str() );
				}

				if( !nodefullVisual && !nodelessVisual )
				{
					name = model->readString( "nodelessVisual" );
					if (!name.empty())
					{
						std::string visualName = BWResource::removeExtension( name );
						visualName += ".visual";
						nodelessVisual = Moo::VisualManager::instance()->get( visualName.c_str() );
					}
					else
					{
						nodelessVisual = NULL;
					}
				}

				std::vector<Moo::EffectMaterialPtr>	materials;

				if ( nodefullVisual )
					nodefullVisual->collateOriginalMaterials( materials );
				else if( nodelessVisual )
					nodelessVisual->collateOriginalMaterials( materials );

				for( std::vector<Moo::EffectMaterialPtr>::iterator iter = materials.begin(); iter != materials.end();
					++iter)
				{
					if( existingMaterialOverrides.find( (*iter)->identifier() ) == existingMaterialOverrides.end() )
					{
						DataSectionPtr matSec = new XMLSection( "material" );
						MaterialUtility::save( *iter, matSec );
						matSec->writeString( "identifier", (*iter)->identifier() );
						Moo::EffectMaterialPtr mat = new Moo::EffectMaterial;
						mat->load( matSec );
						materialOverride_.push_back( new ChunkMaterial( mat ) );
						existingMaterialOverrides.insert( mat->identifier() );
					}
				}
			}
		}

		// After loading,count the amount of primitive groups in all the
		// visuals.
		std::vector<std::string> visuals = extractVisualNames();
		std::vector<std::string>::iterator i = visuals.begin();
		for (; i != visuals.end(); ++i)
		{
			DataSectionPtr visualSection = BWResource::openSection( *i );
			if (!visualSection)
				continue;

			// Check for a custom bsp while we're here
			if (visualSection->readBool( "customBsp", false ))
				customBsp_ = true;

			std::vector<DataSectionPtr> renderSets;
			visualSection->openSections( "renderSet", renderSets );

			std::vector<DataSectionPtr>::iterator j = renderSets.begin();
			for (; j != renderSets.end(); ++j)
			{
				std::vector<DataSectionPtr> geoms;
				(*j)->openSections( "geometry", geoms );

				std::vector<DataSectionPtr>::iterator k = geoms.begin();
				for (; k != geoms.end(); ++k)
				{
					DataSectionPtr geom = *k;

					std::vector<DataSectionPtr> primGroups;
					(*k)->openSections( "primitiveGroup", primGroups );

					primGroupCount_ += primGroups.size();
				}
			}
		}
	}

	desc_ = pSection->readString( "resource" );
	std::string::size_type pos = desc_.find_last_of( "/" );
	if (pos != std::string::npos)
		desc_ = desc_.substr( pos + 1 );
	pos = desc_.find_last_of( "." );
	if (pos != std::string::npos)
		desc_ = desc_.substr( 0, pos );

	return ok;
}

void EditorChunkModel::loadModels( Chunk* chunk )
{
	std::vector<std::string> models;
	pOwnSect_->readStrings( "resource", models );
	if( models.size() )
	{
		SmartPointer<Model> model = Model::get( models[0] );
		if( model )
		{
			model->reload();
			load( pOwnSect_, chunk );
		}
	}
}

void EditorChunkModel::edPostLoad()
{
	// Don't put here code that might cause frame rate spikes (for example,
	// code that reads from or writes to disk).
}


void EditorChunkModel::clearLightingFashion()
{
	BW_GUARD;

	if (pStaticLightFashion_)
	{
		const FashionPtr pFashion = pStaticLightFashion_;
		fv_.erase( std::find( fv_.begin(), fv_.end(), pFashion ) );
		pStaticLightFashion_ = NULL;
	}
}


/**
 * We just loaded up with srcItems lighting data, create some new stuff of our
 * own
 */
void EditorChunkModel::edPostClone( EditorChunkItem* srcItem )
{
	BW_GUARD;

	clearLightingFashion();

	if (isModelNodeless())
		StaticLighting::markChunk( pChunk_ );

	BoundingBox bb = BoundingBox::s_insideOut_;
	edBounds( bb );
	bb.transformBy( edTransform() );
	bb.transformBy( chunk()->transform() );
	WorldManager::instance().markTerrainShadowsDirty( bb );

	EditorChunkItem::edPostClone( srcItem );
	EditorChunkBspHolder::postClone();
}

void EditorChunkModel::edPostCreate()
{
	BW_GUARD;

	if (isModelNodeless())
		StaticLighting::markChunk( pChunk_ );

	BoundingBox bb = BoundingBox::s_insideOut_;
	edBounds( bb );
	bb.transformBy( edTransform() );
	bb.transformBy( chunk()->transform() );
	WorldManager::instance().markTerrainShadowsDirty( bb );

	EditorChunkItem::edPostCreate();
}


bool EditorChunkModel::calculateLighting( StaticLighting::StaticLightContainer& lights,
										 Moo::VisualPtr visual, std::vector<size_t>& sizes )
{
	BW_GUARD;

	Moo::VertexXYZNUV* vertices;
	Moo::IndicesHolder indices;
	uint32 numVertices;
	uint32 numIndices;
	Moo::EffectMaterialPtr material;

	visual->createCopy( vertices, indices, numVertices, numIndices, material );

	if (numVertices == 0)
	{
		delete [] vertices;
		return true;
	}

	// If there's no lights, and we're not connected to anything, just set it 
	// all to medium illumination, so you can see the shell
	if (lights.empty() && pChunk_->pbegin() == pChunk_->pend())
	{
		sizes.push_back( pChunk_->lightValueCache()->size() * sizeof( D3DCOLOR ));
		sizes.push_back( numVertices * sizeof( D3DCOLOR ));

		pChunk_->lightValueCache()->addDefault( numVertices );
		delete [] vertices;

		return true;
	}

	sizes.push_back( pChunk_->lightValueCache()->size() * sizeof( D3DCOLOR ));
	sizes.push_back( numVertices * sizeof( D3DCOLOR ));

	D3DCOLOR* colours = pChunk_->lightValueCache()->allocate( numVertices, lights.ambient() );

	Matrix xform = chunk()->transform();
	xform.preMultiply( edTransform() );

	for (uint32 i = 0; i < numVertices; i++)
	{
		Vector3 vertexPos = xform.applyPoint( vertices[i].pos_ );
		Vector3 vertexNormal = xform.applyVector( vertices[i].normal_ );

		colours[i] = lights.calcLight( vertexPos, vertexNormal );

		WorldManager::instance().fiberPause();
		if( !WorldManager::instance().isWorkingChunk( chunk() ) )
		{
			delete [] vertices;
			return false;
		}
	}

	delete [] vertices;
	return true;
}

namespace
{
	std::string toString( int i )
	{
		BW_GUARD;

		char buf[128];
		itoa( i, buf, 10 );
		return buf;
	}
}


void EditorChunkModel::edInvalidateStaticLighting()
{
	BW_GUARD;

	edSave( pOwnSect() );
}

bool EditorChunkModel::edRecalculateLighting( StaticLighting::StaticLightContainer& lights )
{
	BW_GUARD;

	ChunkItemPtr holder( this );

	if (!isModelNodeless())
		return true;

	// Grab the bounding box of this object and transform
	// it to world space
	BoundingBox modelBB;
	pSuperModel_->localVisibilityBox( modelBB );
	modelBB.transformBy( transform_ );
	modelBB.transformBy( pChunk_->transform() );

	// Cull the lights to only includes ones that intersect
	// the bounding box
	StaticLighting::StaticLightContainer localLights;
	localLights.addLights( &lights, modelBB );
	localLights.rebuildLightVolumes();
	localLights.ambient( lights.ambient() );

	MF_ASSERT( pOwnSect() );

	std::vector<Moo::VisualPtr> visuals = extractVisuals();
	std::vector<size_t> sizes;

	// Generate a new set of static lighting data for each of the models
	for (uint i = 0; i < visuals.size(); ++i)
	{
		if (visuals[i])
		{
			if (!calculateLighting( localLights, visuals[i], sizes ))
			{
				return false;
			}
		}
	}

	// Don't bother going ahead if we've been deleted while recalculating
	if (!pChunk_)
		return false;

	// Update the datasection with the lighting data
	edSave( pOwnSect() );

	pOwnSect()->deleteSections( "staticLighting" );

	for (std::vector<size_t>::iterator iter = sizes.begin();
		iter != sizes.end(); iter += 2)
	{
		DataSectionPtr lighting = pOwnSect()->newSection( "staticLighting" );

		lighting->writeUInt( "offset", (uint32)*iter );
		lighting->writeUInt( "size", (uint32)*( iter + 1 ) );
	}

	addStaticLighting( pOwnSect(), pChunk_ );

	return true;
}


void EditorChunkModel::addStaticLighting( DataSectionPtr ds, Chunk* pChunk )
{
	BW_GUARD;

	clearLightingFashion();

	bool staticLighting = pChunk && ( !pChunk->isOutsideChunk() || pChunk->space()->staticLightingOutside() );

	if (staticLighting && ds->openSection( "staticLighting" ))
	{
		pStaticLightFashion_ = StaticLightFashion::get(
			pChunk->lightValueCache(), *pSuperModel_, ds );

		if (pStaticLightFashion_)
		{
			fv_.push_back( pStaticLightFashion_ );
		}
	}
}

bool EditorChunkModel::isVisualFileNewer() const
{
	BW_GUARD;

	MF_ASSERT(chunk());
	MF_ASSERT(pStaticLightFashion_);

	std::vector<StaticLightValuesPtr> vals = pStaticLightFashion_->staticLightValues();
	std::vector<std::string> visualNames = extractVisualNames();

	MF_ASSERT( !vals.empty() );
	MF_ASSERT( vals.size() == visualNames.size() );

	std::string cdataName = chunk()->binFileName();

	for (uint i = 0; i < vals.size(); ++i)
	{
		if (vals[i])
		{
			if (BWResource::isFileOlder( cdataName, visualNames[i] ))
			{
				// ok, the lighting data is out of date, bail
				return true;
			}
		}
	}

	return false;
}


/**
 *	This method does extra stuff when this item is tossed between chunks.
 *
 *	It updates its datasection in that chunk.
 */
void EditorChunkModel::toss( Chunk * pChunk )
{
	BW_GUARD;

	if (pChunk_ != NULL && pOwnSect_)
	{
		EditorChunkCache::instance( *pChunk_ ).
			pChunkSection()->delChild( pOwnSect_ );
		pOwnSect_ = NULL;
	}

	this->ChunkModel::toss( pChunk, OptionsEditorProxies::visible() ? pEditorModel_ : NULL  );

	if (pChunk_ != NULL && !pOwnSect_)
	{
		pOwnSect_ = EditorChunkCache::instance( *pChunk_ ).
			pChunkSection()->newSection( sectionName() );
		this->edSave( pOwnSect_ );
	}

	if (firstToss_)
	{
		// check lighting files are up to date (can't do this on load as chunk() is NULL)
		if ( pChunk_ && pStaticLightFashion_ && isVisualFileNewer() )
			StaticLighting::markChunk( pChunk_ );

		firstToss_ = false;
	}

	// If we havn't got our static lighting calculated yet, mark the new
	// chuck as dirty. This will only be the case for newly created items.
	// Marking a chunk as dirty when moving chunks around is taken care of 
	// in edTransform()
	if (pChunk && 
		!pStaticLightFashion_ && 
		isModelNodeless() &&
		!(pChunk->isOutsideChunk() && !pChunk->space()->staticLightingOutside()))
	{
		StaticLighting::markChunk( pChunk );
	}

	// Update the database
	ItemInfoDB::instance().toss( this, pChunk != NULL );
}


/**
 *	Save to the given section
 */
bool EditorChunkModel::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	if (standinModel_)
	{
		// only change its transform (want to retain knowledge of what it was)
		pSection->copy( originalSect_ );
		pSection->writeMatrix34( "transform", transform_ );
		return true;
	}

	if (pSuperModel_)
	{
		for (int i = 0; i < pSuperModel_->nModels(); i++)
		{
			pSection->writeString( "resource",
				pSuperModel_->topModel(i)->resourceID() );
		}

		if (pAnimation_)
		{
			DataSectionPtr pAnimSec = pSection->openSection( "animation", true );
			pAnimSec->writeString( "name", animName_ );
			pAnimSec->writeFloat( "frameRateMultiplier", animRateMultiplier_ );
		}
		else
		{
			pSection->delChild( "animation" );
		}

		while( pSection->findChild( "dye" ) )
			pSection->delChild( "dye" );
		for( std::map<std::string,std::string>::iterator iter = tintName_.begin(); iter != tintName_.end(); ++iter )
		{
			if( iter->second != LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/DEFAULT_TINT_NAME") )
			{
				DataSectionPtr pDyeSec = pSection->newSection( "dye" );
				pDyeSec->writeString( "name", iter->first );
				pDyeSec->writeString( "tint", iter->second );
			}
		}

		while( pSection->findChild( "material" ) )
			pSection->delChild( "material" );
		for( std::vector<ChunkMaterialPtr>::iterator iter = materialOverride_.begin(); iter != materialOverride_.end(); ++iter )
		{
			if( changedMaterials_.find( (*iter)->material_->identifier() ) != changedMaterials_.end() )
			{
				DataSectionPtr pMaterialSec = pSection->newSection( "material" );
				if( ToolsCommon::isEval() ||
					Options::getOptionInt("objects/materialOverrideMode", 0) )
					MaterialUtility::save( (*iter)->material_, pMaterialSec, false );
				else
					MaterialUtility::save( (*iter)->material_, pMaterialSec, true );
				pMaterialSec->writeString( "identifier", (*iter)->material_->identifier() );
			}
		}

		pSection->writeMatrix34( "transform", transform_ );
	}

	//Editor only data.
	if (outsideOnly_ && !resourceIsOutsideOnly())
		pSection->writeBool( "editorOnly/outsideOnly", true );
	else
		pSection->delChild( "editorOnly/outsideOnly" );

	pSection->writeBool( "editorOnly/castsShadow", castsShadow_ );

	pSection->setString( label_ );

	pSection->writeBool( "reflectionVisible", reflectionVisible_ );

	pSection->deleteSections( "staticLighting" );
	pSection->deleteSection( "lighting" );

	bool staticLighting = chunk() && ( !chunk()->isOutsideChunk() || chunk()->space()->staticLightingOutside() );

	if (staticLighting && pStaticLightFashion_ && pStaticLightFashion_->isDataValid())
	{
		// Save the static lighting data
		std::vector<StaticLightValuesPtr> v = pStaticLightFashion_->staticLightValues();

		MF_ASSERT( !v.empty() );

		for (uint i = 0; i < v.size(); ++i)
		{
			if (v[i])
			{
				v[i]->save( pSection );
			}
		}
	}

	return true;
}

/**
 *	Called when our containing chunk is saved
 */
void EditorChunkModel::edChunkSave()
{
}


/**
 *	This method sets this item's transform for the editor
 *	It takes care of moving it into the right chunk and recreating the
 *	collision scene and all that
 */
bool EditorChunkModel::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	// find out where we belong now
	BoundingBox lbb( Vector3(0.f,0.f,0.f), Vector3(1.f,1.f,1.f) );
	if (pSuperModel_) pSuperModel_->localBoundingBox( lbb );
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyPoint(
		(lbb.minBounds() + lbb.maxBounds()) * 0.5f ) );
	if (pNewChunk == NULL) return false; // failure, outside the space!

	// for transient transforms there are no more, checks, update the transform
    // and return
	if (transient)
	{
		transform_ = m;
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
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );

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
	if (isModelNodeless())
	{
		StaticLighting::markChunk( pNewChunk );
		StaticLighting::markChunk( pOldChunk );

	}
	this->syncInit();
	return true;
}


void EditorChunkModel::edPreDelete()
{
	BW_GUARD;

	if (isModelNodeless())
	{
		clearLightingFashion();
		StaticLighting::markChunk( pChunk_ );
	}

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
void EditorChunkModel::edBounds( BoundingBox& bbRet ) const
{
	BW_GUARD;

	if (pSuperModel_)
		pSuperModel_->localBoundingBox( bbRet );

	BoundingBox ebb;
	if (OptionsEditorProxies::visible() && pEditorModel_)
	{
		pEditorModel_->localBoundingBox( ebb );
		bbRet.addBounds( ebb );
	}
}


/**
 *	This method returns whether or not this model should cast a shadow.
 *
 *  @return		Returns whether or not this model should cast a shadow
 */
bool EditorChunkModel::edAffectShadow() const
{
	return castsShadow_;
}


/**
 *	Helper struct for gathering matter names
 */
struct MatterDesc
{
	std::set<std::string>	tintNames;
};
typedef std::map<std::string,MatterDesc> MatterDescs;

/// I can't believe there's not an algorithm for this...
template <class It, class S> void seq_append( It F, It L, S & s)
	{ for(; F != L; ++F) s.push_back( *F ); }


extern "C"
{
/**
 *	This property makes a dye from a matter name to one of a number of tints
 */
class ModelDyeProperty : public GeneralProperty
{
public:
	ModelDyeProperty( const std::string & name,
			const std::string & current, const MatterDesc & tints,
			EditorChunkModel * pModel ) :
		GeneralProperty( name ),
		curval_( current ),
		pModel_( pModel )
	{
		BW_GUARD;

		tints_.push_back( "Default" );
		seq_append( tints.tintNames.begin(), tints.tintNames.end(), tints_ );

		GENPROPERTY_MAKE_VIEWS()
	}

	virtual const ValueType & valueType() const { RETURN_VALUETYPE( STRING ); }

	virtual PyObject * EDCALL pyGet()
	{
		BW_GUARD;

		return PyString_FromString( curval_.c_str() );
	}

	virtual int EDCALL pySet( PyObject * value )
	{
		BW_GUARD;

		// try it as a string
		if (PyString_Check( value ))
		{
			const char * valStr = PyString_AsString( value );

			std::vector<std::string>::iterator found =
				std::find( tints_.begin(), tints_.end(), valStr );
			if (found == tints_.end())
			{
				std::string errStr = "GeneralEditor.";
				errStr += name_;
				errStr += " must be set to a valid tint string or an index.";
				errStr += " Valid tints are: ";
				for (uint i = 0; i < tints_.size(); i++)
				{
					if (i)
					{
						if (i+1 != tints_.size())
							errStr += ", ";
						else
							errStr += ", or ";
					}
					errStr += tints_[i];
				}

				PyErr_Format( PyExc_ValueError, errStr.c_str() );
				return -1;
			}

			curval_ = valStr;
			// TODO: Set this in the editor chunk item!
			return 0;
		}

		// try it as a number
		int idx = 0;
		if (Script::setData( value, idx ) == 0)
		{
			if (idx < 0 || idx >= (int) tints_.size())
			{
				PyErr_Format( PyExc_ValueError, "GeneralEditor.%s "
					"must be set to a string or an index under %d",
					name_, tints_.size() );
				return -1;
			}

			curval_ = tints_[ idx ];
			// TODO: Set this in the editor chunk item!
			return 0;
		}

		// give up
		PyErr_Format( PyExc_TypeError, "GeneralEditor.%s, "
			"being a dye property, must be set to a string or an index",
			name_ );
		return NULL;
	}

private:
	std::string					curval_;
	std::vector<std::string>	tints_;
	EditorChunkModel *			pModel_;

	GENPROPERTY_VIEW_FACTORY_DECLARE( ModelDyeProperty )
};
}

GENPROPERTY_VIEW_FACTORY( ModelDyeProperty )

static std::string matUIName( EditorEffectProperty* pProperty )
{
	BW_GUARD;

	std::string uiName = MaterialUtility::UIName( pProperty );
	if( uiName.size() == 0 )
		uiName = pProperty->name();
	return uiName;
}

EditorEffectProperty* EditorChunkModel::findMaterialPropertyByName( const std::string& name ) const
{
	BW_GUARD;

	std::string matName = name.substr( 0, name.find( '/' ) );
	std::string propName = name.substr( name.find( '/' ) + 1, name.npos );

	for( std::vector<ChunkMaterialPtr>::const_iterator iter = materialOverride_.begin();
		iter != materialOverride_.end(); ++iter )
	{
		Moo::EffectMaterialPtr mat = (*iter)->material_;
		
		if( mat->identifier() == matName )
		{
			if (mat->pEffect() )
			{
				Moo::EffectMaterial::Properties& properties = mat->properties();
				Moo::EffectMaterial::Properties::iterator it = properties.begin();
				Moo::EffectMaterial::Properties::iterator end = properties.end();

				while ( it != end )
				{
					MF_ASSERT( it->second );
					D3DXHANDLE hParameter = it->first;
					EditorEffectProperty* pProperty = dynamic_cast<EditorEffectProperty*>(it->second.get());
					MF_ASSERT( pProperty );

					if ( (ToolsCommon::isEval() ||
						(Options::getOptionInt("objects/materialOverrideMode", 0)) ||
						MaterialUtility::worldBuilderEditable( pProperty )) &&
						pProperty->name() == propName)
					{

						return pProperty;
					}

					it++;
				}
			}
		}
	}

	// It should never get here.
	ERROR_MSG( "EditorChunkModel::findMaterialByName1: Failed to find %s.\n", name.c_str() );

	return NULL;
}

EditorEffectProperty* EditorChunkModel::findOriginalMaterialPropertyByName( const std::string& name ) const
{
	BW_GUARD;

	std::string matName = name.substr( 0, name.find( '/' ) );
	std::vector< Moo::Visual::PrimitiveGroup * > primGroup;
	pSuperModel_->topModel(0)->gatherMaterials( matName, primGroup );

	if (  primGroup.size() && primGroup[0]->material_ && primGroup[0]->material_->pEffect()  )
	{
		std::string propName = name.substr( name.find( '/' ) + 1, name.npos );
		Moo::EffectMaterialPtr mat = primGroup[0]->material_;

		Moo::EffectMaterial::Properties& properties = mat->properties();
		Moo::EffectMaterial::Properties::iterator it = properties.begin();
		Moo::EffectMaterial::Properties::iterator end = properties.end();

		while ( it != end )
		{
			MF_ASSERT( it->second );
			D3DXHANDLE hParameter = it->first;
			EditorEffectProperty* pProperty = dynamic_cast<EditorEffectProperty*>(it->second.get());
			MF_ASSERT( pProperty );

			if ( (ToolsCommon::isEval() ||
				(Options::getOptionInt("objects/materialOverrideMode", 0)) ||
				MaterialUtility::worldBuilderEditable( pProperty )) &&
				pProperty->name() == propName)
			{

				return pProperty;
			}

			it++;
		}
	}

	// It should never get here.
	ERROR_MSG( "EditorChunkModel::findOriginalMaterialByName: Failed to find %s.\n", name.c_str() );

	return NULL;
}
void EditorChunkModel::changedMaterial( const std::string& materialPropertyName )
{
	changedMaterials_.insert( materialPropertyName.substr( 0, materialPropertyName.find('/') ) );
}

bool EditorChunkModel::getMaterialBool( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );
	MaterialBoolProxy* pBoolProxy = dynamic_cast<MaterialBoolProxy*>( pProperty );
	if (pBoolProxy)
	{
		return pBoolProxy->get();
	}
	return false;
}

bool EditorChunkModel::setMaterialBool( const std::string& name, const bool& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );
	MaterialBoolProxy* pBoolProxy = dynamic_cast<MaterialBoolProxy*>( pProperty );
	if (pBoolProxy)
	{
		pBoolProxy->set(value, false);
		changedMaterial( name );
	}
	return true;
}

std::string EditorChunkModel::getMaterialString( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialTextureProxy* pTextureProxy = dynamic_cast<MaterialTextureProxy*>( pProperty );
	MaterialTextureFeedProxy* pTextureFeedProxy = dynamic_cast<MaterialTextureFeedProxy*>( pProperty );

	if (pTextureProxy)
	{
		return pTextureProxy->get();
	}
	else if (pTextureFeedProxy)
	{
		return pTextureFeedProxy->get();
	}

	return std::string();
}

bool EditorChunkModel::setMaterialString( const std::string& name, const std::string& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialTextureProxy* pTextureProxy = dynamic_cast<MaterialTextureProxy*>( pProperty );
	MaterialTextureFeedProxy* pTextureFeedProxy = dynamic_cast<MaterialTextureFeedProxy*>( pProperty );

	if (pTextureProxy)
	{
		pTextureProxy->set(value, false);
	}
	else if (pTextureFeedProxy)
	{
		pTextureFeedProxy->set(value, false);
	}
	changedMaterial( name );
	return true;
}

float EditorChunkModel::getMaterialFloat( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty );

	if (pFloatProxy)
	{
		return pFloatProxy->get();
	}

	return 0.f;
}

bool EditorChunkModel::setMaterialFloat( const std::string& name, const float& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty );

	if (pFloatProxy)
	{
		pFloatProxy->set(value, false);
		changedMaterial( name );
	}

	return true;
}

bool EditorChunkModel::getMaterialFloatRange( const std::string& name, float& min, float& max, int& digits )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty );

	if (pFloatProxy)
	{
		return pFloatProxy->getRange(min, max, digits);
	}

	return false;
}

bool EditorChunkModel::getMaterialFloatDefault( const std::string& name, float& def )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findOriginalMaterialPropertyByName( name );

	MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty );

	if (pFloatProxy)
	{
		def = pFloatProxy->get();
	}

	return true;
}

void EditorChunkModel::setMaterialFloatToDefault( const std::string& name )
{
	BW_GUARD;

	float def;
	if( getMaterialFloatDefault( name, def ) )
	{
		EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

		MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty );

		if (pFloatProxy)
		{
			pFloatProxy->set(def, false);
		}
	}
}

Vector4 EditorChunkModel::getMaterialVector4( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialVector4Proxy* pVector4Proxy = dynamic_cast<MaterialVector4Proxy*>( pProperty );

	if (pVector4Proxy)
	{
		return pVector4Proxy->get();
	}

	return Vector4::zero();
}

bool EditorChunkModel::setMaterialVector4( const std::string& name, const Vector4& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialVector4Proxy* pVector4Proxy = dynamic_cast<MaterialVector4Proxy*>( pProperty );

	if (pVector4Proxy)
	{
		pVector4Proxy->set(value, false);
		changedMaterial( name );
	}

	return true;
}

Matrix EditorChunkModel::getMaterialMatrix( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialMatrixProxy* pMatrixProxy = dynamic_cast<MaterialMatrixProxy*>( pProperty );

	Matrix m;

	if (pMatrixProxy)
	{
		pMatrixProxy->getMatrix(m, true);
	}
	else
	{
		m.setIdentity();
	}

	return m;
}

bool EditorChunkModel::setMaterialMatrix( const std::string& name, const Matrix& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialMatrixProxy* pMatrixProxy = dynamic_cast<MaterialMatrixProxy*>( pProperty );

	Matrix m;

	if (pMatrixProxy)
	{
		pMatrixProxy->setMatrix( value );
	}

	return true;
}

int EditorChunkModel::getMaterialInt( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>( pProperty );

	if (pIntProxy)
	{
		return pIntProxy->get();
	}

	return 0;
}

bool EditorChunkModel::setMaterialInt( const std::string& name, const int& value )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>( pProperty );

	if (pIntProxy)
	{
		pIntProxy->set(value, false);
		changedMaterial( name );
	}

	return true;
}

bool EditorChunkModel::getMaterialIntRange( const std::string& name, int& min, int& max, int& digits )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>( pProperty );

	if (pIntProxy)
	{
		return pIntProxy->getRange(min, max);
	}

	return false;
}

Moo::EffectMaterialPtr EditorChunkModel::findMaterialByName( const std::string& name ) const
{
	BW_GUARD;

	std::string matName = name.substr( 0, name.find( '/' ) );
	std::string propName = name.substr( name.find( '/' ) + 1, name.npos );

	for( std::vector<ChunkMaterialPtr>::const_iterator iter = materialOverride_.begin();
		iter != materialOverride_.end(); ++iter )
	{
		Moo::EffectMaterialPtr mat = (*iter)->material_;
		if( mat->identifier() == matName )
			return mat;
	}

	// It should never get here.
	ERROR_MSG( "EditorChunkModel::findMaterialByName2: Failed to find %s.\n", name.c_str() );

	return NULL;
}

std::string EditorChunkModel::getMaterialCollision( const std::string& name ) const
{
	BW_GUARD;

	Moo::EffectMaterialPtr mat = findMaterialByName( name );
	for( StringHashMap<int>::const_iterator iter = collisionFlags_.begin();
		iter != collisionFlags_.end(); ++iter )
	{
		if( iter->second == mat->collisionFlags() )
			return iter->first;
	}
	for( StringHashMap<int>::const_iterator iter = collisionFlags_.begin();
		iter != collisionFlags_.end(); ++iter )
	{
		if( iter->second == 0 )
			return iter->first;
	}
	return "";
}

bool EditorChunkModel::setMaterialCollision( const std::string& name, const std::string& collisionType )
{
	BW_GUARD;

	Moo::EffectMaterialPtr mat = findMaterialByName( name );
	mat->collisionFlags( collisionFlags_.find( collisionType )->second );
	mat->bspModified_ = true;
	changedMaterials_.insert( mat->identifier() );
	return true;
}

std::string EditorChunkModel::getMaterialKind( const std::string& name ) const
{
	BW_GUARD;

	Moo::EffectMaterialPtr mat = findMaterialByName( name );
	for( StringHashMap<int>::const_iterator iter = s_materialKinds_.begin();
		iter != s_materialKinds_.end(); ++iter )
	{
		if( iter->second == mat->materialKind() )
			return iter->first;
	}
	for( StringHashMap<int>::const_iterator iter = s_materialKinds_.begin();
		iter != s_materialKinds_.end(); ++iter )
	{
		if( iter->second == 0 )
			return iter->first;
	}
	return "";
}

bool EditorChunkModel::setMaterialKind( const std::string& name, const std::string& collisionType )
{
	BW_GUARD;

	Moo::EffectMaterialPtr mat = findMaterialByName( name );
	mat->materialKind( s_materialKinds_.find( collisionType )->second );
	changedMaterials_.insert( mat->identifier() );
	return true;
}

std::string EditorChunkModel::getMaterialEnum( const std::string& name ) const
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>( pProperty );

	if (pIntProxy)
	{
		std::string enumType;
		pIntProxy->stringAnnotation( "EnumType", enumType );
		return DXEnum::instance().name( enumType, pIntProxy->get() );
	}
	return false;
}

bool EditorChunkModel::setMaterialEnum( const std::string& name, const std::string& enumValue )
{
	BW_GUARD;

	EditorEffectProperty* pProperty = findMaterialPropertyByName( name );

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>( pProperty );

	if (pIntProxy)
	{
		std::string enumType;
		pIntProxy->stringAnnotation( "EnumType", enumType );

		pIntProxy->set( DXEnum::instance().value( enumType, enumValue ), false );
		changedMaterial( name );
	}
	return true;
}

void EditorChunkModel::edit( Moo::EffectMaterialPtr material, GeneralEditor & editor )
{
	BW_GUARD;

	GeneralProperty* pProp;
#define NO_DEFAULT_MATERIAL_ATTRIBUTES
#ifndef NO_DEFAULT_MATERIAL_ATTRIBUTES
	//Add the two default properties; material kind and collision flags
	if( collisionFlags_.empty() )
	{
		DataSectionPtr pFile = BWResource::openSection( "resources/flags.xml" );
		DataSectionPtr pSect = pFile->openSection( "collisionFlags" );
		MF_ASSERT( pSect.hasObject() );

		for (DataSectionIterator it = pSect->begin(); it != pSect->end(); ++it )
		{
			std::string name = pSect->unsanitise((*it)->sectionName());
			if( collisionFlags_.find( name ) == collisionFlags_.end() )
			{
				collisionFlags_[ name ] = ( *it )->asInt();
				collisionFlagNames_.push_back( name );
			}
		}
	}

	pProp = new ListTextProperty( "Collision Flags",
		new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
			this, material->identifier() + "/" + "Collision Flags",
			getMaterialCollision, setMaterialCollision ), collisionFlagNames_ );
	pProp->setGroup( std::string( "Material/" ) + material->identifier() );
	editor.addProperty( pProp );

	// load the material kinds
	if( s_materialKinds_.empty() )
	{
		s_materialKinds_[ "(Use Visual's)" ] = 0;
		MaterialKinds::instance().createDescriptionMap( s_materialKinds_ );		
	}

	pProp = new ListTextProperty( "Material Kind",
		new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
			this, material->identifier() + "/" + "Material Kind",
			getMaterialKind, setMaterialKind ), materialKindNames_ );
	pProp->setGroup( std::string( "Material/" ) + material->identifier() );
	editor.addProperty( pProp );
#endif// NO_DEFAULT_MATERIAL_ATTRIBUTES
	//Now add the material's own properties.
	material->replaceDefaults();

	std::vector<Moo::EffectPropertyPtr> existingProps;

	if ( material->pEffect() )
	{
		Moo::EffectMaterial::Properties& properties = material->properties();
		Moo::EffectMaterial::Properties::iterator it = properties.begin();
		Moo::EffectMaterial::Properties::iterator end = properties.end();

		while ( it != end )
		{
			MF_ASSERT( it->second );
			D3DXHANDLE hParameter = it->first;
			EditorEffectProperty* pProperty = dynamic_cast<EditorEffectProperty*>(it->second.get());

			if ( ToolsCommon::isEval() ||
				(Options::getOptionInt("objects/materialOverrideMode", 0)) ||
				MaterialUtility::worldBuilderEditable( pProperty ) )
			{
				if( MaterialBoolProxy* pBoolProxy = dynamic_cast<MaterialBoolProxy*>(pProperty) )
				{
					pProp = new GenBoolProperty( matUIName( pProperty ),
						new AccessorDataProxyWithName< EditorChunkModel, BoolProxy >(
							this, material->identifier() + "/" + pProperty->name(), 
							&EditorChunkModel::getMaterialBool, 
							&EditorChunkModel::setMaterialBool ) );
				}
				else if( MaterialTextureProxy* pTextureProxy = dynamic_cast<MaterialTextureProxy*>(pProperty) )
				{
					pProp = new TextProperty( matUIName( pProperty ),
						new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
							this, material->identifier() + "/" + pProperty->name(), 
							&EditorChunkModel::getMaterialString, 
							&EditorChunkModel::setMaterialString ) );
					((TextProperty*)pProp)->fileFilter( L"Texture files(*.jpg;*.tga;*.bmp)|*.jpg;*.tga;*.bmp||" );
					((TextProperty*)pProp)->canTextureFeed( false );
				}
				else if( MaterialTextureFeedProxy* pTextureFeedProxy = dynamic_cast<MaterialTextureFeedProxy*>(pProperty) )
				{
					pProp = new TextProperty( matUIName( pProperty ),
						new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
							this, material->identifier() + "/" + pProperty->name(), 
							&EditorChunkModel::getMaterialString, 
							&EditorChunkModel::setMaterialString ) );
					((TextProperty*)pProp)->fileFilter( L"Texture files(*.jpg;*.tga;*.bmp)|*.jpg;*.tga;*.bmp||" );
					((TextProperty*)pProp)->canTextureFeed( false );
				}
				else if( MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>(pProperty) )
				{
					pProp = new GenFloatProperty( matUIName( pProperty ),
						new AccessorDataProxyWithName< EditorChunkModel, FloatProxy >(
							this, material->identifier() + "/" + pProperty->name(), 
							&EditorChunkModel::getMaterialFloat, 
							&EditorChunkModel::setMaterialFloat, 
							&EditorChunkModel::getMaterialFloatRange,
							&EditorChunkModel::getMaterialFloatDefault, 
							&EditorChunkModel::setMaterialFloatToDefault ) );
				}
				else if( MaterialVector4Proxy* pVector4Proxy = dynamic_cast<MaterialVector4Proxy*>(pProperty) )
				{
					std::string UIWidget = MaterialUtility::UIWidget( pProperty );

					if ((UIWidget == "Color") || (UIWidget == "Colour"))
					{
						pProp = new ColourProperty( matUIName( pProperty ),
							new AccessorDataProxyWithName< EditorChunkModel, Vector4Proxy >(
								this, material->identifier() + "/" + pProperty->name(), 
								&EditorChunkModel::getMaterialVector4, 
								&EditorChunkModel::setMaterialVector4 ) );
					}
					else // Must be a vector
					{
						pProp = new Vector4Property( matUIName( pProperty ),
							new AccessorDataProxyWithName< EditorChunkModel, Vector4Proxy >(
								this, material->identifier() + "/" + pProperty->name(), 
								&EditorChunkModel::getMaterialVector4, 
								&EditorChunkModel::setMaterialVector4 ) );
					}
				}
				else if( MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>(pProperty) )
				{
					std::string enumType;
					if( pProperty->stringAnnotation("EnumType", enumType ) &&
						DXEnum::instance().isEnum( enumType ) )
					{
						std::vector<std::wstring> materialEnumNames;
						for( DXEnum::size_type i = 0; i < DXEnum::instance().size( enumType ); ++i )
						{
							materialEnumNames.push_back( bw_utf8tow( DXEnum::instance().entry( enumType, i ) ) );
						}

						pProp = new ListTextProperty(
							matUIName( pProperty ),
							new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
								this, material->identifier() + "/" + pProperty->name(),
								&EditorChunkModel::getMaterialEnum, 
								&EditorChunkModel::setMaterialEnum ), 
							materialEnumNames );
					}
					else
					{
						pProp = new GenIntProperty( matUIName( pProperty ),
							new AccessorDataProxyWithName< EditorChunkModel, IntProxy >(
								this, material->identifier() + "/" + pProperty->name(), 
								&EditorChunkModel::getMaterialInt, 
								&EditorChunkModel::setMaterialInt, 
								&EditorChunkModel::getMaterialIntRange ) );
					}
				}
				else if( MaterialMatrixProxy* pMatrixProxy = dynamic_cast<MaterialMatrixProxy*>(pProperty) )
				{
					pProp = new GenMatrixProperty( matUIName( pProperty ),
						new AccessorDataProxyWithName< EditorChunkModel, MatrixProxy >(
							this, material->identifier() + "/" + pProperty->name(), 
							&EditorChunkModel::getMaterialMatrix, 
							&EditorChunkModel::setMaterialMatrix ) );
				}
				pProp->UIDesc( bw_utf8tow( MaterialUtility::UIDesc( pProperty ) ) );
				pProp->canExposeToScript( false );
				pProp->setGroup( std::wstring( L"Material/" ) + bw_utf8tow( material->identifier() ) );
				editor.addProperty( pProp );
			}

			it++;
		}
	}
}


/**
 *	This method adds this item's properties to the given editor
 */
bool EditorChunkModel::edEdit( GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	// can only move this model if it's not in the shells directory
	if (!isShellModel())
	{
		MatrixProxy * pMP = new ChunkItemMatrix( this );
		editor.addProperty( new ChunkItemPositionProperty(
			LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/POSITION"), pMP, this ) );
		editor.addProperty( new GenRotationProperty(
			LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/ROTATION"), pMP ) );
		editor.addProperty( new GenScaleProperty(
			LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/SCALE"), pMP ) );

		// can affect shadow?
		editor.addProperty( new GenBoolProperty(
			LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/CASTS_SHADOW"),
			new AccessorDataProxy< EditorChunkModel, BoolProxy >(
				this, "castsShadow", 
				&EditorChunkModel::getCastsShadow, 
				&EditorChunkModel::setCastsShadow ) ) );

		// can flag models as outside-only
		editor.addProperty( new GenBoolProperty(
			LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/OUTSIDE_ONLY"),
			new AccessorDataProxy< EditorChunkModel, BoolProxy >(
				this, "outsideOnly", 
				&EditorChunkModel::getOutsideOnly, 
				&EditorChunkModel::setOutsideOnly ) ) );
	}

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/REFLECTION_VISIBLE"),
		new AccessorDataProxy< EditorChunkModel, BoolProxy >(
			this, "reflectionVisible",
			&EditorChunkModel::getReflectionVis,
			&EditorChunkModel::setReflectionVis ) ) );

	editor.addProperty( new ListTextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/ANIMATION"),
		new AccessorDataProxy< EditorChunkModel, StringProxy >(
			this, "animation", 
			&EditorChunkModel::getAnimation, 
			&EditorChunkModel::setAnimation ), animationNamesW_ ) );

	for (std::map<std::string, std::vector<std::wstring> >::iterator
			iter = dyeTintsW_.begin(); iter != dyeTintsW_.end(); ++iter)
	{
		ListTextProperty* ltProperty = 
			new ListTextProperty( std::string( iter->first ),
				new AccessorDataProxyWithName< EditorChunkModel, StringProxy >(
					this, std::string( iter->first ), 
					&EditorChunkModel::getDyeTints, 
					&EditorChunkModel::setDyeTints ), 
					iter->second );
		ltProperty->setGroup( L"dye" );
		editor.addProperty( ltProperty );
	}

	for( std::vector<ChunkMaterialPtr>::iterator iter = materialOverride_.begin();
		iter != materialOverride_.end(); ++iter )
	{
		edit( (*iter)->material_, editor );
	}

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/ANIMATION_SPEED"),
			new AccessorDataProxy< EditorChunkModel, FloatProxy >(
				this, "animation speed", 
				&EditorChunkModel::getAnimRateMultiplier, 
				&EditorChunkModel::setAnimRateMultiplier ) ) );


	editor.addProperty( new StaticTextProperty( pSuperModel_->nModels() == 1 ?
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/MODEL_NAME") :
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/MODEL_NAMES"), new
		ConstantDataProxy<StringProxy>( getModelName() ) ) );

	if (pSuperModel_ != NULL)
	{
		MatterDescs	mds;

		// for each model
		for (int i = 0; i < pSuperModel_->nModels(); i++)
		{
			ModelPtr pTop = pSuperModel_->topModel(i);

			// get each matter element pointer
			for (int j = 0; ; j++)
			{
				const Matter * pmit = pTop->lookupLocalMatter( j );
				if (pmit == NULL)
					break;

				// and collect its tints it has
				const Matter::Tints & mts = pmit->tints_;
				for (Matter::Tints::const_iterator tit = mts.begin()+1;
					tit != mts.end();	// except default...
					tit++)
				{
					mds[ pmit->name_ ].tintNames.insert( (*tit)->name_ );
				}
			}
		}

		// now add them all as properties
		for (MatterDescs::iterator it = mds.begin(); it != mds.end(); it++)
		{
			editor.addProperty( new ModelDyeProperty(
				it->first, "Default", it->second, this ) );
		}

		// Storing localised string on a static for speed.
		static std::wstring s_assetMetadataGroup =
			Localise( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/ASSET_META_DATA" );

		editor.addingAssetMetadata( true );
		pSuperModel_->topModel(0)->metaData().edit( editor, s_assetMetadataGroup, true );
		editor.addingAssetMetadata( false );
	}

	return true;
}


/**
 *	Find the drop chunk for this item
 */
Chunk * EditorChunkModel::edDropChunk( const Vector3 & lpos )
{
	BW_GUARD;

	Vector3 npos = pChunk_->transform().applyPoint( lpos );

	Chunk * pNewChunk = NULL;

	if ( !this->outsideOnly_ )
		pNewChunk = pChunk_->space()->findChunkFromPointExact( npos );
	else
		pNewChunk = EditorChunk::findOutsideChunk( npos );

	if (pNewChunk == NULL)
	{
		ERROR_MSG( "Cannot move %s to (%f,%f,%f) "
			"because it is not in any loaded chunk!\n",
			this->edDescription().c_str(), npos.x, npos.y, npos.z );
		return NULL;
	}

	return pNewChunk;
}



/**
 * Are we the interior mesh for the chunk?
 *
 * We check by seeing if the model is in the shells directory
 */
bool EditorChunkModel::isShellModel() 
{
	BW_GUARD;

	return ChunkModel::isShellModel( pOwnSect() );
}

/**
 * Which section name shall we use when saving?
 */
const char* EditorChunkModel::sectionName()
{
	BW_GUARD;

	return isShellModel() ? "shell" : "model";
}

/**
 * Look in the .model file to see if it's nodeless or nodefull
 */
void EditorChunkModel::detectModelType()
{
	BW_GUARD;

	isModelNodeless_ = true;

	std::vector<std::string> models;
	pOwnSect_->readStrings( "resource", models );

	std::vector<Moo::VisualPtr> v;
	v.reserve( models.size() );

	for (uint i = 0; i < models.size(); i++)
	{
		DataSectionPtr modelSection = BWResource::openSection( models[i] );
		if (!modelSection)
			continue;

		std::string visualName = modelSection->readString( "nodelessVisual" );

		if (visualName.empty())
		{
			isModelNodeless_ = false;
			return;
		}
	}
}

std::string EditorChunkModel::edDescription()
{
	BW_GUARD;

	if (isShellModel())
	{
		if (pChunk_)
		{
			return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/ED_DESCRIPTION", pChunk_->identifier() );
		}
		else
		{
			return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/UNKNOWN_CHUNK");
		}
	}
	else
	{
		//return ChunkModel::edDescription();
		return desc_;
	}
}


/**
 *	This returns the number of triangles of this model.
 */
int EditorChunkModel::edNumTriangles() const
{
	BW_GUARD;

	if (pSuperModel_)
	{
		return pSuperModel_->numTris();
	}

	return 0;
}


/**
 *	This returns the number of primitive groups of this model.
 */
int EditorChunkModel::edNumPrimitives() const
{
	BW_GUARD;

	if (pSuperModel_)
	{
		return pSuperModel_->numPrims();
	}

	return 0;
}


/**
 *	This returns the file path of the asset file.
 */
std::string EditorChunkModel::edAssetName() const
{
	BW_GUARD;

	if (pSuperModel_)
	{
		return BWResource::getFilename( pSuperModel_->topModel( 0 )->resourceID() );
	}

	return "";
}


/**
 *	This returns the file path of the asset file.
 */
std::string EditorChunkModel::edFilePath() const
{
	BW_GUARD;

	if (pSuperModel_)
	{
		return pSuperModel_->topModel( 0 )->resourceID();
	}

	return "";
}


std::vector<std::string> EditorChunkModel::edCommand( const std::string& path ) const
{
	BW_GUARD;

	std::vector<std::string> commands;
	std::vector<std::string> models;
	pOwnSect_->readStrings( "resource", models );
	if( !models.empty() )
	{
		commands.push_back( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/EDIT_IN_MODEL_EDITOR") );
/*		if( animationNames_.size() > 1 )
		{
			commands.push_back( "animation: (default)" );
			for( std::vector<std::string>::const_iterator iter = animationNames_.begin() + 1;
				iter != animationNames_.end(); ++iter )
				commands.insert( commands.end(), std::string( "animation: " ) + *iter );
		}*/
	}
	return commands;
}

bool EditorChunkModel::edExecuteCommand( const std::string& path, std::vector<std::string>::size_type index )
{
	BW_GUARD;

	if( path.empty() && index == 0 )
	{
		std::vector<std::string> models;
		pOwnSect_->readStrings( "resource", models );

		std::string commandLine = "-o ";
		commandLine += '\"' + BWResource::resolveFilename( models[0] ) + '\"';
		std::replace( commandLine.begin(), commandLine.end(), '/', '\\' );
		commandLine += ' ';
		commandLine += BWResource::getPathAsCommandLine( true );

		PyObject * function = NULL;
		PyObject* pModule = PyImport_ImportModule( "WorldEditor" );
		if (pModule != NULL)
		{
			function = PyObject_GetAttrString( pModule, "launchTool" );
		}

		return Script::call( function,
							Py_BuildValue( "(ss)", "modeleditor",
											commandLine.c_str() ), 
							"WorldEditor" );
	}
	else if( path.empty() && animationNames_.size() > 1 && index - 1 < animationNames_.size() )
	{
		setAnimation( animationNames_[ index - 1 ] );
	}

	return false;
}

Vector3 EditorChunkModel::edMovementDeltaSnaps()
{
	BW_GUARD;

	if (isShellModel())
	{
		return Options::getOptionVector3( "shellSnaps/movement", Vector3( 0.f, 0.f, 0.f ) );
	}
	else
	{
		return EditorChunkItem::edMovementDeltaSnaps();
	}
}

float EditorChunkModel::edAngleSnaps()
{
	BW_GUARD;

	if (isShellModel())
	{
		return Options::getOptionFloat( "shellSnaps/angle", 0.f );
	}
	else
	{
		return EditorChunkItem::edAngleSnaps();
	}
}


std::string EditorChunkModel::getModelName()
{
	std::string modelNames;

	if (pSuperModel_ != NULL)
	{
		for (int i = 0; i < pSuperModel_->nModels(); i++)
		{
			if (i)
			{
				modelNames += ", ";
			}

			modelNames += pSuperModel_->topModel(i)->resourceID();
		} 
	}

	return modelNames;
}


bool EditorChunkModel::setAnimation(const std::string & newAnimationName)
{
	BW_GUARD;

	if (newAnimationName.empty())
	{
		animName_ = "";
		if (pAnimation_)
		{
			const FashionPtr pFashion = pAnimation_;
			fv_.erase( std::find( fv_.begin(), fv_.end(), pFashion ) );
		}

		pAnimation_ = 0;

		return true;
	}
	else
	{
		SuperModelAnimationPtr newAnimation = pSuperModel_->getAnimation(
			newAnimationName );

		if (!newAnimation)
			return false;

		if (newAnimation->pSource( *pSuperModel_ ) == NULL)
			return false;

		newAnimation->time = 0.f;
		newAnimation->blendRatio = 1.0f;

		if (pAnimation_)
		{
			const FashionPtr pFashion = pAnimation_;
			fv_.erase( std::find( fv_.begin(), fv_.end(), pFashion ) );
		}

		pAnimation_ = newAnimation;
		fv_.push_back( pAnimation_ );

		animName_ = newAnimationName;

		return true;
	}
}

std::string EditorChunkModel::getDyeTints( const std::string& dye ) const
{
	BW_GUARD;

	if( tintName_.find( dye ) == tintName_.end() )
		return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/DEFAULT_TINT_NAME");
	return tintName_.find( dye )->second;
}

bool EditorChunkModel::setDyeTints( const std::string& dye, const std::string& tint )
{
	BW_GUARD;

	SuperModelDyePtr newDye = pSuperModel_->getDye( dye, tint );

	if( !newDye )
		return false;

	if( tintMap_[ dye ] )
	{
		const FashionPtr pFashion = tintMap_[ dye ];
		fv_.erase( std::find( fv_.begin(), fv_.end(), pFashion ) );
	}

	tintMap_[ dye ] = newDye;
	fv_.push_back( tintMap_[ dye ] );

	tintName_[ dye ] = tint;

	return true;
}

bool EditorChunkModel::setAnimRateMultiplier( const float& f )
{
	BW_GUARD;

	if ( f < 0.f )
		return false;

	// limit animation preview speed to 100x the original speed
	float mult = f;
	if ( mult > 100.0f )
		mult = 100.0f;

	animRateMultiplier_ = mult;

	return true;
}


bool EditorChunkModel::resourceIsOutsideOnly() const
{
	BW_GUARD;

	if ( !pOwnSect_ )
		return false;

	DataSectionPtr modelResource = BWResource::openSection(
		pOwnSect_->readString( "resource") ) ;
	if ( modelResource )
	{
		return modelResource->readBool( "editorOnly/outsideOnly", false );
	}

	return false;
}


bool EditorChunkModel::setOutsideOnly( const bool& outsideOnly )
{
	BW_GUARD;

	if ( outsideOnly_ != outsideOnly )
	{
		//We cannot turn off outsideOnly, if the model resource
		//specifies it to be true.
		if ( !outsideOnly && this->resourceIsOutsideOnly() )
		{
			ERROR_MSG( "Cannot turn off outsideOnly because the .model file overrides the chunk entry\n" );
			return false;
		}

		outsideOnly_ = outsideOnly;
		if (!edTransform( transform_, false ))
		{
			ERROR_MSG( "Changed outsideOnly flag, but could not change the chunk for this model\n" );
		}
		return true;
	}

	return false;
}


bool EditorChunkModel::setCastsShadow( const bool& castsShadow )
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
		if (isModelNodeless())
			StaticLighting::markChunk( pChunk_ );

		return true;
	}

	return false;
}

Moo::LightContainerPtr EditorChunkModel::edVisualiseLightContainer()
{
	if (!chunk())
		return NULL;

	Moo::LightContainerPtr lc = new Moo::LightContainer;

	BoundingBox bb = this->localBB();
	bb.transformBy( this->transform_ );
	bb.transformBy( this->chunk()->transform() );

	bool selfStatic = isShellModel() || 
		(isModelNodeless() &&
		!(chunk()->isOutsideChunk() && !chunk()->space()->staticLightingOutside()));

	lc->init( ChunkLightCache::instance( *chunk() ).pAllLights(), bb, false, selfStatic );

	return lc;
}


/// Write the factory statics stuff
/// Write the factory statics stuff
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk)
IMPLEMENT_CHUNK_ITEM( EditorChunkModel, model, 1 )
IMPLEMENT_CHUNK_ITEM_ALIAS( EditorChunkModel, shell, 1 )

// editor_chunk_model.cpp
