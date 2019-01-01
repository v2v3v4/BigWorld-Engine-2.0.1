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

#ifndef NODEFULL_MODEL_HPP
#define NODEFULL_MODEL_HPP

#include "model.hpp"

#include "model_animation.hpp"


class NodefullModelAnimation;


/**
 *	Class for models whose bulk is a visual with nodes
 */
class NodefullModel : public Model, public Aligned
{
	//friend class ModelAnimation;
	friend class DefaultAnimation;
	friend class NodefullModelAnimation;
public:
	NodefullModel( const std::string & resourceID, DataSectionPtr pFile );
	~NodefullModel();

#ifdef EDITOR_ENABLED
	virtual void reload();
#endif//EDITOR_ENABLED

	virtual bool valid() const;

	virtual void dress();
	virtual void draw( bool checkBB );

	virtual const BSPTree * decompose() const;

	virtual const BoundingBox & boundingBox() const;
	virtual const BoundingBox & visibilityBox() const;
	virtual bool hasNode( Moo::Node * pNode,
		MooNodeChain * pParentChain ) const;

	virtual NodeTreeIterator nodeTreeBegin() const;
	virtual NodeTreeIterator nodeTreeEnd() const;

	void	batched( bool state ) { batched_ = state; }
	bool	batched( ) const { return batched_; }

	virtual MaterialOverride overrideMaterial( const std::string& identifier, Moo::EffectMaterialPtr material );
	virtual int gatherMaterials( const std::string & materialIdentifier, std::vector< Moo::Visual::PrimitiveGroup * > & primGroups, Moo::ConstEffectMaterialPtr * ppOriginal = NULL )
	{
		return bulk_->gatherMaterials( materialIdentifier, primGroups, ppOriginal );
	}

	Moo::VisualPtr getVisual() { return bulk_; }

private:
	Matrix						world_;	// set when model is dressed (traversed)
	BoundingBox					visibilityBox_;
	MaterialOverrides			materialOverrides_;
	SmartPointer<Moo::Visual>	bulk_;

	NodeTree					nodeTree_;

	bool						batched_;

	Moo::StreamedDataCache	* pAnimCache_;

	virtual int initMatter( Matter & m );
	virtual bool initTint( Tint & t, DataSectionPtr matSect );
};



#endif // NODEFULL_MODEL_HPP
