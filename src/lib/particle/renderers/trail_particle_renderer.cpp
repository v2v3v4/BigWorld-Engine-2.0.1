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
#include "trail_particle_renderer.hpp"
#include "moo/render_context.hpp"
#include "moo/moo_math.hpp"
#include "romp/geometrics.hpp"
#include "moo/visual_channels.hpp"
#include "moo/fog_helper.hpp" 

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

PROFILER_DECLARE( TrailParticleRenderer_realDraw, "TrailParticleRenderer RealDraw" );

const std::string TrailParticleRenderer::nameID_ = "TrailParticleRenderer";


// -----------------------------------------------------------------------------
//	Section - TrailParticleRenderer
// -----------------------------------------------------------------------------

TrailParticleRenderer::TrailParticleRenderer():	
	textureName_( "" ),
	useFog_( true ),
	width_( 0.25f ),
	cache_( NULL ),
	cacheIdx_( 0 ),
	capacity_( 0 ),
	frame_( 0 ),
	skip_( 0 )
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

	this->steps( 5 );
}


TrailParticleRenderer::~TrailParticleRenderer()
{
	BW_GUARD;
	if (cache_ != NULL)
	{
		delete [] cache_;
	}
}


/**
 *	This method sets the texture name for the renderer.
 */
void TrailParticleRenderer::textureName( const std::string& v )
{
	BW_GUARD;
	Moo::BaseTexturePtr pTexture = Moo::TextureManager::instance()->get( v, true, true, true, "texture/particle" );
	if ( !pTexture.hasObject() )
		return;
	textureName_ = v;
	material_.textureStage(0).pTexture( pTexture );
}


void TrailParticleRenderer::draw( const Matrix & worldTransform,
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


void TrailParticleRenderer::update( Particles::iterator beg,
		Particles::iterator end, float dTime )
{
	BW_GUARD;
	if ( beg == end ) return;
	frame_++;
	if ( (skip_==0) || ((frame_%skip_)==0) )
	{
		cacheIdx_ = (cacheIdx_+1)%steps_;
		cache_[cacheIdx_].copy( beg, end );
	}
}


/**
 *	This method views the given particle system using this renderer.
 */
void TrailParticleRenderer::realDraw( const Matrix & worldTransform,
	Particles::iterator beg,
	Particles::iterator end )
{
	BW_GUARD_PROFILER( TrailParticleRenderer_realDraw );

	if ( beg == end ) return;

	//set render states
	Moo::rc().setPixelShader( 0 );
	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
	Moo::rc().setIndices( NULL );
	material_.set();
	if (material_.fogged())
	{
		Moo::FogHelper::setFog( Moo::rc().fogNear(), Moo::rc().fogFar(),  
 			D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ); 
	}

	//draw
	// Iterate through the particles.

	Vector3 a;
	Vector3 b;

	float uvStep = 1.f / (float)steps_;

	if ( Geometrics::beginTexturedWorldLines() )
	{
		Particles::iterator iter = beg;
		while ( iter != end )
		{
			Particle &particle = *iter++;

			if (!particle.isAlive())
				continue;

			a = particle.position();
			float age = particle.age();
			float previousCacheAge = age;
			int idx = cacheIdx_;
			float uvStart = 0.f;

			for ( int i=0; i<steps_; i++ )
			{
				Cache& cache = cache_[idx];

				b = cache.position_[particle.index()];

				// these checks assume the cache is ordered with steadily decreasing particle ages.
				// if a particle is suddenly killed off (splat, barrier, collide, sink on min speed)
				// then an older particle may be put in its place and you will have a jump in the cache ages
				// e.g. 7 s old particle to 0.5s old particle. In this situation the renderer will draw a line
				// between these two positions (as it assumes steadily decreasing ages) which may be far apart.
				// FOR WHOEVER ATTEMPTS FIXING THIS: in update function check to see if particles have been rearranged
				// by looking at their index positions. If this has happened then you may assume that a particle has sunk
				// so clear entries for that cache! (although this may be slow solution)

				// If the cache's age has suddenly gone back up then it means its cache info for a previous particle
				if ( cache.age_[particle.index()] > previousCacheAge )
					break;

				// Draw connections apart from the first one (as it will be a == b the first time)
				if ( i > 0 )
					Geometrics::texturedWorldLine( a, b, width_, Moo::Colour(particle.colour()), uvStep, uvStart );

				// once you reach the end of this particle's cache information stop.
				// break AFTER drawing connection as otherwise the last step won't be drawn
				if ( cache.age_[particle.index()] == 0.f ) 
					break;

				previousCacheAge = cache.age_[particle.index()];
				a = b;
				idx = (idx+(steps_-1))%steps_;
				uvStart += uvStep;
			}
		}
		Geometrics::endTexturedWorldLines( local() ? worldTransform : Matrix::identity );
	}

	Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
}


void TrailParticleRenderer::capacity(int c)
{
	BW_GUARD;
	if ( c==0 )
		return;

	capacity_ = c;
	for ( int i=0; i<steps_; i++ )
	{
		cache_[i].capacity(capacity_);
	}
}

size_t TrailParticleRenderer::sizeInBytes() const
{
	BW_GUARD;
	int cacheCapacity(0);
	for ( int i=0; i<steps_; i++ )
	{
		cacheCapacity += cache_[i].sizeInBytes();
	}

	return sizeof(TrailParticleRenderer) + cacheCapacity;
}


/**
 *	Cosntructor
 */
TrailParticleRenderer::Cache::Cache() :
	size_( 0 )
{
}

/**
 *	Destructor
 */
TrailParticleRenderer::Cache::~Cache()
{
}

void TrailParticleRenderer::Cache::capacity( int c )
{
	BW_GUARD;
	size_ = c;
	Vector3 zeroVector(0,0,0);
	position_.resize( c, zeroVector );
	age_.resize( c, 1e23f );
}

size_t TrailParticleRenderer::Cache::sizeInBytes() const
{
	return sizeof(TrailParticleRenderer::Cache) + (sizeof(float) + sizeof(Vector3)) * size_;
}


void TrailParticleRenderer::Cache::copy( Particles::iterator iter, Particles::iterator end )
{
	BW_GUARD;
	//TWO ASSUMPTIONS
	//1) particle indices never exceed the particle system capacity
	//2) the cache has already been setup ( its capacity() has been called )
	Particles::iterator theEnd = end;
	if ( (end-iter) > size_ )
	{
		ERROR_MSG( "Warning - size is %d but num particles incoming is %d\n", size_, (end-iter) );
		theEnd = iter + size_;	// be graceful
	}

	while ( iter != theEnd )
	{
		Particle &particle = *iter++;
		if (particle.isAlive())
		{
			this->age_[particle.index()] = particle.age();
			this->position_[particle.index()] = particle.position();
		}
	}
}

void TrailParticleRenderer::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, textureName_, String, load);
	SERIALISE(pSect, width_, Float, load);
	SERIALISE(pSect, skip_, Int, load);
	SERIALISE_TO_FUNCTION(pSect, useFog, Bool, load);
	SERIALISE_TO_FUNCTION(pSect, steps, Int, load);

	if ( load )
		this->textureName( textureName_ );
}


