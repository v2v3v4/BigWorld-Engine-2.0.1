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

#include "boids_filter.hpp"

#include "entity.hpp"
#include "terrain/base_terrain_block.hpp"


DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


PY_TYPEOBJECT( BoidsFilter )

PY_BEGIN_METHODS( BoidsFilter )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( BoidsFilter )

	/*~ attribute BoidsFilter.influenceRadius
	 *
	 *	The influence radius simply determines how close
	 *	a boid must be to another to affect it at all.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( influenceRadius )

	/*~ attribute BoidsFilter.collisionFraction
	 *
	 *	The collision fraction defines the proportion of the influence radius
	 *	within which neighbours will be considered colliding.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( collisionFraction )

	/*~ attribute BoidsFilter.approachRadius
	 *
	 *	When the boids are in the process of landing, and
	 *	a boid is inside the approach radius, then it will
	 *	point directly at its goal, instead of smoothly turning
	 *	towards it.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( approachRadius )

	/*~ attribute BoidsFilter.stopRadius
	 *
	 *	When the boids are in the process of landing, and
	 *	a boid is inside the stop radius, it will be positioned
	 *	exactly at the goal, and its speed will be set to 0.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( stopRadius )

	/*~ attribute BoidsFilter.speed
	 *
	 *	The normal speed of the boids.  While boids will speed
	 *	up and slow down during flocking, their speed will always
	 *	be damped toward this speed.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( speed )

	/*~ attribute BoidsFilter.speed
	 *
	 * Controls whether the boids are flying or landing. By default they are in
	 * the flying state which is specified with 0. The landing state will cause
	 * the boids to land and is specified with 1. When a boid lands the method
	 * boidsLanded is called.
	 *
	 *	@type	float
	 */
	PY_ATTRIBUTE( state )

PY_END_ATTRIBUTES()

/*~ function BigWorld.BoidsFilter
 *
 *	This function creates a new BoidsFilter.  This is used to filter the
 *	movement of an Entity which consists of several models (boids) for which
 *	flocking behaviour is desired.
 *
 *	@return a new BoidsFilter
 */
PY_FACTORY( BoidsFilter, BigWorld )




///////////////////////////////////////////////////////////////////////////////
///  Member Documentation for BoidsFilter
///////////////////////////////////////////////////////////////////////////////


/**
 *	@property	float BoidsFilter::INFLUENCE_RADIUS
 *
 *	This member is used to define the distance at which boids begin receiving
 *	influence from other boids.
 */


/**
 *	@property	float BoidsFilter::COLLISION_FRACTION
 *
 *	The collision fraction defines the proportion of the influence radius
 *	within which neighbours will be considered colliding.
 */


/**
 *	@property	float BoidsFilter::NORMAL_SPEED
 *
 *	The normal speed of the boids.  While boids will speed up and slow down
 *	during flocking, their speed will always be damped toward this speed.
 *
 *	@note	Python name is 'speed'
 */


/**
 *	@property	float BoidsFilter::state_
 *
 * Controls whether the boids are flying or landing. By default they are in
 * the flying state which is specified with 0. The landing state will cause
 * the boids to land and is specified with 1. When a boid lands the method
 * boidsLanded is called.
 *
 *	@note	Python name is 'state_'
 */


 /**
 *	@property	float BoidsFilter::ANGLE_TWEAK
 *
 *	This member determines the speed at which the boids adjust their pitch.\n
 *	Units: radians per 30th of a second.

 *
 *	@see	BoidsFilter::BoidData::updateModel()
 */


/**
 *	@property	float BoidsFilter::PITCH_TO_SPEED_RATIO
 *
 *	This member defines how much the boids will slowdown or speedup based on
 *	their rate of climb or decent.
 *
 *	Units: metres per second per radian
 */


/**
 *	@property	float BoidsFilter::GOAL_APPROACH_RADIUS
 *
 *	This member holds the distance within which landing boids will snap to
 *	face their target.
 */


/**
 *	@property	float BoidsFilter::GOAL_STOP_RADIUS
 *
 *	This member holds the distance within which boids are considered to have
 *	reached their target and will stop.
 */


/**
 *	@property	Boids BoidsFilter::boidData_
 *
 *	This member holds the data for all the boids in the flock. It is sized to
 *	mach the number of auxiliary embodiments possessed by its owner entity.
 */


/**
 *	@property	double BoidsFilter::prevTime_
 *
 *	This member holds the last time output was produced so that the correct
 *	delta time can be calculated for the ticking of the boids. 
 */


