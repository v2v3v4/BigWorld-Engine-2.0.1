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

#include "matrix_providers.hpp"
#include "entity.hpp"


extern double getServerTime();

DECLARE_DEBUG_COMPONENT2( "Model", 0 );

// -----------------------------------------------------------------------------
// Section: EntityDirProvider
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( EntityDirProvider)

PY_BEGIN_METHODS( EntityDirProvider )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( EntityDirProvider )	

	/*~ attribute EntityDirProvider source
	 *  The entity whose facing direction this provides.
	 *  @type Read-Write Entity.
	 */
	PY_ATTRIBUTE ( source )

	/*~ attribute EntityDirProvider pitchIndex
	 *  The index of the entity's axis which is to be interpreted 
	 *  as pitch (0:yaw, 1:pitch, 2:roll).
	 *  @type Read-Write Integer.
	 */
	PY_ATTRIBUTE ( pitchIndex )

	/*~ attribute EntityDirProvider yawIndex
	 *  The index of the entity's axis which is to be interpreted 
	 *  as yaw (0:yaw, 1:pitch, 2:roll).
	 *  @type Read-Write Integer.
	 */
	PY_ATTRIBUTE ( yawIndex )

PY_END_ATTRIBUTES()

/*~ function BigWorld EntityDirProvider
 *  Create a new EntityDirProvider. This is a MatrixProvider with
 *  the direction of a specified entity.
 *  
 *  Code Example:
 *  @{
 *  # return a new EntityDirProvider which provides the direction
 *  # faced by the entity ent.
 *  BigWorld.EntityDirProvider( ent )
 *  @}
 *  @param entity The entity whose facing direction this provides.
 *  @param pitchIndex An integer which describes which of the entity's axis
 *  are to be interpreted as pitch (0:yaw, 1:pitch, 2:roll).
 *  @param yawIndex An integer which describes which of the entity's axis
 *  are to be interpreted as yaw (0:yaw, 1:pitch, 2:roll).
 *  @return The new EntityDirProvider.
 */
PY_FACTORY( EntityDirProvider, BigWorld )

/**
 *	This method creates a direction provider to return the direction between the
 *  two entities
 */
PyObject * EntityDirProvider::pyNew( PyObject *args )
{
	BW_GUARD;
	// Get our Entity to be targeted.
	PyObject *pSource;
	int pitchIndex;
	int yawIndex;
	if ( !PyArg_ParseTuple( args, "Oii", &pSource, &pitchIndex, &yawIndex ) ||
		!Entity::Check( pSource ))
	{
		PyErr_SetString( PyExc_TypeError,
			"EntityDirProvider: Requires an entity, pitch index and yaw index" );
		return NULL;
	}

	return new EntityDirProvider(
		static_cast<Entity*>(pSource), pitchIndex, yawIndex );
}

EntityDirProvider::EntityDirProvider( Entity* pSource, 
									  int pitchIndex,
									  int yawIndex,
									  PyTypePlus * pType)
		: MatrixProvider( false, pType ),
	pitchIndex_( pitchIndex ),
	yawIndex_( yawIndex ),
	pSource_( NULL )
{
	BW_GUARD;
	this->pySet_source(pSource);
}

EntityDirProvider::~EntityDirProvider()
{}

void EntityDirProvider::matrix( Matrix & m ) const
{
	BW_GUARD;
	float yaw, pitch;
	this->getYawPitch(yaw, pitch);
	m.setRotate(yaw, pitch, 0.0f );
}

void EntityDirProvider::getYawPitch( float& yaw, float& pitch) const
{
	BW_GUARD;
	// If we are tracking another Entity, then grab its position
	// and subtract ours from that to get a direction. Convert the
	// direction into pitch and yaw values.
	if (pSource_.exists() && 
		PyWeakref_CheckProxy( pSource_.getObject() ))
	{
		Entity* source = static_cast<Entity*>((PyObject*) PyWeakref_GET_OBJECT( pSource_.getObject() ));
		pitch = source->auxVolatile()[pitchIndex_];
		yaw   = source->auxVolatile()[yawIndex_];		
	}
}

