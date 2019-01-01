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

#ifndef SUPER_MODEL_ANIMATION_HPP
#define SUPER_MODEL_ANIMATION_HPP


#include "fashion.hpp"

#include "super_model.hpp"


class Animation;


/**
 *	This class represents an animation from a SuperModel perspective.
 *
 *	@note This is a variable-length class, so don't try to make these
 *		at home, kids.
 */
class SuperModelAnimation : public Fashion
{
public:
	float	time;
	float	lastTime;
	float	blendRatio;

	/*const*/ ModelAnimation * pSource( SuperModel & superModel ) const;

	void tick( SuperModel & superModel, float dtime );

private:
	int		index[1];

	SuperModelAnimation( SuperModel & superModel, const std::string & name );
	friend class SuperModel;

	virtual void dress( SuperModel & superModel );
};

typedef SmartPointer<SuperModelAnimation> SuperModelAnimationPtr;



#endif // SUPER_MODEL_ANIMATION_HPP
