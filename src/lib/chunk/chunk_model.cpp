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

#include "cstdmf/guard.hpp"

#include "chunk_model.hpp"
#include "chunk_model_obstacle.hpp"
#include "chunk_manager.hpp"

#include "math/blend_transform.hpp"

#include "romp/static_light_fashion.hpp"
#include "romp/model_compound.hpp"

#include "model/model_animation.hpp"

#include "moo/primitive_file_structs.hpp"
#include "moo/visual_compound.hpp"
#include "moo/visual_channels.hpp"

#include "model/fashion.hpp"
#include "model/super_model_animation.hpp"
#include "model/super_model_dye.hpp"

#include "moo/resource_load_context.hpp"


#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#include "umbra_draw_item_collection.hpp"
#include "chunk_umbra.hpp"
#endif

#include "fmodsound/sound_manager.hpp"

int ChunkModel_token;

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

PROFILER_DECLARE( ChunkModel_tick, "ChunkModel Tick" );

// ----------------------------------------------------------------------------
// Section: ChunkMaterial
// ----------------------------------------------------------------------------
ChunkMaterial::~ChunkMaterial()
{
}

void ChunkMaterial::dress( SuperModel & superModel )
{
	if( superModel.curModel( 0 ) )
		override_ = superModel.curModel( 0 )->overrideMaterial( material_->identifier(), material_ );
	else
		override_.savedMaterials_.clear();
}

void ChunkMaterial::undress( SuperModel & superModel )
{
	override_.reverse();
}

// ----------------------------------------------------------------------------
// Section: ChunkModel
// ----------------------------------------------------------------------------

#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk)
IMPLEMENT_CHUNK_ITEM( ChunkModel, model, 0 )
IMPLEMENT_CHUNK_ITEM_ALIAS( ChunkModel, shell, 0 )


ChunkModel::ChunkModel() :
	ChunkItem( (WantFlags)(WANTS_DRAW) ),
	pSuperModel_( NULL ),
	pAnimation_( NULL ),
	tickMark_( ChunkManager::instance().tickMark() ),
	lastTickTimeInMS_( ChunkManager::instance().totalTickTimeInMS() ),
	animRateMultiplier_( 1.f ),
	transform_( Matrix::identity ),
	reflectionVisible_( false ),
	calculateIsShellModel_( true ),
	cachedIsShellModel_( false )
#if UMBRA_ENABLE	
	,umbraOccluder_( false )
#endif
{
}


ChunkModel::~ChunkModel()
{
	BW_GUARD;
	fv_.clear();
	pAnimation_ = NULL;

	if (pSuperModel_ != NULL)
	{
		delete pSuperModel_;
	}
}

#include "cstdmf/diary.hpp"

// in chunk.cpp
extern void readMooMatrix( DataSectionPtr pSection, const std::string & tag,
	Matrix &result );

/**
 *	Load yourself from this section
 */
