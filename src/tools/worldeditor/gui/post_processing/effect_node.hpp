/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_NODE_HPP
#define EFFECT_NODE_HPP


#include "base_post_processing_node.hpp"


// Forward declarations
namespace PostProcessing
{
	class Effect;
	typedef SmartPointer<Effect> EffectPtr;
}

class EffectNode;
typedef SmartPointer< EffectNode > EffectNodePtr;


/**
 *	This class implements an Effect node, implementing the necessary additional
 *	editor info for a Post-processing Effect.
 */
class EffectNode : public BasePostProcessingNode
{
public:
	EffectNode( PostProcessing::Effect * pyEffect, NodeCallback * callback );
	~EffectNode();

	bool active() const;
	void active( bool active );

	const std::string & name() const { return name_; }

	virtual EffectNode * effectNode() { return this; }

	PostProcessing::EffectPtr pyEffect() const { return pyEffect_; }

	PyObject * pyObject() const { return (PyObject *)(pyEffect_.get()); }

	EffectNodePtr clone() const;

	void edEdit( GeneralEditor * editor );
	bool edLoad( DataSectionPtr pDS );
	bool edSave( DataSectionPtr pDS );

	std::string getName() const;
	bool setName( const std::string & name );

	Vector4 getBypass() const;
	bool setBypass( const Vector4 & bypass );

private:
	PostProcessing::EffectPtr pyEffect_;
	std::string name_;
};


#endif // EFFECT_NODE_HPP
