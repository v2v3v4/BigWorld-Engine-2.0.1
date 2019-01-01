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
#include "base_mesh_particle_renderer.hpp"
#include "resmgr/bwresource.hpp"


DECLARE_DEBUG_COMPONENT2( "Particle", 0 )


BaseMeshParticleRenderer::BaseMeshParticleRenderer():
	pVisual_(NULL)
{
}


/**
 *  We do know about particle's radius if there is a visual.
 */
/*virtual*/ bool BaseMeshParticleRenderer::knowParticleRadius() const
{
    BW_GUARD;
	return pVisual_.hasObject();
}


/**
 *  Query for the largest particle size.  We base this on the particle's 
 *  bounding box.
 */
/*virtual*/ float BaseMeshParticleRenderer::particleRadius() const
{
    BW_GUARD;
	if (pVisual_.hasObject())
    {
        const BoundingBox & bb = pVisual_->boundingBox();
        float hw = 0.5f*bb.width ();
        float hh = 0.5f*bb.height();
        float hd = 0.5f*bb.depth ();
        return std::sqrt(hw*hw + hh*hh + hd*hd);
    }
    else
    {
        return 0.0f;
    }
}
