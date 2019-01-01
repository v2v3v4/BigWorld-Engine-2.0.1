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

#include "jitter_psa.hpp"
#include "vector_generator.hpp"
#include "particle/particle_system.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "jitter_psa.ipp"
#endif


ParticleSystemActionPtr JitterPSA::clone() const
{
    return ParticleSystemAction::clonePSA(*this);
}


// -----------------------------------------------------------------------------
// Section: Constructor(s) and Destructor for JitterPSA.
// -----------------------------------------------------------------------------
/**
 *	This is the destructor for JitterPSA.
 */
JitterPSA::~JitterPSA()
{
	BW_GUARD;
	if ( pPositionSrc_ != NULL )
	{
		delete pPositionSrc_;
		pPositionSrc_ = NULL;
	}
	if ( pVelocitySrc_ != NULL )
	{
		delete pVelocitySrc_;
		pVelocitySrc_ = NULL;
	}
}


// -----------------------------------------------------------------------------
// Section: Methods for JitterPSA.
// -----------------------------------------------------------------------------

/**
 *	This sets a vector generator as the new generator for jitter of the
 *	particle positions.
 *
 *	@param pPositionSrc		The new source for jitter positions.
 */
void JitterPSA::setPositionSource( VectorGenerator *pPositionSrc )
{
	BW_GUARD;
	if ( pPositionSrc != pPositionSrc_ )
	{
		if ( pPositionSrc_ != NULL )
		{
			delete pPositionSrc_;
			pPositionSrc_ = NULL;
		}
		pPositionSrc_ = pPositionSrc;
	}
}

/**
 *	This sets a vector generator as the new generator for jitter of the
 *	particle velocities.
 *
 *	@param pVelocitySrc		The new source for jitter velocities.
 */
void JitterPSA::setVelocitySource( VectorGenerator *pVelocitySrc )
{
	BW_GUARD;
	if ( pVelocitySrc != pVelocitySrc_ )
	{
		if ( pVelocitySrc_ != NULL )
		{
			delete pVelocitySrc_;
			pVelocitySrc_ = NULL;
		}
		pVelocitySrc_ = pVelocitySrc;
	}
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
PROFILER_DECLARE( JitterPSA_execute, "PSA Jitter Execute" );

void JitterPSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER( JitterPSA_execute );

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
		Particle &particle = *current++;
		if (particle.isAlive())
		{
			if ( affectPosition() && ( pPositionSrc_ != NULL ) )
			{
				Vector3 jitter;
				pPositionSrc_->generate( jitter );

				particle.position() += jitter * dTime;
			}

			if ( affectVelocity() && ( pVelocitySrc_ != NULL ) )
			{
				Vector3 jitter;
				pVelocitySrc_->generate( jitter );

				Vector3 velocity;
				particle.getVelocity( velocity );
				particle.setVelocity( velocity + jitter );
			}
		}
	}
}


/**
 *	This method determines the memory footprint of the action
 */
size_t JitterPSA::sizeInBytes() const
{
    size_t sz = sizeof(JitterPSA);
    if (pPositionSrc_ != NULL)
        sz += pPositionSrc_->sizeInBytes();
    if (pVelocitySrc_ != NULL)
        sz += pVelocitySrc_->sizeInBytes();
    return sz;
}


/**
 *	This is the serialiser for JitterPSA properties
 */
void JitterPSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, affectPosition_, Bool, load);
	SERIALISE(pSect, affectVelocity_, Bool, load);

	const std::string PositionSourceString("pPositionSrc");
	const std::string VelocitySourceString("pVelocitySrc");

	// manage the vector generators
	if (load)
	{
		DataSectionPtr pVGSect = pSect->openSection( PositionSourceString );
		if (pVGSect)
		{
			// create a new one, use iterators for convenience
			DataSectionIterator it;
			for(it = pVGSect->begin(); it != pVGSect->end(); it++)
			{
				DataSectionPtr pDS = *it;
				std::string vgType = pDS->sectionName();
				pPositionSrc_ = VectorGenerator::createGeneratorOfType(vgType);
				MF_ASSERT_DEV(pPositionSrc_);
				if( pPositionSrc_ )
					pPositionSrc_->serialise(pDS, load);
			}
		}

		pVGSect = pSect->openSection( VelocitySourceString );
		if (pVGSect)
		{
			// create a new one
			DataSectionIterator it;
			for(it = pVGSect->begin(); it != pVGSect->end(); it++)
			{
				DataSectionPtr pDS = *it;
				std::string vgType = pDS->sectionName();
				pVelocitySrc_ = VectorGenerator::createGeneratorOfType(vgType);
				MF_ASSERT_DEV(pVelocitySrc_);
				if( pVelocitySrc_ )
					pVelocitySrc_->serialise(pDS, load);
			}
		}
	}
	else
	{
		if (pPositionSrc_)
		{
			DataSectionPtr pVGSect = pSect->newSection( PositionSourceString );
			pPositionSrc_->serialise(pVGSect, load);
		}

		if (pVelocitySrc_)
		{
			DataSectionPtr pVGSect = pSect->newSection( VelocitySourceString );
			pVelocitySrc_->serialise(pVGSect, load);
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyJitterPSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyJitterPSA )

/*~ function Pixie.JitterPSA
 *	Factory function to create and return a new PyJitterPSA object. JitterPSA is a
 *	ParticleSystemAction that adds random motion to the particles.
 *	@return A new PyJitterPSA object.
 */
PY_FACTORY_NAMED( PyJitterPSA, "JitterPSA", Pixie )

PY_BEGIN_METHODS( PyJitterPSA )
	/*~ function PyJitterPSA.setPositionSource
	 *	Sets a new VectorGenerator as a jitter position source for the
	 *	particles.
	 *	@param generator VectorGenerator. Jitter position source for the
	 *	particles.
	 */
	PY_METHOD( setPositionSource )
	/*~ function PyJitterPSA.setVelocitySource
	 *	Sets a new VectorGenerator as a jitter velocity source for the
	 *	particles.
	 *	@param generator VectorGenerator. Jitter velocity source for the
	 *	particles.
	 */
	PY_METHOD( setVelocitySource )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyJitterPSA )
	/*~ attribute PyJitterPSA.affectPosition
	 *	This is a flag to determine whether JitterPSA affects position. Default
	 *	value is 0 (false).
	 *	@type Integer as boolean. 0 or 1.
	 */
	PY_ATTRIBUTE( affectPosition )
	/*~ attribute PyJitterPSA.affectVelocity
	 *	This is a flag to determine whether JitterPSA affects velocity. Default
	 *	is 0 (false).
	 *	@type Integer as boolean. 0 or 1.
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
PyObject *PyJitterPSA::pyGetAttribute( const char *attr )
{
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
int PyJitterPSA::pySetAttribute( const char *attr, PyObject *value )
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
 *					be two lists (vector generator descriptions for position
 *					and velocity.) Both arguments are optional.
 */
PyObject *PyJitterPSA::pyNew( PyObject *args )
{
	BW_GUARD;
	PyObject *positionDesc = NULL;
	PyObject *velocityDesc = NULL;

	if ( PyArg_ParseTuple( args, "|OO", &positionDesc, &velocityDesc ) )
	{
		VectorGenerator *posGen = NULL;
		if ( positionDesc != NULL )
		{
			posGen = VectorGenerator::parseFromPython( positionDesc );
			if ( posGen == NULL )
			{
				return NULL;	// Error already set by the parser.
			}
		}

		VectorGenerator *vecGen = NULL;
		if ( velocityDesc != NULL )
		{
			vecGen = VectorGenerator::parseFromPython( velocityDesc );
			if ( vecGen == NULL )
			{
				return NULL;	// Error already set by the parser.
			}
		}

		JitterPSAPtr pAction = new JitterPSA( posGen, vecGen );
		return new PyJitterPSA(pAction);
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "JitterPSA:"
			"Expected two lists describing vector generators." );
		return NULL;
	}
}

/**
 *	This Python method allows the script to specify the parameters for a
 *	new vector generator to the jitter positions.
 *
 *	@param args		The list of parameters passed from Python. This should
 *					be a list describing a vector generator.
 */
PyObject *PyJitterPSA::py_setPositionSource( PyObject *args )
{
	BW_GUARD;
	PyObject *positionDesc = NULL;

	if ( PyArg_ParseTuple( args, "O", &positionDesc ) )
	{
		VectorGenerator *vGen = VectorGenerator::parseFromPython(
			positionDesc );
		if ( vGen != NULL )
		{
			pAction_->setPositionSource( vGen );
			Py_Return;
		}
		else
		{
			return NULL;	// Error already set by the parser.
		}
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "JitterPSA.setPositionSource: "
			"Expected a list describing vector generators." );
		return NULL;
	}
}

/**
 *	This Python method allows the script to specify the parameters for a
 *	new vector generator to the jitter velocities.
 *
 *	@param args		The list of parameters passed from Python. This should
 *					be a list describing a vector generator.
 */
PyObject *PyJitterPSA::py_setVelocitySource( PyObject *args )
{
	BW_GUARD;
	PyObject *velocityDesc = NULL;

	if ( PyArg_ParseTuple( args, "O", &velocityDesc ) )
	{
		VectorGenerator * vg = VectorGenerator::parseFromPython( velocityDesc );
		if ( vg != NULL )
		{
			pAction_->setVelocitySource( vg );
			Py_Return;
		}
		else
		{
			return NULL;	// Error already set by the parser.
		}
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "JitterPSA.setVelocitySource: "
			"Expected a list describing vector generators." );
		return NULL;
	}
}


PY_SCRIPT_CONVERTERS( PyJitterPSA )
