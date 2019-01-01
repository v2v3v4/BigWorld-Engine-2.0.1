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
#include "sprite_particle_renderer.hpp"
#ifndef CODE_INLINE
#include "sprite_particle_renderer.ipp"
#endif
#include "moo/dynamic_vertex_buffer.hpp"
#include "moo/visual_channels.hpp"
#include "math/planeeq.hpp"
#include "job_system/job_system.hpp"
#include "moo/fog_helper.hpp" 


const std::string SpriteParticleRenderer::nameID_ = "SpriteParticleRenderer";

PROFILER_DECLARE( SpriteParticleRenderer_realDraw, "SpriteParticleRenderer RealDraw" );


// -----------------------------------------------------------------------------
// Section: Constructor(s) and Destructor for SpriteParticleRenderer.
// -----------------------------------------------------------------------------

/**
 *	This is the constructor for SpriteParticleRenderer.
 *
 *	@param newTextureName	String containing the file name of the sprite.
 */
SpriteParticleRenderer::SpriteParticleRenderer(
		const std::string newTextureName ) :
	materialFX_( FX_BLENDED ),
	textureName_( newTextureName ),
	useFog_( true ),
	materialSettingsChanged_( false ),
	frameCount_( 0 ),
	frameRate_( 0.f ),
	rotated_( false ),
	explicitOrientation_(0,0,0)
{
	BW_GUARD;
	// Set up the texture coordinate in UV space for the sprite, horizontal
	// blur, and vertical blur meshes.
	sprite_[0].uv_.set( 0.0f, 1.0f );
	sprite_[1].uv_.set( 0.0f, 0.0f );
	sprite_[2].uv_.set( 1.0f, 0.0f );
	sprite_[3].uv_.set( 1.0f, 1.0f );

	// Set specular (actually fog values) for the vertices.
	sprite_[0].specular_ = 0xffffffff;
	sprite_[1].specular_ = 0xffffffff;
	sprite_[2].specular_ = 0xffffffff;
	sprite_[3].specular_ = 0xffffffff;

	// Initialise the material for the sprite.
	if (textureName_.size() > 0)
		updateMaterial();
}

/**
 *	This is the destructor for SpriteParticleRenderer.
 */
INLINE SpriteParticleRenderer::~SpriteParticleRenderer()
{
}



// -----------------------------------------------------------------------------
// Section: Renderer Overrides for the SpriteParticleRenderer.
// -----------------------------------------------------------------------------

/**
 *	This is the draw method that does the drawing for the frame. In theory,
 *	a sprite particle renderer could draw multiple particle systems provided
 *	they all use the same texture and effects.
 */
void SpriteParticleRenderer::draw( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox &bb )
{
	BW_GUARD;
	float particleDistConvert = 65535.f / Moo::rc().camera().farPlane();
	
	// Make sure the texture is ready.
	if ( materialSettingsChanged_ )
	{
		updateMaterial();
	}

	if (beg != end)
	{
		Matrix view = Moo::rc().view();
		float distance = 0.f;
		if ( local() )
		{
			view.preMultiply(worldTransform);
		}

		if (material_.destBlend() == Moo::Material::ONE)
		{
			BoundingBox bounds = bb;
			bounds.transformBy( view );
			distance = (bounds.maxBounds().z + bounds.minBounds().z) * 0.5f;
		}
		else if (!viewDependent())
		{
			Particles::iterator it = beg;
			const Vector3& pos = it->position();
			float maxDist = pos.x * view[0][2] +
				pos.y * view[1][2] +
				pos.z * view[2][2] +
				view[3][2];

			float minDist = maxDist;
			it++;

			while (it != end)
			{
				const Vector3& p = it->position();
				float dist = p.x * view[0][2] +
					p.y * view[1][2] +
					p.z * view[2][2] +
					view[3][2];

				if (dist <= 0)
					it->distance(0);
				else if (dist >= Moo::rc().camera().farPlane())
					it->distance(65535);
				else
					it->distance(uint16(particleDistConvert * dist));


				maxDist = max( maxDist, dist );
				minDist = min( minDist, dist );

				it++;
			}
			distance = (maxDist + minDist) * 0.5f;
		}

		sortedDrawItem_.set( this, worldTransform, beg, end, distance );

		if ( SpriteParticleRenderer::FX_SHIMMER != materialFX() )
		{
			Moo::SortedChannel::addDrawItem( &sortedDrawItem_ );
		}
		else
		{
			Moo::ShimmerChannel::addDrawItem( &sortedDrawItem_ );
		}
	}
}


