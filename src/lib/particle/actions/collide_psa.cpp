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

#include "collide_psa.hpp"
#include "source_psa.hpp"
#include "particle/particle_system.hpp"
#include "romp/romp_collider.hpp"
#include "romp/romp_sound.hpp"
#include "physics2/worldtri.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )

#ifndef CODE_INLINE
#include "collide_psa.ipp"
#endif


ParticleSystemActionPtr CollidePSA::clone() const
{
    BW_GUARD;
	return ParticleSystemAction::clonePSA(*this);
}

/**
 *	This is the serialiser for CollidePSA properties
 */
void CollidePSA::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, spriteBased_, Bool, load);
	SERIALISE(pSect, elasticity_, Float, load);
	SERIALISE(pSect, minAddedRotation_, Float, load);
	SERIALISE(pSect, maxAddedRotation_, Float, load);
	SERIALISE(pSect, entityID_, Int, load);
	SERIALISE(pSect, soundTag_, String, load);
	SERIALISE(pSect, soundEnabled_, Bool, load);
	SERIALISE(pSect, soundSrcIdx_, Int, load);
	SERIALISE(pSect, soundProject_, String, load);
	SERIALISE(pSect, soundGroup_, String, load);
	SERIALISE(pSect, soundName_, String, load);
}


/**
 *	This method executes the action for the given frame of time. The dTime
 *	parameter is the time elapsed since the last call.
 *
 *	@param particleSystem	The particle system on which to operate.
 *	@param dTime			Elapsed time in seconds.
 */
PROFILER_DECLARE( CollidePSA_execute, "PSA Collide Execute" );
void CollidePSA::execute( ParticleSystem &particleSystem, float dTime )
{
	BW_GUARD_PROFILER( CollidePSA_execute );
	SourcePSA* pSource = static_cast<SourcePSA*>( &*particleSystem.pAction( PSA_SOURCE_TYPE_ID ) );

	if ( !pSource )
		return;

	RompColliderPtr pGS = pSource->groundSpecifier();
	if ( !pGS )
		return;

	uint64	tend = timestamp() + stampsPerSecond() / 2000;

	bool soundHit = false;
	float maxVelocity = 0;
	Vector3 soundPos;
	uint materialKind;

	Particles::iterator it = particleSystem.begin();
	Particles::iterator end = particleSystem.end();

	WorldTriangle tri;

	//bust out of the loop if we take more than 0.5 msec

	//Sprite particles don't calculate spin
	while ( it != end && timestamp()<tend )
	{
		Particle &particle = *it++;

		if (!particle.isAlive())
		{		
			continue;
		}

		//note - particles get moved after actions.
		Vector3 velocity;
		particle.getVelocity(velocity);
		Vector3 pos;
		Vector3 newPos;

		if(particleSystem.isLocal())
		{
			Matrix world = particleSystem.worldTransform();
			pos = world.applyPoint(particle.position());
			Vector3 nPos;
			particleSystem.predictPosition( particle, dTime, nPos );
			newPos = world.applyPoint(nPos);
		}
		else
		{
			pos = particle.position();
			particleSystem.predictPosition( particle, dTime, newPos );
		}


		float tValue = pGS->collide( pos, newPos, tri );
		if ( tValue >= 0.f && tValue <= 1.f )
		{
			// calc v as a dotprod of the two normalised vectors (before and after collision)
			Vector3 oldVel = velocity / velocity.length();
			tri.bounce( velocity, elasticity_ );
			particle.setVelocity( velocity );
			float newSpeed = velocity.length();
			Vector3 newVel(velocity / newSpeed);
			float severity = oldVel.dotProduct(newVel);
			//DEBUG_MSG("severity: %1.3f, speed=%1.3f\n", severity, newSpeed);
			float v = (1 - severity) * newSpeed;

			//now spin the particle ( mesh only )
			if ( !spriteBased_ )
			{
				//first, calculate the current rotation, and update the pitch/yaw value.
				Matrix currentRot;
				currentRot.setRotate( particle.yaw(), particle.pitch(), 0.f );
				Matrix spin;
                float spinSpeed = particle.meshSpinSpeed();
                Vector3 meshSpinAxis = particle.meshSpinAxis();
                // If there is no spin direction then creating a rotation 
                // matrix can create weird matrices - e.g. matrices with
                // negative scale components and a translation.  We choose the
                // velocity as the spin direction (aribitrarily choosing, for
                // example up looks weird).
                if (meshSpinAxis == Vector3::zero())
                {
                    meshSpinAxis = velocity;
                    meshSpinAxis.normalise();
                }
				
				D3DXMatrixRotationAxis
                ( 
                    &spin, 
                    &meshSpinAxis, 
                    spinSpeed * (particle.age()-particle.meshSpinAge()) 
                );
				
				currentRot.preMultiply( spin );		
				particle.pitch( currentRot.pitch() );
				particle.yaw( currentRot.yaw() );

				//now, reset the age of the spin 
				particle.meshSpinAge( particle.age() );

				//finally, update the spin ( stored in the particle's colour )
				float addedSpin = unitRand() * (maxAddedRotation_-minAddedRotation_) + minAddedRotation_;
				addedSpin *= min( newSpeed, 1.f );				
				spinSpeed = Math::clamp( 0.f, spinSpeed + addedSpin, 1.f );
                particle.meshSpinSpeed(spinSpeed);
                particle.meshSpinAxis(meshSpinAxis);
			}

			if ( soundEnabled_ && v > 0.5f )
			{
				soundHit = true;
				if (v > maxVelocity) {
					maxVelocity = v;
					soundPos = particle.position();
					materialKind = tri.materialKind();
				}
			}
		}
	}

	if ( soundHit )
	{
		SmartPointer < RompSound > rs = RompSound::getProvider();
		if (rs)
		{
			if (!soundTag_.empty())
			{
				rs->playParticleSound( soundTag_.c_str(), soundPos, maxVelocity, soundSrcIdx_, materialKind );
			}
		}
	}
}


