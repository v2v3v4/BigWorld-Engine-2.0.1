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

#include "scaler_psa.hpp"
#include "particle/particle_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "scaler_psa.ipp"
#endif


ParticleSystemActionPtr ScalerPSA::clone() const
{
    return ParticleSystemAction::clonePSA(*this);
}

// -----------------------------------------------------------------------------
// Section: Overrides to the Particle System Action Interface.
// -----------------------------------------------------------------------------

PROFILER_DECLARE( ScalerPSA_execute, "PSA Scaler Execute" );

/**
 *	This method executes the action for the given frame of time. The dTime
 *	parameter is the time elapsed since the last call.
 *
 *	@param particleSystem	The particle system on which to operate.
 *	@param dTime			Elapsed time in seconds.
 */
void ScalerPSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER( ScalerPSA_execute );

	// Do nothing if the dTime passed was zero or if the particle system
	// is not quite old enough to be active.
	if ( ( age_ < delay() ) || ( dTime <= 0.0f ) )
	{
		age_ += dTime;
		return;
	}


	Particles::iterator current = particleSystem.begin();
	Particles::iterator endOfParticles = particleSystem.end();
	while ( current != endOfParticles )
	{
		Particle &particle = *current;

		if (particle.isAlive())
		{
			float size = particle.size();

			if ( size > size_ )
			{
				size -= rate_ * dTime;
				if ( size < size_ )
				{
					size = size_;
				}
			}
			else if ( size < size_ )
			{
				size += rate_ * dTime;
				if ( size > size_ )
				{
					size = size_;
				}
			}

			particle.size( size );
		}

		++current;
	}
}


/**
 *	This is the serialiser for ScalerPSA properties
 */
void ScalerPSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, size_, Float, load);
	SERIALISE(pSect, rate_, Float, load);
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyScalerPSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyScalerPSA )

/*~ function Pixie.ScalerPSA
 *	Factory function to create and return a new PyScalerPSA object. ScalarPSA is a
 *	ParticleSystemAction that scales particle size over time.
 *	@return A new PyScalerPSA object.
 */
PY_FACTORY_NAMED( PyScalerPSA, "ScalerPSA", Pixie )

PY_BEGIN_METHODS( PyScalerPSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyScalerPSA )
	/*~ attribute PyScalerPSA.size
	 *	Eventual size of the particles as a scaling factor. It's effect is
	 *	dependant upon the ParticleSystemRenderer. For a SpriteParticleRenderer
	 *	the size is applied to the width and height of the sprites. Particles
	 *	are scaled up or down linearly over time to reach this value.
	 *	Default is 0.0.
	 *	@type Float.
	 */
	PY_ATTRIBUTE( size )
	/*~ attribute PyScalerPSA.rate
	 *	Rate per second at which particles scale towards the value size.
	 *	Default is 0.0.
	 *	@type Float.
	 */
	PY_ATTRIBUTE( rate )
PY_END_ATTRIBUTES()

/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyScalerPSA::pyGetAttribute( const char *attr )
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
int PyScalerPSA::pySetAttribute( const char *attr, PyObject *value )
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
 *					be two floats ( size and rate.) Both arguments are
 *					optional.
 */
PyObject *PyScalerPSA::pyNew( PyObject *args )
{
	BW_GUARD;
	float newSize = 1.0;
	float newRate = 0.0;

	if ( PyArg_ParseTuple( args, "|ff", &newSize, &newRate ) )
	{
		ScalerPSAPtr pAction = new ScalerPSA( newSize, newRate );
		return new PyScalerPSA(pAction);
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "ScalerPSA:"
			"Expected two lists describing vector generators." );
		return NULL;
	}
}


PY_SCRIPT_CONVERTERS( PyScalerPSA )