class SpriteParticleJob : public Job
{
	friend class SpriteParticleRenderer;

public:
	virtual void execute();

private:
	uint numParticles_;
	Moo::VertexXYZDUV* pVertex_;
	int frameCount_;
	Matrix view_;
	Matrix worldTransform_;
	float fogNear_;
	float fogRange_;
	SpriteParticleRenderer::MaterialFX materialFX_;
	bool local_;
	Vector3 explicitOrientation_;
	bool rotated_;
	float frameRate_;
	uint* numTrisPtr_;
};

static uint s_numTris = 1;	// Not zero so when using jobs we don't skip DrawPrimitive
Particle* s_particles;

void SpriteParticleJob::execute()
{
	Moo::VertexXYZDUV	quad[4];

	Moo::VertexXYZDUV* pVertex = pVertex_;
	int frameCount = frameCount_;
	Matrix view = view_;
	Matrix worldTransform = worldTransform_;
	float fogNear = fogNear_;
	float fogRange = fogRange_;
	SpriteParticleRenderer::MaterialFX materialFX = materialFX_;

	float u = 0.f;
	float ux = 1.f;

	if (frameCount_ > 0)
	{
		ux = 1.f / frameCount_;
	}

	// set the minsize to be a quarter of a pixel in screenspace
	float minSize = 0.25f / Moo::rc().halfScreenWidth();

	bool reOrient = (explicitOrientation_ != Vector3(0,0,0));

	uint numTris = 0;
	Particle* particles;
	if ( Moo::rc().usingWrapper() )
	{
		particles = ( Particle* )( this + 1 );
	}
	else
	{
		particles = s_particles;
	}

	Vector3 position;

	for ( uint i = 0; i < numParticles_; i++ )
	{
		Particle& p = particles[i];

		if (!p.isAlive())
			continue;

		if (frameCount_ > 0)
		{
			u = floorf( p.age() * frameRate_ ) * ux;
		}

		Matrix transform( view );
		if ( local_ )
		{
			transform.preMultiply( worldTransform );
		}
		transform.applyPoint( position, p.position() );
		if (position.z < 0)
			continue;

		float size = p.size();

		// skip this particle if it's too small.
		if (size < minSize * position.z)
			continue;

		// apply the rotation
		const float initialRotation = p.yaw();
		float rotationSpeed = p.pitch();
		if (rotationSpeed > MATH_PI)
			rotationSpeed -= MATH_2PI;
		rotationSpeed *= MATH_2PI;// convert from rev/s to angular speed

		const float actualRotation = initialRotation + rotationSpeed * p.age();
		const float sinx = rotated_ ? (float)sin(actualRotation) : 0.f;
		const float cosx = rotated_ ? (float)cos(actualRotation) : 1.f;

		if (reOrient)
		{		
			PlaneEq plane(explicitOrientation_,0.f);
			Vector3 xdir,ydir;
			plane.basis(xdir,ydir);

			xdir = view_.applyVector(xdir*size);
			ydir = view_.applyVector(ydir*size);

			quad[0].pos_ = position - ydir*cosx + xdir*sinx;
			quad[0].uv_.set( u, 0.0f );

			quad[1].pos_ = position + xdir*cosx + ydir*sinx;
			quad[1].uv_.set( u + ux, 0.0f );

			quad[2].pos_ = position + ydir*cosx - xdir*sinx;
			quad[2].uv_.set( u + ux, 1.0f );

			quad[3].pos_ = position - xdir*cosx - ydir*sinx;
			quad[3].uv_.set( u, 1.0f );
		}
		else
		{
			// set up the 4 vertices for the sprite
			quad[0].pos_.x = position.x - size * cosx - size * sinx;		
			quad[0].pos_.y = position.y - size * sinx + size * cosx;
			quad[0].uv_.set( u, 0.0f );
			quad[1].pos_.x = position.x + size * cosx - size * sinx;
			quad[1].pos_.y = position.y + size * sinx + size * cosx;
			quad[1].uv_.set( u + ux, 0.0f );
			quad[2].pos_.x = position.x + size * cosx + size * sinx;
			quad[2].pos_.y = position.y + size * sinx - size * cosx;
			quad[2].uv_.set( u + ux, 1.0f );
			quad[3].pos_.x = position.x - size * cosx + size * sinx;
			quad[3].pos_.y = position.y - size * sinx - size * cosx;
			quad[3].uv_.set( u, 1.0f );
		}

		if ( materialFX == SpriteParticleRenderer::FX_ADDITIVE_ALPHA )
		{
			float dist = 1.f - Math::clamp( 0.f, ((position.z - fogNear) / fogRange), 1.f );					
			for (int j=0; j < 4; j++)
			{
				if (!reOrient)
					quad[j].pos_.z = position.z;
				quad[j].colour_ = Colour::getUint32(Colour::getVector4(p.colour()) * dist);
			}
		}
		else
		{
			for (int j=0; j < 4; j++)
			{
				if (!reOrient)
					quad[j].pos_.z = position.z;
				quad[j].colour_ = p.colour();
			}
		}

		// add the 2 triangles to the triangle-list
		*(pVertex++) = quad[0];
		*(pVertex++) = quad[1];
		*(pVertex++) = quad[2];
		
		*(pVertex++) = quad[0];
		*(pVertex++) = quad[2];
		*(pVertex++) = quad[3];

		numTris += 2;
	}

	*numTrisPtr_ = numTris;
}





