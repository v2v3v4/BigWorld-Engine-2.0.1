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

#include "avatar_drop_filter.hpp"


#include "entity.hpp"
#include "filter_utility_functions.hpp"
#include "romp/geometrics.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )

static SmartPointer<AvatarDropFilter> g_debugFilter;

/*~ class GroundNormalProvider
 *
 *	This class implements a Vector4Provider to the ground normal of
 *	an AvatarDropFilter. It does this by maintaining a weak reference to the
 *	filter instance.
 */
/**
 *	This class implements a Vector4Provider to the ground normal of
 *	an AvatarDropFilter. It does this by maintaining a weak reference to the
 *	filter instance.
 */
class GroundNormalProvider : public Vector4Provider
{
	Py_Header( GroundNormalProvider, Vector4Provider )

public:
	GroundNormalProvider( AvatarDropFilter & avatarDropFilter, PyTypePlus * pType = &s_type_ )
		: Vector4Provider( false, pType ),
		avatarDropFilter_( &avatarDropFilter )
	{
		BW_GUARD;
	}

	~GroundNormalProvider()
	{
		BW_GUARD;
		if (avatarDropFilter_)
		{
			avatarDropFilter_->onGroundNormalProviderDestroyed();
		}
	}

	PY_RO_ATTRIBUTE_DECLARE( avatarDropFilter_, avatarDropFilter );

	void onAvatarDropFilterDestroyed()
	{
		BW_GUARD;
		avatarDropFilter_ = NULL;
	}

	virtual void output( Vector4 & val )
	{
		BW_GUARD;
		if (avatarDropFilter_)
		{
			val.set( avatarDropFilter_->groundNormal().x, avatarDropFilter_->groundNormal().y, avatarDropFilter_->groundNormal().z, 0 );
		}
	}

	PyObject * pyGetAttribute( const char * attr )
	{
		BW_GUARD;
		PY_GETATTR_STD();

		return Vector4Provider::pyGetAttribute( attr );
	}

	int pySetAttribute( const char * attr, PyObject * value )
	{
		BW_GUARD;
		PY_SETATTR_STD();

		return Vector4Provider::pySetAttribute( attr, value );
	}

private:
	AvatarDropFilter * avatarDropFilter_;
};

PY_TYPEOBJECT( GroundNormalProvider )

PY_BEGIN_METHODS( GroundNormalProvider )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( GroundNormalProvider )
	/*~ attribute GroundNormalProvider.avatarDropFilter
	 *
	 *	This attribute returns the AvatarDropFilter weakly reference by the
	 *	provider or None if the filter has been destroyed.
	 *
	 *	@type AvatarDropFilter
	 */
	PY_ATTRIBUTE(avatarDropFilter)
PY_END_ATTRIBUTES()




PY_TYPEOBJECT( AvatarDropFilter )

PY_BEGIN_METHODS( AvatarDropFilter )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( AvatarDropFilter )
	/*~ attribute AvatarDropFilter.alignToGround
	 *
	 *	This attribute controls whether the entity should also be rotated to
	 *	match the slope of the ground on which they are standing.
	 *
	 *	@type bool
	 */
	PY_ATTRIBUTE( alignToGround )
	/*~ attribute AvatarDropFilter.groundNormal
	 *
	 *	The attribute returns a Vector4 provider to the normalised ground
	 *	normal of the drop filter in the form (x, y, z, 0).
	 *
	 *	@type GroundNormalProvider
	 */
	PY_ATTRIBUTE( groundNormal )
PY_END_ATTRIBUTES()


/*~ function BigWorld.AvatarDropFilter
 *
 *	This function creates a new AvatarDropFilter, which is used to move avatars
 *	around the same as an AvatarFilter does, but also to keep them on the
 *	ground beneath their specified positions.
 *
 *  @param	an optional AvatarFilter to initialise the filter with
 *
 *	@return a new AvatarDropFilter
 */
/**
 *	
 */
PY_FACTORY( AvatarDropFilter, BigWorld );



/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 */
AvatarDropFilter::AvatarDropFilter( PyTypePlus * pType ) :
	AvatarFilter( pType ),
	alignToGround_( false ),
	groundNormal_( 0, 1, 0 ),
	groundNormalProvider_( NULL )
{
	BW_GUARD;	
}


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 *	@param	filter	AvatarFilter to copy.
 */
