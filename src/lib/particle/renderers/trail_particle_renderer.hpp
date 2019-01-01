/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TRAIL_PARTICLE_RENDERER_HPP
#define TRAIL_PARTICLE_RENDERER_HPP

#include "particle_system_renderer.hpp"
#include "particle_system_draw_item.hpp"

// -----------------------------------------------------------------------------
// Section: TrailParticleRenderer.
// -----------------------------------------------------------------------------
/**
 * TODO: to be documented.
 */
class TrailParticleRenderer : public ParticleSystemRenderer
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	TrailParticleRenderer();
	~TrailParticleRenderer();
	//@}

	/// @name Trail particle renderer methods.
	//@{
	void textureName( const std::string& v );
	const std::string& textureName() const	{ return textureName_; }
	void capacity( int i );
	//@}

	///	@name Accessors for effects.
	//@{
	float width() { return width_; }
	void width(float value) { width_ = value; }

	int steps() { return steps_; }
	void steps( int value );

	int skip() { return skip_; }
	void skip( int value )	{ skip_ = value; }

	bool useFog() const { return useFog_; }
	void useFog( bool b){ useFog_ = b; material_.fogged(b); }
	//@}	


	///	@name Renderer Overrides.
	//@{
	void TrailParticleRenderer::update( Particles::iterator beg,
		Particles::iterator end, float dTime );
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );
	virtual void realDraw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end );
	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );
	//@}

	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;
	virtual ParticlesPtr createParticleContainer() const
	{
		return new FixedIndexParticles;
	}
	virtual size_t sizeInBytes() const;

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	ParticleSystemDrawItem sortedDrawItem_;
	Moo::Material		material_;
	std::string			textureName_;
	bool				useFog_;	///< Use scene fogging or not.
	float				width_;
	int					steps_;

	class Cache
	{
	public:
		Cache();
		~Cache();
		void			capacity( int c );
		void			copy( Particles::iterator beg, Particles::iterator end );
		size_t			sizeInBytes() const;

		int				size_;
		std::vector<Vector3> position_;
		std::vector<float>	 age_;
	};

	Cache				*cache_;
	uint8				cacheIdx_;
	int					capacity_;	//from the particle system
	int					skip_;		//for each cache::copy, skip n updates
	int					frame_;
};


typedef SmartPointer<TrailParticleRenderer> TrailParticleRendererPtr;


/*~ class Pixie.PyTrailParticleRenderer
 *
 *	TrailParticleRenderer is a ParticleSystemRenderer that lofts out a trail
 *	based on the cached history of particles.  The trail is a textured 3D
 *	line that looks good from most angles, except when going straight into/out
 *	of the screen, in which case you will see a thin cross.
 *
 *	Because the history of a particles position is cached, the trail of each
 *	particle may bend ( as opposed to the BlurParticleRenderer, which will
 *	always have straight-line trails )
 *
 *	The TrailParticleRenderer should be used judiciously, because of the memory
 *	overhead of the particle history cache.  However, it can create great effects
 *	using just a small amount of particles.
 *
 *	This renderer will use an extra n * s memory, where n is the capacity of the
 *	particle system, and s is the number of steps in each trail.
 *
 *	A new PyTrailParticleRenderer is created using Pixie.TrailParticleRenderer
 *	function.
 */
class PyTrailParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PyTrailParticleRenderer, PyParticleSystemRenderer )

public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyTrailParticleRenderer( TrailParticleRendererPtr pRenderer, PyTypePlus *pType = &s_type_ );	
	//@}

	///	@name Accessors for effects.
	//@{
	float width() { return pR_->width(); }
	void width(float value) { pR_->width(value); }

	int steps() { return pR_->steps(); }
	void steps( int value )	{ pR_->steps(value); }

	const std::string& textureName() const	{ return pR_->textureName(); }
	void textureName( const std::string& v )	{ pR_->textureName(v); }

	int skip() { return pR_->skip(); }
	void skip( int value )	{ pR_->skip(value); }

	bool useFog() const { return pR_->useFog(); }
	void useFog( bool b ){ pR_->useFog(b); }
	//@}

	///	@name Python Interface to the TrailParticleRenderer.
	//@{
	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, textureName, textureName )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, width, width )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, steps, steps )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, skip, skip )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, useFog, useFog )

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );
	//@}
private:
	TrailParticleRendererPtr pR_;
};

#endif // TRAIL_PARTICLE_RENDERER_HPP
