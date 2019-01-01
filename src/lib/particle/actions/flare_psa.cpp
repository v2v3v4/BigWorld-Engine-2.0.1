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

#include "flare_psa.hpp"
#include "particle/particle_system.hpp"
#include "particle/renderers/particle_system_renderer.hpp"
#include "romp/lens_effect_manager.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "flare_psa.ipp"
#endif


ParticleSystemActionPtr FlarePSA::clone() const
{
    BW_GUARD;
	return ParticleSystemAction::clonePSA(*this);
}


/*static*/ void FlarePSA::prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output )
{
	BW_GUARD;
	const std::string& flareName = pSection->readString( "flareName_" );
	if (flareName.length())
		output.insert(flareName);
}


/**
 *	This is the serialiser for FlarePSA properties
 */
void FlarePSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, flareName_, String, load);
	if (load)
		loadLe();

	SERIALISE(pSect, flareStep_, Int, load);
	SERIALISE(pSect, colourize_, Bool, load);
	SERIALISE(pSect, useParticleSize_, Bool, load);
}


/**
 *	Load the associated lens effect from the name already set
 */
void FlarePSA::loadLe( void )
{
	BW_GUARD;
	if ( !flareName_.length() )
	{
		return;
	}

	DataSectionPtr flareSection = BWResource::openSection( flareName_ );
	if ( !flareSection || !LensEffect::isLensEffect( flareName_ ) )
	{
		ERROR_MSG( "FlarePSA::loadLe: Couldn't load lens effect %s\n",
			flareName_.c_str() );
		return;
	}

	le_.load( flareSection );
}


/**
 *	This method executes the action for the given frame of time. The dTime
 *	parameter is the time elapsed since the last call.
 *
 *	@param particleSystem	The particle system on which to operate.
 *	@param dTime			Elapsed time in seconds.
 */
PROFILER_DECLARE( FlarePSA_execute, "Flare Barrier Execute" );
void FlarePSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER(FlarePSA_execute);

	// This is a special case condition since particle systems do not have
	// any control over the rendering of their own flares.
	if (!particleSystem.enabled()) 
	{
		return;
	}

	Matrix worldTransform;
	bool isMesh = particleSystem.pRenderer()->isMeshStyle();

	if (particleSystem.pRenderer()->local())
	{
		worldTransform = particleSystem.worldTransform();
	}
	else
	{
		worldTransform = Matrix::identity;
	}

	if (flareStep_ == 0)
	{
		// Only affect the leader.
		if (particleSystem.size() > 0)
		{
			//This now uses a new method to determine the last particle added (bug 5236)
			Particles::iterator current = particleSystem.particles()->lastAdded();
			const Particle& particle = *current;

			if (useParticleSize_ && !isMesh)
			{
				le_.size( particle.size() );
			}

			if (colourize_)
			{
				le_.colour( particle.colour() );
			}
			else
			{
				le_.defaultColour();
			}

			Vector3 position( particle.position() );
			Vector3 lePosition( worldTransform.applyPoint( position ) );
			LensEffectManager::instance().add( ParticleSystem::getUniqueParticleID( current, particleSystem ), lePosition, le_ );
			particleSystem.addFlareID( ParticleSystem::getUniqueParticleID( current, particleSystem ) );
		}
	}
	else if (flareStep_ > 0)
	{
		Particles::iterator current = particleSystem.begin();
		Particles::iterator endOfParticles = particleSystem.end();

		while (current != endOfParticles)
		{
			Particle &particle = *current;

			if (particle.isAlive())
			{
				// Use the particle index to ensure that particles keep their flare
				// in the case where there is a sink (bug 5236).
				size_t index = particleSystem.particles()->index(current);
				if (index % flareStep_ == 0)
				{
					if (useParticleSize_ && !isMesh)
					{
						le_.size( particle.size() );
					}

					if (colourize_)
					{
						le_.colour( particle.colour() );
					}
					else
					{
						le_.defaultColour();
					}

					Vector3 position( particle.position() );
					Vector3 lePosition( worldTransform.applyPoint( position ) );
					// Use the particle index here to avoid issues with sinks (bug 5236).
					LensEffectManager::instance().add( ParticleSystem::getUniqueParticleID( current, particleSystem ), lePosition, le_ );
					particleSystem.addFlareID( ParticleSystem::getUniqueParticleID( current, particleSystem ) );
				}
			}

			++current;
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Python Interface to the PyFlarePSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyFlarePSA )

/*~ function Pixie.FlarePSA
 *	Factory function to create and return a new PyFlarePSA object. FlarePSA is a
 *	ParticleSystemAction that draws a lens flare at the location of the oldest
 *	particle.
 *	@param filename Name of the flare resource to use.
 *	@return A new PyFlarePSA object.
 */
 PY_FACTORY_NAMED( PyFlarePSA, "FlarePSA", Pixie )

PY_BEGIN_METHODS( PyFlarePSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyFlarePSA )
	/*~ attribute PyFlarePSA.flareName
	 *	This value determines which lens effect resource to use for flares.
	 *	Default is empty string.
	 *	@type String.
	 */
	PY_ATTRIBUTE( flareName )
	/*~ attribute PyFlarePSA.flareStep
	 *	This value determines which particles have a flare drawn on them. 0 is 
	 *	leader only, 1 is all particles, 2..x is every X particles.
	 *	Default is 0.
	 *	@type Integer. 0..n.
	 */
	PY_ATTRIBUTE( flareStep )
	/*~ attribute PyFlarePSA.colourize
	 *	Specifies whether the flare should be colourized from the colour of the
	 *	particle.
	 *	Default is 0 (false).
	 *	@type Integer as boolean. 0 or 1.
	 */
	PY_ATTRIBUTE( colourize )
	/*~ attribute PyFlarePSA.useParticleSize
	 *	Specifies whether the effect should use the size of the particle for its
	 *	size.
	 *	Default is 0 (false).
	 *	@type Integer as boolean.
	 */
	PY_ATTRIBUTE( useParticleSize )
PY_END_ATTRIBUTES()


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyFlarePSA::pyGetAttribute( const char *attr )
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
int PyFlarePSA::pySetAttribute( const char *attr, PyObject *value )
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
PyObject *PyFlarePSA::pyNew( PyObject *args )
{
	BW_GUARD;
	char * flareType = NULL;
	int flareStep = 0;	//leader only
	int col = 1;
	int size = 0;

	if ( PyArg_ParseTuple( args, "s|iii", &flareType, &flareStep, &col, &size ) )
	{
		FlarePSAPtr pAction = new FlarePSA( flareType, flareStep, col, size );
		return new PyFlarePSA(pAction);
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "FlarePSA:"
			"Expected a lens flare filename and optional params (flare step),(colourize),(useParticleSize)." );
		return NULL;
	}
}


PY_SCRIPT_CONVERTERS( PyFlarePSA )
