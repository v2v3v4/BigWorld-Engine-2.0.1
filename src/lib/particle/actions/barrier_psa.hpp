/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BARRIER_PSA_HPP
#define BARRIER_PSA_HPP

#include "particle_system_action.hpp"

// Forward Class Declarations.
class ParticleSystem;

/**
 *	This action acts as an invisible barrier to the particles. The barrier PSA
 *	can be a plane, a box, a vertical cylinder, or a sphere. It monitors
 *	particles that travel from within it to outside of it.
 *
 *	If a particle is stopped by the barrier, it can remove the particle or
 *	bounce the particle backwards. The barrier itself is weak - that is,
 *	depending on the updates, it can allow particles to partially escape for
 *	a short period of time as it pushes the particles back into the interior.
 *
 *	Only one shape and one collision action can be active at any one time for
 *	a given barrier PSA.
 */
class BarrierPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	BarrierPSA();	
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Methods for the BarrierPSA.
	//@{
	void none();
	void verticalCylinder( const Vector3 &pointOnAxis, float radius );
	void box( const Vector3 &corner, const Vector3 &oppositeCorner );
	void sphere( const Vector3 &centre, float radius );

	void bounceParticles( void );
	void removeParticles( void );
	void allowParticles( void );
	void wrapParticles( void );
	//@}


	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(BarrierPSA); }
	//@}

	static const std::string nameID_;

	///	@name BarrierPSA Types.
	//@{

	/// Enumeration Type for the Shape.
	enum Shape { NONE, VERTICAL_CYLINDER, BOX, SPHERE, SHAPE_MAX };
	static std::string shapeTypeNames_[SHAPE_MAX];
	Shape shape() const { return shape_; }

	/// Enumeration Type for the Reaction to collision.
	enum Reaction { BOUNCE, REMOVE, ALLOW, WRAP, REACTION_MAX };
	static std::string reactionTypeNames_[REACTION_MAX];
	Reaction reaction() const { return reaction_; }

	float resilience() const {return resilience_; }
	void resilience(float r) { resilience_ = r; }
	//@}

	///	@name accessors for editor
	//@{

	void verticalCylinderPointOnAxis( const Vector3 &pointOnAxis ) { vecA_ = pointOnAxis; }
	Vector3	verticalCylinderPointOnAxis() const { return vecA_; }
	void verticalCylinderRadius( float radius ) { radius_ = radius; }
	float verticalCylinderRadius() const { return radius_; }

	void boxCorner( const Vector3 &corner ) { vecA_ = corner; }
	Vector3 boxCorner() const { return vecA_; }
	void boxOppositeCorner( const Vector3 &corner ) { vecB_ = corner; }
	Vector3 boxOppositeCorner() const { return vecB_; }

	void sphereCentre( const Vector3 &centre ) { vecA_ = centre; }
	Vector3 sphereCentre() const { return vecA_; }
	void sphereRadius( float radius ) { radius_ = radius; }
	float sphereRadius() const { return radius_; }

	//@}

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:

	///	@name Private Helper Methods for BarrierPSA.
	//@{
	void collideWithCylinder( ParticleSystem &particleSystem, float dTime );
	void collideWithBox( ParticleSystem &particleSystem, float dTime );
	void collideWithSphere( ParticleSystem &particleSystem, float dTime );
	//@}

	///	@name Auxiliary Variables for the BarrierPSA.
	//@{
	static int typeID_;		///< TypeID of the BarrierPSA.

	Shape shape_;			///< The shape currently being used.
	Reaction reaction_;		///< The reaction used for collisions.

	// The following values get different interpretations depending on the
	// shapeID value.
	Vector3 vecA_;			///< Auxiliary Vector A.
	Vector3 vecB_;			///< Auxiliary Vector B.
	float radius_;			///< Radius.

	float resilience_;		///< Elasticity of the Reaction behaviour (1.0 is default)
	//@}
};

typedef SmartPointer<BarrierPSA> BarrierPSAPtr;


/*~ class Pixie.PyBarrierPSA
 *
 *	PyBarrierPSA is a PyParticleSystemAction that acts as an invisible barrier
 *	to particles. It can take the form of a plane, box, vertical cylinder, or
 *	sphere. The barrier is "weak" in the sense that particles could possibly
 *	penetrate the barrier for a short period of time (the barrier only acts
 *	once the particle has penetrated the barrier), before being pushed
 *	back into the interior or removed.
 *
 *	The barrier can remove particles that hit the barrier, or bounce them off
 *	the barrier. Only one shape, and collision action can be active at one time
 *	for each BarrierPSA.
 *
 *	A new PyBarrierPSA is created using Pixie.BarrierPSA function.
 */
class PyBarrierPSA : public PyParticleSystemAction
{
	Py_Header( PyBarrierPSA, PyParticleSystemAction )
public:
	PyBarrierPSA( BarrierPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Python Interface to the BarrierPSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_METHOD_DECLARE( py_verticalCylinder )
	PY_METHOD_DECLARE( py_box )
	PY_METHOD_DECLARE( py_sphere )

	PY_METHOD_DECLARE( py_bounceParticles )
	PY_METHOD_DECLARE( py_removeParticles )
	PY_METHOD_DECLARE( py_allowParticles )
	PY_METHOD_DECLARE( py_wrapParticles )
	//@}
private:
	BarrierPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyBarrierPSA )


#ifdef CODE_INLINE
#include "barrier_psa.ipp"
#endif

#endif


/* barrier_psa.hpp */
