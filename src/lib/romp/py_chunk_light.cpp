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
#include "py_chunk_light.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"

// -----------------------------------------------------------------------------
// Section: PyChunkLight
// -----------------------------------------------------------------------------

PROFILER_DECLARE( PyChunkLight_tick, "PyChunkLight Tick" );
PROFILER_DECLARE( PyChunkLight_dtor, "PyChunkLight dtor" );

PY_TYPEOBJECT( PyChunkLight )

/*~ function PyChunkLight recalc
 *  Recalculates the effects of the light. The initial setup of a PyChunkLight
 *  does not require this to be called.
 *  @return None.
 */
PY_BEGIN_METHODS( PyChunkLight )
	PY_METHOD( recalc )
PY_END_METHODS()

/*~ attribute PyChunkLight visible
 *  Dictates whether this light illuminates things.
 *  @type Read-Write boolean
 */
/*~ attribute PyChunkLight position
 *  The centre of the light source. This is overwritten with the output of the
 *  source attribute if it has been specified.
 *  @type Read-Write Vector3
 */
/*~ attribute PyChunkLight colour
 *  The colour of the light source in RGBA form. This is overwritten with the 
 *  output of the shader attribute if it has been specified.
 *  @type Read-Write Vector4
 */
/*~ attribute PyChunkLight innerRadius
 *  The radius around the centre of the light within which the strength of the
 *  light does not drop off. This is overwritten with the first component of
 *  the output of the bounds attribute if it has been specified.
 *  @type Read-Write Float
 */
/*~ attribute PyChunkLight outerRadius
 *  If greater than innerRadius, this is the radius around the centre of the
 *  light at which the strength of the light becomes zero. If this is less than
 *  or equal to innerRadius, then there is no area around the light in which
 *  its effect dissipates. This is overwritten with the second component of the
 *  output of the bounds attribute if it has been specified.
 *  @type Read-Write Float
 */
/*~ attribute PyChunkLight specular
 *  If true, this provides a light source for specular shaders. This defaults
 *  to false.
 *  @type Read-Write Boolean
 */
/*~ attribute PyChunkLight diffuse
 *  If true, this provides a light source for diffuse shaders. This defaults
 *  to true.
 *  @type Read-Write Boolean
 */
/*~ attribute PyChunkLight source
 *  When specified, this overwrites the position attribute with it's
 *  translation.
 *  @type Read-Write MatrixProvider
 */
/*~ attribute PyChunkLight bounds
 *  When specified, this overwrites the innerRadius and outerRadius attributes
 *  with the first and second components of the Vector4 it provides,
 *  respectively.
 *  @type Read-Write Vector4Provider
 */
/*~ attribute PyChunkLight shader
 *  When specified, this overwrites the colour attribute with the Vector4 it
 *  provides.
 *  @type Read-Write Vector4Provider
 */
/*~ attribute PyChunkLight priority
 *	A priority bias used when sorting lights to determine the most
 *	relevant set of lights for a particular object. This is useful
 *	to ensure important lights will always be considered before other
 *	lights (e.g. a player's torch).
 *  @type Read-Write Integer
 */
PY_BEGIN_ATTRIBUTES( PyChunkLight )
	PY_ATTRIBUTE( visible )
	PY_ATTRIBUTE( position )
	PY_ATTRIBUTE( colour )
	PY_ATTRIBUTE( priority )
	PY_ATTRIBUTE( innerRadius )
	PY_ATTRIBUTE( outerRadius )
	PY_ATTRIBUTE( specular )
	PY_ATTRIBUTE( diffuse )
	PY_ATTRIBUTE( source )
	PY_ATTRIBUTE( bounds )
	PY_ATTRIBUTE( shader )
PY_END_ATTRIBUTES()

/*~ function BigWorld PyChunkLight
 *  Creates a new PyChunkLight. This is a script controlled chunk omni light 
 *  which can be used as a diffuse and/or specular light source for models and
 *  shells.
 */
PY_FACTORY( PyChunkLight, BigWorld )


/**
 *	Constructor.
 */