/** 
 * This method returns the source Entity to a python script
 */
PyObject * EntityDirProvider::pyGet_source()
{
	BW_GUARD;
	Entity * source = NULL;
	if (pSource_.exists() && 
		PyWeakref_CheckProxy( pSource_.getObject() ))
	{
		source = static_cast<Entity*>((PyObject*) PyWeakref_GET_OBJECT( pSource_.getObject() ));
	}
	return Script::getData( source );
}


/** 
 * This method allows a python script to set the Source Entity
 */
int EntityDirProvider::pySet_source( PyObject * value )
{
	BW_GUARD;
	// first make sure we're setting it to a model
	if (value != NULL && Entity::Check(value))
	{
		pSource_ = PyWeakref_NewProxy(static_cast<PyObject*>(value), NULL);
	}
	else if (value == Py_None)
	{		
		pSource_ = NULL;
	}
	else 
	{
		PyErr_SetString( PyExc_TypeError,
			"EntityDirProvider:: Source must be set to an Entity or None" );
		return -1;
	}

	return 0;
}


/**
 *	This allows scripts to get various properties of a EntityDirProvider.
 */
PyObject * EntityDirProvider::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This allows scripts to set various properties of a EntityDirProvider
 */
int EntityDirProvider::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

// -----------------------------------------------------------------------------
// Section: DiffDirProvider
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( DiffDirProvider )

PY_BEGIN_METHODS( DiffDirProvider )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( DiffDirProvider )	

	/*~ attribute DiffDirProvider source
	 *  Gives the position from which the matrix this provides points.
	 *  @type Read-Write MatrixProvider.
	 */
	PY_ATTRIBUTE ( source )

	/*~ attribute DiffDirProvider target
	 *  Gives the position towards which the matrix this provides points.
	 *  @type Read-Write MatrixProvider.
	 */
	PY_ATTRIBUTE ( target )

PY_END_ATTRIBUTES()

/*~ function BigWorld DiffDirProvider
 *  Creates a DiffDirProvider. This provides a matrix representing the
 *  direction between the points specified by two matrices.
 *  
 *  Code Example:
 *  @{
 *  # returns a new DiffDirProvider which provides a matrix which points from
 *  # matrix e1 to matrix e2
 *  BigWorld.DiffDirProvider( e1.matrix, e2.matrix )
 *  @}  
 *  @param source The MatrixProvider that gives the position from which the matrix this provides points.
 *  @param target The MatrixProvider that gives the position towards which the matrix this provides points.
 *  @return The new DiffDirProvider
 */
PY_FACTORY( DiffDirProvider, BigWorld )

/**
 *	This method creates a direction provider to return the direction between the
 *  two entities
 */
PyObject * DiffDirProvider::New( MatrixProviderPtr pSource,
	MatrixProviderPtr pTarget )
{
	BW_GUARD;
	return new DiffDirProvider( pSource, pTarget );
}

DiffDirProvider::DiffDirProvider( MatrixProviderPtr pSource, 
								  MatrixProviderPtr pTarget, 
								  PyTypePlus * pType)
		: MatrixProvider( false, pType ),
	pTarget_( pTarget ),
	pSource_( pSource )
{	
}

DiffDirProvider::~DiffDirProvider()
{
}

void DiffDirProvider::matrix( Matrix & m ) const
{
	Matrix msource, mtarget;
	pSource_->matrix( msource );
	pTarget_->matrix( mtarget );

	const Vector3 & spos = msource.applyToOrigin();
	const Vector3 & tpos = mtarget.applyToOrigin();
	m.lookAt( spos, tpos - spos, Vector3( 0.f, 1.f, 0.f ) );
	m.invert();
}


/**
 *	This allows scripts to get various properties of a DiffDirProvider.
 */
PyObject * DiffDirProvider::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This allows scripts to set various properties of a DiffDirProvider
 */
int DiffDirProvider::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


// -----------------------------------------------------------------------------
// Section: ScanDirProvider
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( ScanDirProvider )

PY_BEGIN_METHODS( ScanDirProvider )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ScanDirProvider )
PY_END_ATTRIBUTES()

