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

#include "super_model.hpp"

//#include "cstdmf/concurrency.hpp"
//#include "cstdmf/dogwatch.hpp"
//#include "math/mathdef.hpp"
//#include "resmgr/bwresource.hpp"
//#include "resmgr/auto_config.hpp"
//#include "resmgr/sanitise_helper.hpp"
//#include "moo/moo_math.hpp"
//#include "math/blend_transform.hpp"
//#include "moo/node.hpp"
//#include "moo/node_catalogue.hpp"
//#include "moo/visual.hpp"
//#include "moo/visual_manager.hpp"
//#include "moo/base_texture.hpp"
//#include "moo/texture_manager.hpp"
//#include "moo/animation_manager.hpp"
//#include "moo/discrete_animation_channel.hpp"
//#include "moo/interpolated_animation_channel.hpp"
//#include "moo/streamed_animation_channel.hpp"
//#include "moo/render_context.hpp"
//#include "moo/morph_vertices.hpp"
//#include "moo/render_target.hpp"
//#include "moo/visual_manager.hpp"
//#include "moo/visual.hpp"
//#include "moo/visual_channels.hpp"
//#include "romp/lod_settings.hpp"
//#include "model_common.hpp"
//#include "romp/geometrics.hpp"
#ifdef EDITOR_ENABLED
#include "../../tools/common/material_properties.hpp"
#endif

//#include "physics2/bsp.hpp"

#include <set>

#include "moo/morph_vertices.hpp"
#include "romp/lod_settings.hpp"
#include "romp/static_light_values.hpp"

#include "model_actions_iterator.hpp"
#include "super_model_dye.hpp"
#include "super_model_animation.hpp"
#include "super_model_action.hpp"
#include "tint.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 *	Init the material and its default tint and return how many we found of it.
 */
int initMatter_NewVisual( Matter & m, Moo::Visual & bulk )
{
	BW_GUARD;
	Moo::EffectMaterialPtr pOriginal;
	Matter::PrimitiveGroups primGroups;

	int num = bulk.gatherMaterials( m.replaces_, primGroups, &pOriginal );
	if (num == 0)
		return num;

	std::swap<>( primGroups, m.primitiveGroups_ );

	MF_ASSERT_DEV( pOriginal != NULL );
	MF_ASSERT_DEV( !m.tints_.empty() );
	for (uint i=0; i < m.tints_.size(); i++)
	{
		if( pOriginal.exists() )
			m.tints_[i]->effectMaterial_ = new Moo::EffectMaterial( *pOriginal );
		else
			m.tints_[i]->effectMaterial_ = new Moo::EffectMaterial;
	}

	return num;
}

/**
 *	Init the tint from the given material section, return true if ok.
 *	Should always leave the Tint in a good state.
 */
bool initTint_NewVisual( Tint & t, DataSectionPtr matSect )
{
	BW_GUARD;
	t.effectMaterial_ = NULL;
	t.effectMaterial_ = new Moo::EffectMaterial();

	if (!matSect)
		return false;

	return t.effectMaterial_->load( matSect );
}




Moo::EffectPropertyPtr findTextureProperty( Moo::EffectMaterialPtr m )
{
	BW_GUARD;
	if ( m && m->pEffect() )
	{
		ComObjectWrap<ID3DXEffect> pEffect = m->pEffect()->pEffect();

		Moo::EffectMaterial::Properties& properties = m->properties();
		Moo::EffectMaterial::Properties::iterator it = properties.begin();
		Moo::EffectMaterial::Properties::iterator end = properties.end();

		while ( it != end )
		{
			IF_NOT_MF_ASSERT_DEV( it->second )
			{
				continue;
			}
			D3DXHANDLE hParameter = it->first;
			Moo::EffectPropertyPtr& pProperty = it->second;

			D3DXPARAMETER_DESC desc;
			HRESULT hr = pEffect->GetParameterDesc( hParameter, &desc );
			if ( SUCCEEDED(hr) )
			{
				if (desc.Class == D3DXPC_OBJECT &&
					desc.Type == D3DXPT_TEXTURE)
				{
					return pProperty;
				}
			}
			it++;
		}
	}

	return NULL;
}




// ----------------------------------------------------------------------------
// Section: SuperModel
// ----------------------------------------------------------------------------





/**
 *	SuperModel constructor
 */
