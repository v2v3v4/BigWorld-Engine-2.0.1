/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORCE_PSA_HPP
#define FORCE_PSA_HPP

#include "particle_system_action.hpp"

// Forward Class Declarations.
class ParticleSystem;
class VectorGenerator;


/*~ class Pixie.PyForcePSA
 *	PyForcePSA is a PyParticleSystemAction that applies a constant acceleration to
 *	particles in a particular direction.
 *
 *	A new PyForcePSA is created using Pixie.ForcePSA function.
 */

/**
 *	This action acts as a force to the particles. The force PSA is the
 *	Newtonian concept of a force, that is, it applies an acceleration to
 *	the particles along a particular direction.
 *
 */
class ForcePSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s)
	//@{
	ForcePSA( float x = 0.0f, float y = 0.0f, float z = 0.0f );
	ForcePSA( const Vector3 &newVector );
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Accessors for the ForcePSA.
	//@{
	const Vector3 &vector( void ) const;
	void vector( const Vector3 &newVector );
	//@}

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(ForcePSA); }
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Auxiliary Variables for the ForcePSA.
	//@{
	static int typeID_;		///< TypeID of the ForcePSA.

	Vector3 vector_;		///< The vector describing the force.
	//@}
};

typedef SmartPointer<ForcePSA> ForcePSAPtr;


class PyForcePSA : public PyParticleSystemAction
{
	Py_Header( PyForcePSA, PyParticleSystemAction )
public:
	PyForcePSA( ForcePSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Accessors for the PyForcePSA.
	//@{
	const Vector3 &vector( void ) const;
	void vector( const Vector3 &newVector );
	//@}

	///	@name Python Interface to the PyCollidePSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()
	
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, vector, vector )
	//@}
private:
	ForcePSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyForcePSA )


#ifdef CODE_INLINE
#include "force_psa.ipp"
#endif

#endif


/* force_psa.hpp */
