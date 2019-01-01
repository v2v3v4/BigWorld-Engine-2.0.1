/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLASH_BANG_EFFECT_HPP
#define FLASH_BANG_EFFECT_HPP


#include "moo/material.hpp"


namespace Moo
{
	class RenderTarget;
};


/**
 *	This class implements a full-screen filter that
 *	simulates the effect of a flash-bang grenade
 *	going off, and interrupting the player's vision.
 */
class FlashBangEffect
{
public:
	FlashBangEffect();
	~FlashBangEffect();

	void draw();

	const Vector4& fadeValues() const {return fadeValues_;};
	void	fadeValues( const Vector4& fadeValues )  {fadeValues_ = fadeValues;};

private:

	Moo::Material	blendMaterial_;
	Moo::RenderTarget* pRT_;
	bool			haveLastFrame_;
	Vector4			fadeValues_;

	FlashBangEffect( const FlashBangEffect& );
	FlashBangEffect& operator=( const FlashBangEffect& );
};


#endif // FLASH_BANG_EFFECT_HPP
