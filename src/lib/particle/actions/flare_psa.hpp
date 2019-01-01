/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLARE_PSA_HPP
#define FLARE_PSA_HPP

#include "particle_system_action.hpp"
#include "romp/lens_effect.hpp"

/**
 *	This class finds the oldest particle, and draws a lens flare.
 */
class FlarePSA : public ParticleSystemAction
{
public:
	FlarePSA::FlarePSA( const std::string& flareName = "",
		int step = 0, int colour = 0, int size = 0 );

	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );

    ParticleSystemActionPtr clone() const;

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(FlarePSA); }
	//@}

	///	@name Accessors to FlarePSA properties.
	//@{
	const std::string & flareName();
	void flareName( const std::string & name );

	int flareStep();
	void flareStep( int step );

	bool colourize();
	void colourize( bool colour );

	bool useParticleSize();
	void useParticleSize( bool size );
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Auxiliary Variables for the FlarePSA.
	//@{
	static int typeID_;			///< TypeID of the FlarePSA.

	LensEffect le_;
	std::string flareName_;
	void loadLe( void );		// perform the load of the lens effect

	int			flareStep_;
	bool		colourize_;
	bool		useParticleSize_;
	//@}
};

typedef SmartPointer<FlarePSA> FlarePSAPtr;


/*~ class Pixie.PyFlarePSA
 *	PyFlarePSA is a PyParticleSystemAction that draws a lens flare at
 *	the location of one or more particles (either the oldest particle, all
 *	particles or every n particles). The flare resource used to draw the flare
 *	is specified when the PyFlarePSA is created using the FlarePSA factory
 *	function in the Pixie module.
 */
class PyFlarePSA : public PyParticleSystemAction
{
	Py_Header( PyFlarePSA, PyParticleSystemAction )
public:
	PyFlarePSA( FlarePSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	const std::string & flareName();
	void flareName( const std::string & name );

	int flareStep();
	void flareStep( int step );

	bool colourize();
	void colourize( bool colour );

	bool useParticleSize();
	void useParticleSize( bool size );

	///	@name Python Interface to the PyCollidePSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, flareName, flareName );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, flareStep, flareStep );	//0 == leader only, 1 == all, 2..x == every X particles do a flare
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, colourize, colourize );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, useParticleSize, useParticleSize );
	//@}
private:
	FlarePSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyFlarePSA )


#ifdef CODE_INLINE
#include "flare_psa.ipp"
#endif

#endif


/* flare_psa.hpp */
