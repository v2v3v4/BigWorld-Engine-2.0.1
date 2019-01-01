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

#include "nodeless_model.hpp"

#include "moo/visual_manager.hpp"

#include "super_model.hpp"
#include "nodeless_model_static_lighting.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


Moo::NodePtr NodelessModel::s_sceneRootNode_ = NULL;



/**
 *	Constructor
 */
NodelessModel::NodelessModel( const std::string & resourceID, DataSectionPtr pFile ) :
	SwitchedModel<Moo::VisualPtr>( resourceID, pFile ),
	batched_( false ),
	occluder_( false )
{
	BW_GUARD;
	// load standard switched model stuff
	if (!this->wireSwitch( pFile, &loadVisual, "nodelessVisual", "visual" ))
		return;

#ifndef EDITOR_ENABLED
	// see if we're batching it
	batched_ = pFile->readBool( "batched", false );
#endif

	occluder_ = pFile->readBool( "dpvsOccluder", false );
	occluder_ = pFile->readBool( "umbraOccluder", occluder_ );

	// load dyes
	this->readDyes( pFile, true );
}


#ifdef EDITOR_ENABLED
/**
 *	This function reload the current model from the disk.
 *	This function isn't thread safe. It will only be called
 *	For reload an externally-modified model in WorldEditor
 */
void NodelessModel::reload()
{
	std::string res = resourceID_;

	long rc = refCount();
	this->~NodelessModel();
	new (this) NodelessModel( res, BWResource::openSection( res ) );
	addToMap();
	incRef( rc );
}
#endif//EDITOR_ENABLED

/**
 *	This method dresses this nodeless model.
 */
void NodelessModel::dress()
{
	BW_GUARD;
	this->SwitchedModel<Moo::VisualPtr>::dress();

	if (s_sceneRootNode_ != NULL)
	{
		s_sceneRootNode_->blendClobber(
			Model::blendCookie(), Matrix::identity );
		s_sceneRootNode_->visitSelf( Moo::rc().world() );
	}
}


/**
 *	This method draws this nodeless model.
 */
void NodelessModel::draw( bool checkBB )
{
	BW_GUARD;
#ifndef EDITOR_ENABLED
	if (!batched_)
		frameDraw_->draw( !checkBB, false );
	else
		frameDraw_->batch( !checkBB, false );
#else
		frameDraw_->draw( !checkBB, false );
#endif
}


/**
 *	This method returns a BSPTree of this model when decomposed
 */
const BSPTree * NodelessModel::decompose() const
{
	BW_GUARD;
	return bulk_ ? bulk_->pBSPTree() : NULL;
}


/**
 *	This method returns the bounding box of this model
 */
const BoundingBox & NodelessModel::boundingBox() const
{
	BW_GUARD;
	if (!bulk_)
		return BoundingBox::s_insideOut_;;
	return bulk_->boundingBox();
}

/**
 *	This method returns the bounding box of this model
 */
const BoundingBox & NodelessModel::visibilityBox() const
{
	BW_GUARD;
	return boundingBox();
}


/**
 *	This method gets our kind of static lighting
 */
ModelStaticLightingPtr NodelessModel::getStaticLighting(
	StaticLightValueCachePtr cache, const DataSectionPtr section )
{
	BW_GUARD;

	if (section)
	{
		StaticLightValuesPtr pSLV = new StaticLightValues( cache, section, bulk_->nVertices() );

		if (pSLV->valid())
		{
			return new NodelessModelStaticLighting( bulk_, pSLV );
		}
	}

	return NULL;
}



MaterialOverride NodelessModel::overrideMaterial( const std::string& identifier, Moo::EffectMaterialPtr material )
{
	BW_GUARD;
	if( materialOverrides_.find( identifier ) == materialOverrides_.end() )
	{// create a new one
		MaterialOverride mo;
		mo.identifier_ = identifier;
		bulk_->gatherMaterials( identifier, mo.effectiveMaterials_, NULL );
		materialOverrides_[ identifier ] = mo;
	}
	materialOverrides_[ identifier ].update( material );
//	bulk_->overrideMaterial( identifier, *material );
	return materialOverrides_[ identifier ];
}



int NodelessModel::gatherMaterials(	const std::string & materialIdentifier,
									std::vector< Moo::Visual::PrimitiveGroup * > & primGroups,
									Moo::ConstEffectMaterialPtr * ppOriginal )
{
	BW_GUARD;
	return bulk_->gatherMaterials( materialIdentifier, primGroups, ppOriginal );
}


/*static*/ Moo::VisualPtr NodelessModel::loadVisual( Model & m, const std::string & resourceID )
{
	BW_GUARD;
	std::string visualName = resourceID + ".static.visual";
	Moo::VisualPtr vis = Moo::VisualManager::instance()->get( visualName );
	if (!vis)
	{
		visualName = resourceID + ".visual";
		vis = Moo::VisualManager::instance()->get( visualName );
	}
	if (vis)
	{
		// complain if this visual has nodes
		if (vis->rootNode()->nChildren() != 0)
		{
			WARNING_MSG( "NodelessModel::loadVisual: "
				"Visual %s has multiple nodes! (attachments broken)\n",
				resourceID.c_str() );
		}
		else
		{
			// but all have a root node
			Moo::Node * pCatNode =
				Moo::NodeCatalogue::findOrAdd( vis->rootNode() );
			if (!s_sceneRootNode_) s_sceneRootNode_ = pCatNode;
			IF_NOT_MF_ASSERT_DEV( s_sceneRootNode_ == pCatNode )
			{
				s_sceneRootNode_ = pCatNode;
			}
		}			
	}
	return vis;
}




int NodelessModel::initMatter( Matter & m )
{
	BW_GUARD;
	return initMatter_NewVisual( m, *bulk_ );
}


bool NodelessModel::initTint( Tint & t, DataSectionPtr matSect )
{
	BW_GUARD;
	return initTint_NewVisual( t, matSect );
}





// nodeless_model.cpp
