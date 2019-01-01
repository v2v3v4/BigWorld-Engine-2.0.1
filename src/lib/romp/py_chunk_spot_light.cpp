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
#include "py_chunk_spot_light.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"

PROFILER_DECLARE( PyChunkSpotLight_dtor, "PyChunkSpotLight dtor" );


// -----------------------------------------------------------------------------
// Section: PyChunkSpotLight
// -----------------------------------------------------------------------------


PY_TYPEOBJECT( PyChunkSpotLight )

/*~ function PyChunkSpotLight recalc
 *  Recalculates the effects of the light. The initial setup of a PyChunkSpotLight
 *  does not require this to be called.
 *  @return None.
 */
PY_BEGIN_METHODS( PyChunkSpotLight )
	PY_METHOD( recalc )
PY_END_METHODS()

/*~ attribute PyChunkSpotLight visible
 *  Dictates whether this light illuminates things.
 *  @type Read-Write boolean
 */
/*~ attribute PyChunkSpotLight position
 *  The centre of the light source. This is overwritten with the output of the
 *  source attribute if it has been specified.
 *  @type Read-Write Vector3
 */
/*~ attribute PyChunkSpotLight direction
 *  The direction of the light source. This is overwritten with the output of the
 *  source attribute if it has been specified.
 *  @type Read-Write Vector3
 */
/*~ attribute PyChunkSpotLight colour
 *  The colour of the light source in RGBA form. This is overwritten with the 
 *  output of the shader attribute if it has been specified.
 *  @type Read-Write Vector4
 */
/*~ attribute PyChunkSpotLight innerRadius
 *  The radius around the centre of the light within which the strength of the
 *  light does not drop off. This is overwritten with the first component of
 *  the output of the bounds attribute if it has been specified.
 *  @type Read-Write Float
 */
/*~ attribute PyChunkSpotLight outerRadius
 *  If greater than innerRadius, this is the radius around the centre of the
 *  light at which the strength of the light becomes zero. If this is less than
 *  or equal to innerRadius, then there is no area around the light in which
 *  its effect dissipates. This is overwritten with the second component of the
 *  output of the bounds attribute if it has been specified.
 *  @type Read-Write Float
 */
/*~ attribute PyChunkSpotLight cosConeAngle
 *  The cos of the cone angle.
 *	1.0 = zero size cone
 *	0.0 = 180 degree cone (90 degrees on each side)
 *  @type Read-Write Float
 */
/*~ attribute PyChunkSpotLight specular
 *  If true, this provides a light source for specular shaders. This defaults
 *  to false.
 *  @type Read-Write Boolean
 */
/*~ attribute PyChunkSpotLight diffuse
 *  If true, this provides a light source for diffuse shaders. This defaults
 *  to true.
 *  @type Read-Write Boolean
 */
/*~ attribute PyChunkSpotLight source
 *  When specified, this overwrites the position attribute with it's
 *  translation.
 *  @type Read-Write MatrixProvider
 */
/*~ attribute PyChunkSpotLight bounds
 *  When specified, this overwrites the innerRadius and outerRadius attributes
 *  with the first and second components of the Vector4 it provides,
 *  respectively.
 *  @type Read-Write Vector4Provider
 */
/*~ attribute PyChunkSpotLight shader
 *  When specified, this overwrites the colour attribute with the Vector4 it
 *  provides.
 *  @type Read-Write Vector4Provider
 */
 /*~ attribute PyChunkSpotLight priority
 *	A priority bias used when sorting lights to determine the most
 *	relevant set of lights for a particular object. This is useful
 *	to ensure important lights will always be considered before other
 *	lights (e.g. a player's torch).
 *  @type Read-Write Integer
 */
PY_BEGIN_ATTRIBUTES( PyChunkSpotLight )
	PY_ATTRIBUTE( visible )
	PY_ATTRIBUTE( position )
	PY_ATTRIBUTE( direction )
	PY_ATTRIBUTE( colour )
	PY_ATTRIBUTE( priority )
	PY_ATTRIBUTE( innerRadius )
	PY_ATTRIBUTE( outerRadius )
	PY_ATTRIBUTE( cosConeAngle )
	PY_ATTRIBUTE( specular )
	PY_ATTRIBUTE( diffuse )
	PY_ATTRIBUTE( source )
	PY_ATTRIBUTE( bounds )
	PY_ATTRIBUTE( shader )
PY_END_ATTRIBUTES()

/*~ function BigWorld PyChunkSpotLight
 *  Creates a new PyChunkSpotLight. This is a script controlled chunk spot light 
 *  which can be used as a diffuse and/or specular light source for models and
 *  shells.
 */
PY_FACTORY( PyChunkSpotLight, BigWorld )


/**
 *	Constructor.
 */
PyChunkSpotLight::PyChunkSpotLight( PyTypePlus * pType ) :
	PyObjectPlusWithVD( pType ),
	ChunkSpotLight( (WantFlags)WANTS_TICK ),
	visible_( false )
{
	BW_GUARD;
	s_currentLights_.push_back( this );

	this->dynamicLight( true );
	this->specularLight( false );

	pLight_->dynamic( true );
	// this is always the case for us - re-light statically lit stuff
}


/**
 *	Destructor.
 */
PyChunkSpotLight::~PyChunkSpotLight()
{
	BW_GUARD_PROFILER( PyChunkSpotLight_dtor );
	if (this->visible()) this->visible( false );
	PyChunkSpotLights::iterator found = std::find(
		s_currentLights_.begin(), s_currentLights_.end(), this );

	MF_ASSERT_DEV( found != s_currentLights_.end() );

	if (found != s_currentLights_.end())
	{
		s_currentLights_.erase( found );
	}
}



/**
 *	Python get attribute method
 */
