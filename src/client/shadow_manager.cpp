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
#include "shadow_manager.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"
#include "duplo/chunk_dynamic_obstacle.hpp"
#include "duplo/pymodel.hpp"
#include "duplo/shadow_caster_common.hpp"
#include "entity.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/render_context.hpp"
#include "moo/visual_channels.hpp"
#include "resmgr/datasection.hpp"
#include "romp/flora.hpp"
#include "romp/time_of_day.hpp"
#include "speedtree/speedtree_renderer.hpp"

DECLARE_DEBUG_COMPONENT2( "App", 0 );

typedef SmartPointer<PyModel> PyModelPtr;

namespace
{
	// Each entity can have a PyModel or PyModelObstacle, they are
	// unrelated classes so we need to do some magic to figure them
	// out.
	void GetPyModelOrObstacleFromEntity(Entity *			entity,
										PyModel*&			pyModel,
										PyModelObstacle*&	pyModelObstacle )
	{
		BW_GUARD;
		pyModel			= entity->pPrimaryModel();
		pyModelObstacle = NULL;

		// Its an PyModelObstacle if it has an embodiment and the 
		// embodiment checks out as a PyModelObstacle.
		ChunkEmbodiment*	pShadowEmb	= entity->pPrimaryEmbodiment();
		if ( !pyModel && pShadowEmb && 
			PyModelObstacle::Check( pShadowEmb->pPyObject().getObject() ) )
		{
			pyModelObstacle = static_cast< PyModelObstacle* >
				( pShadowEmb->pPyObject().getObject() );							
		}
	}
}

ShadowManager::ShadowManager()
:	maxShadowCount_(4),
	curShadowCount_(4),
	halfRes_( true ),
	active_( true )
{
}

ShadowManager::~ShadowManager()
{

}

ShadowManager& ShadowManager::instance()
{
	static ShadowManager inst;
	return inst;
}

/**
 *	This method inits the shadow manager setting up the properties
 *  used by the shadows.
 *	@param pSection the datasection that contains the shadow settings
 */
void ShadowManager::init( DataSectionPtr pSection )
{
	BW_GUARD;
	bool supported = false;

	// Read in the enabled flag.
	bool enabled = pSection->readBool( "enabled", true );
		
	if (enabled &&
		Moo::rc().supportsTextureFormat( D3DFMT_G16R16F ) &&
		Moo::rc().vsVersion() >= 0x200 &&
		Moo::rc().psVersion() >= 0x200)
	{
		UINT texMem = Moo::rc().device()->GetAvailableTextureMem();

		// Read the half res property, it ensures that the dimensions of
		// additional render targets are halved.
		halfRes_ = pSection->readBool( "halfRes", halfRes_ );

		// Read in the number of shadow casters to use.
		this->maxShadowCount_ = pSection->readInt( "maxCount", this->maxShadowCount_ );
		
		// get nearest of two
		float lValue = logf(float(this->maxShadowCount_))/logf(2.f);
		float power = floorf(lValue);
		this->maxShadowCount_ = int(powf(2,power));

		// Read the common properties for the shadow casters
		pShadowCommon_ = new ShadowCasterCommon;
		pShadowCommon_->init( pSection );

		supported = true;

		INFO_MSG( "ShadowManager::init: Tex mem delta %dkb\n", 
			(Moo::rc().device()->GetAvailableTextureMem() - texMem) / 1024 );
	}
	else
	{
		active_   = false;
		supported = false;
		INFO_MSG( "ShadowManager::init - shadows not supported on this hardware" );
	}

	//
	// Register graphics settings
	//		
	
	// shadow quality settings
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;
	qualitySettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"SHADOWS_QUALITY", "Shadows Quality", *this, 
			&ShadowManager::setQualityOption,
			-1, false, false);
			
	qualitySettings_->addOption("HIGH",  "High", supported);
	qualitySettings_->addOption("MEDIUM", "Medium", supported);
	qualitySettings_->addOption("LOW", "Low", supported);
	qualitySettings_->addOption("OFF", "Off", true);
	Moo::GraphicsSetting::add(qualitySettings_);
	
	// number of shadows settings
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;
	castersSettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"SHADOWS_COUNT", "Shadows Count", *this, 
			&ShadowManager::setCastersOption, 
			-1, false, false);
				
	int shadowCount = this->maxShadowCount_ << 1;
	while (shadowCount > 0)
	{
		shadowCount = shadowCount >> 1;
		std::stringstream countStr;
		countStr << shadowCount;
		castersSettings_->addOption(countStr.str(), countStr.str(), supported || shadowCount == 0);
	}
	int optionsCount = castersSettings_->options().size();
	Moo::GraphicsSetting::add(castersSettings_);

	Moo::EffectManager::instance().addListener(this);
}

