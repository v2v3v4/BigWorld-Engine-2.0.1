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
#include "matrix_swarm_psa.hpp"
#include "particle/particle_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "matrix_swarm_psa.ipp"
#endif


ParticleSystemActionPtr MatrixSwarmPSA::clone() const
{
    return ParticleSystemAction::clonePSA(*this);
}


/**
 *	This method executes the action for the given frame of time. The dTime
 *	parameter is the time elapsed since the last call.
 *
 *	@param particleSystem	The particle system on which to operate.
 *	@param dTime			Elapsed time in seconds.
 */
PROFILER_DECLARE( MatrixSwarmPSA_execute, "PSA Matrix Swarm Execute" );

void MatrixSwarmPSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER(MatrixSwarmPSA_execute);
	if ( !targets_.size() )
		return;

	Particles::iterator it = particleSystem.begin();
	Particles::iterator end = particleSystem.end();

	int i = 0;
	int m = targets_.size();
	if ( particleSystem.size() < m )
		i = rand();

	while ( it != end )
	{
		Particle &particle = *it++;

		if (particle.isAlive())
		{
			Matrix tgt;
			if (targets_[i%m]!=NULL)
			{
				targets_[(i++)%m]->matrix( tgt );
				particle.position() = tgt.applyToOrigin();
			}
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyMatrixSwarmPSA.
// -----------------------------------------------------------------------------


PY_TYPEOBJECT( PyMatrixSwarmPSA )

/*~ function Pixie.MatrixSwarmPSA
 *
 *	Factory function to create and return a new PyMatrixSwarmPSA object.
 *	MatrixSwarmPSA takes a list of target matrix providers, and
 *	for each particle, assigns the position of one of the targets to it.
 *	@return A new PyMatrixSwarmPSA object.
 */
PY_FACTORY_NAMED( PyMatrixSwarmPSA, "MatrixSwarmPSA", Pixie )

PY_BEGIN_METHODS( PyMatrixSwarmPSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyMatrixSwarmPSA )
	/*~ attribute PyMatrixSwarmPSA.targets
	 *	This is a list of target MatrixProvider objects to apply to particles
	 *	instead of source of the particle systems transform.
	 *	@type List of MatrixProvider objects. Default is empty [].
	 */
	PY_ATTRIBUTE( targets )
PY_END_ATTRIBUTES()


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyMatrixSwarmPSA::pyGetAttribute( const char *attr )
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
int PyMatrixSwarmPSA::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyParticleSystemAction::pySetAttribute( attr, value );
}


/**
 *	This is a static Python factory method. This is declared through the
 *	factory declaration in the class definition.
 *
 *	@param args		The list of parameters passed from Python.
 */
PyObject *PyMatrixSwarmPSA::pyNew( PyObject *args )
{
	BW_GUARD;
	MatrixSwarmPSAPtr pAction = new MatrixSwarmPSA;
	return new PyMatrixSwarmPSA(pAction);
}


/**
 *	This is the release method for matrices.
 *
 *	@param pMatrix		The matrix to be released.
 */
void MatrixProviderPtr_release( MatrixProvider *&pMatrix )
{
	BW_GUARD;
	Py_XDECREF( pMatrix );
}


PY_SCRIPT_CONVERTERS( PyMatrixSwarmPSA )
