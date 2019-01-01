/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_PARTICLE_SYSTEM_HPP
#define PY_PARTICLE_SYSTEM_HPP

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "pyscript/stl_to_py.hpp"
#include "duplo/py_attachment.hpp"
#include "particle_system.hpp"

class ParticleSystemRenderer;
typedef SmartPointer<ParticleSystemRenderer> ParticleSystemRendererPtr;

/*~ class Pixie.PyParticleSystem
 *
 *	The PyParticleSystem class provides a base particle system to manage
 *	particles. A particle system must have particle system actions attached to
 *	it to specify how the particle system evolves over time.
 *	
 *	A particle system must have at least a souce particle generator (SourcePSA)
 *	and a renderer specified for it to be able to generate particles. The two
 *	main types of particle renderers are SpriteParticleRenderer and
 *	MeshParticleRenderer.
 *
 *	A new PyParticleSystem is created using Pixie.ParticleSystem function.
 */
/**
 *	This class is the abstract picture of the particles that is invariant
 *	to a renderer.
 */
class PyParticleSystem : public PyAttachment
{
	Py_Header( PyParticleSystem, PyAttachment )

public:	
	/// @name Constructor(s) and Destructor.
	//@{
	PyParticleSystem( ParticleSystemPtr pSystem, PyTypePlus *pType = &s_type_ );
	~PyParticleSystem();
	//@}

	/// A collection of PyActions for a Particle System.
	typedef std::vector<ParticleSystemActionPtr> PyActions;

	/// @name PyAttachment overrides
	//@{
	virtual void tick( float dTime );
	virtual void draw( const Matrix & worldTransform, float lod );
	virtual void localBoundingBox( BoundingBox & bb, bool skinny = false );
	virtual void localVisibilityBox( BoundingBox & vbb, bool skinny = false );
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false );
	virtual void worldVisibilityBox( BoundingBox & vbb, const Matrix& world, bool skinny = false );
	virtual bool attach( MatrixLiaison * pOwnWorld );
	virtual void detach();
	virtual void enterWorld();
	virtual void leaveWorld();
	//@}


	/// @name Particle System attribute accessors.
	//@{
	ParticleSystemPtr pSystem()		{ return pSystem_; }

	int capacity( void ) const		{ return pSystem_->capacity(); }
	void capacity( int number )		{ pSystem_->capacity( number );}

	float windFactor( void ) const	{ return pSystem_->windFactor(); }
	void windFactor( float ratio )	{ pSystem_->windFactor( ratio ); }

	void explicitPosition( const Vector3& v )	{ pSystem_->explicitPosition(v); }
	const Vector3& explicitPosition() const		{ return pSystem_->explicitPosition(); }

	void explicitDirection( const Vector3& v )	{ pSystem_->explicitDirection(v); }
	const Vector3& explicitDirection() const	{ return pSystem_->explicitDirection(); }

	float fixedFrameRate() const	{ return pSystem_->fixedFrameRate(); }
	void fixedFrameRate( float f )	{ pSystem_->fixedFrameRate(f); }

	void maxLod(float m)			{ pSystem_->maxLod( m ); }
	float maxLod() const			{ return pSystem_->maxLod(); }

	float duration() const			{ return pSystem_->duration(); }

	void pRenderer( PyParticleSystemRendererPtr pNewRenderer )
	{
		pSystem_->pRenderer( pNewRenderer->pRenderer() );
	}

	PyParticleSystemRendererPtr pRenderer()
	{		
		return PyParticleSystemRenderer::createPyRenderer(pSystem_->pRenderer());
	}


	/// @name The Python Interface to the Particle System.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_METHOD_DECLARE( py_addAction )
	PY_METHOD_DECLARE( py_removeAction )
	PY_METHOD_DECLARE( py_action )
	PY_METHOD_DECLARE( py_clear )
	PY_METHOD_DECLARE( py_update )
	PY_METHOD_DECLARE( py_render )
	PY_METHOD_DECLARE( py_load )
	PY_METHOD_DECLARE( py_save )
	PY_METHOD_DECLARE( py_size )
	PY_METHOD_DECLARE( py_force )
	PY_AUTO_METHOD_DECLARE( RETDATA, duration, END )

	PY_RW_ATTRIBUTE_DECLARE( actionsHolder_, actions )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, fixedFrameRate, fixedFrameRate )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, capacity, capacity )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, windFactor, windFactor )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyParticleSystemRendererPtr, pRenderer, renderer )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, explicitPosition, explicitPosition )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, explicitDirection, explicitDirection )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, maxLod, maxLod )	
	//@}

private:
	ParticleSystemPtr		pSystem_;

	/// Python Interface to the Actions vector.
	PySTLSequenceHolder<ParticleSystem::Actions> actionsHolder_;
};


void ParticleSystemAction_release( ParticleSystemAction *&pAction );
/*
 *	Wrapper to let the C++ vector of actions work as a vector/list in Python.
 */
namespace PySTLObjectAid
{
	/// Releaser is same for all PyObject's
	template <> struct Releaser< ParticleSystemAction * >
	{
		static void release( ParticleSystemAction *&pAction )
        {
        	ParticleSystemAction_release( pAction );
        }
	};
}

PY_SCRIPT_CONVERTERS_DECLARE( PyParticleSystem )

#ifdef CODE_INLINE
#include "py_particle_system.ipp"
#endif

#endif


/* py_particle_system.hpp */
