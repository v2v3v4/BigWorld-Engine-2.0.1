/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BLUR_PARTICLE_RENDERER_HPP
#define BLUR_PARTICLE_RENDERER_HPP

#include "particle_system_renderer.hpp"
#include "particle_system_draw_item.hpp"

// -----------------------------------------------------------------------------
// Section: BlurParticleRenderer.
// -----------------------------------------------------------------------------
/**
 * TODO: to be documented.
 */
class BlurParticleRenderer : public ParticleSystemRenderer
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	BlurParticleRenderer();
	~BlurParticleRenderer();
	//@}

	/// @name Blur particle renderer methods.
	//@{
	void textureName( const std::string& v );
	const std::string& textureName() const	{ return textureName_; }
	//@}

	///	@name Accessors for effects.
	//@{
	float width() { return width_; }
	void width(float value) { width_ = value; }

	float time() { return time_; }
	void time( float value ) { time_ = value; }

	bool useFog() const { return useFog_; }
	void useFog( bool b){ useFog_ = b; material_.fogged(b); }
	//@}


	///	@name Renderer Overrides.
	//@{
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );
	virtual void realDraw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end );
	//@}

	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;
	virtual ParticlesPtr createParticleContainer() const
	{
		return new ContiguousParticles;
	}
	virtual size_t sizeInBytes() const { return sizeof(BlurParticleRenderer); }

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	ParticleSystemDrawItem sortedDrawItem_;
	Moo::Material		material_;
	std::string			textureName_;
	bool				useFog_;	///< Use scene fogging or not.
	float				width_;
	float				time_;
};


typedef SmartPointer<BlurParticleRenderer> BlurParticleRendererPtr;


/*~ class Pixie.PyBlurParticleRenderer
 *
 *	BlurParticleRenderer is a ParticleSystemRenderer that draws a trail
 *	for each particle, but does so in a more efficient manner than the
 *	TrailParticleRenderer.
 *
 *	The BlurParticleRenderer draws a single straight trail for each
 *	particle, by drawing a textured line using the particle's current
 *	velocity negated.
 *
 *	Due to this, the trails drawn by the BlurParticleRenderer will not
 *	exhibit bends - however, there is no memory overhead for using this
 *	renderer.
 *
 *	A new PyBlurParticleRenderer is created using Pixie.BlurParticleRenderer
 *	function.
 */
class PyBlurParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PyBlurParticleRenderer, PyParticleSystemRenderer )

public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyBlurParticleRenderer( BlurParticleRendererPtr pR, PyTypePlus *pType = &s_type_ );	
	//@}

	/// @name Blur particle renderer methods.
	//@{
	void textureName( const std::string& v )	{ pR_->textureName(v); }
	const std::string& textureName() const	{ return pR_->textureName(); }
	//@}

	///	@name Accessors for effects.
	//@{
	float width() { return pR_->width(); }
	void width(float value) { pR_->width(value); }

	float time() { return pR_->time(); }
	void time( float value ) { pR_->time(value); }

	bool useFog() const { return pR_->useFog(); }
	void useFog( bool b ){ pR_->useFog(b); }
	//@}

	///	@name Python Interface to the BlurParticleRenderer.
	//@{
	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, textureName, textureName )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, width, width )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, time, time )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, useFog, useFog )

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );
	//@}
private:
	BlurParticleRendererPtr pR_;
};

#endif // BLUR_PARTICLE_RENDERER_HPP
