/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POINT_SPRITE_PARTICLE_RENDERER_HPP
#define POINT_SPRITE_PARTICLE_RENDERER_HPP

#include "sprite_particle_renderer.hpp"


/**
 * TODO: to be documented.
 */
class PointSpriteParticleRenderer : public SpriteParticleRenderer
{
public:

	/// @name Constructor(s) and Destructor.
	//@{
	PointSpriteParticleRenderer( const std::string newTextureName );
	~PointSpriteParticleRenderer();
	//@}

	///	@name Renderer Overrides.
	//@{
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );

	void realDraw( const Matrix& worldTransform,
		Particles::iterator beg,
		Particles::iterator end );

	virtual size_t sizeInBytes() const { return sizeof(PointSpriteParticleRenderer); }
	//@}


	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;
	virtual ParticlesPtr createParticleContainer() const
	{
		return new ContiguousParticles;
	}

protected:
	///	@name Auxiliary Methods.
	//@{
	void updateMaterial();
	void updateMaterial(Moo::BaseTexturePtr texture);
	friend BGUpdateData<PointSpriteParticleRenderer>;
	//@}
};


typedef SmartPointer<PointSpriteParticleRenderer> PointSpriteParticleRendererPtr;


/*~ class Pixie.PyPointSpriteParticleRenderer
 *
 *	PointSpriteParticleRenderer is a ParticleSystemRenderer which renders each
 *	particle as a point sprite.
 *
 *	Texture animation (rolling uv) is not supported, nor are particle sizes
 *	on the screen > 64 pixels.
 *
 *	This particle renderer is the quickest to draw, and uses the least amount
 *	of graphics memory ( push buffer ).
 *
 *	A new PyPointSpriteParticleRenderer is created using Pixie.PointSpriteParticleRenderer
 *	function.
 */
class PyPointSpriteParticleRenderer : public PySpriteParticleRenderer
{
	Py_Header( PyPointSpriteParticleRenderer, PySpriteParticleRenderer )

public:

	/// @name Constructor(s) and Destructor.
	//@{
	PyPointSpriteParticleRenderer( PointSpriteParticleRendererPtr pR, PyTypePlus *pType = &s_type_ );
	//@}

	/// @name The Python Interface to PointSpriteParticleRenderer.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()
	//@}
};


#endif // POINT_SPRITE_PARTICLE_RENDERER_HPP