///////////////////////////////////////////////////////////////////////////////
///  End Member Documentation for BoidsFilter
///////////////////////////////////////////////////////////////////////////////


// These constants are not used in favour of member and local constants of
// the same name. However they have been left in the code for now in case
// their values are still useful. 
//
//const float INFLUENCE_RADIUS				= 10.0f;
//const float INFLUENCE_RADIUS_SQUARED		= INFLUENCE_RADIUS * INFLUENCE_RADIUS;
//const float COLLISION_FRACTION				= 0.5f;
//const float INV_COLLISION_FRACTION			= 1.0f/(1.0f-COLLISION_FRACTION);
//const float NORMAL_SPEED					= 0.5f;
//const float ANGLE_TWEAK						= 0.02f;
//const float PITCH_TO_SPEED_RATIO			= 0.002f;
//const float GOAL_APPROACH_RADIUS_SQUARED	= (10.0f * 10.0f);
//const float GOAL_STOP_RADIUS_SQUARED		= (1.0f * 1.0f);


/**
 *	This is the state value of the flock when it is landed.
 */
const int	STATE_LANDING					= 1;



const float MAX_STEP						= 0.1f;


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 */
BoidsFilter::BoidsFilter( PyTypePlus * pType ) :
	AvatarFilter( pType ),
	prevTime_( -1.f ),
	INFLUENCE_RADIUS( 10.f ),
	COLLISION_FRACTION( 0.5f ),
	NORMAL_SPEED( 0.5f ),
	ANGLE_TWEAK( 0.02f ),
	PITCH_TO_SPEED_RATIO( 0.002f ),
	GOAL_APPROACH_RADIUS( 10.f ),
	GOAL_STOP_RADIUS( 1.f ),
	initialHeight_( -20.f )
{
	BW_GUARD;	
}


/**
 *	Constructor
 *
 *	@param	pType	The python object defining the type of the filter. 
 *  @param	filter	AvatarFilter to copy.
 */
BoidsFilter::BoidsFilter( const AvatarFilter & filter, PyTypePlus * pType ) :
	AvatarFilter( filter, pType ),
	prevTime_( -1.f ),
	INFLUENCE_RADIUS( 10.f ),
	COLLISION_FRACTION( 0.5f ),
	NORMAL_SPEED( 0.5f ),
	state_( 0 ),
	ANGLE_TWEAK( 0.02f ),
	PITCH_TO_SPEED_RATIO( 0.002f ),
	GOAL_APPROACH_RADIUS( 10.f ),
	GOAL_STOP_RADIUS( 1.f ),
	initialHeight_( -20.f )
{
	BW_GUARD;	
}


/**
 *	Destructor
 */
BoidsFilter::~BoidsFilter()
{
	BW_GUARD;	
}


/*~ callback Entity.boidsLanded
 *
 *	This callback method is called on entities using the boids filter.
 *	It lets the entity know when the boids have landed. A typical response
 *	to this notifier is to delete the boids models.
 *
 *	@see BigWorld.BoidsFilter
 */
/**
 *	This method updates its owner entities position as well as the auxiliary
 *	embodiment of the entity in such a was as to produce flocking behaviour of
 *	those embodiments. The individual 'boids' are each encapsulated in a
 *	BoidData object which are each updated during this output.\n\n
 *	This method also initiates a python callback on the entity when the
 *	'landed' state changes.\n
 *	Entity.boidsLanded()
 *
 *	@param	time	The client game time in seconds that the entity's volatile
 *					members should be updated for.
 *
 *	@see	BoidData::updateModel()
 */
