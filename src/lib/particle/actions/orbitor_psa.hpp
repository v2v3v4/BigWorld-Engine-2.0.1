/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ORBITOR_PSA_HPP
#define ORBITOR_PSA_HPP

#include "particle_system_action.hpp"

/**
 *	This action orbits particles around a point. It does not use Newtonian
 *	physics - it just displaces the particle an angle around the point in
 *	space.
 */
class OrbitorPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	OrbitorPSA( float x = 0.0f, float y = 0.0f, float z = 0.0f,
		float newVelocity = 0.0f );
	OrbitorPSA( const Vector3 &newPoint, float newVelocity = 0.0f );	
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Accessors to OrbitorPSA properties.
	//@{
	const Vector3 &point( void ) const;
	void point( const Vector3 &newVector );

	float angularVelocity( void ) const;
	void angularVelocity( float newVelocity );

	bool affectVelocity( void ) const;
	void affectVelocity( bool flag );
	//@}

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(OrbitorPSA); }
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Auxiliary Variables for the OrbitorPSA.
	//@{
	static int typeID_;			///< TypeID of the OrbitorPSA.

	Vector3 point_;				///< Centre of the orbitor.
	float angularVelocity_;		///< Angular velocity of orbit in radians.
	bool affectVelocity_;		///< If true, applies rotation to velocity.
	std::vector<Vector3> indexedOrbits_;
	//@}
};

typedef SmartPointer<OrbitorPSA> OrbitorPSAPtr;


/*~ class Pixie.PyOrbitorPSA
 *
 *	PyOrbitorPSA is a PyParticleSystemAction that causes particles to orbit
 *	around a point.
 *
 *	A new PyOrbitorPSA is created using Pixie.OrbitorPSA function.
 */
class PyOrbitorPSA : public PyParticleSystemAction
{
	Py_Header( PyOrbitorPSA, PyParticleSystemAction )
public:
	PyOrbitorPSA( OrbitorPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Accessors to OrbitorPSA properties.
	//@{
	const Vector3 &point( void ) const;
	void point( const Vector3 &newVector );

	float angularVelocity( void ) const;
	void angularVelocity( float newVelocity );

	bool affectVelocity( void ) const;
	void affectVelocity( bool flag );
	//@}

	///	@name Python Interface to the OrbitorPSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, point, point )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, angularVelocity, angularVelocity )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, affectVelocity, affectVelocity )
	//@}
private:
	OrbitorPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyOrbitorPSA )


#ifdef CODE_INLINE
#include "orbitor_psa.ipp"
#endif

#endif


/* orbitor_psa.hpp */