SuperModel::SuperModel( const std::vector< std::string > & modelIDs ) :
	lod_( 0.f ),
	nModels_( 0 ),
	lodNextUp_( -1.f ),
	lodNextDown_( 1000000.f ),
	checkBB_( true ),
	redress_( false ),
	isOK_( true )
{
	BW_GUARD;
	for (uint i = 0; i < modelIDs.size(); i++)
	{
		ModelPtr pModel = Model::get( modelIDs[i] );
		if (pModel == NULL)
		{
			isOK_ = false;
			continue;	// Model::get will already have reported an error
		}

		ModelStuff	ms;
		ms.topModel = pModel;
		ms.curModel = ms.topModel.getObject();
		ms.preModel = NULL;
		models_.push_back( ms );

		lodNextDown_ = min( lodNextDown_, ms.topModel->extent() );
	}
	nModels_ = models_.size();
}

/**
 *	SuperModel destructor
 */
SuperModel::~SuperModel()
{
}


#ifdef EDITOR_ENABLED
/**
 *	This function reload all models from the disk
 */
void SuperModel::reload()
{
	BW_GUARD;
	for (std::vector<ModelStuff>::iterator iter = models_.begin();
		iter != models_.end(); ++iter)
	{
		iter->topModel->reload();
	}
}
#endif//EDITOR_ENABLED

/**
 *	Draw this SuperModel at the current Moo::rc().world() transform
 */
float SuperModel::draw( const FashionVector * pFashions, int nLateFashions,
	float atLod, bool doDraw )
{
	BW_GUARD;
	Model * pModel;

	const Matrix & mooWorld = Moo::rc().world();
	const Matrix & mooView = Moo::rc().view();

	// Step 1: Figure out if our bounding box is visible
	// if (bb.calculateOutcode( Moo::rc().worldViewProjection() )) return;

	// Step 2: Figure out what LODs we want to use
	if (atLod < 0.f)
	{
		// we want the z component from the object-to-view matrix
		//Matrix objectToView = Moo::rc().world() x_multiply_x Moo::rc().view()
		//lod_ = objectToView[4-1][3-1] / max( Moo::rc().world().yscale, 1.f )
		float distance = mooWorld.applyToOrigin().dotProduct(
			Vector3( mooView._13, mooView._23, mooView._33 ) ) + mooView._43;
		float yscale = mooWorld.applyToUnitAxisVector(1).length();

		distance = LodSettings::instance().applyLodBias(distance);
		lod_ = (distance / max( 1.f, yscale )) * Moo::rc().zoomFactor();
	}
	else
	{
		lod_ = atLod;
	}

	IF_NOT_MF_ASSERT_DEV( -100000.f < mooWorld.applyToOrigin().x &&
		mooWorld.applyToOrigin().x < 100000.f &&
		-100000.f < mooWorld.applyToOrigin().z &&
		mooWorld.applyToOrigin().z < 100000.f )
	{
		ERROR_MSG( "SuperModel::draw: Tried to draw a SuperModel at bad "
			"translation: ( %f, %f, %f )\n",
			mooWorld.applyToOrigin().x,
			mooWorld.applyToOrigin().y,
			mooWorld.applyToOrigin().z);
		return lod_;
	}

	if ((lod_ > lodNextDown_) || (lod_ <= lodNextUp_))
	{
		for (int i=0; i < nModels_; i++)
		{
			models_[i].preModel = NULL;
			Model* model = models_[i].topModel.getObject();

			while ((model) && (model->extent() != -1.f) && (model->extent() < lod_))
			{

				if ((model->extent() != 0.f) &&	// The current model is visible AND either
					((models_[i].preModel == NULL) ||	//the previous model doesn't exist OR
						(model->extent() > models_[i].preModel->extent())))	// it is closer than the current model
				{
					models_[i].preModel = model;	// Save it as the previous one...
				}

				model = model->parent();	// And move on to the next
			}

			models_[i].curModel = model;

		}

		lodNextUp_ = -1.f;
		lodNextDown_ = 1000000.f;

		for (int i=0; i < nModels_; i++)
		{
			ModelStuff & ms = models_[i];
			if (ms.preModel != NULL) lodNextUp_ =
				max( lodNextUp_, ms.preModel->extent() );
			if (ms.curModel != NULL) lodNextDown_ =
				min( lodNextDown_, ms.curModel->extent() );
		}
	}

	static DogWatch fashionTimer( "Fashion" );

	fashionTimer.start();


	// Step 3: Dress up in the latest fashion
	Model::incrementBlendCookie();
	redress_ = false;

	// HACK: increment the morph animation cookie.
	Moo::MorphVertices::incrementCookie();

	FashionVector::const_iterator fashIt;
	if (pFashions != NULL)
	{
		FashionVector::const_iterator fashLate =
			pFashions->end() - nLateFashions;
		for (fashIt = pFashions->begin(); fashIt != fashLate; fashIt++)
		{
			(*fashIt)->dress( *this );
		}
	}

	for (int i = 0; i < nModels_; i++)
	{
		if ((pModel = models_[i].curModel) != NULL)
		{
			pModel->dress();
		}
	}

	if (pFashions != NULL)
	{
		FashionVector::const_iterator fashEnd = pFashions->end();
		for (; fashIt != fashEnd; fashIt++)
		{
			(*fashIt)->dress( *this );
		}
	}

	// HACK!
	if (redress_) for (int i = 0; i < nModels_; i++)
	{
		if ((pModel = models_[i].curModel) != NULL)
		{
			pModel->dress();
		}
	}
	// !HACK

	fashionTimer.stop();

	{
		static DogWatch modelDrawTimer( "ModelDraw" );

		modelDrawTimer.start();

		// Step 4: Draw all the models
		for (int i = 0; i < nModels_; i++)
		{
			if ((pModel = models_[i].curModel) != NULL)
			{
				// commit the dye setup per model.
				pModel->soakDyes();
				if (doDraw)
					pModel->draw(checkBB_);
			}
		}
		modelDrawTimer.stop();
	}

	// Step 5: undress
	if( pFashions != NULL )
	{
		FashionVector::const_reverse_iterator fashRIt;
		for( fashRIt = pFashions->rbegin(); fashRIt != pFashions->rend(); fashRIt++ )
		{
			(*fashRIt)->undress( *this );
		}
	}

	// Step 6: Profit
	return lod_;
}