PyObject * PyChunkSpotLight::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	return this->PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Python set attribute method
 */
int PyChunkSpotLight::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();
	return this->PyObjectPlus::pySetAttribute( attr, value );
}



bool PyChunkSpotLight::visible() const
{
	return visible_;
}

void PyChunkSpotLight::visible( bool v )
{
	BW_GUARD;
	if (visible_ == v) return;
	visible_ = v;
	if (visible_)
	{
		this->recalc();
	}
	else
	{
		if (pChunk_ != NULL)
		{
			pChunk_->delDynamicItem( this, false );
		}
	}
}


const Vector3 & PyChunkSpotLight::position() const
{
	BW_GUARD;
	return pLight_->worldPosition();
}

void PyChunkSpotLight::position( const Vector3 & npos )
{
	BW_GUARD;
	Chunk* oldChunk = pChunk_;

	Vector3 opos = pLight_->worldPosition();

	// find which chunk light was in, and move it in that chunk
	if (pChunk_ != NULL)
	{
		pChunk_->modDynamicItem( this, opos, npos );
	}
	else if (visible_)
	{
		ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
		if (pSpace.exists())
		{
			Chunk * pDest = pSpace->findChunkFromPoint( npos );
			if (pDest != NULL)
			{
				pDest->addDynamicItem( this );
			}
		}
	}

	// find which chunk light is now in, and update its position for that chunk
	if (pChunk_ != NULL)
	{
		pLight_->position( pChunk_->transformInverse().applyPoint( npos ) );
		pLight_->worldTransform( pChunk_->transform() );

		// If the light actually moved, and we're still in the same chunk
		// then let the light cache know if it needs to push this light into
		// other chunks.
		if ( oldChunk == pChunk_ && !almostEqual( opos - npos, Vector3::zero() ) )
		{
			Vector3 locOldPos = pChunk_->transformInverse().applyPoint( opos );
			ChunkLightCache::instance( *pChunk_ ).moveSpot( pLight_, locOldPos, pLight_->outerRadius() );
		}
	}
	else
	{
		pLight_->position( npos );
		pLight_->worldTransform( Matrix::identity );
	}
}


const Vector3 & PyChunkSpotLight::direction() const
{
	BW_GUARD;
	return pLight_->worldDirection();
}

void PyChunkSpotLight::direction( const Vector3 & ndir )
{
	BW_GUARD;
	if (pChunk_ != NULL)
	{
		pLight_->direction( pChunk_->transformInverse().applyVector( ndir ) );
	}
	else
	{
		pLight_->direction( ndir );
	}
}

float PyChunkSpotLight::innerRadius() const
{
	BW_GUARD;
	return pLight_->innerRadius();
}

void PyChunkSpotLight::innerRadius( float v )
{
	BW_GUARD;
	float orad = pLight_->outerRadius();
	pLight_->innerRadius( v );
	radiusUpdated( orad );
}

float PyChunkSpotLight::outerRadius() const
{
	BW_GUARD;
	return pLight_->outerRadius();
}

void PyChunkSpotLight::outerRadius( float v )
{
	BW_GUARD;
	float orad = pLight_->outerRadius();
	pLight_->outerRadius( v );
	radiusUpdated( orad );
}

void PyChunkSpotLight::radiusUpdated( float orad )
{
	BW_GUARD;
	pLight_->worldTransform( (pChunk_ != NULL) ?
		pChunk_->transform() : Matrix::identity );

	if ( !almostEqual( orad, pLight_->outerRadius() ) && pChunk_ != NULL )
	{
		ChunkLightCache::instance( *pChunk_ ).moveSpot( pLight_, 
			pLight_->position(), 
			orad );
	}
}

void PyChunkSpotLight::rev()
{
	BW_GUARD;
	if (pSource_)
	{
		Matrix m;
		pSource_->matrix( m );
		this->position( m.applyToOrigin() );
		this->direction( m.applyToUnitAxisVector(2) );
	}
}

void PyChunkSpotLight::revAll()
{
	BW_GUARD;
	for (PyChunkSpotLights::iterator it = s_currentLights_.begin();
		it != s_currentLights_.end();
		it++)
	{
		(*it)->rev();
	}
}


void PyChunkSpotLight::tick( float dTime )
{
	BW_GUARD;
	// update inner and outer radii
	if (pBounds_)
	{
		Vector4 val;
		pBounds_->output( val );
		if (val.x > val.y && val.x > 0.001 && val.y < 500.0)
		{
			float orad = pLight_->outerRadius();

			pLight_->innerRadius( val.x );
			pLight_->outerRadius( val.y );

			pLight_->worldTransform( (pChunk_ != NULL) ?
				pChunk_->transform() : Matrix::identity );

			if ( !almostEqual( orad, val.y ) && pChunk_ != NULL )
			{
				ChunkLightCache::instance( *pChunk_ ).moveSpot( pLight_, 
					pLight_->position(), 
					orad );
			}
		}
	}

	// update colour
	if (pShader_)
	{
		Vector4 val;
		pShader_->output( val );
		val *= 1.f/255.f;
		pLight_->colour( reinterpret_cast<Moo::Colour&>( val ) );
	}
}

void PyChunkSpotLight::nest( ChunkSpace * pSpace )
{
	BW_GUARD;
	Chunk * pDest = pSpace->findChunkFromPoint( this->position() );
	if (pDest != pChunk_)
	{
		if (pChunk_ != NULL)
			pChunk_->delDynamicItem( this, false );
		else
			pSpace->delHomelessItem( this );

		if (pDest != NULL)
			pDest->addDynamicItem( this );
		else
			pSpace->addHomelessItem( this );
	}
}

/// static initialiser
PyChunkSpotLight::PyChunkSpotLights PyChunkSpotLight::s_currentLights_;
