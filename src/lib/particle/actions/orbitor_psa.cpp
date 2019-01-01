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

#pragma warning (disable:4355)	// 'this' : used in base member initialiser list
#pragma warning (disable:4503)	// 'identifier' : decorated name length exceeded, name was truncated
#pragma warning (disable:4786)	// 'identifier' : identifier was truncated to 'number' characters in the debug information

#include "orbitor_psa.hpp"
#include "particle/particle_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "orbitor_psa.ipp"
#endif


ParticleSystemActionPtr OrbitorPSA::clone() const
{
    return ParticleSystemAction::clonePSA(*this);
}

// -----------------------------------------------------------------------------
// Section: Overrides to the Particle System Action Interface.
// -----------------------------------------------------------------------------

/**
 *	This method executes the action for the given frame of time. The dTime
 *	parameter is the time elapsed since the last call.
 *
 *	@param particleSystem	The particle system on which to operate.
 *	@param dTime			Elapsed time in seconds.
 */
PROFILER_DECLARE( OrbitorPSA_execute, "PSA Orbitor Execute" );

void OrbitorPSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER( OrbitorPSA_execute );

	// Do nothing if the dTime passed was zero or if the particle system
	// is not quite old enough to be active. Or if angular velocity is zero.
	if ( ( age_ < delay() ) || ( dTime <= 0.0f ) || angularVelocity_ == 0.0f )
	{
		age_ += dTime;
		return;
	}

	if ( particleSystem.particles()->capacity() > indexedOrbits_.size() )
	{
		Vector3 blueprint( FLT_MAX, FLT_MAX, FLT_MAX );
		indexedOrbits_.resize( particleSystem.particles()->capacity(), blueprint );
	}
	
	// Get the position in world space of the particle system.
	Vector3 positionOfPS = particleSystem.objectToWorld().applyToOrigin();

	Particles::iterator begin = particleSystem.begin();
	Particles::iterator current = begin;
	Particles::iterator endOfParticles = particleSystem.end();

	while (current != endOfParticles)
	{
		Particle &particle = *current;

		size_t index = particleSystem.particles()->index(current);

		if (particle.isAlive())
		{
			// If we don't have an entry or our entry has been replaced with 
			// a new particle
			if ((indexedOrbits_[index].x == FLT_MAX) || 
					almostEqual( particle.age(), 0 ))
			{
				indexedOrbits_[ index ] = positionOfPS;
			}

			const Vector3& orbit = indexedOrbits_[ index ];

			// transform the rotation by the stored PS position that relates 
			// to the particles' age
			Matrix rotation;
			rotation.setRotateY( fmodf(angularVelocity_*dTime, MATH_PI * 2.f ));
			rotation.preTranslateBy( -1 * orbit );
			rotation.preTranslateBy( -1 * this->point() );
			rotation.postTranslateBy( this->point() );
			rotation.postTranslateBy( orbit );
			rotation.applyPoint( particle.position(), particle.position() );

			if (affectVelocity())
			{
				Vector3 velocity;
				particle.getVelocity( velocity );
				rotation.applyVector( velocity, velocity );
				particle.setVelocity( velocity );
			}
		}
		else
		{
			indexedOrbits_[index].x = FLT_MAX;
		}

		++current;
	}
}


/**
 *	This is the serialiser for OrbitorPSA properties
 */
void OrbitorPSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, point_, Vector3, load);
	SERIALISE(pSect, angularVelocity_, Float, load);
	SERIALISE(pSect, affectVelocity_, Bool, load);
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyOrbitorPSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyOrbitorPSA )

/*~ function Pixie.OrbitorPSA
 *	Factory function to create and return a new PyOrbitorPSA object. OrbitorPSA is
 *	a ParticleSystemAction that causes particles to orbit around a point.
 *	@return A new PyOrbitorPSA object.
 */
PY_FACTORY_NAMED( PyOrbitorPSA, "OrbitorPSA", Pixie )

PY_BEGIN_METHODS( PyOrbitorPSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyOrbitorPSA )
	/*~ attribute PyOrbitorPSA.point
	 *	point specifies the center point around which the particles orbit.
	 *	Default is (0.0, 0.0, 0.0).
	 *	@type Sequence of 3 floats.
	 */
	PY_ATTRIBUTE( point )
	/*~ attribute PyOrbitorPSA.angularVelocity
	 *	angularValocity is used to specify the angular velocity of the orbit
	 *	in degrees per second.
	 *	@type Float.
	 */
	PY_ATTRIBUTE( angularVelocity )
	/*~ attribute PyOrbitorPSA.affectVelocity
	 *	affectVelocity is used to specify whether the OrbitorPSA affects
	 *	a particle's velocity as well as it's position. Default is 0 (false).
	 *	@type Integer as boolean.
	 */
	PY_ATTRIBUTE( affectVelocity )
PY_END_ATTRIBUTES()


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyOrbitorPSA::pyGetAttribute( const char *attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyParticleSystemAction::pyGetAttribute( attr );
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the writable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 *	@param value	The value to assign to the attribute.
 */
int PyOrbitorPSA::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyParticleSystemAction::pySetAttribute( attr, value );
}


/**
 *	This is a static Python factory method. This is declared through the
 *	factory declaration in the class definition.
 *
 *	@param args		The list of parameters passed from Python. The NodeClamp
 *					action takes no parameters and ignores any passed to this
 *					method.
 */
PyObject *PyOrbitorPSA::pyNew( PyObject *args )
{
	BW_GUARD;
	Vector3 newPoint( 0.0f, 0.0f, 0.0f );
	float velocity = 0.0f;

	if ( PyTuple_Size( args ) > 0 )
	{
		if ( Script::setData( PyTuple_GetItem( args, 0 ), newPoint,
			"OrbitorPSA() first argument" ) != 0 )
		{
			return NULL;
		}

		if ( PyTuple_Size( args ) > 1 )
		{
			if ( Script::setData( PyTuple_GetItem( args, 1 ), velocity,
				"OrbitorPSA() second argument" ) != 0 )
			{
				return NULL;
			}
			else
			{
				OrbitorPSAPtr pAction = new OrbitorPSA( newPoint, velocity );
				return new PyOrbitorPSA(pAction);
			}
		}
		else
		{
			OrbitorPSAPtr pAction = new OrbitorPSA( newPoint, velocity );
			return new PyOrbitorPSA(pAction);
		}
	}
	else
	{
		OrbitorPSAPtr pAction = new OrbitorPSA;
		return new PyOrbitorPSA(pAction);
	}
}


PY_SCRIPT_CONVERTERS( PyOrbitorPSA )
