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

#ifndef NODEFULL_MODEL_DEFAULT_ANIMATION_HPP
#define NODEFULL_MODEL_DEFAULT_ANIMATION_HPP


#include "model_animation.hpp"

#include "math/blend_transform.hpp"

#include "node_tree.hpp"

/**
 *	Nodefull model's default animation
 */
class NodefullModelDefaultAnimation : public ModelAnimation
{
public:
	NodefullModelDefaultAnimation( NodeTree & rTree,
		int itinerantNodeIndex,
		std::avector< Matrix > & transforms );
	virtual ~NodefullModelDefaultAnimation();

	virtual bool valid() const;

	virtual void play( float time, float blendRatio, int flags );

	virtual void flagFactor( int flags, Matrix & mOut ) const;

	const std::avector< Matrix > & cTransforms() { return cTransforms_; }

private:
	NodeTree		& rTree_;
	int				itinerantNodeIndex_;

	std::vector< BlendTransform >		bTransforms_;
	const std::avector< Matrix >		cTransforms_;
};


#endif // NODEFULL_MODEL_DEFAULT_ANIMATION_HPP