bool ChunkModel::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;
	pAnimation_ = NULL;
	tintMap_.clear();
	materialOverride_.clear();
	fv_.clear();
	label_.clear();
	calculateIsShellModel_ = true;
	cachedIsShellModel_ = false;

	bool good = false;

	label_ = pSection->asString();

	std::vector<std::string> models;
	pSection->readStrings( "resource", models );

	//uint64 ltime = timestamp();
	std::string mname;
	if (!models.empty()) mname = models[0].substr( models[0].find_last_of( '/' )+1 );
	DiaryEntryPtr de = Diary::instance().add( "model " + mname );

	Moo::ScopedResourceLoadContext resLoadCtx( mname );

	pSuperModel_ = new SuperModel( models );
	de->stop();
	//ltime = timestamp() - ltime;
	//dprintf( "\t%s took %f ms\n", models[0].c_str(),
	//	float(double(int64(ltime)) / stampsPerSecondD()) * 1000.f );

	if (pSuperModel_->nModels() > 0)
	{
		good = true;

		// load the specified animation
		DataSectionPtr pAnimSec = pSection->openSection( "animation" );
		if (pAnimSec)
		{
			pAnimation_ = pSuperModel_->getAnimation(
				pAnimSec->readString( "name" ) );
			pAnimation_->time = 0.f;
			pAnimation_->blendRatio = 1.0f;
			animRateMultiplier_ = pAnimSec->readFloat( "frameRateMultiplier", 1.f );

			if (pAnimation_->pSource( *pSuperModel_ ) == NULL)
			{
				ERROR_MSG( "SuperModel can't find its animation %s\n",
					pAnimSec->readString( "name" ).c_str() );
				pAnimation_ = NULL;
			}
			else
			{
				fv_.push_back( pAnimation_ );
			}
		}

		// Load up the legacy dyes first
		int i = 0;
		while (1)
		{
			SuperModelDyePtr pDye;
			char legName[128];
			bw_snprintf( legName, sizeof( legName ), "Legacy-%d", i );

			pDye = pSuperModel_->getDye( legName, "MFO" );
			if (!pDye) break;

			WARNING_MSG( "ChunkModel::load - encountered legacy dye in "
				"chunk %s, model %s, this has been deprecated in BigWorld 1.9\n",
				pChunk->identifier().c_str(), models[0].c_str() );

			fv_.push_back( pDye );
			tintMap_[legName] = pDye;
			i++;
		}

		// Load up dyes
		std::vector<DataSectionPtr> dyeSecs;
		pSection->openSections( "dye", dyeSecs );

		for( std::vector<DataSectionPtr>::iterator iter = dyeSecs.begin(); iter != dyeSecs.end(); ++iter )
		{
			std::string dye = ( *iter )->readString( "name" );
			// If there is no dye name, try old style dyes
			if (dye.empty())
			{
				dye = (*iter)->readString( "matter" );
				if (!dye.empty())
				{
					WARNING_MSG( "ChunkModel::load - encountered old style <matter> tag "
						"in chunk '%s' for model '%s', this has been deprecated in BigWorld 1.9, "
						"please resave this chunk\n",
						pChunk->identifier().c_str(), models[0].c_str() );
				}
			}

			std::string tint = ( *iter )->readString( "tint" );
			SuperModelDyePtr dyePtr = pSuperModel_->getDye( dye, tint );

			if( dyePtr )
			{
				fv_.push_back( dyePtr );
				tintMap_[ dye ] = dyePtr;
			}
		}

		// load material overrides
		std::vector<DataSectionPtr> materialSecs;
		pSection->openSections( "material", materialSecs );

		for( std::vector<DataSectionPtr>::iterator iter = materialSecs.begin(); iter != materialSecs.end(); ++iter )
		{
			std::string identifier = (*iter)->readString( "identifier" );
			std::vector< Moo::Visual::PrimitiveGroup * > primGroups;
			if( pSuperModel_->topModel( 0 )->gatherMaterials( identifier, primGroups ) != 0 )
			{
				Moo::EffectMaterialPtr material = new Moo::EffectMaterial( *primGroups[0]->material_ );

				if( material->load( *iter, false ) )
				{
					materialOverride_.push_back( new ChunkMaterial( material ) );
				}
			}
		}

		DiaryEntryPtr de2 = Diary::instance().add( "lighting" );

		addStaticLighting( pSection, pChunk );

		de2->stop();

		// load our transform
		readMooMatrix( pSection, "transform", transform_ );

#ifndef EDITOR_ENABLED
		// This model might be suitable for the model compound.
		// Only outdoor models with no fashions or material overrides
		// are allowed to use in the compound
		if (models.size() == 1 && 
			!fv_.size() && 
			!materialOverride_.size() &&
			pChunk &&
			pChunk->isOutsideChunk())
		{
			// Create the model compound
			Matrix m = pChunk->transform();
			m.preMultiply( transform_ );
			pModelCompound_ = ModelCompound::get( models[0], m, 
				uint32(pChunk) );
		}
#endif

#if UMBRA_ENABLE
		// This model might be suitable as an occluder 
		// We only create occluders when Umbra is in software mode
		// We allow models with static lighting to be occluders
		// but no animated models or models with material overrides.
		if (ChunkUmbra::softwareMode() &&
			models.size() == 1 &&
			!pAnimation_)
		{
			// Check if the model is a umbra occluder
			umbraOccluder_ = pSuperModel_->topModel(0)->occluder();
			if (umbraOccluder_)
			{
				umbraModelName_ = models[0];
			}
		}
#endif

		reflectionVisible_ = pSection->readBool( "reflectionVisible", reflectionVisible_ );
		//reflectionVisible_ |= this->resourceIsOutsideOnly();
	}
	else
	{
		WARNING_MSG( "No models loaded into SuperModel\n" );

		delete pSuperModel_;
		pSuperModel_ = NULL;
	}
	return good;
}