void ShadowManager::initCasters(int shadowCount)
{
	BW_GUARD;
	this->curShadowCount_ = shadowCount;

	// create shadowCount_ shadow casters.
	pCasters_.clear();
	Moo::RenderTarget* pBaseTarget = NULL;
	for (uint32 i=0; i<this->curShadowCount_; i++)
	{
		ShadowCasterPtr pCaster = new ShadowCaster;
		if (pCaster->init(pShadowCommon_, (i>0) && halfRes_, i, pBaseTarget))
		{
			pCasters_.push_back(pCaster);
		}
		
		if (pBaseTarget == NULL)
		{
			pBaseTarget = pCaster->shadowTarget();
		}
	}
}

static DogWatch s_tick("shadowTick");
static DogWatch s_capture("shadowCapture");
static DogWatch s_render("shadowRender");

/**
 * Tick method for the shadow manager.
 * This method sorts the entities front to back, selecting the closest
 * entities for shadowing.
 */
void ShadowManager::tick( float dTime )
{
	BW_GUARD;
	s_tick.start();
	if (active_ && pCasters_.size() > 0)
	{
		// Get the viewprojection
		const Matrix& vp = Moo::rc().viewProjection();

		// list of distances the shadow entities are from the camera
		static VectorNoDestructor<float> distances;
		distances.clear();

		// Go through our on hold entities
		// Entities without a model are kept on hold until they get a model.
		// If they get one they are added to the entity list.
		uint32 i = 0;
		while (i < pEntitiesOnHold_.size())
		{
			Entity * entity = this->getEntityOnHold(i);
			if (entity != NULL)
			{
				// Entity may hold a PyModel or PyModelObstacle or nothing.
				PyModel*			pModel			= NULL;
				PyModelObstacle*	pModelObstacle	= NULL;
				GetPyModelOrObstacleFromEntity( entity, pModel, pModelObstacle );

				if (pModel != NULL || pModelObstacle != NULL)
				{
					pEntities_.push_back( pEntitiesOnHold_[i] );
					pEntitiesOnHold_[i] = pEntitiesOnHold_.back();
					pEntitiesOnHold_.pop_back();
				}
				else
				{
					i++;
				}
			}
			else
			{
				// remove invalid weakref
				pEntitiesOnHold_[i] = pEntitiesOnHold_.back();
				pEntitiesOnHold_.pop_back();
			}
		}

		// Iterate over our entities and calculate the distance
		// from the camera
		i = 0;
		while (i < pEntities_.size())
		{
			Entity * entity = this->getEntity(i);
			if (entity != NULL)
			{
				// Entity may hold a PyModel or PyModelObstacle or nothing.
				PyModel*			pModel			= NULL;
				PyModelObstacle*	pModelObstacle	= NULL;
				GetPyModelOrObstacleFromEntity( entity, pModel, pModelObstacle );

				if ( pModel || pModelObstacle )
				{
					Matrix		m;
					BoundingBox bb;
			
					if ( pModel )
					{
						m	= pModel->worldTransform();
						pModel->localBoundingBox(bb,true);
					}
					else
					{
						m	= pModelObstacle->worldTransform();
						pModelObstacle->localBoundingBox(bb,true);
					}

					// Calculate the worldViewProj matrix
					m.postMultiply( vp );
					// Get the model boundingbox, if it's off screen it 
					// doesn't get a shadow.
					bb.calculateOutcode( m );
					
					float dist = FLT_MAX;
					if (!bb.combinedOutcode())
					{
						dist = m[3][3];
					}
					
					distances.push_back(dist);
					i++;
				}
				else
				{
					// If the entity has lost its model we will
					// put it on hold again.
					pEntitiesOnHold_.push_back( pEntities_[i] );
					pEntities_[i] = pEntities_.back();
					pEntities_.pop_back();
				}
			}
			else
			{
				// remove invalid weakref
				pEntities_[i] = pEntities_.back();
				pEntities_.pop_back();
			}
		}

		// Sort the distances, but only sort the number of shadow targets.
		for (uint32 i = 0; i < distances.size();i++)
		{
			for (uint32 j = 0; (j < distances.size()) && (j < this->curShadowCount_); j++)
			{
				if (distances[j] > distances[i])
				{
					std::swap( distances[j], distances[i] );
					std::swap( pEntities_[j], pEntities_[i] );
				}
			}
		}
	}
	s_tick.stop();
}