void SpriteParticleRenderer::realDraw( const Matrix& worldTransform, Particles::iterator beg, Particles::iterator end )
{
	BW_GUARD_PROFILER( SpriteParticleRenderer_realDraw );	

	if (material_.numTextureStages() == 0 ||
		(SpriteParticleRenderer::FX_SHIMMER == materialFX() &&
		!Moo::Material::shimmerMaterials))
	{
		return;
	}

	// The particles may be either in view space or world space.

	Matrix view(Matrix::identity);

	if (!viewDependent())
	{
		view = Moo::rc().view();
	}

	DX::Device* pDev = Moo::rc().device();

	pDev->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
	pDev->SetTransform( D3DTS_VIEW, &Matrix::identity );
	pDev->SetTransform( D3DTS_WORLD, &Matrix::identity );
	Moo::rc().setPixelShader( NULL );
	Moo::rc().setVertexShader( NULL );
	Moo::rc().setFVF( Moo::VertexXYZDUV::fvf() );
	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );

	if ( Moo::rc().usingWrapper() )
	{
		DX::newBlock();
	}

	DX::ScopedWrapperFlags swf( DX::getWrapperFlags() | DX::WRAPPER_FLAG_DEFERRED_LOCK );

	Moo::DynamicVertexBufferBase2<Moo::VertexXYZDUV>& vb = Moo::DynamicVertexBufferBase2<Moo::VertexXYZDUV>::instance();
	Moo::VertexXYZDUV* pVertex = vb.lock2((end - beg)*6);
	if (!pVertex)
		return;
	// store colorwriteenable in case we are shimmering
	Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE );

	material_.set();

	float fogNear = Moo::rc().fogNear();
	float fogFar = Moo::rc().fogFar();
	float fogRange = (fogFar - fogNear);
	if (material_.fogged())
	{
		Moo::FogHelper::setFog( fogNear, fogFar, D3DRS_FOGTABLEMODE, D3DFOG_LINEAR ); 
	}

	Vector3 position;

	if (material_.destBlend() != Moo::Material::ONE)
	{
		std::sort( beg, end, Particle::sortParticlesReverse );
	}

	// set up job
	SpriteParticleJob immediateJob;
	SpriteParticleJob* job = &immediateJob;

	Particle* p = &(*beg);
	uint numParticles = &(*end) - p;
	uint size = numParticles * sizeof( Particle );

	if ( Moo::rc().usingWrapper() )
	{
		JobSystem* jobSystem = JobSystem::pInstance();
		job = jobSystem->allocJob<SpriteParticleJob>( size );
	}

	job->numParticles_ = numParticles;
	job->pVertex_ = pVertex;
	job->frameCount_ = frameCount_;
	job->view_ = view;
	job->worldTransform_ = worldTransform;
	job->fogNear_ = fogNear;
	job->fogRange_ = fogRange;
	job->materialFX_ = materialFX();
	job->local_ = local();
	job->explicitOrientation_ = explicitOrientation_;
	job->rotated_ = rotated_;
	job->frameRate_ = frameRate_;

	if ( Moo::rc().usingWrapper() )
	{
		CommandBuffer::memcpyNTA( job + 1, p, size );
	}
	else
	{
		s_particles = p;
		job->numTrisPtr_ = &s_numTris;
		job->execute();
	}

	// Render
	vb.unlock();	
	uint32 lockIndex = vb.lockIndex();

	if ( s_numTris != 0 )
	{
		vb.set( 0 );
		Moo::rc().drawPrimitive( D3DPT_TRIANGLELIST, lockIndex, s_numTris );

		if ( Moo::rc().usingWrapper() )
		{
			job->numTrisPtr_ = DX::getPatchForPrimCount();
		}
	}

	// set color write enable in case we are shimmering
	Moo::rc().popRenderState();
}

