/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef NODELESS_MODEL_HPP
#define NODELESS_MODEL_HPP


#include "switched_model.hpp"
#include "romp/static_light_values.hpp"

class ModelStaticLighting;
typedef SmartPointer<ModelStaticLighting> ModelStaticLightingPtr;

/**
 *	Class for models whose bulk is a visual without nodes
 */
class NodelessModel : public SwitchedModel< Moo::VisualPtr >
{
public:
	NodelessModel( const std::string & resourceID, DataSectionPtr pFile );

#ifdef EDITOR_ENABLED
	virtual void reload();
#endif//EDITOR_ENABLED

	virtual void draw( bool checkBB );
	virtual void dress();

	virtual const BSPTree * decompose() const;

	virtual const BoundingBox & boundingBox() const;
	virtual const BoundingBox & visibilityBox() const;

	virtual ModelStaticLightingPtr getStaticLighting(
		StaticLightValueCachePtr cache, const DataSectionPtr section );

	virtual MaterialOverride overrideMaterial(	
								const std::string& identifier,
								Moo::EffectMaterialPtr material );

	Moo::VisualPtr getVisual();

	virtual bool occluder() const;

private:
	virtual int gatherMaterials(	
							const std::string & materialIdentifier,
							std::vector< Moo::Visual::PrimitiveGroup * > & primGroups,
							Moo::ConstEffectMaterialPtr * ppOriginal = NULL );

	MaterialOverrides materialOverrides_;
	bool		batched_;

	static Moo::VisualPtr loadVisual(	Model & m,
										const std::string & resourceID );



	virtual int initMatter( Matter & m );
	virtual bool initTint( Tint & t, DataSectionPtr matSect );

	static Moo::NodePtr s_sceneRootNode_;

	bool occluder_;
};




inline Moo::VisualPtr NodelessModel::getVisual()
{
	return bulk_;
}


inline bool NodelessModel::occluder() const
{
	return occluder_;
}




#endif // NODELESS_MODEL_HPP