/*~ function BigWorld ScanDirProvider
 *  Creates a new ScanDirProvider. This is a MatrixProvider whose direction 
 *  oscillates around the y axis, and may be instructed to call a function 
 *  when it hits it's leftmost and rightmost limits.
 *
 *  Code Example:
 *  @{
 *  # return a new ScanDirProvider which swings left and right by pi/2 radians
 *  # every 10 seconds, offset by 0 percent of its period from time 0. It calls
 *  # limitFunct( 4 ) when it hits it's leftmost limit, and limitFunct( 5 ) 
 *  # when it hits it's rightmost limit.
 *  BigWorld.ScanDirProvider( math.pi/2, 10, 0, limitFunct )
 *  @}
 *  @param amplitude A float representing the angle about 0 to which this should
 *  swing around the y axis, in radians.
 *  @param period A float giving the time in seconds for a full sweep left & 
 *  right.
 *  @param offset An optional float giving the offset of the sweep cycle from 
 *  time 0, as a proportion of the period.
 *  @param callback An optional function with a single argument. This function 
 *  is called with value 4 when this hits it's leftmost limit, and 5 when it 
 *  hits it's rightmost limit.
 *  @return The new ScanDirProvider
 */
PY_FACTORY( ScanDirProvider, BigWorld )

/**
 *	This method creates a direction provider to return the direction between the
 *  two entities
 */
PyObject * ScanDirProvider::pyNew( PyObject *args )
{
	BW_GUARD;
	float amplitude;
	float period;
	float offset = 0.f;
	PyObject * pCallback = NULL;

	if ( !PyArg_ParseTuple( args, "ff|fO", &amplitude, &period, &offset, &pCallback ) ||
		( pCallback != NULL && !PyCallable_Check( pCallback ) ))
	{
		PyErr_SetString( PyExc_TypeError,
			"Amplitude, period and optional offset and hit limit callback expected.\n" );
		return NULL;
	}

	return new ScanDirProvider(amplitude, period, offset, pCallback);
}

ScanDirProvider::ScanDirProvider( float amplitude, 
								  float period, 
								  float offset,
								  PyObject *pHitLimitCallBack,			 
								  PyTypePlus * pType)
	:MatrixProvider( false, pType )
{	
	BW_GUARD;
	amplitude_ = amplitude;
	period_ = period;
	offset_ = offset;
	pHitLimitCallBack_ = pHitLimitCallBack;
	oldPosInCycle_ = -1.f;
}

void ScanDirProvider::matrix( Matrix & m ) const
{
	BW_GUARD;
	float yaw, pitch;
	this->getYawPitch(yaw, pitch);
	m.setRotate(yaw, pitch, 0.0f );
}

void ScanDirProvider::getYawPitch( float& yaw, float& pitch ) const
{
	BW_GUARD;
	float newPitch = 0.f;
	float newYaw   = 0.f;

	double timeNow = getServerTime();

	float posInCycle = float( fmod( timeNow / period_ + offset_, 1.0 ) );
	newYaw = 2.f * MATH_PI * posInCycle;
	newYaw = amplitude_ * sinf( newYaw );

	if (oldPosInCycle_ >= 0.f && pHitLimitCallBack_)
	{
		bool goingLeft		= (0.25f < posInCycle)		&& (posInCycle < 0.75f);
		bool wasGoingLeft	= (0.25f < oldPosInCycle_)	&& (oldPosInCycle_ < 0.75f);

		if (goingLeft != wasGoingLeft)
		{
			Py_INCREF( pHitLimitCallBack_.getObject() );
			Script::call( &*pHitLimitCallBack_,
				Py_BuildValue( "(i)", goingLeft ? 4 : 5 ),
				"ScanDirProvider hit limit: " );
		}
	}

	oldPosInCycle_ = posInCycle;

	yaw = newYaw;
	pitch = newPitch;
}

/**
 *	This allows scripts to get various properties of a ScanDirProvider.
 */
PyObject * ScanDirProvider::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This allows scripts to set various properties of a ScanDirProvider
 */
int ScanDirProvider::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

// matrix_providers.cpp
