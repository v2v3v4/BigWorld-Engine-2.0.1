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

#include "node_clamp_psa.hpp"
#include "particle/particle_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "node_clamp_psa.ipp"
#endif


ParticleSystemActionPtr NodeClampPSA::clone() const
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
PROFILER_DECLARE( NodeClampPSA_execute, "PSA Node Clamp Execute" );

void NodeClampPSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER(SplatPSA_execute);
	// Do nothing if the dTime passed was zero or if the particle system
	// is not quite old enough to be active.
	if ( ( age_ < delay() ) || ( dTime <= 0.0f ) )
	{
		age_ += dTime;
		return;
	}

	// Calculate the object-to-world transform.
	Matrix objectToWorld = particleSystem.objectToWorld();

	// Get the position in world space of the particle system.
	Vector3 positionOfPS = objectToWorld.applyToOrigin();

	if ( fullyClamp_ )
	{
		Particles::iterator current = particleSystem.begin();
		Particles::iterator endOfParticles = particleSystem.end();
		while ( current != endOfParticles )
		{
			Particle &particle = *current++;
			if (particle.isAlive())
				particle.position() = positionOfPS;
		}
	}
	else
	{
		// Calculate our delta position..
		Vector3 displacement = positionOfPS - ( firstUpdate_ ? positionOfPS :
			lastPositionOfPS_ );


		// Move the particles accordingly if the particle system has moved.
		// Drones and leaders are affected by this action.
		if ( displacement.lengthSquared() > 0.0f )
		{
			Particles::iterator current = particleSystem.begin();
			Particles::iterator endOfParticles = particleSystem.end();
			while ( current != endOfParticles )
			{
				Particle &particle = *current++;
				if (particle.isAlive())
					particle.position() += displacement;
			}
		}

		firstUpdate_ = false;
		lastPositionOfPS_ = positionOfPS;
	}	
}


/**
 *	This is the serialiser for NodeClampPSA properties
 */
void NodeClampPSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, fullyClamp_, Bool, load);
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyNodeClampPSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyNodeClampPSA )

/*~ function Pixie.NodeClampPSA
 *	Factory function to create and return a new PyNodeClampPSA object.
 *	NodeClampPSA is a ParticleSystemAction that locks a particle to a Node.
 *	@return A new PyNodeClampPSA object.
 */
PY_FACTORY_NAMED( PyNodeClampPSA, "NodeClampPSA", Pixie )

PY_BEGIN_METHODS( PyNodeClampPSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyNodeClampPSA )
	/*~ attribute PyNodeClampPSA.fullyClamp
	 *	If set to true, the particles are clamped directly to the particle system
	 *	position.  If false, the particles are moved relatively each frame by the
	 *	amount the particle system moved by.
	 *
	 *	@type Bool
	 */
	PY_ATTRIBUTE( fullyClamp )
PY_END_ATTRIBUTES()


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyNodeClampPSA::pyGetAttribute( const char *attr )
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
int PyNodeClampPSA::pySetAttribute( const char *attr, PyObject *value )
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
PyObject *PyNodeClampPSA::pyNew( PyObject *args )
{
	BW_GUARD;
	NodeClampPSAPtr pAction = new NodeClampPSA;
	return new PyNodeClampPSA(pAction);
}


PY_SCRIPT_CONVERTERS( PyNodeClampPSA )