// -----------------------------------------------------------------------------
// Section: Auxiliary Methods for SpriteParticleRenderer.
// -----------------------------------------------------------------------------

/**
 *	This is the Set-Accessor for the texture file name of the sprite.
 *
 *	@param newString	The new file name as a string.
 */
void SpriteParticleRenderer::textureName( const std::string &newString )
{
	BW_GUARD;
	textureName_ = newString;
	materialSettingsChanged_ = true;
}


/**
 *	This method is called whenever a material property for the sprite has
 *	been changed. It prepares any changes for drawing.
 */
void SpriteParticleRenderer::updateMaterial( void )
{
	BW_GUARD;
	typedef BGUpdateData<SpriteParticleRenderer> SPRBGUpdateData;
	SPRBGUpdateData * data = NULL;
    try
    {
        data = new SPRBGUpdateData(this); 

	    SPRBGUpdateData::loadTexture(data);
	    SPRBGUpdateData::updateMaterial(data);
        data = NULL;
    }
    catch (...)
    {
        delete data; throw;
    }
    		
	materialSettingsChanged_ = false;
}		


void SpriteParticleRenderer::updateMaterial( Moo::BaseTexturePtr texture )
{
	BW_GUARD;
	if ( material_.numTextureStages() == 0 )
	{
		// Set up the texture stages if they have not
		// been created beforehand and load the texture file.

		Moo::TextureStage textureStage;
		textureStage.pTexture( texture );
		textureStage.colourOperation( Moo::TextureStage::MODULATE,
			Moo::TextureStage::TEXTURE,
			Moo::TextureStage::DIFFUSE );
		textureStage.alphaOperation( Moo::TextureStage::MODULATE,
			Moo::TextureStage::TEXTURE,
			Moo::TextureStage::DIFFUSE );
		material_.addTextureStage( textureStage );

		// add a second texture stage that is disabled
		textureStage.colourOperation( Moo::TextureStage::DISABLE );
		textureStage.alphaOperation( Moo::TextureStage::DISABLE );
		textureStage.pTexture( NULL );
		material_.addTextureStage( textureStage );

		material_.alphaBlended( true );
		material_.selfIllum( 255.0f );
		material_.srcBlend( Moo::Material::ONE );
		material_.destBlend( Moo::Material::INV_SRC_ALPHA);
		material_.sorted( true );
		material_.zBufferRead( true );
		material_.zBufferWrite( false );
		material_.fogged( true );
	}
	else
	{
		// If the texture stages have already been set
		// up, then simply load the new texture file.
		material_.textureStage( 0 ).pTexture( texture );
	}

	// Update the Moo::Material with the current setting for materialFX.
	switch ( materialFX() )
	{
	case SpriteParticleRenderer::FX_ADDITIVE:
		material_.srcBlend( Moo::Material::SRC_ALPHA );
		material_.destBlend( Moo::Material::ONE );
		material_.shimmer( false );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
    	break;
	case SpriteParticleRenderer::FX_ADDITIVE_ALPHA:
		material_.srcBlend( Moo::Material::ONE );
		material_.destBlend( Moo::Material::INV_SRC_ALPHA );
		material_.shimmer( false );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_BLENDED:
		material_.srcBlend( Moo::Material::SRC_ALPHA );
		material_.destBlend( Moo::Material::INV_SRC_ALPHA );
		material_.shimmer( false );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_BLENDED_COLOUR:
		material_.srcBlend( Moo::Material::SRC_COLOUR );
		material_.destBlend( Moo::Material::INV_SRC_COLOUR );
		material_.shimmer( false );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_BLENDED_INVERSE_COLOUR:
		//NOTE : this option removed because fogging does not work.
		//changed to do the same as BLENDED_COLOUR.		
		material_.srcBlend( Moo::Material::SRC_COLOUR );
		material_.destBlend( Moo::Material::INV_SRC_COLOUR );
		material_.shimmer( false );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_SOLID:
		material_.srcBlend( Moo::Material::ONE );
		material_.destBlend( Moo::Material::ZERO );
		material_.shimmer( false );
        material_.alphaBlended( false );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_SHIMMER:
		material_.solid( false );
		material_.sorted( false );
		material_.shimmer( true );
        material_.alphaBlended( true );
        material_.alphaTestEnable( false );
		break;
	case SpriteParticleRenderer::FX_SOURCE_ALPHA:
		material_.srcBlend( Moo::Material::ONE );
		material_.destBlend( Moo::Material::ZERO );
		material_.shimmer( false );
        material_.alphaBlended( false );
        material_.alphaTestEnable( true );
        material_.alphaReference( 0x80 );
		break;
	}

	material_.fogged( useFog_ );
	material_.doubleSided( true );
}


