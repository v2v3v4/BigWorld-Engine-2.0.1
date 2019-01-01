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

#ifndef SWITCHED_MODEL_ANIMATION_HPP
#define SWITCHED_MODEL_ANIMATION_HPP


#include "model_animation.hpp"


template< class BULK >
class SwitchedModel;


/**
 *	SwitchedModel's animation
 */
template < class BULK >
class SwitchedModelAnimation : public ModelAnimation
{
public:
	SwitchedModelAnimation( SwitchedModel<BULK> & owner, const std::string & name, float frameRate ) :
		owner_( owner ), identifier_( name ), frameRate_( frameRate )
	{
		BW_GUARD;	
	}

	virtual ~SwitchedModelAnimation()
	{
		BW_GUARD;	
	}

	virtual void play( float time, float blendRatio, int /*flags*/ )
	{
		BW_GUARD;
		// Animations should not be added to the list if
		//  they have no frames_!
		if (owner_.blend( Model::blendCookie() ) < blendRatio)
		{
			owner_.setFrame( frames_[ uint32(time*frameRate_) % frames_.size() ] );
			owner_.blend( Model::blendCookie(), blendRatio );
		}
	}

	void addFrame( BULK frame )
	{
		BW_GUARD;
		frames_.push_back( frame );
		duration_ = float(frames_.size()) / frameRate_;
		looped_ = true;
	}

	int numFrames() { return frames_.size(); }

	std::vector<BULK> & frames() { return frames_; }

private:
	SwitchedModel<BULK> & owner_;

	std::string			identifier_;
	float				frameRate_;
	std::vector<BULK>	frames_;
};


#endif // SWITCHED_MODEL_ANIMATION_HPP