PyChunkLight::PyChunkLight( PyTypePlus * pType ) :
	PyObjectPlusWithVD( pType ),
	ChunkOmniLight( (WantFlags)WANTS_TICK ),
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
PyChunkLight::~PyChunkLight()
{
	BW_GUARD_PROFILER( PyChunkLight_dtor );
	if (this->visible()) this->visible( false );
	PyChunkLights::iterator found = std::find(
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
PyObject * PyChunkLight::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	return this->PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Python set attribute method
 */
int PyChunkLight::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();
	return this->PyObjectPlus::pySetAttribute( attr, value );
}



bool PyChunkLight::visible() const
{
	return visible_;
}

void PyChunkLight::visible( bool v )
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


const Vector3 & PyChunkLight::position() const
{
	BW_GUARD;
	return pLight_->worldPosition();
}

void PyChunkLight::position( const Vector3 & npos )
{
	BW_GUARD;

	Chunk* oldChunk = pChunk_;

	Vector3 opos = pLight_->worldPosition();

	// find which chunk light was in, and move it in that chunk
	if (pChunk_ != NULL)
	{
		pChunk_->modDynamicItem( this, opos, npos );

		// Check again as modDynamicItem can put us outside the known chunks
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
				ChunkLightCache::instance( *pChunk_ ).moveOmni( pLight_, locOldPos, pLight_->outerRadius() );
			}
		}
	}
	else if (visible_)
	{
		ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
		if (pSpace.exists())
		{
			Chunk * pDest = pSpace->findChunkFromPoint( npos );
			if (pDest != NULL)
			{
				//since the newly created one's position_==worldPosition_, 
				//so we convert the position_ to be local in chunk space first
				pLight_->position( pDest->transformInverse().applyPoint( npos ) );

				pDest->addDynamicItem( this );//ChunkLight::toss will be called here 
				//which will call it's pLight_->worldTransform(..), so we don't need call 
				//pLight_->worldTransform(..) again
			}
		}
	}

	if (pChunk_ == NULL)
	{
		pLight_->position( npos );
		pLight_->worldTransform( Matrix::identity );
	}
}

float PyChunkLight::innerRadius() const
{
	BW_GUARD;
	return pLight_->innerRadius();
}

void PyChunkLight::innerRadius( float v )
{
	BW_GUARD;
	float orad = pLight_->outerRadius();
	pLight_->innerRadius( v );
	radiusUpdated( orad );
}

float PyChunkLight::outerRadius() const
{
	BW_GUARD;
	return pLight_->outerRadius();
}

void PyChunkLight::outerRadius( float v )
{
	BW_GUARD;
	float orad = pLight_->outerRadius();
	pLight_->outerRadius( v );
	radiusUpdated( orad );
}

void PyChunkLight::radiusUpdated( float orad )
{
	BW_GUARD;
	pLight_->worldTransform( (pChunk_ != NULL) ?
		pChunk_->transform() : Matrix::identity );

	if ( !almostEqual( orad, pLight_->outerRadius() ) && pChunk_ != NULL )
	{
		ChunkLightCache::instance( *pChunk_ ).moveOmni( pLight_, 
			pLight_->position(), 
			orad );
	}
}

void PyChunkLight::rev()
{
	BW_GUARD;
	if (pSource_)
	{
		Matrix m;
		pSource_->matrix( m );
		this->position( m.applyToOrigin() );
	}
}

void PyChunkLight::revAll()
{
	BW_GUARD;
	for (PyChunkLights::iterator it = s_currentLights_.begin();
		it != s_currentLights_.end();
		it++)
	{
		(*it)->rev();
	}
}

void PyChunkLight::tick( float dTime )
{
	BW_GUARD_PROFILER( PyChunkLight_tick );

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
				ChunkLightCache::instance( *pChunk_ ).moveOmni( pLight_, 
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

void PyChunkLight::nest( ChunkSpace * pSpace )
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
PyChunkLight::PyChunkLights PyChunkLight::s_currentLights_;



// -----------------------------------------------------------------------------
// Section: Script converters
// -----------------------------------------------------------------------------


namespace Script
{
	int setData( PyObject * pObject, Moo::Colour & colour,
		const char * varName )
	{
		int ret = Script::setData( pObject, reinterpret_cast<Vector4&>( colour ), varName );
		if (ret == 0) reinterpret_cast<Vector4&>( colour ) *= 1.f/255.f;
		return ret;
	}

	PyObject * getData( const Moo::Colour & colour )
	{
		return Script::getData( reinterpret_cast<const Vector4&>( colour ) * 255.f );
	}
}

// py_chunk_light.cpp