AvatarDropFilter::AvatarDropFilter( const AvatarFilter & filter, PyTypePlus * pType ) :
	AvatarFilter( filter, pType ),
	alignToGround_( false ),
	groundNormal_( 0, 1, 0 ),
	groundNormalProvider_( NULL )
{
	BW_GUARD;

	if (AvatarDropFilter::Check( const_cast<AvatarFilter*>( &filter ) ))
	{
		const AvatarDropFilter * avatarDropFilter = static_cast<const AvatarDropFilter*>( &filter );

		alignToGround_ = avatarDropFilter->alignToGround_;
		groundNormal_ = avatarDropFilter->groundNormal_;
		groundNormalProvider_ = avatarDropFilter->groundNormalProvider_;
	}
}



/**
 *	Destructor
 */
AvatarDropFilter::~AvatarDropFilter()
{
	BW_GUARD;

	if (groundNormalProvider_)
	{
		groundNormalProvider_->onAvatarDropFilterDestroyed();
	}
}


/**
 *	This method takes the input position and does a collision check to drop it
 *	onto the collision scene. Other than this dropping of the position the
 *	parameters are passed unchanged into the AvatarFilter.
 *
 *	@param	time		The estimated client time when the input was sent from
 *						the server.
 *	@param	spaceID		The server space that the position resides in.
 *	@param	vehicleID	The ID of the vehicle in who's coordinate system the
 *						position is defined. A null vehicle ID means that the
 *						position is in world coordinates. 
 *	@param	pos			The new position in either local vehicle or 
 *						world space/common coordinates. The player relative
 *						compression will have already been decoded at this
 *						point by the network layer. 
 *	@param	posError	The amount of uncertainty in the position.
 *	@param	auxFiltered	If not NULL, a pointer to an array of two floats
 *						representing yaw and pitch.
 */
void AvatarDropFilter::input(	double time,
								SpaceID spaceID,
								EntityID vehicleID,
								const Position3D & pos,
								const Vector3 & posError,
								float * auxFiltered )
{
	BW_GUARD;
	AvatarFilter::input( time, spaceID, vehicleID, pos, posError, auxFiltered );
}


/**
 *	This method just calls AvatarFilter::output
 *	
 *	Post avatar filter dropping is currently disabled in favour of doing it in
 *	the input method for performance reasons.
 *
 *	@param	time	The client game time in seconds that the entity's volatile
 *					members should be updated for.
 */
void AvatarDropFilter::output( double time )
{
	BW_GUARD;


	this->AvatarFilter::output(time);

	static DogWatch dwAvatarDropFilter("AvatarDropFilter");
	dwAvatarDropFilter.start();

	Position3D position = entity_->position();


	float direction[3] = {	entity_->auxVolatile()[0],
							entity_->auxVolatile()[1],
							entity_->auxVolatile()[2] };

	Position3D filteredPosition(position);
	groundNormal_.set( 0, 1, 0 );

	FilterUtilityFunctions::filterDropPoint( &*entity_->pSpace(), position, filteredPosition, 100.0f, &groundNormal_ );
	groundNormal_.normalise();

	if (alignToGround_)
	{
		Matrix rotation;
		rotation.setIdentity();
		rotation.setRotateY( direction[0] );

		// Note: As the normal and up vectors converge the rotation of the
		// quaternion becomes zero making this cross product safe.
		Vector3 slopeAxes( Vector3(0,1,0).crossProduct( groundNormal_ ) );

		Quaternion slopeRotation(slopeAxes, 1 + Vector3(0,1,0).dotProduct( groundNormal_ ));
		slopeRotation.normalise();

		Matrix slopeRotationMatrix;
		slopeRotationMatrix.setIdentity();
		slopeRotationMatrix.setRotate( slopeRotation );
		rotation.postMultiply( slopeRotationMatrix );

		direction[1] = rotation.pitch();
		direction[2] = rotation.roll();
	}


	entity_->pos(filteredPosition, direction, 3, entity_->velocity());

	dwAvatarDropFilter.stop();
}


/**
 *	The normal of the world polygon intersected during the last output drop
 *	collide, otherwise (0,1,0).
 */
const Vector3 & AvatarDropFilter::groundNormal() const
{
	BW_GUARD;
	return groundNormal_;
}

/**
 *	This function returns the GroundNormalProvider instance and creates one if
 *	needed. The GroundNormalProvider object maintains a week reference
 *	relationship with its originating AvatarDropFilter.
 */