void SpriteParticleRenderer::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, textureName_, String, load);
	SERIALISE_ENUM(pSect, materialFX_, MaterialFX, load);
	SERIALISE(pSect, frameCount_, Int, load );
	SERIALISE(pSect, frameRate_, Float, load );
	SERIALISE(pSect, explicitOrientation_, Vector3, load);
	SERIALISE(pSect, useFog_, Bool, load);

	// flag update of material
	if (load)
	{
		this->updateMaterial();
	}
}


/*static*/ void SpriteParticleRenderer::prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output )
{
	const std::string& textureName = pSection->readString( "textureName_" );
	if (textureName.length())
		output.insert(textureName);
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to PySpriteParticleRenderer.
// -----------------------------------------------------------------------------

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE PySpriteParticleRenderer::
PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( MaterialFX, materialFX,
	materialFX )

PY_TYPEOBJECT( PySpriteParticleRenderer )

/*~ function Pixie.SpriteRenderer
 *	Factory function to create and return a new PySpriteParticleRenderer object. A
 *	SpriteParticleRenderer is a ParticleSystemRenderer which renders each
 *	particle as a facing polygon.
 *	@param spritename Name of sprite to use for particles.
 *	@return A new PySpriteParticleRenderer object.
 */
PY_FACTORY_NAMED( PySpriteParticleRenderer, "SpriteRenderer", Pixie )

PY_BEGIN_METHODS( PySpriteParticleRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PySpriteParticleRenderer )
	/*~ attribute PySpriteParticleRenderer.viewDependent
	 *	Specifies whether the coordinates of the particle system are in camera space (true)
	 *	rather than world space (false). Default is 0 (false - world space).
	 *	@type Integer as boolean. 0 or 1.
	 */
	PY_ATTRIBUTE( viewDependent )
	/*~ attribute PySpriteParticleRenderer.textureName
	 *	Texture file name of the sprite.
	 *	@type String.
	 */
	PY_ATTRIBUTE( textureName )
	/*~ attribute PySpriteParticleRenderer.materialFX
	 *	Specifies the MaterialFX to use to draw the sprites into the scene.
	 *	Each value specifies either a special effect, or a source and destination
	 *	alpha-blend setting for the material. If no value is specified, materialFX
	 *	defaults to 'BLENDED'. The possible values are listed below:
	 *
	 *	ADDITIVE: alpha blended. source = SRC_ALPHA, destination = ONE
	 *
	 *	ADDITIVE_ALPHA: alpha blended. source = ONE, destination = INV_SRC_ALPHA
	 *
	 *	BLENDED: alpha blended. source = SRC_ALPHA, destination = INV_SRC_ALPHA
	 *
	 *	BLENDED_COLOUR: alpha blended. source = SRC_COLOUR, destination = INV_SRC_COLOUR
	 *
	 *	BLENDED_INVERSE_COLOUR: alpha blended. source = INV_SRC_COLOUR, destination = SRC_COLOUR
	 *
	 *	SOLID: alpha blended. source = ONE, destination = ZERO
	 *
	 *	SHIMMER: special effect not implemented on PC
	 *
	 *	Refer to the DirectX documentation for a description of how material
	 *	settings affect alpha blending.
	 *
	 *	@type String.
	 */
	PY_ATTRIBUTE( materialFX )
	/*~ attribute PySpriteParticleRenderer.frameCount
	 *	Specifies the number of frames of animation in the texture. Default is 0.
	 *	@type Integer.
	 */
	PY_ATTRIBUTE( frameCount )
	/*~ attribute PySpriteParticleRenderer.frameRate
	 *	Specifies the number of frames of animation per second of particle age.
	 *	Default is 0.
	 *	@type Float.
	 */
	PY_ATTRIBUTE( frameRate )
	/*~ attribute PySpriteParticleRenderer.useFog
	 *	Specifies whether the renderer should enable scene fogging or not.  You
	 *	may want to turn off scene fogging if you are using particle systems in
	 *	the sky box, or if you want to otherwise explicitly control the amount
	 *	of fogging via the tint shader fog blend control.
	 */
	PY_ATTRIBUTE( useFog )
PY_END_ATTRIBUTES()

PY_ENUM_MAP( PySpriteParticleRenderer::MaterialFX )
PY_ENUM_CONVERTERS_CONTIGUOUS( PySpriteParticleRenderer::MaterialFX )


/**
 *	Constructor.
 */
PySpriteParticleRenderer::PySpriteParticleRenderer( SpriteParticleRendererPtr pR, PyTypePlus *pType ):
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
PyObject *PySpriteParticleRenderer::pyGetAttribute( const char *attr )
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
int PySpriteParticleRenderer::pySetAttribute( const char *attr, PyObject *value )
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

PyObject *PySpriteParticleRenderer::pyNew( PyObject *args )
{
	BW_GUARD;
	char *nameFromArgs = "None";
	if (!PyArg_ParseTuple( args, "|s", &nameFromArgs ) )
	{
		PyErr_SetString( PyExc_TypeError, "SpriteRenderer() expects "
			"a optional texture name string" );
		return NULL;
	}

	SpriteParticleRenderer* spr = new SpriteParticleRenderer( nameFromArgs );
	return new PySpriteParticleRenderer( spr );
}
