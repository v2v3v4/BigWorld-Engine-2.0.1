/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPRITE_PARTICLE_RENDERER_HPP
#define SPRITE_PARTICLE_RENDERER_HPP

#include "particle_system_draw_item.hpp"
#include "particle_system_renderer.hpp"

/**
 *	This class displays the particle system with each particle being a single
 *	spin-to-face polygon.
 */
class SpriteParticleRenderer : public ParticleSystemRenderer
{
	friend class SpriteParticleJob;

public:
	/// An enumeration of the possible material effects.
	enum MaterialFX
	{
		FX_ADDITIVE,
		FX_ADDITIVE_ALPHA,
		FX_BLENDED,
		FX_BLENDED_COLOUR,
		FX_BLENDED_INVERSE_COLOUR,
		FX_SOLID,
		FX_SHIMMER,
        FX_SOURCE_ALPHA,
		FX_MAX	// keep this element at the end
	};

	/// @name Constructor(s) and Destructor.
	//@{
	SpriteParticleRenderer( const std::string newTextureName );
	~SpriteParticleRenderer();
	//@}


	///	@name Accessors for Sprite Effects.
	//@{
	MaterialFX materialFX() const;
	void materialFX( MaterialFX newMaterialFX );

	const std::string &textureName() const;
	void textureName( const std::string &newString );

	int frameCount() const { return frameCount_; }
	void frameCount( int c ) { frameCount_ = c; }

	float frameRate() const { return frameRate_; }
	void frameRate( float r ) { frameRate_ = r; }

	bool useFog() const { return useFog_; }
	void useFog( bool b){ useFog_ = b; }
	//@}


	///	@name Renderer Overrides.
	//@{
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );

	virtual void realDraw( const Matrix& worldTransform,
		Particles::iterator beg,
		Particles::iterator end );

	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );

	virtual size_t sizeInBytes() const { return sizeof(SpriteParticleRenderer); }
	//@}


	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;

	virtual const Vector3 & explicitOrientation() const { return explicitOrientation_; }
	virtual void explicitOrientation( const Vector3 & newVal ) { explicitOrientation_ = newVal; }

	virtual ParticlesPtr createParticleContainer() const
	{
		return new ContiguousParticles;
	}

	void rotated( bool option ) { rotated_ = option; }
    bool rotated() const { return rotated_; }

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

	///	@name Render Methods.
	//@{
	virtual void updateMaterial();
	void updateMaterial(Moo::BaseTexturePtr texture);
	friend BGUpdateData<SpriteParticleRenderer>;
	//@}


	///	@name Sprite Material/Texture Properties.
	//@{
	MaterialFX materialFX_;			///< Stored material FX setting.
	std::string textureName_;		///< The name of the texture.
	bool	useFog_;				///< Use scene fogging or not.

	Moo::Material material_;		///< The Material.
	bool materialSettingsChanged_;	///< Dirty flag for material settings.

	int		frameCount_;			///< Frames of animation in texture.
	float	frameRate_;				///< Frames per second of particle age.

	Vector3 explicitOrientation_;	///< Explicit orientation of the sprite particles.

	bool	rotated_;
	//@}


	///	@name Mesh Properties of the Sprite
	//@{
	Moo::VertexTDSUV2 sprite_[4];	///< The sprite plane.
	//@}

	ParticleSystemDrawItem sortedDrawItem_;
};


typedef SmartPointer<SpriteParticleRenderer> SpriteParticleRendererPtr;


/*~ class Pixie.PySpriteParticleRenderer
 *
 *	SpriteParticleRenderer is a ParticleSystemRenderer which renders each
 *	particle as a single spin to face polygon.
 *
 *	A new PySpriteParticleRenderer is created using Pixie.SpriteRenderer
 *	function.
 */
class PySpriteParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PySpriteParticleRenderer, PyParticleSystemRenderer )

public:

	/// An enumeration of the possible material effects.
	enum MaterialFX
	{
		FX_ADDITIVE,
		FX_ADDITIVE_ALPHA,
		FX_BLENDED,
		FX_BLENDED_COLOUR,
		FX_BLENDED_INVERSE_COLOUR,
		FX_SOLID,
		FX_SHIMMER,
        FX_SOURCE_ALPHA,
		FX_MAX	// keep this element at the end
	};

	/// @name Constructor(s) and Destructor.
	//@{
	PySpriteParticleRenderer( SpriteParticleRendererPtr pR, PyTypePlus *pType = &s_type_ );	
	//@}


	///	@name Accessors for Sprite Effects.
	//@{
	MaterialFX materialFX() const				{ return (MaterialFX)pR_->materialFX(); }
	void materialFX( MaterialFX newMaterialFX )	{ pR_->materialFX((SpriteParticleRenderer::MaterialFX)newMaterialFX); }

	const std::string &textureName() const		{ return pR_->textureName(); }
	void textureName( const std::string &newString )	{ pR_->textureName(newString); }

	int frameCount() const { return pR_->frameCount(); }
	void frameCount( int c ) { pR_->frameCount(c); }

	float frameRate() const { return pR_->frameRate(); }
	void frameRate( float r ) { pR_->frameRate(r); }

	bool useFog() const { return pR_->useFog(); }
	void useFog( bool b ){ pR_->useFog(b); }
	//@}


	/// @name The Python Interface to SpriteParticleRenderer.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_DEFERRED_ATTRIBUTE_DECLARE( materialFX )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, textureName, textureName )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, frameCount, frameCount )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frameRate, frameRate )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, useFog, useFog )

	// Python Wrappers for handling Emumerations in Python.
	PY_BEGIN_ENUM_MAP( MaterialFX, FX_ )
		PY_ENUM_VALUE( FX_ADDITIVE )
		PY_ENUM_VALUE( FX_ADDITIVE_ALPHA )
		PY_ENUM_VALUE( FX_BLENDED )
		PY_ENUM_VALUE( FX_BLENDED_COLOUR )
		PY_ENUM_VALUE( FX_BLENDED_INVERSE_COLOUR )
		PY_ENUM_VALUE( FX_SOLID )
		PY_ENUM_VALUE( FX_SHIMMER )
        PY_ENUM_VALUE( FX_SOURCE_ALPHA )
	PY_END_ENUM_MAP()
	//@}

protected:
	SpriteParticleRendererPtr	pR_;
};

PY_ENUM_CONVERTERS_DECLARE( PySpriteParticleRenderer::MaterialFX )


#ifdef CODE_INLINE
#include "sprite_particle_renderer.ipp"
#endif


#endif // SPRITE_PARTICLE_RENDERER_HPP