/*static*/ void TrailParticleRenderer::prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output )
{
	BW_GUARD;
	const std::string& name = pSection->readString( "textureName_" );
	if (name.length())
		output.insert(name);
}



void TrailParticleRenderer::steps( int value )
{
	BW_GUARD;
	if (cache_ != NULL)
	{
		delete [] cache_;
	}

	steps_ = value;
	cache_ = new Cache[steps_];
	cacheIdx_ = 0;

	this->capacity( capacity_ );
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to the PyTrailParticleRenderer.
// -----------------------------------------------------------------------------


#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE PyTrailParticleRenderer::

PY_TYPEOBJECT( PyTrailParticleRenderer )

/*~ function Pixie.TrailParticleRenderer
 *	This function creates and returns a new PyTrailParticleRenderer object.
 */
PY_FACTORY_NAMED( PyTrailParticleRenderer, "TrailParticleRenderer", Pixie )

PY_BEGIN_METHODS( PyTrailParticleRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyTrailParticleRenderer )	
	/*~ attribute PyTrailParticleRenderer.textureName
	 *	This attribute specifies the name of the texture used to render the
	 *	trails of the particles.
	 */
	PY_ATTRIBUTE( textureName )
	/*~ attribute PyTrailParticleRenderer.width
	 *	This attribute specifies the width of the line renderer as the trail
	 *	on the particles.
	 */
	PY_ATTRIBUTE( width )
	/*~ attribute PyTrailParticleRenderer.steps
	 *	The step attribute specifies the number of old particle positions
	 *	to remember for each of the particles. These are points which the
	 *	TrailParticleRenderer uses to render the trail lines for each particle.
	 *	Default value is 5.
	 *	@type Integer.
	 */
	PY_ATTRIBUTE( steps )
	/*~ attribute PyTrailParticleRenderer.skip
	 *	The attribute skip is used to skip the render caching of trail elements
	 *	every skip-1 frames. Thus if skip is 5, TrailParticleRenderer only
	 *	caches render elements every 5 frames - based off the formula
	 *	(frame % skip) == 0. Default value is 0 (no skipping).
	 *	@type Integer.
	 */
	PY_ATTRIBUTE( skip )
	/*~ attribute PyTrailParticleRenderer.useFog
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
PyTrailParticleRenderer::PyTrailParticleRenderer( TrailParticleRendererPtr pR, PyTypePlus *pType ):
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
PyObject *PyTrailParticleRenderer::pyGetAttribute( const char *attr )
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
int PyTrailParticleRenderer::pySetAttribute( const char *attr, PyObject *value )
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
PyObject *PyTrailParticleRenderer::pyNew( PyObject *args )
{
	BW_GUARD;
	char *nameFromArgs = "None";
	if (!PyArg_ParseTuple( args, "|s", &nameFromArgs ) )
	{
		PyErr_SetString( PyExc_TypeError, "TrailParticleRenderer() expects "
			"an optional texture name string" );
		return NULL;
	}

	TrailParticleRenderer * apr = new TrailParticleRenderer();
	if ( _stricmp(nameFromArgs,"None") )
		apr->textureName( std::string( nameFromArgs ) );

	return new PyTrailParticleRenderer(apr);
}
