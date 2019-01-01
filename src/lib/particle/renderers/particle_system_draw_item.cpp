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
#include "particle_system_draw_item.hpp"
#include "sprite_particle_renderer.hpp"


void ParticleSystemDrawItem::set(  
	ParticleSystemRenderer  *renderer,
    const Matrix&           worldTransform, 
    Particles::iterator     beg, 
    Particles::iterator     end,
	float distance )
{
	renderer_ = renderer;
	worldTransform_ = worldTransform;
	beg_ = beg;
	end_ = end;
	distance_ = distance;	
}


void ParticleSystemDrawItem::draw()
{
	BW_GUARD;
	renderer_->realDraw( worldTransform_, beg_, end_ );
}