/**
 *	Dress this model up in its default fashions, but don't draw it.
 */
void SuperModel::dressInDefault()
{
	BW_GUARD;
	SuperModelAnimationPtr smap = this->getAnimation( "" );
	smap->blendRatio = 1.f;

	Moo::rc().push();
	Moo::rc().world( Matrix::identity );

	Model::incrementBlendCookie();

	smap->dress( *this );

	for (int i = 0; i < nModels_; i++)
	{
		models_[i].topModel->dress();
	}

	Moo::rc().pop();
}


/**
 *	This method gets an animation from the SuperModel. The animation
 *	is only good for this supermodel - don't try to use it with other
 *	supermodels.
 */
SuperModelAnimationPtr SuperModel::getAnimation( const std::string & name )
{
	BW_GUARD;
	if (nModels_ == 0) return NULL;

	char * pBytes = new char[sizeof(SuperModelAnimation)+(nModels_-1)*sizeof(int)];
	// since this is a simple char array, I believe that we don't
	//  have to delete [] it.

	SuperModelAnimation * pAnim = new ( pBytes ) SuperModelAnimation( *this, name );

	return pAnim;
}



/**
 *	This method gets an action from the SuperModel. The action
 *	is only good for this supermodel - don't try to use it with other
 *	supermodels.
 */
SuperModelActionPtr SuperModel::getAction( const std::string & name )
{
	BW_GUARD;
	if (nModels_ == 0) return NULL;

	char * pBytes = new char[sizeof(SuperModelAction)+(nModels_-1)*sizeof(int)];

	SuperModelActionPtr pAction = new ( pBytes ) SuperModelAction( *this, name );

	return pAction->pSource_ != NULL ? pAction : SuperModelActionPtr(NULL);
}


/**
 *	This method gets all the matchable actions in any of the Models in
 *	this SuperModel.
 */
void SuperModel::getMatchableActions(
	std::vector< SuperModelActionPtr > & actions )
{
	BW_GUARD;
	actions.clear();

	// make a set of strings
	std::vector<const std::string*>	matchableNames;

	// add the names of each model's matchable actions to it
	for (int i = nModels_ - 1; i >= 0; i--)
	{
		Model * tM = models_[i].topModel.getObject();
		const ModelAction * pAct;

		for (ModelActionsIterator it = tM->lookupLocalActionsBegin();
			it != tM->lookupLocalActionsEnd() && (pAct = &*it) != NULL;
			it++)
		{
			if (pAct->isMatchable_)
			{
				matchableNames.push_back( &pAct->name_ );
			}
		}
	}

	// iterate over our own actions first then our parent's
	std::set<std::string> seenNames;
	for (int i = 0; i <= int(matchableNames.size())-1; i++)
	{
		// only add this one if we haven't seen it before
		if (seenNames.insert( *matchableNames[i] ).second == true)
		{
			actions.push_back( this->getAction( *matchableNames[i] ) );
		}
	}
}



