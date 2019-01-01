/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef JITTER_PSA_HPP
#define JITTER_PSA_HPP

#include "particle_system_action.hpp"

/**
 *	This action adds random movement to the particles. The jitter PSA can
 *	effect a random jitter to either position or velocity or both at once.
 *
 *	There is a vector generator specified for both effects.
 */
class JitterPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	JitterPSA( VectorGenerator *pPositionSrc = NULL,
		VectorGenerator *pVelocitySrc = NULL );
	~JitterPSA();
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Accessors to JitterPSA properties.
	//@{
	bool affectPosition( void ) const;
	void affectPosition( bool flag );

	bool affectVelocity( void ) const;
	void affectVelocity( bool flag );
	//@}

	///	@name Methods for JitterPSA.
	//@{
	void setPositionSource( VectorGenerator *pPositionSrc );
	VectorGenerator * getPositionSource() { return pPositionSrc_; }
	void setVelocitySource( VectorGenerator *pVelocitySrc );
	VectorGenerator * getVelocitySource() { return pVelocitySrc_; }
	//@}

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const;
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Properties of the JitterPSA.
	//@{
	bool affectPosition_;	///< Flag for jittering particle position.
	bool affectVelocity_;	///< Flag for jittering particle velocity.

	VectorGenerator *pPositionSrc_;	///< Jitter Generator for position.
	VectorGenerator *pVelocitySrc_;	///< Jitter Generator for velocity.
	//@}

	///	@name Auxiliary Variables for the JitterPSA.
	//@{
	static int typeID_;			///< TypeID of the JitterPSA.
	//@}
};

typedef SmartPointer<JitterPSA> JitterPSAPtr;


/*~ class Pixie.PyJitterPSA
 *
 *	PyJitterPSA is a PyParticleSystemAction that adds random motion to the
 *	particles. JitterPSA can affect the position and/or veclocity of the
 *	particles.
 *
 *	The position source and/or velocity source will need to be set before this
 *	ParticleSystemAction will produce a visible effect - both default to
 *	( 0, 0, 0 ).
 *
 *	Position and Velocity "jitter" are calculated each frame.
 *
 *	A new PyJitterPSA is created using Pixie.JitterPSA function.
 */
class PyJitterPSA : public PyParticleSystemAction
{
	Py_Header( PyJitterPSA, PyParticleSystemAction )
public:
	PyJitterPSA( JitterPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Accessors to PyJitterPSA properties.
	//@{
	bool affectPosition( void ) const;
	void affectPosition( bool flag );

	bool affectVelocity( void ) const;
	void affectVelocity( bool flag );
	//@}

	///	@name Python Interface to the PyJitterPSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_METHOD_DECLARE( py_setPositionSource )
	PY_METHOD_DECLARE( py_setVelocitySource )
	
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, affectPosition, affectPosition )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, affectVelocity, affectVelocity )
	//@}
private:
	JitterPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyJitterPSA )

#ifdef CODE_INLINE
#include "jitter_psa.ipp"
#endif

#endif


/* jitter_psa.hpp */