/** 
 *	This internal method updates the sound tag, and should be called whenever
 *	one of the soundProject_, soundGroup_ or soundName_ variables have changed.
 *
 *	If any of the constituent variables are empty, the sound tag is invalid and
 *	will be set to an empty string.
 */
void CollidePSA::updateSoundTag()
{
	if (!soundProject_.empty() && !soundGroup_.empty() && !soundName_.empty())
	{		
		soundTag_ = "/" + soundProject_ + "/" + soundGroup_ + "/" + soundName_;
	}
	else
	{
		soundTag_ = "";
	}
}

// -----------------------------------------------------------------------------
// Section: Python Interface to the PyCollidePSA.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyCollidePSA )

/*~ function Pixie.CollidePSA
 *	Factory function to create and return a new PyCollidePSA object. CollidePSA is
 *	a ParticleSystemAction that collides particles with the collision scene.
 *	@param elasticity Elasticity of the collisions.
 *	@return A new PyCollidePSA object.
 */
PY_FACTORY_NAMED( PyCollidePSA, "CollidePSA", Pixie )

PY_BEGIN_METHODS( PyCollidePSA )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyCollidePSA )
	PY_ATTRIBUTE( spriteBased )
	/*~ attribute PyCollidePSA.elasticity
	 *	This sets the amount of elasticity in the collision. 0 indicates no
	 *	reflection of velocity in the direction of the collision normal. 1
	 *	indicates full reflection of velocity in the direction of collision
	 *	normal. The default value is 0.
	 *	@type Float. Recommend values are 0.0 to 1.0.
	 */
	PY_ATTRIBUTE( elasticity )
	/*~ attribute PyCollidePSA.minAddedRotation
	 *	This specifies the minimum amount of spin added to the particle
	 *	when it collides ( dynamically scaled by velocity ).
	 *	The actual value varies randomly between min and max added rotation.
	 *	The value is interpreted as an absolute amount, so a particle that
	 *	is not currently spinning can begin spinning upon a collision.
	 *	If negative, the spin will decrease / be dampened.
	 *	@type Float. Recommend values are 0.0 to 1.0.
	 */
	PY_ATTRIBUTE( minAddedRotation )
	/*~ attribute PyCollidePSA.maxAddedRotation
	 *	This specifies the maximum amount of spin added to the particle
	 *	when it collides ( dynamically scaled by velocity ).
	 *	The actual value varies randomly between min and max added rotation.
	 *	The value is interpreted as an absolute amount, so a particle that
	 *	is not currently spinning can begin spinning upon a collision.
	 *	If negative, the spin will decrease / be dampened.
	 *	@type Float. Recommend values are 0.0 to 1.0.
	 */
	PY_ATTRIBUTE( maxAddedRotation )
	/*~ attribute PyCollidePSA.soundEnabled
	 *	Not supported.
	 */
	PY_ATTRIBUTE( soundEnabled )
	/*~ attribute PyCollidePSA.soundSrcIdx
	 *	Not supported.
	 */
	PY_ATTRIBUTE( soundSrcIdx )
	/*~ attribute PyCollidePSA.soundTag
	 *	Not supported.
	 */
	PY_ATTRIBUTE( soundTag )
PY_END_ATTRIBUTES()


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyCollidePSA::pyGetAttribute( const char *attr )
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
int PyCollidePSA::pySetAttribute( const char *attr, PyObject *value )
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
PyObject *PyCollidePSA::pyNew( PyObject *args )
{
	BW_GUARD;
	float elasticity;

	if ( PyArg_ParseTuple( args, "f", &elasticity ) )
	{
		CollidePSAPtr pAction = new CollidePSA(elasticity);
		return new PyCollidePSA(pAction);
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "PyCollidePSA:"
			"Expected a float value ( elasticity )." );
		return NULL;
	}
}


PY_SCRIPT_CONVERTERS( PyCollidePSA )