void ChunkModel::addStaticLighting( DataSectionPtr ds, Chunk* pChunk )
{
	BW_GUARD;

	if (ds->openSection( "staticLighting" ))
	{
		StaticLightFashionPtr pSLF = StaticLightFashion::get(
			pChunk->lightValueCache(), *pSuperModel_, ds );

		if (pSLF)
			fv_.push_back( pSLF );
	}
}


/**
 *	overridden draw method
 */
void ChunkModel::draw()
{
	BW_GUARD;
	static DogWatch drawWatch( "ChunkModel" );
	ScopedDogWatch watcher( drawWatch );

	static bool firstTime = true;
	static bool useCompound = true;
	if (firstTime)
	{
		MF_WATCH( "Chunks/Use Compound", useCompound, Watcher::WT_READ_WRITE,
			"When enabled, ChunkModel will use the VisualCompound "
			"to render suitable models" );
		firstTime = false;
	}

	//TODO: optimise
	//TODO: sort out the overriding of reflection (should be able to set the reflection
	// per model but be able to override the setting in the editor.
	//TODO: distance check from water position?
	if (Moo::rc().reflectionScene() && !reflectionVisible_ /*&& !Waters::shouldReflect(this)*/ )
		return;

	BorrowedLightCombinerHolder borrowedLightCombiner( borrowedLightCombiner_, borrowers_ );

	if (pSuperModel_ != NULL && 
		(!pModelCompound_.hasObject() || !useCompound || Moo::VisualCompound::disable() || Moo::rc().reflectionScene()) )
	{
		tickAnimation();

		Moo::rc().push();
		Moo::rc().preMultiply( transform_ );

		std::vector<ChunkMaterialPtr>::size_type size = materialOverride_.size();

		fv_.insert( fv_.end(), materialOverride_.begin(), materialOverride_.end() );

		pSuperModel_->draw( &fv_, materialOverride_.size() );

		fv_.resize( fv_.size() - size );

		Moo::rc().pop();
	}
	else if (pModelCompound_.hasObject() && useCompound)
	{
		if (!pModelCompound_->draw())
		{
			pModelCompound_ = NULL;
		}
	}
}

