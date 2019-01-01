/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_MESH_PARTICLE_RENDERER_HPP
#define BASE_MESH_PARTICLE_RENDERER_HPP


#include "particle_system_renderer.hpp"
#include "moo/visual.hpp"


/**
 *  This is the base class for mesh particle renderers.
 */
class BaseMeshParticleRenderer : public ParticleSystemRenderer
{
public:
	/// @name Constructor.
	//@{
    BaseMeshParticleRenderer();    
	//@}

	///	@name Renderer Interface Methods.
	//@{
    virtual bool knowParticleRadius() const;
    virtual float particleRadius() const;
    //@}

	/// @name visual particle renderer methods.
	//@{
	virtual void visual( const std::string& v ) = 0;
	virtual const std::string& visual() const = 0;
    //@/

protected:
    Moo::VisualPtr		pVisual_;
};


#endif // BASE_MESH_PARTICLE_RENDERER_HPP