PyObjectPtr AvatarDropFilter::groundNormalProvider()
{
	BW_GUARD;
	if (!groundNormalProvider_)
	{
		groundNormalProvider_ = new GroundNormalProvider( *this );
	}

	return groundNormalProvider_;
}

/**
 *	Internal function for dismantling the weak reference with the filter’s
 *	GroundNormalProvider.
 */
void AvatarDropFilter::onGroundNormalProviderDestroyed()
{
	BW_GUARD;
	MF_ASSERT( groundNormalProvider_ != NULL );

	groundNormalProvider_ = NULL;
}


/**
 *	Standard get attribute method.
 */
PyObject * AvatarDropFilter::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return AvatarFilter::pyGetAttribute( attr );
}

/**
 *	Standard set attribute method.
 */
int AvatarDropFilter::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return AvatarFilter::pySetAttribute( attr, value );
}

/**
 *	Python factory method
 */
PyObject * AvatarDropFilter::pyNew( PyObject * args )
{
	BW_GUARD;
	int argc = PyTuple_Size( args );
	PyObject * pFilter = NULL;

	if (!PyArg_ParseTuple( args, "|O", &pFilter ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.AvatarDropFilter() "
			"expects an optional AvatarFilter as argument" );
		return NULL;
	}
	
	if (pFilter != NULL)
	{
		if (!AvatarFilter::Check( pFilter ))
		{
			PyErr_SetString( PyExc_TypeError, "BigWorld.AvatarDropFilter() "
				"expects an optional AvatarFilter as argument" );
			return NULL;
		}
		else
		{
			return new AvatarDropFilter( *static_cast<AvatarFilter*>( pFilter ) );
		}
	}

	return new AvatarDropFilter();
}


/*
 * This function draw a plane given a center point on the plane
 * and two different vectors on the plane
 */
void drawPlane(const Vector3 & center, const Vector3 & normal, float size, Moo::Colour & colour, bool drawAlways)
{
	Vector3 v1 = Vector3(0,1,0).crossProduct( normal );
	v1.normalise();
	
	if ( v1 == Vector3(0,0,0) )
		v1 = Vector3(1,0,0);

	Vector3 v2 = normal.crossProduct( v1 );

	v1 = v1 * size;
	v2 = v2 * size;

	Geometrics::drawLine(center + v1 - v2, 
						 center + v1 + v2, colour, drawAlways);
	Geometrics::drawLine(center - v1 - v2, 
						 center - v1 + v2, colour, drawAlways);
	Geometrics::drawLine(center - v1 - v2, 
						 center + v1 - v2, colour, drawAlways);
	Geometrics::drawLine(center - v1 + v2, 
						 center + v1 + v2, colour, drawAlways);
	Geometrics::drawLine(center - v1 - v2, 
						 center + v1 + v2, colour, drawAlways);
	Geometrics::drawLine(center - v1 + v2, 
						 center + v1 - v2, colour, drawAlways);
}


/*
 * This function draws the models plane on the terrain
 */
void AvatarDropFilter::drawDebugStuff()
{
	if(g_debugFilter != NULL)
	{

		if (g_debugFilter->entity() == NULL)
		{
			g_debugFilter = NULL;
		}
		else
		{
			Moo::Colour blue(0, 0, 1, 0);
			
			Vector3 position = g_debugFilter->entity()->position();
			Vector3 groundNormal = g_debugFilter->groundNormal();

			drawPlane(position, groundNormal, 1.0f, blue, true);
		}
	}
}


/*
 * BigWorld.debugDropFilter
 * This function accepts an AvatarDropFilter to debug visually
 */
static void debugDropFilter(PyObjectPtr pObject)
{
	// clear last camera state
	if (g_debugFilter != NULL)
	{
		g_debugFilter = NULL;
	}

	if (pObject != NULL)
	{
		if ( !AvatarDropFilter::Check( pObject.get() ) )
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.debugDropFilter: "
				"debug drawing expects an AvatarDropFilter\n" );
			return ;
		}

		g_debugFilter = static_cast<AvatarDropFilter *>( pObject.get() );
	}
}

PY_AUTO_MODULE_FUNCTION(
	RETVOID, debugDropFilter, OPTARG(PyObjectPtr, NULL, END), BigWorld )

// avatar_drop_filter.cpp