void BoidsFilter::output( double time )
{
	BW_GUARD;
	static DogWatch dwBoidsFilterOutput("BoidsFilter");
	dwBoidsFilterOutput.start();

	AvatarFilter::output( time );

	Vector3 goal(	entity_->position().x,
					entity_->position().y,
					entity_->position().z );

	if (prevTime_ == -1.f )
	{
		initialHeight_ = goal.y;
	}

	if ( state_ != STATE_LANDING )
	{
		goal.y = Terrain::BaseTerrainBlock::getHeight( goal.x, goal.z ) + 20.f;

		if (goal.y < initialHeight_)
			goal.y = initialHeight_;
	}

	float dTime = 1.f;

	if ( prevTime_ != -1.f )
	{
		dTime = float(time - prevTime_);
	}
	prevTime_ = time;

	ChunkEmbodiments & models = entity_->auxiliaryEmbodiments();
	int size = models.size();

	int oldSize = boidData_.size();

	if (oldSize != size)
	{
		boidData_.resize( size );

		for (int i = oldSize; i < size; i++)
		{
			boidData_[i].pos_ += goal;
		}
	}

	bool boidsLanded = false;

	if ( dTime < 20.f )
	{
		while ( dTime > 0.f )
		{
			float thisStep = min( dTime, MAX_STEP );
			dTime -= MAX_STEP;

			for ( int i = 0; i < size; i++ )
			{
				ChunkEmbodimentPtr pCA = models[i];

				boidData_[i].updateModel( goal, &*pCA, *this, thisStep, state_ );

				if ( state_ == STATE_LANDING && boidData_[i].pos_ == goal )
					boidsLanded = true;
			}
		}
	}

	// If any boids landed, call the script and it will delete the models.

	if ( boidsLanded )
	{
		Script::call(	PyObject_GetAttrString( entity_, "boidsLanded" ),
						PyTuple_New(0),
						"BoidsFilter::output: ",
						true );
	}

	dwBoidsFilterOutput.stop();
}


/**
 *	Standard get attribute method.
 */
PyObject * BoidsFilter::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return AvatarFilter::pyGetAttribute( attr );
}


/**
 *	Standard set attribute method.
 */
int BoidsFilter::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return AvatarFilter::pySetAttribute( attr, value );
}


/**
 *	Constructor.
 */
BoidsFilter::BoidData::BoidData() :
	pos_(	20.0f * (unitRand()-unitRand()),
			 2.0f * unitRand(),
			20.0f * ( unitRand() - unitRand() ) ),
	dir_( unitRand(), unitRand(), unitRand() ),
	yaw_( 0.f ),
	pitch_( 0.f ),
	roll_( 0.f ),
	dYaw_( 0.f ),
	speed_( 0.1f )
{
	// Make sure it is not the zero vector.
	dir_.x += 0.0001f;
	dir_.normalise();
}


/**
 *	Destructor
 */
BoidsFilter::BoidData::~BoidData()
{
	BW_GUARD;	
}


/**
 *	This method updates the state of the model so that it is in its new
 *	position.
 *
 *	@param	goal	The current goal direction of the group.
 *	@param	pCA		The model that will be updated using the position and
 *					rotation of the boid.
 *	@param	filter	The BoidsFilter controlling the group of which this
 *					boid is a member.
 *	@param	dTime	The time in 1/30th of a second since the last update.
 *	@param	state	The state of the group. Landing or not landing.
 *
 *	@see
 *	@link	BoidsFilter::output()
 */
