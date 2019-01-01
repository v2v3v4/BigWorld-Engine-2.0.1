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

#include "force_psa.hpp"
#include "particle/particle_system.hpp"
#include "vector_generator.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "force_psa.ipp"
#endif


ParticleSystemActionPtr ForcePSA::clone() const
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
PROFILER_DECLARE( ForcePSA_execute, "PSA Force Execute" );

void ForcePSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER( ForcePSA_execute );

	// Do nothing if the dTime passed was zero or if the particle system
	// is not quite old enough to be active.
	if ( ( age_ < delay() ) || ( dTime <= 0.0f ) )
	{
		age_ += dTime;
		return;
	}

	Vector3 scaledVector( vector_ * dTime );

	Particles::iterator current = particleSystem.begin();
	Particles::iterator endOfParticles = particleSystem.end();
	while ( current != endOfParticles )
	{
		Particle &particle = *current++;
		if (particle.isAlive())
		{
			Vector3 velocity;
			particle.getVelocity( velocity );
			particle.setVelocity( velocity + scaledVector );
		}
	}
}


/**
 *	This is the serialiser for ForcePSA properties
 */
void ForcePSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	SERIALISE(pSect, vector_, Vector3, load);
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyForcePSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyForcePSA )

/*~ function Pixie.ForcePSA
 *	Factory function to create and return a new PyForcePSA object. ForcePSA is a
 *	ParticleSystemAction that applies a constant acceleration to particles in a
 *	particular direction.
 *	@return A new PyForcePSA object.
 */
PY_FACTORY_NAMED( PyForcePSA, "ForcePSA", Pixie )

PY_BEGIN_METHODS( PyForcePSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyForcePSA )
	/*~ attribute PyForcePSA.vector
	 *	This is a vector describing the direction and magnitude of the
	 *	acceleration to be applied to the particles. The default value
	 *	is (0.0, 0.0, 0.0).
	 *	@type Sequence of 3 floats.
	 */
	PY_ATTRIBUTE( vector )
PY_END_ATTRIBUTES()

/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyForcePSA::pyGetAttribute( const char *attr )
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
int PyForcePSA::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyParticleSystemAction::pySetAttribute( attr, value );
}

/**
 *	This is a static Python factory method. This is declared through the
 *	factory declaration in the class definition.
 *
 *	@param args		The list of parameters passed from Python. This should
 *					be a vector3 in python.
 */
PyObject *PyForcePSA::pyNew( PyObject *args )
{
	BW_GUARD;
	Vector3 newVector( 0.0f, 0.0f, 0.0f );

	if ( PyTuple_Size( args ) > 0 )
	{
		if ( Script::setData( PyTuple_GetItem( args, 0 ), newVector,
			"ForcePSA() argument " ) != 0 )
		{
			return NULL;
		}
		else
		{
			ForcePSAPtr pAction = new ForcePSA( newVector );
			return new PyForcePSA(pAction);
		}
	}
	else
	{
		ForcePSAPtr pAction = new ForcePSA( newVector );
		return new PyForcePSA(pAction);
	}
}


PY_SCRIPT_CONVERTERS( PyForcePSA )
