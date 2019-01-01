/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MAGNET_PSA_HPP
#define MAGNET_PSA_HPP

#include "particle_system_action.hpp"

/**
 *	This action acts as a magnet to the particles. The magnet PSA is the
 *	like a black hole, it applies an acceleration to
 *	the particles towards a particular point, based on distanceSq
 */
class MagnetPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	MagnetPSA( float s = 1.f );
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(MagnetPSA); }
	//@}

	/// Accessors to MagnetPSA properties.
	//@{
	void strength( float s );
	float strength() const;

	void minDist( float s );
	float minDist() const;

	void source( MatrixProviderPtr s );
	MatrixProviderPtr source() const;
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Auxiliary Variables for the MagnetPSA.
	//@{
	static int typeID_;		///< TypeID of the MagnetPSA.

	float strength_;		///< The strength of the magnet.
	float minDist_;			///< To avoid singularities in the magnetic field.
	MatrixProviderPtr source_;///< The origin of the magnetic field.
	//@}
};

typedef SmartPointer<MagnetPSA> MagnetPSAPtr;


/*~ class Pixie.PyMagnetPSA
 *
 *	The PyMagnetPSA PyParticleSystemAction sets up an acceleration on particles
 *	towards a particular point (magnitude based upon the square of the distance
 *	from that point).
 *
 *	A new PyMagnetPSA is created using Pixie.MagnetPSA function.
 *
 */
class PyMagnetPSA : public PyParticleSystemAction
{
	Py_Header( PyMagnetPSA, PyParticleSystemAction )
public:
	PyMagnetPSA( MagnetPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	/// Accessors to PyMagnetPSA properties.
	//@{
	void strength( float s );
	float strength() const;

	void minDist( float s );
	float minDist() const;

	void source( MatrixProviderPtr s );
	MatrixProviderPtr source() const;
	//@}

	///	@name Python Interface to the PyCollidePSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, strength, strength )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( MatrixProviderPtr, source, source )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, minDist, minDist )
	//@}
private:
	MagnetPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyMagnetPSA )


#ifdef CODE_INLINE
#include "magnet_psa.ipp"
#endif

#endif


/* magnet_psa.hpp */