void BoidsFilter::BoidData::updateModel(	const Vector3 & goal,
											ChunkEmbodiment * pCA,
											const BoidsFilter & filter,
											float dTime,
											int state)
{
	BW_GUARD;
	// The settings currently assume that dTime is in 30th of a second.
	dTime *= 30.f;

	const BoidsFilter::Boids & boids = filter.boidData_;
	Vector3 deltaPos = Vector3::zero();
	Vector3 deltaDir = Vector3::zero();
	float count = 0;
	const float INFLUENCE_RADIUS_SQUARED = filter.INFLUENCE_RADIUS * filter.INFLUENCE_RADIUS;
	const float INV_COLLISION_FRACTION = 1.0f / ( 1.0f - filter.COLLISION_FRACTION );
	const float GOAL_APPROACH_RADIUS_SQUARED = filter.GOAL_APPROACH_RADIUS * filter.GOAL_APPROACH_RADIUS;
	const float GOAL_STOP_RADIUS_SQUARED = filter.GOAL_STOP_RADIUS * filter.GOAL_STOP_RADIUS;

	for ( uint i = 0; i < boids.size(); i++ )
	{
		const BoidData & otherBoid = boids[i];

		if ( &otherBoid != this )
		{
			Vector3 diff = pos_ - otherBoid.pos_;
			float dist = diff.lengthSquared();
			dist = INFLUENCE_RADIUS_SQUARED - dist;

			if (dist > 0.f)
			{
				dist /= INFLUENCE_RADIUS_SQUARED;
				count += 1.f;

				diff.normalise();
				float collWeight = dist - filter.COLLISION_FRACTION;

				if ( collWeight > 0.f )
				{
					collWeight *= INV_COLLISION_FRACTION;
				}
				else
				{
					collWeight = 0.f;
				}

				if ( dist - ( 1.f - filter.COLLISION_FRACTION ) > 0.f )
				{
					collWeight -= dist * ( 1.f - filter.COLLISION_FRACTION );
				}

				Vector3 delta = collWeight * diff;
				deltaPos += delta;
				deltaDir += otherBoid.dir_ * dist;
			}
		}
	}

	if ( count != 0.f )
	{
		deltaDir /= count;
		deltaDir -= dir_;
		deltaDir *= 1.5f;
	};

	Vector3 delta = deltaDir + deltaPos;

	// Add in the influence of the global goal
	Vector3 goalDir = goal - pos_;
	goalDir.normalise();
	goalDir *= 0.5f;
	delta += goalDir;

	// First deal with pitch changes
	if ( delta.y > 0.01f )
	{   // We're too low
		pitch_ += filter.ANGLE_TWEAK * dTime;

		if ( pitch_ > 0.8f )
		{
			pitch_ = 0.8f;
		}
	}
	else if ( delta.y < -0.01f )
	{   // We're too high
		pitch_ -= filter.ANGLE_TWEAK * dTime;
		if ( pitch_ < -0.8f )
		{
			pitch_ = -0.8f;
		}
	}
	else
	{
		// Add damping
		pitch_ *= 0.98f;
	}

	// Speed up or slow down depending on angle of attack
	speed_ -= pitch_ * filter.PITCH_TO_SPEED_RATIO;

	// Damp back to normal
	speed_ = (speed_ - filter.NORMAL_SPEED) * 0.99f + filter.NORMAL_SPEED;

	if ( speed_ < filter.NORMAL_SPEED / 2 )
		speed_ = filter.NORMAL_SPEED / 2;
	if ( speed_ > filter.NORMAL_SPEED * 5 )
		speed_ = filter.NORMAL_SPEED * 5;

	// Now figure out yaw changes
	Vector3 offset	= delta;

	if ( state != 1 )
		offset[Y_COORD]		= 0.0f;

	delta				= dir_;
	offset.normalise( );
	float dot = offset.dotProduct( delta );

	// Speed up slightly if not turning much
	if ( dot > 0.7f )
	{
		dot -= 0.7f;
		speed_ += dot * 0.05f;
	}

	Vector3 vo = offset.crossProduct( delta );
	dot = (1.0f - dot)/2.0f * 0.07f;

	if ( vo.y > 0.05f )
		dYaw_ = (dYaw_*19.0f + dot) * 0.05f;
	else if ( vo.y < -0.05f )
		dYaw_ = (dYaw_*19.0f - dot) * 0.05f;
	else
		dYaw_ *= 0.98f; // damp it

	yaw_ += dYaw_ * dTime;
	roll_ = -dYaw_ * 20.0f * dTime;


	Matrix m;
	m.setTranslate( pos_ );
	m.preRotateY( -yaw_ );
	m.preRotateX( -pitch_ );
	m.preRotateZ( -roll_ );

	dir_ = m.applyToUnitAxisVector( Z_AXIS );

	if ( state == STATE_LANDING )
	{
		Vector3 goalDiff = pos_ - goal;
		float len = goalDiff.lengthSquared();

		if ( len < GOAL_APPROACH_RADIUS_SQUARED )
			dir_ = goalDir;

		if ( len > 0 && len < GOAL_STOP_RADIUS_SQUARED )
		{
			pos_ = goal;
			speed_ = 0;
		}
	}

	pos_ += dir_ * speed_ * dTime;

	((ChunkAttachment*)pCA)->setMatrix( m );
}


/**
 *	Python factory method
 */
PyObject * BoidsFilter::pyNew( PyObject * args )
{
	BW_GUARD;
	int argc = PyTuple_Size( args );
	PyObject * pFilter = NULL;

	if (!PyArg_ParseTuple( args, "|O", &pFilter ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.BoidsFilter() "
			"expects an optional AvatarFilter as arguement" );
		return NULL;
	}
	
	if (pFilter != NULL)
	{
		if (!AvatarFilter::Check( pFilter ))
		{
			PyErr_SetString( PyExc_TypeError, "BigWorld.BoidsFilter() "
				"expects an optional AvatarFilter as arguement" );
			return NULL;
		}
		else
		{
			return new BoidsFilter( *static_cast<AvatarFilter*>( pFilter ), &s_type_ );
		}
	}

	return new BoidsFilter();
}


// boids_filter.cpp
