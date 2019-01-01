/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PARTICLE_SYSTEM_RENDERER_HPP
#define PARTICLE_SYSTEM_RENDERER_HPP

// Standard MF Library Headers.
#include "moo/moo_math.hpp"
#include "moo/device_callback.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "moo/material.hpp"
#include "moo/vertex_declaration.hpp"

#include "particle/particle.hpp"
#include "particle/particles.hpp"

class BoundingBox;

#include "moo/vertex_formats.hpp"
#include <set>

/**
 *	This is the base class for displaying a particle system on screen.
 */
class ParticleSystemRenderer : public ReferenceCount
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	ParticleSystemRenderer();
	virtual ~ParticleSystemRenderer();
	//@}


	///	@name Renderer Interface Methods.
	//@{
	virtual void update( Particles::iterator beg,
		Particles::iterator end, float dTime )	{}
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb ) = 0;

	//Callback fn for ParticleSystemDrawItem sorted draw
	virtual void realDraw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end ) = 0;

	bool viewDependent() const;
	void viewDependent( bool flag );

	bool local() const;
	void local( bool flag );

	virtual bool isMeshStyle() const	{ return false; }
	virtual ParticlesPtr createParticleContainer() const = 0;

    virtual bool knowParticleRadius() const { return false; }
    virtual float particleRadius() const { return 0.0f; }

	virtual void capacity( int i )	{}

	virtual size_t sizeInBytes() const = 0;
	//@}	

	void serialise(DataSectionPtr pSect, bool load);	
	virtual const std::string & nameID() = 0;

	static ParticleSystemRenderer * createRendererOfType(const std::string type, DataSectionPtr ds = NULL);	

	// Return the resources required for a ParticleSystemRenderer
	static void prerequisitesOfType( DataSectionPtr pSection, std::set<std::string>& output );
	static void prerequisites( DataSectionPtr pSection, std::set<std::string>& output )	{};

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load) = 0;

private:
	bool viewDependent_;
	bool local_;
};


typedef SmartPointer<ParticleSystemRenderer> ParticleSystemRendererPtr;


/**
 *	Helper struct used to load material in the background.
 */
template <typename RenderT>
struct BGUpdateData
{
	BGUpdateData(RenderT * spr) :
		spr_(spr),
		texture_(NULL)
	{}

	static void loadTexture(void * data)
	{
		BGUpdateData * updateData = static_cast<BGUpdateData *>(data);
		updateData->texture_ = Moo::TextureManager::instance()->get(
			updateData->spr_->textureName(), true, true, true, "texture/particle");
	}

	static void updateMaterial(void * data)
	{
		BGUpdateData * updateData = static_cast<BGUpdateData *>(data);
		updateData->spr_->updateMaterial(updateData->texture_);
		delete updateData;
	}

	RenderT *                           spr_;
	Moo::BaseTexturePtr                 texture_;
	// std::auto_ptr<class BackgroundTask> task_;
};


class PyParticleSystemRenderer;
typedef SmartPointer<PyParticleSystemRenderer> PyParticleSystemRendererPtr;


/*~ class Pixie.PyParticleSystemRenderer
 *	ParticleSystemRenderer is an abstract base class for PyParticleSystem
 *	renderers.
 */
class PyParticleSystemRenderer : public PyObjectPlus
{
	Py_Header( PyParticleSystemRenderer, PyObjectPlus )

public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyParticleSystemRenderer( ParticleSystemRendererPtr pRenderer, PyTypePlus *pType = &s_type_ );
	virtual ~PyParticleSystemRenderer()	{};
	//@}

	ParticleSystemRendererPtr pRenderer()	{ return pRenderer_; }

	bool viewDependent() const	{ return pRenderer_->viewDependent(); }
	void viewDependent( bool flag )	{ pRenderer_->viewDependent(flag); }

	bool local() const			{ return pRenderer_->local(); }
	void local( bool flag )		{ pRenderer_->local(flag); }

	///	@name Python Interface to the ParticleSystemRenderer.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, viewDependent, viewDependent )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, local, local )
	//@}	

	static PyParticleSystemRendererPtr createPyRenderer(ParticleSystemRendererPtr pRenderer);

private:
	ParticleSystemRendererPtr	pRenderer_;
};


PY_SCRIPT_CONVERTERS_DECLARE( PyParticleSystemRenderer )


#ifdef CODE_INLINE
#include "particle_system_renderer.ipp"
#endif

#endif // PARTICLE_SYSTEM_RENDERER_HPP