/**
 *	This method captures the shadows for the entities
 *	The closest entities are rendered into individual
 *	shadow maps, for later being applied as shadows to
 *	the scenery.
 */
void ShadowManager::captureShadows()
{
	BW_GUARD;
	s_capture.start();
	if (active_ && pCasters_.size() > 0)
	{
		pEmbodiments_.clear();
		if (ChunkManager::instance().cameraSpace() )
		{
			EnviroMinder & enviro =
				ChunkManager::instance().cameraSpace()->enviro();

			// Iterate over the shadow casters and entities and render the model
			// of the entity to the shadow render target.
			for (uint32 i = 0; (i < pCasters_.size()) && (i < pEntities_.size()); i++)
			{
				Entity * entity = this->getEntity(i);
				if (entity != NULL)
				{
					ShadowCasterPtr pCaster = pCasters_[i];
					
					// Entity may hold a PyModel or PyModelObstacle
					PyModel*			pModel			= NULL;
					PyModelObstacle*	pModelObstacle	= NULL;
					ChunkEmbodimentPtr  pShadowEmb      = entity->pPrimaryEmbodiment();
					GetPyModelOrObstacleFromEntity( entity, pModel, pModelObstacle );

					BoundingBox bb; 
					if ( pModel )
					{
						pModel->localBoundingBox(bb,true);
						bb.transformBy( pModel->worldTransform() );
					}
					else 
					{
						pModelObstacle->localBoundingBox(bb,true);
						bb.transformBy( pModelObstacle->worldTransform() );
					}

					// If we are rendering outside use the sun/moon direction as 
					// the shadowing direction.
					Vector3 lightTrans( enviro.timeOfDay()->lighting().mainLightDir() );				

					// If we are inside, use a static direction vector.
					if (pShadowEmb->chunk() && !pShadowEmb->chunk()->isOutsideChunk())
					{
						lightTrans.set( -.85f, -.53f, -.1f );
						lightTrans.normalise();
					}

					// Fade out the shadows when the angle gets too sharp..
					float angle = fabsf(atan2f( -lightTrans.x, -lightTrans.y ));
					angle = min( angle, DEG_TO_RAD(80.f)) / DEG_TO_RAD( 80 );
					float intensity = min((1.f - angle ) * 30.f, 1.f);
					pCaster->angleIntensity( intensity );

					// Render the shadow
					pCaster->begin( bb, lightTrans );
					pShadowEmb->draw();
					pCaster->end();

					// Store the embodiment of the shadow entity.
					pEmbodiments_.push_back( pShadowEmb );
				}
			}
		}
	}
	s_capture.stop();
}

/**
 *	This method renders shadows on the environment, needs to be called 
 *	after everything has been rendered.
 */
void ShadowManager::renderShadows( bool useTerrain )
{
	BW_GUARD;
	//store information for during renderFloraShadows
	nEmbodiments_ = pEmbodiments_.size();

	s_render.start();
	if (active_ && pCasters_.size() > 0)
	{
		// Iterate over shadow casters and caster embodiments and
		// render the shadows.
		for (uint32 i = 0; (i < pCasters_.size()) && (i < nEmbodiments_); i++)
		{
			speedtree::SpeedTreeRenderer::beginFrame(
				NULL, pCasters_[i].getObject());

			pCasters_[i]->beginReceive( useTerrain );
			pEmbodiments_[i]->draw();
			pCasters_[i]->endReceive();

			speedtree::SpeedTreeRenderer::endFrame();
		}
		pEmbodiments_.clear();
	}
	s_render.stop();
}

void ShadowManager::renderFloraShadows( Flora* flora )
{
	BW_GUARD;
	if (active_ && pCasters_.size() > 0)
	{
		// Iterate over shadow casters and caster embodiments and
		// render the shadows.
		for (uint32 i = 0; (i < pCasters_.size()) && (i < nEmbodiments_); i++)
		{			
			flora->drawShadows( pCasters_[i] );		
		}		
	}	
}

/**
 *	This method destroys the shadows resources, this should 
 *	be called at the end of the app.
 */
void ShadowManager::fini()
{
	BW_GUARD;
	Moo::EffectManager::instance().delListener(this);

	pEntities_.clear();
	pCasters_.clear();
	pEmbodiments_.clear();
	pEntitiesOnHold_.clear();
	
	if (pShadowCommon_)
	{
		delete pShadowCommon_;
		pShadowCommon_ = NULL;
	}
}

/**
 *	This method adds an entity to the potential shadow caster list.
 *	@param pEntity the entity to add
 */
