/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_PARTICLE_RENDERER_HPP
#define VISUAL_PARTICLE_RENDERER_HPP

#include "base_mesh_particle_renderer.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/device_callback.hpp"

/**
 *	This class displays the particle system with each particle drawing a visual.
 */
// -----------------------------------------------------------------------------
// Section: VisualParticleRenderer.
// -----------------------------------------------------------------------------
class VisualParticleRenderer : public BaseMeshParticleRenderer
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	VisualParticleRenderer();
	~VisualParticleRenderer();
	//@}

	/// @name visual particle renderer methods.
	//@{
	virtual void visual( const std::string& v );
	virtual const std::string& visual() const	{ return visualName_; }
	//@}	


	///	@name Renderer Overrides.
	//@{
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );

	virtual void realDraw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end )	{};

	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );
	//@}

	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;

	virtual bool isMeshStyle() const	{ return true; }
	virtual ParticlesPtr createParticleContainer() const
	{
		// TODO: Improve performance with Contiguous but need to 
		// still be able to calculate particle index (Flex. Part. Formats)
		return new FixedIndexParticles;
	}

	virtual size_t sizeInBytes() const { return sizeof(VisualParticleRenderer); }

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	std::string			visualName_;   
};


typedef SmartPointer<VisualParticleRenderer> VisualParticleRendererPtr;


/*~ class Pixie.PyVisualParticleRenderer
 *	VisualParticleRenderer is a ParticleSystemRenderer which renders each particle
 *	as a visual object.  Any visual can be used, although the TintShader will only
 *	work on visuals that use effect files that use MeshParticleTint.
 */
class PyVisualParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PyVisualParticleRenderer, PyParticleSystemRenderer )

public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyVisualParticleRenderer( VisualParticleRendererPtr pR, PyTypePlus *pType = &s_type_ );
	//@}

	/// @name visual particle renderer methods.
	//@{
	virtual void visual( const std::string& v )	{ pR_->visual(v); }
	virtual const std::string& visual() const	{ return pR_->visual(); }
	//@}

	///	@name Python Interface to the VisualParticleRenderer.
	//@{
	PY_FACTORY_DECLARE()
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, visual, visual )
	//@}

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );
	//@}

private:
	VisualParticleRendererPtr	pR_;
};

#endif // VISUAL_PARTICLE_RENDERER_HPP
