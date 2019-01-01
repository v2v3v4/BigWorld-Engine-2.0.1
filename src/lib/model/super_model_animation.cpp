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

#include "super_model_animation.hpp"

#include "model.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 *	SuperModelAnimation constructor
 */
SuperModelAnimation::SuperModelAnimation( SuperModel & superModel,
		const std::string & name ) :
	time( 0.f ),
	lastTime( 0.f ),
	blendRatio( 0.f )
{
	BW_GUARD;
	for (int i = 0; i < superModel.nModels(); i++)
	{
		index[i] = superModel.topModel(i)->getAnimation( name );
	}
}


/**
 *	This method ticks the animations represented by this class in
 *	the input supermodel. It must be the same as it was created with.
 *	Only the top model's animations are ticked.
 *	This is a helper method, since ticks are discouraged in SuperModel code.
 */
void SuperModelAnimation::tick( SuperModel & superModel, float dtime )
{
	BW_GUARD;
	for (int i = 0; i < superModel.nModels(); i++)
	{
		Model * cM = &*superModel.topModel(i);
		if (cM != NULL) cM->tickAnimation( index[i], dtime, lastTime, time );
	}

	lastTime = time;
}


/**
 *	This method applies the animation represented by this class to
 *	the input supermodel. It must be the same as it was created with.
 */
void SuperModelAnimation::dress( SuperModel & superModel )
{
	BW_GUARD;
	for (int i = 0; i < superModel.nModels(); i++)
	{
		Model * cM = superModel.curModel(i);
		if (cM != NULL && index[i] != -1)
			cM->playAnimation( index[i], time, blendRatio, 0 );
	}
}


/**
 *	This method returns the given animation, for the top lod of the first
 *	model it exists in.
 *
 *	@return the animation, or NULL if none exists
 */
/*const*/ ModelAnimation * SuperModelAnimation::pSource( SuperModel & superModel ) const
{
	BW_GUARD;
	for (int i = 0; i < superModel.nModels(); i++)
	{
		if (index[i] != -1)
		{
			return superModel.topModel(i)->lookupLocalAnimation( index[i] );
		}
	}

	return NULL;	// no models have this animation
}


// super_model_animation.cpp