void ChunkModel::syncInit()
{
	BW_GUARD;

	Matrix m = pChunk_->transform();
	m.preMultiply( transform_ );
#if UMBRA_ENABLE

	// Delete any old umbra draw item
	delete pUmbraDrawItem_;

	// Grab the visibility bounding box
	BoundingBox bb = BoundingBox::s_insideOut_;
	pSuperModel_->localVisibilityBox(bb);

	UmbraObjectProxyPtr pUmbraObject;

	// If this object is a umbra occluder, create a umbra object from its geometry
	Moo::VisualPtr pVisual = pSuperModel_->topModel(0)->getVisual();
	if (pVisual	&& umbraOccluder_)
	{
		// If we have occluder already, create a copy of the object, but retain the models,
		// so that we don't waste memory on multiple instances of the umbra geometry
		pUmbraObject = UmbraObjectProxy::getCopy( umbraModelName_ );
		if (!pUmbraObject.exists())
		{
			// We assume that any occluder object is a static model with only one renderset.
			Moo::Visual::RenderSetVector& renderSets = pVisual->renderSets();
			if (renderSets.size() && renderSets[0].geometry_.size())
			{

				UmbraModelProxyPtr pUmbraModel;

				Moo::Visual::Geometry& geometry = renderSets[0].geometry_[0];

				IF_NOT_MF_ASSERT_DEV( geometry.vertices_ && geometry.primitives_ )
				{
					return;
				}

				// Iterate over the primitive groups and collect the triangles
				std::vector<uint32> indices;
				for (uint32 i = 0; i < geometry.primitiveGroups_.size(); i++)
				{
					Moo::Visual::PrimitiveGroup& pg = geometry.primitiveGroups_[i];
					
					// If the material fails or it has a channel (i.e. sorted) do
					// not create occlusion geometry.
					if (pg.material_->channel() == NULL && pg.material_->begin())
					{
						pg.material_->end();

						BOOL alphaTest = FALSE;
						if (FAILED(pg.material_->pEffect()->pEffect()->GetBool( "alphaTestEnable", &alphaTest )))
							alphaTest = FALSE;

						// Only create occlusion geometry for solid objects
						if (!alphaTest)
						{
							// Copy triangle indices
							const Moo::PrimitiveGroup& primGroup = geometry.primitives_->primitiveGroup(pg.groupIndex_);
							if (geometry.primitives_->indices().format() == D3DFMT_INDEX16)
							{
								const uint16* pInd = (const uint16*)geometry.primitives_->indices().indices();
								indices.insert( indices.end(), pInd + primGroup.startIndex_, 
									pInd + primGroup.startIndex_ + primGroup.nPrimitives_ * 3);
							}
							else
							{
								const uint32* pInd = (const uint32*)geometry.primitives_->indices().indices();
								indices.insert( indices.end(), pInd + primGroup.startIndex_, 
									pInd + primGroup.startIndex_ + primGroup.nPrimitives_ * 3);
							}
						}
					}
				}
				
				if (indices.size())
				{
					// Create occlusion model using our indices
					pUmbraModel = UmbraModelProxy::getMeshModel( &geometry.vertices_->vertexPositions().front(),
						&indices.front(), geometry.vertices_->vertexPositions().size(), indices.size() / 3 );

					// If the occlusion geometry and the model geometry is the same use the occlusion model
					// as both test and write model, otherwise use the bounding box as test model.
					if (indices.size() == geometry.primitives_->indices().size())
					{
						pUmbraObject = UmbraObjectProxy::get( pUmbraModel, pUmbraModel, umbraModelName_ );
					}
					else
					{
						UmbraModelProxyPtr pBBModel = UmbraModelProxy::getObbModel( &geometry.vertices_->vertexPositions().front(),
							geometry.vertices_->vertexPositions().size() );
						pUmbraObject = UmbraObjectProxy::get( pBBModel, pUmbraModel, umbraModelName_ );
					}
				}
			}
		}
	}
	
	// If the umbra object has not been created create a umbra model based on the bounding box of the
	// model, and do not use as occluder.
	if (!pUmbraObject.hasObject())
	{
		UmbraModelProxyPtr pUmbraModel = UmbraModelProxy::getObbModel( bb.minBounds(), bb.maxBounds() );
		pUmbraObject = UmbraObjectProxy::get( pUmbraModel );

		// This is not an occluder
		umbraOccluder_ = false;
	}
	

	UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
	pUmbraChunkItem->init( this, pUmbraObject, m, pChunk_->getUmbraCell() );
	
	pUmbraDrawItem_ = pUmbraChunkItem;
	this->updateUmbraLenders();

	// We only want to be combined with other objects if we are not
	// an occluder
	if (!umbraOccluder_)
	{
		pChunk_->addUmbraDrawItem( pUmbraChunkItem );
	}
#endif

#if FMOD_SUPPORT
	if (SoundManager::instance().modelOcclusionEnabled())
	{
		soundOccluder_.construct( pSuperModel_ );
		soundOccluder_.update( m );
	}
#endif
}

