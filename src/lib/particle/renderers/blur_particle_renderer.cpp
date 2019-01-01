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
#include "blur_particle_renderer.hpp"
#include "moo/render_context.hpp"
#include "moo/moo_math.hpp"
#include "romp/geometrics.hpp"
#include "moo/visual_channels.hpp"
#include "moo/fog_helper.hpp" 


// -----------------------------------------------------------------------------
//	Section - BlurParticleRenderer
// -----------------------------------------------------------------------------
BlurParticleRenderer::BlurParticleRenderer():
	textureName_( "" ),
	useFog_( true ),
	width_( 0.25f ),
	time_( 0.25f )
{
	BW_GUARD;
	material_.alphaTestEnable( false );
	material_.alphaBlended( true );
	material_.destBlend( Moo::Material::ONE );
	material_.srcBlend( Moo::Material::ONE );
	material_.zBufferRead( true );
	material_.zBufferWrite( false );
	material_.fogged( true );
	material_.doubleSided( true );
	material_.selfIllum( 255.f );

	Moo::TextureStage ts;
	ts.colourOperation( Moo::TextureStage::MODULATE,
		Moo::TextureStage::CURRENT,
		Moo::TextureStage::TEXTURE );
	ts.alphaOperation( Moo::TextureStage::DISABLE );
	material_.addTextureStage( ts );
	ts.colourOperation( Moo::TextureStage::DISABLE );
	material_.addTextureStage( ts );
}


BlurParticleRenderer::~BlurParticleRenderer()
{
}


/**
 *	This method sets the texture name for the renderer.
 */
void BlurParticleRenderer::textureName( const std::string& v )
{
	BW_GUARD;
	Moo::BaseTexturePtr pTexture = Moo::TextureManager::instance()->get( v, true, true, true, "texture/particle" );
	if ( !pTexture.hasObject() )
		return;
	textureName_ = v;
	material_.textureStage(0).pTexture( pTexture );
}


void BlurParticleRenderer::draw( const Matrix & worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	if ( beg == end ) return;

	float distance = 
		(worldTransform.applyToOrigin() - Moo::rc().invView().applyToOrigin()).length();
	sortedDrawItem_.set( this, worldTransform, beg, end, distance );
	Moo::SortedChannel::addDrawItem( &sortedDrawItem_ );
}


/**
 *	This method views the given particle system using this renderer.
 */
void BlurParticleRenderer::realDraw( const Matrix & worldTransform,
	Particles::iterator beg,
	Particles::iterator end )
{
	BW_GUARD;
	if ( beg == end ) return;

	//set render states
	Moo::rc().setPixelShader( 0 );
	Moo::rc().setIndices( NULL );
	material_.set();
	if (material_.fogged())
	{
		Moo::FogHelper::setFog( Moo::rc().fogNear(), Moo::rc().fogFar(), 
 			D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ); 
	}

	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
	//draw
	// Iterate through the particles.
	if ( Geometrics::beginTexturedWorldLines() )
	{
		Particles::iterator iter = beg;
		while ( iter != end )
		{
			Particle &particle = *iter++;

			if (particle.isAlive())
			{
				Vector3 velocity;
				particle.getVelocity( velocity );
				
				Geometrics::texturedWorldLine( 
					particle.position(), 
					particle.position() - (velocity * time_), width_, 
					Moo::Colour(particle.colour()), 1.f, 0.f );
			}
		}
		Geometrics::endTexturedWorldLines( local() ? worldTransform : Matrix::identity );
	}

	Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
}


void BlurParticleRenderer::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, textureName_, String, load);
	SERIALISE(pSect, width_, Float, load);
	SERIALISE(pSect, time_, Float, load);
	SERIALISE_TO_FUNCTION(pSect, useFog, Bool, load);

	if ( load )
		this->textureName( textureName_ );
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to the PyBlurParticleRenderer.
// -----------------------------------------------------------------------------

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE PyBlurParticleRenderer::

PY_TYPEOBJECT( PyBlurParticleRenderer )

/*~ function Pixie.BlurParticleRenderer
 *	This function creates and returns a new PyBlurParticleRenderer object.
 */
PY_FACTORY_NAMED( PyBlurParticleRenderer, "BlurParticleRenderer", Pixie )

PY_BEGIN_METHODS( PyBlurParticleRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyBlurParticleRenderer )	
	/*~ attribute PyBlurParticleRenderer.textureName
	 *	This attribute specifies the name of the texture used to render the
	 *	trails of the particles.
	 */
	PY_ATTRIBUTE( textureName )
	/*~ attribute PyBlurParticleRenderer.width
	 *	This attribute specifies the width of the line renderer as the trail
	 *	on the particles.
	 */
	PY_ATTRIBUTE( width )
	/*~ attribute PyBlurParticleRenderer.time
	 *	The time attribute specifies the length of the trail, in seconds.
	 *	The length of the tail is (particle velocity * time)
	 *	Default value is 0.25.
	 *	@type Float.
	 */
	 PY_ATTRIBUTE( time )
	 /*~ attribute PyBlurParticleRenderer.useFog
	 *	Specifies whether the renderer should enable scene fogging or not.  You
	 *	may want to turn off scene fogging if you are using particle systems in
	 *	the sky box, or if you want to otherwise explicitly control the amount
	 *	of fogging via the tint shader fog blend control.
	 */
	PY_ATTRIBUTE( useFog )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
PyBlurParticleRenderer::PyBlurParticleRenderer( BlurParticleRendererPtr pR, PyTypePlus *pType ):
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
PyObject *PyBlurParticleRenderer::pyGetAttribute( const char *attr )
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
int PyBlurParticleRenderer::pySetAttribute( const char *attr, PyObject *value )
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
PyObject *PyBlurParticleRenderer::pyNew( PyObject *args )
{
	BW_GUARD;
	char *nameFromArgs = "None";
	if (!PyArg_ParseTuple( args, "|s", &nameFromArgs ) )
	{
		PyErr_SetString( PyExc_TypeError, "BlurParticleRenderer() expects "
			"an optional texture name string" );
		return NULL;
	}

	BlurParticleRenderer * apr = new BlurParticleRenderer();
	if ( _stricmp(nameFromArgs,"None") )
		apr->textureName( std::string( nameFromArgs ) );

	return new PyBlurParticleRenderer(apr);
}

const std::string BlurParticleRenderer::nameID_ = "BlurParticleRenderer";