/**
 *	This method gets a dye from the SuperModel. The dye is only good
 *	for this supermodel - don't try to use it with other supermodels.
 */
SuperModelDyePtr SuperModel::getDye( const std::string & matter,
	const std::string & tint)
{
	BW_GUARD;
	if (nModels_ == 0)
		return NULL;

	SuperModelDyePtr pDye = new SuperModelDye( *this, matter, tint );

	return pDye->effective( *this ) ? pDye : SuperModelDyePtr(NULL);
}


/**
 *	This returns the number of triangles in the supermodel.
 */
int SuperModel::numTris() const
{
	BW_GUARD;
	int tris = 0;

	for (int i = 0; i < nModels_; i++)
	{
		Moo::VisualPtr pVisual = this->models_[ i ].topModel->getVisual();

		if (pVisual)
		{
			tris += pVisual->nTriangles();
		}
	}

	return tris;
}


/**
 *	This returns the number of primitive groups in the supermodel.
 */
int SuperModel::numPrims() const
{
	BW_GUARD;
	int prims = 0;

	for (int i = 0; i < nModels_; i++)
	{
		Moo::VisualPtr pVisual = this->models_[ i ].topModel->getVisual();

		if (pVisual)
		{
			for (Moo::Visual::RenderSetVector::iterator rsit = pVisual->renderSets().begin();
				rsit != pVisual->renderSets().end(); ++rsit)
			{
				for (Moo::Visual::GeometryVector::iterator git = (*rsit).geometry_.begin();
					git != (*rsit).geometry_.end(); ++git)
				{
					prims += (*git).primitiveGroups_.size();
				}
			}
		}
	}

	return prims;
}


/**
 *	Get the root node. Assumes that all models have the same named
 *	root node as 'Scene Root'
 */
Moo::NodePtr SuperModel::rootNode()
{
	BW_GUARD;
	return Moo::NodeCatalogue::find( "Scene Root" );
}

/**
 *	Get the node by name.
 */
Moo::NodePtr SuperModel::findNode( const std::string & nodeName,
	MooNodeChain * pParentChain )
{
	BW_GUARD;
	// see if any model has this node (and get its node pointer)
	Moo::NodePtr pFound = Moo::NodeCatalogue::find( nodeName.c_str() );
	if (!pFound) return NULL;

	// see if one of our models has this node
	Moo::Node * pMaybe = pFound.getObject();
	for (int i = 0; i < nModels_; i++)
	{
		if (models_[i].topModel->hasNode( pMaybe, pParentChain ))
			return pFound;
	}

	// see if it's the root node (bit of a hack for nodeless models)
	if (nodeName == "Scene Root")
	{
		if (pParentChain != NULL) pParentChain->clear();
		return pFound;
	}

	// ok, no such node then
	return NULL;
}


/**
 *	Calculate the bounding box of this supermodel
 */
void SuperModel::localBoundingBox( BoundingBox & bbRet ) const
{
	BW_GUARD;
	if (nModels_ != 0)
	{
		bbRet = models_[0].topModel->boundingBox();

		for (int i = 1; i < nModels_; i++)
		{
			bbRet.addBounds( models_[i].topModel->boundingBox() );
		}
	}
	else
	{
		bbRet = BoundingBox( Vector3(0,0,0), Vector3(0,0,0) );
	}
}

/**
 *	Calculate the bounding box of this supermodel
 */
void SuperModel::localVisibilityBox( BoundingBox & vbRet ) const
{
	BW_GUARD;
	if (nModels_ != 0)
	{
		vbRet = models_[0].topModel->visibilityBox();

		for (int i = 1; i < nModels_; i++)
		{
			vbRet.addBounds( models_[i].topModel->visibilityBox() );
		}
	}
	else
	{
		vbRet = BoundingBox( Vector3(0,0,0), Vector3(0,0,0) );
	}
}

#ifndef CODE_INLINE
	#include "super_model.ipp"
#endif