void ShadowManager::addShadowEntity( Entity * pEntity )
{
	BW_GUARD;
	if (pEntity != NULL)
	{
		PyObject * entity = PyWeakref_NewProxy(static_cast<PyObject*>(pEntity), NULL);
		if (entity != NULL)
		{
			pEntitiesOnHold_.push_back(PyObjectPtr(entity));
		}
	}
}

/**
 *	Finds entity in list of weakrefs to entities.
 */
struct FindEntity
{
	FindEntity(Entity * entity) : entity_(entity) {}
	bool operator() (PyObjectPtr object)
	{
		Entity * entity = static_cast<Entity*>(
			(PyObject*)PyWeakref_GET_OBJECT(object.getObject()));
		return entity == entity_;
	}
	Entity * entity_;
};

/**
 *	This method removes an entity from the potential shadow caster list.
 *	@param pEntity the entity to add
 */
void ShadowManager::delShadowEntity( Entity * pEntity )
{
	BW_GUARD;
	if (pEntity == NULL)
	{
		return;
	}
	
	// Check both lists of entities and remove the entity if found.
	std::vector< PyObjectPtr >::iterator it;
	it = std::find_if( pEntities_.begin(), pEntities_.end(), FindEntity(pEntity));
	if (it != pEntities_.end())
		pEntities_.erase( it );
	it = std::find_if( pEntitiesOnHold_.begin(), pEntitiesOnHold_.end(), FindEntity(pEntity));
	if (it != pEntitiesOnHold_.end())
		pEntitiesOnHold_.erase( it );
}

/**
 *	Returns pointer to entity in list of shadow 
 *	entities or NULL if entity is no longer with us.
 */
Entity * ShadowManager::getEntity(int index)
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV(uint(index) < pEntities_.size())
	{
		return NULL;
	}

	return static_cast<Entity*>(
		(PyObject*)PyWeakref_GET_OBJECT(pEntities_[index].getObject()));
}

/**
 *	Returns pointer to entity in list of entities 
 *	on holdor NULL if entity is no longer with us.
 */
Entity * ShadowManager::getEntityOnHold(int index)
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV(uint(index) < pEntitiesOnHold_.size())
	{
		return NULL;
	}

	return static_cast<Entity*>(
		(PyObject*)PyWeakref_GET_OBJECT(pEntitiesOnHold_[index].getObject()));
}

/*virtual*/ void ShadowManager::onSelectPSVersionCap(int psVerCap)
{
	this->setQualityOption( qualitySettings_->activeOption() );
	this->setCastersOption( castersSettings_->activeOption() );
}

/**
 *	Adjsuts shadow quality. Implicitly called whenever
 *	the user changes the SHADOWS_QUALITY setting.
 */
void ShadowManager::setQualityOption(int optionIndex)
{
	BW_GUARD;
	if (optionIndex < 3 && (Moo::EffectManager::instance().PSVersionCap() > 1))
	{
		this->pShadowCommon_->shadowQuality(2-optionIndex);
		this->active_ = true;
	}
	else
	{
		this->active_ = false;
	}
}

/**
 *	Sets the number of shadow casters. Implicitly called 
 *	whenever the user changes the SHADOWS_COUNT setting.
 */
void ShadowManager::setCastersOption(int optionIndex)
{
	BW_GUARD;
	if (Moo::EffectManager::instance().PSVersionCap() > 1)
	{
		int shadowsCount = this->maxShadowCount_ >> optionIndex;
		this->active_ = shadowsCount > 0;
		this->initCasters(shadowsCount);
	}
	else
	{
		this->active_ = false;
	}
}

/*~ function BigWorld.addShadowEntity
 *
 *	Adds the given entity to the dynamic shadow manager. The manager
 *	holds onto a weak reference to the entity.
 *
 *	@param Entity	entity to be added to the shadow manager.
 */
void addShadowEntity( EntityPtr pEntity )
{
	BW_GUARD;
	ShadowManager::instance().addShadowEntity( pEntity.getObject() );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, addShadowEntity, ARG( EntityPtr, END ), BigWorld )

/*~ function BigWorld.delShadowEntity
 *
 *	Removes the given entity from the dynamic shadow manager. Does nothing 
 *	if the entity was not previously added.
 *
 *	@param Entity	entity to be removed from the shadow manager.
 */
void delShadowEntity( EntityPtr pEntity )
{
	BW_GUARD;
	ShadowManager::instance().delShadowEntity( pEntity.getObject() );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, delShadowEntity, ARG( EntityPtr, END ), BigWorld )

// shadow_manager.cpp