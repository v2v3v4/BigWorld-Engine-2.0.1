/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#ifndef BOIDS_FILTER_HPP
#define BOIDS_FILTER_HPP


#include "avatar_filter.hpp"


/*~ class BigWorld.BoidsFilter
 *
 *	This subclass of AvatarFilter implements a filter which implements flocking
 *	behaviour for the models that make up an entity.  This would normally
 *	be used for an entity such as a flock of birds or a school of fish which
 *	has many models driven by one entity.
 *
 *	The interpolation of the Entity position is done using the same logic
 *	as its parent AvatarFilter.  However, it also updates the positions
 *	of individual models (which are known, according to flocking conventions,
 *	as boids) that are attached to the entity, using standard
 *	flocking rules.
 *
 *	When the flock lands, the boidsLanded method is called on the entity.
 *
 *	A new BoidsFilter is created using the BigWorld.BoidsFilter function.
 */
/**
 *	This filter is a specialisation of AvatarFilter that implements flocking
 *	behaviour for the models that make up an entity.  This would normally
 *	be used for an entity such as a flock of birds or a school of fish which
 *	has many models driven by one entity.
 *
 *	The interpolation of the Entity position is done using the same logic
 *	as its parent AvatarFilter.  However, it also updates the positions
 *	of individual models (which are known, according to flocking conventions,
 *	as boids) that are attached to the entity, using standard
 *	flocking rules.
 */
class BoidsFilter : public AvatarFilter
{
	Py_Header( BoidsFilter, AvatarFilter );

private:

	/**
	 *	This class represents the data for an individual boid in the
	 *	flock.
	 */
	class BoidData
	{
	public:
		BoidData();
		~BoidData();

		void updateModel(	const Vector3 & goal,
							ChunkEmbodiment * pModel,
							const BoidsFilter & filter,
							float dTime,
							int state);

		Vector3			pos_;
		Vector3			dir_;		// Current direction
		float			yaw_, pitch_, roll_, dYaw_;
		float			speed_;
	};

	/**
	 *	This typedef defines an STL vector of BoidData.
	 *
	 *	@see	BoidsFilter::boids_
	 */
	typedef std::vector< BoidData >	Boids;

public:
	BoidsFilter( PyTypePlus * pType = &s_type_ );
	BoidsFilter( const AvatarFilter & filter, PyTypePlus * pType = &s_type_ );
	~BoidsFilter();

	// Overrides
	virtual void output( double time );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE();
	PY_RW_ATTRIBUTE_DECLARE( INFLUENCE_RADIUS, influenceRadius );
	PY_RW_ATTRIBUTE_DECLARE( COLLISION_FRACTION, collisionFraction );
	PY_RW_ATTRIBUTE_DECLARE( GOAL_APPROACH_RADIUS, approachRadius );
	PY_RW_ATTRIBUTE_DECLARE( GOAL_STOP_RADIUS, stopRadius );
	PY_RW_ATTRIBUTE_DECLARE( NORMAL_SPEED, speed );
	PY_RW_ATTRIBUTE_DECLARE( state_, state );

protected:
	// Doxygen comments for all members can be found in the .cpp
	float INFLUENCE_RADIUS;
	float COLLISION_FRACTION;
	float NORMAL_SPEED;
	uint state_;
	float ANGLE_TWEAK;
	float PITCH_TO_SPEED_RATIO;
	float GOAL_APPROACH_RADIUS;
	float GOAL_STOP_RADIUS;

private:
	Boids	boidData_;
	double	prevTime_;
	float	initialHeight_;
};



#endif // BOIDS_FILTER_HPP


// boids_filter.hpp
