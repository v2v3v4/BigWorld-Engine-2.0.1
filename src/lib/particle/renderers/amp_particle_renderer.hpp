/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef AMP_PARTICLE_RENDERER_HPP
#define AMP_PARTICLE_RENDERER_HPP

// -----------------------------------------------------------------------------
// Section: AmpParticleRenderer.
// -----------------------------------------------------------------------------
#include "particle_system_renderer.hpp"
#include "particle_system_draw_item.hpp"


/**
 * TODO: to be documented.
 */
class AmpParticleRenderer : public ParticleSystemRenderer
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	AmpParticleRenderer();
	~AmpParticleRenderer();
	//@}

	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );

	/// @name Amp particle renderer methods.
	//@{
	void textureName( const std::string& v );
	const std::string& textureName() const	{ return textureName_; }
	//@}

	///	@name Accessors for effects.
	//@{
	float width() { return width_; }
	void width(float value) { width_ = value; }

	float height() { return height_; }
	void height(float value) { height_ = value; }

	int steps() { return steps_; }
	void steps(int value) { steps_ = value; }

	float variation() { return variation_; }
	void variation(float value) { variation_ = value; }

	bool circular() { return circular_; }
	void circular(bool state) { circular_ = state; }

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
	virtual size_t sizeInBytes() const { return sizeof(AmpParticleRenderer); }

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	ParticleSystemDrawItem sortedDrawItem_;
	Moo::Material		material_;
	std::string			textureName_;
	bool				useFog_;	///< Use scene fogging or not.
	float				width_;
	float				height_;
	int					steps_;
	float				variation_;
	bool				circular_;
};


typedef SmartPointer<AmpParticleRenderer>	AmpParticleRendererPtr;


/*~ class Pixie.PyAmpParticleRenderer
 *	AmpPartileRenderer is a ParticleRenderer that renders segmented lines
 *	from the particle position back to the origin of the particle system. It
 *	can be used for effects such as electricty.
 *
 *	Eg. (bad ascii art warning) 3 line segments from start point (s) to end
 *	point (e)
 *	@{
 *    /\
 *	 /  \
 *	s    \  e
 *        \/
 *	@}
 *	
 *	The origin referred to below is either the source position of the
 *	particle system (in the case of a non-circular AmpParticleRenderer)
 *	or the position of the previous particle (ordered by the age of the
 *	particle - in the case of a circular AmpParticleRenderer).
 *
 *	A new PyAmpParticleRenderer is created using Pixie.AmpParticleRenderer
 *	function.
 */
class PyAmpParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PyAmpParticleRenderer, PyParticleSystemRenderer )
public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyAmpParticleRenderer( AmpParticleRendererPtr pR,  PyTypePlus *pType = &s_type_ );
	//@}

	///	@name Accessors for effects.
	//@{
	void textureName( const std::string& v )	{ pR_->textureName(v); }
	const std::string& textureName() const		{ return pR_->textureName(); }

	float width() { return pR_->width(); }
	void width(float value) { pR_->width(value); }

	float height() { return pR_->height(); }
	void height(float value) { pR_->height(value); }

	int steps() { return pR_->steps(); }
	void steps(int value) { pR_->steps(value); }

	float variation() { return pR_->variation(); }
	void variation(float value) { pR_->variation(value); }

	bool circular() { return pR_->circular(); }
	void circular(bool state) { pR_->circular(state); }

	bool useFog() const { return pR_->useFog(); }
	void useFog( bool b ){ pR_->useFog(b); }
	//@}

	///	@name Python Interface to the AmpParticleRenderer.
	//@{
	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, textureName, textureName )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, width, width )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, height, height )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, steps, steps )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, variation, variation )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, circular, circular )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, useFog, useFog )

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );
	//@}
private:
	AmpParticleRendererPtr pR_;
};

#endif // AMP_PARTICLE_RENDERER_HPP
