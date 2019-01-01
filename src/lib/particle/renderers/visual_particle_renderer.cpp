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
#include "visual_particle_renderer.hpp"
#include "moo/visual_channels.hpp"
#include "moo/render_context.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/visual_manager.hpp"
#include "moo/moo_math.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )


const std::string VisualParticleRenderer::nameID_ = "VisualParticleRenderer";


// -----------------------------------------------------------------------------
//	Section - VisualParticleRenderer
// -----------------------------------------------------------------------------

VisualParticleRenderer::VisualParticleRenderer():	
	visualName_( "" )
{
}


VisualParticleRenderer::~VisualParticleRenderer()
{
}


/**
 *	This method sets the visual name for the renderer.
 */
void VisualParticleRenderer::visual( const std::string& v )
{
	BW_GUARD;
	visualName_ = v;
	if (v.empty())
    {
		pVisual_ = NULL;
    }
	else
    {        
		pVisual_ = Moo::VisualManager::instance()->get(visualName_);
    }
}


/**
 * TODO: to be documented.
 */
class ParticleMeshTintConstant : public Moo::EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		pEffect->SetVector( constantHandle, (D3DXVECTOR4*)&tint_ );		

		return true;
	}

    void tint( const Vector4 & colour ) { tint_ = colour; }

private:
	Vector4 tint_;
};


static SmartPointer<ParticleMeshTintConstant> s_particleMeshTintConstant = 
    new ParticleMeshTintConstant;


/**
 *	This method views the given particle system using this renderer.
 */
void VisualParticleRenderer::draw( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	if ( beg == end ) return;
	if ( !pVisual_ ) return;

	DX::Device* device = Moo::rc().device();	

	Moo::rc().push();

	int nParticles = end - beg;
	Matrix particleTransform;

	Particles::iterator it = beg;

	while ( it!=end )
	{		
		Particle &particle = *it++;

		if (!particle.isAlive())
			continue;

        // Set particle transform as world matrix
		particleTransform.setRotate( particle.yaw(), particle.pitch(), 0.f );
		particleTransform.translation( particle.position() );
        // Add spin:
        float spinSpeed = particle.meshSpinSpeed();
		if (spinSpeed != 0.f )
		{
            Vector3 spinAxis = particle.meshSpinAxis();
			Matrix spin;
			D3DXMatrixRotationAxis
            ( 
                &spin, 
                &spinAxis, 
                spinSpeed * PARTICLE_MESH_MAX_SPIN * (particle.age() - particle.meshSpinAge()) 
            );
			particleTransform.preMultiply( spin );
		}

		if (local())
			particleTransform.postMultiply(worldTransform);
		Moo::rc().world( particleTransform );

        static Moo::EffectConstantValuePtr* pMeshTint_ = NULL;
		if ( !pMeshTint_ )
		{
			pMeshTint_ = Moo::EffectConstantValue::get( "ParticleMeshTint" );
		}
        s_particleMeshTintConstant->tint(Colour::getVector4Normalised(particle.colour()));
		*pMeshTint_ = s_particleMeshTintConstant;

		if (pVisual_)
            pVisual_->draw(true);
	}

	Moo::rc().pop();
}


void VisualParticleRenderer::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, visualName_, String, load);

	if (load)
    {
		this->visual(visualName_);
    }
}


/*static*/ void VisualParticleRenderer::prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output )
{
	BW_GUARD;
	const std::string& name = pSection->readString( "visualName_" );
	if (name.length())
		output.insert(name);
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to the PyVisualParticleRenderer.
// -----------------------------------------------------------------------------

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE PyVisualParticleRenderer::

PY_TYPEOBJECT( PyVisualParticleRenderer )

/*~ function Pixie.VisualParticleRenderer
 *	Factory function to create and return a new PyVisualParticleRenderer object. A
 *	VisualParticleRenderer is a ParticleSystemRenderer which renders each particle
 *	as a visual.
 *	Note that it is quicker to use MeshParticle objects exported specifically for
 *	use with the MeshParticleRenderer, as they can be drawn 15-at-a-time; however
 *	the MeshParticle objects have more limited material functionality.  The
 *	VisualParticleRenderer can render any material.
 *	@return A new PyVisualParticleRenderer object.
 */
PY_FACTORY_NAMED( PyVisualParticleRenderer, "VisualParticleRenderer", Pixie )

PY_BEGIN_METHODS( PyVisualParticleRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyVisualParticleRenderer )	
	/*~ attribute PyVisualParticleRenderer.visual
	 *	This is the name of the visual particle type to use for the particles.
	 *	Default value is "".
	 *	@type String.
	 */
	PY_ATTRIBUTE( visual )    
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
PyVisualParticleRenderer::PyVisualParticleRenderer( VisualParticleRendererPtr pR, PyTypePlus *pType ):
	PyParticleSystemRenderer( pR, pType ),
	pR_( pR )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pR_.hasObject() )
	{
		MF_EXIT( "NULL renderer" );
	}
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyVisualParticleRenderer::pyGetAttribute( const char *attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyParticleSystemRenderer::pyGetAttribute( attr );
}

/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the writable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 *	@param value	The value to assign to the attribute.
 */
int PyVisualParticleRenderer::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyParticleSystemRenderer::pySetAttribute( attr, value );
}


/**
 *	Static Python factory method. This is declared through the factory
 *	declaration in the class definition.
 *
 *	@param	args	The list of parameters passed from Python. This should
 *					just be a string (textureName.)
 */
PyObject *PyVisualParticleRenderer::pyNew( PyObject *args )
{
	BW_GUARD;
	char *nameFromArgs = "None";
	if (!PyArg_ParseTuple( args, "|s", &nameFromArgs ) )
	{
		PyErr_SetString( PyExc_TypeError, "VisualParticleRenderer() expects "
			"an optional visual name string" );
		return NULL;
	}

	VisualParticleRenderer * vpr = new VisualParticleRenderer();
	if ( _stricmp(nameFromArgs,"None") )
		vpr->visual( std::string( nameFromArgs ) );

	return new PyVisualParticleRenderer(vpr);
}