/**
 *	overridden lend method
 */
void ChunkModel::lend( Chunk * pLender )
{
	BW_GUARD;
	if (pSuperModel_ != NULL && pChunk_ != NULL)
	{
		Matrix world( pChunk_->transform() );
		world.preMultiply( this->transform_ );

		BoundingBox bb;
		pSuperModel_->localVisibilityBox( bb );
		bb.transformBy( world );

		this->lendByBoundingBox( pLender, bb );
	}
}


/**
 *	label accessor
 */
const char * ChunkModel::label() const
{
	return label_.c_str();
}


/**
 *	Add this model to (or remove it from) this chunk
 */
void ChunkModel::toss( Chunk * pChunk )
{
	BW_GUARD;
	// remove it from old chunk
	if (pChunk_ != NULL)
	{
		ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
	}

	// call base class method
	this->ChunkItem::toss( pChunk );

	// add it to new chunk
	if (pChunk_ != NULL)
	{
		Matrix world( pChunk_->transform() );
		world.preMultiply( this->transform_ );

		for (int i = 0; i < this->pSuperModel_->nModels(); i++)
		{
			ChunkModelObstacle::instance( *pChunk_ ).addModel(
				this->pSuperModel_->topModel( i ), world, this );
		}
	}
}


void ChunkModel::toss( Chunk * pChunk, SuperModel* extraModel )
{
	BW_GUARD;
	this->ChunkModel::toss(pChunk);

	if (pChunk_ != NULL && extraModel != NULL)
	{
		Matrix world( pChunk_->transform() );
		world.preMultiply( this->transform_ );
		
		ChunkModelObstacle::instance( *pChunk_ ).addModel(
			extraModel->topModel( 0 ), world, this, true );
	}
}


bool ChunkModel::addYBounds( BoundingBox& bb ) const
{
	BW_GUARD;
	if (pSuperModel_)
	{
		BoundingBox me;
		pSuperModel_->localVisibilityBox( me );
		me.transformBy( transform_ );
		bb.addYBounds( me.minBounds().y );
		bb.addYBounds( me.maxBounds().y );
	}
	return true;
}


/**
 * Are we the interior mesh for the chunk?
 *
 * We check by seeing if the model is in the shells directory
 */
bool ChunkModel::isShellModel( const DataSectionPtr pSection ) const
{
	BW_GUARD;
	// Commented out so we can check on load
	/*
	if (!chunk())
		return false;

	if (chunk()->isOutsideChunk())
		return false;
	*/

	if (chunk() && chunk()->isOutsideChunk())
		return false;

	if( !pSuperModel_ || !pSuperModel_->topModel( 0 ) )
		return false;

	if (calculateIsShellModel_)
	{
		calculateIsShellModel_ = false;
		std::string itemRes = pSuperModel_->topModel( 0 )->resourceID();

		cachedIsShellModel_ = itemRes.substr( 0, 7 ) == "shells/" ||
							itemRes.find( "/shells/" ) != std::string::npos;
	}
	return cachedIsShellModel_;
}


/**
 *	This method ticks the animation.
 */
void ChunkModel::tickAnimation()
{
	// If we have an animation and we have not been ticked this frame,
	// update the animation time
	if (pAnimation_ && tickMark_ != ChunkManager::instance().tickMark())
	{
		// Get the delta time since last time the animation time was updated
		float dTime = float(ChunkManager::instance().totalTickTimeInMS() - lastTickTimeInMS_) /
			1000.f;

		// Update our stored tick mark and tick time
		tickMark_ = ChunkManager::instance().tickMark();
		lastTickTimeInMS_ = ChunkManager::instance().totalTickTimeInMS();

		// Update animation time
		pAnimation_->time += dTime * animRateMultiplier_;
		float duration = pAnimation_->pSource( *pSuperModel_ )->duration_;
		pAnimation_->time -= int64( pAnimation_->time / duration ) * duration;
	}
}


// chunk_model.cpp
