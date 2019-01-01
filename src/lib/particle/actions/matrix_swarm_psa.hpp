/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATRIX_SWARM_PSA_HPP
#define MATRIX_SWARM_PSA_HPP

#include "particle_system_action.hpp"

typedef std::vector< MatrixProviderPtr > Matrices;

/**
 *	This class causes particles to "swarm" around a list of targets. This
 *	ParticleSystemAction causes the particle to have is position transformed
 *	by the matrix of one of the targets rather than the source of the
 *	particle system. The particles are divided evenly among the targets.
 */
class MatrixSwarmPSA : public ParticleSystemAction
{
public:
	MatrixSwarmPSA();
	~MatrixSwarmPSA();

    ParticleSystemActionPtr clone() const;

	///	 Accessors to MatrixSwarmPSA properties.
	//@{
	Matrices& targets( void );
	//@}

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(MatrixSwarmPSA) + sizeof(std::vector< MatrixProvider * >) * targets_.capacity() + sizeof(PySTLSequenceHolder<Matrices>); }
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load) {}

private:
	///	@name Auxiliary Variables for the MatrixSwarmPSA.
	//@{
	static int typeID_;			///< TypeID of the MatrixSwarmPSA.
	Matrices						targets_;
	//@}
};

typedef SmartPointer<MatrixSwarmPSA> MatrixSwarmPSAPtr;


/*~ class Pixie.PyMatrixSwarmPSA
 *
 *	PyMatrixSwarmPSA causes particles to "swarm" around a list of targets. This
 *	PyParticleSystemAction causes the particle to have is position transformed
 *	by the matrix of one of the targets rather than the source of the
 *	particle system. The particles are divided evenly among the targets.
 *
 *	A new PyMatrixSwarmPSA is created using Pixie.MatrixSwarmPSA function.
 */
class PyMatrixSwarmPSA : public PyParticleSystemAction
{
	Py_Header( PyMatrixSwarmPSA, PyParticleSystemAction )
public:
	PyMatrixSwarmPSA( MatrixSwarmPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Python Interface to the PyMatrixSwarmPSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ATTRIBUTE_DECLARE( targetHolders_, targets )
	//@}
private:
	MatrixSwarmPSAPtr pAction_;

	PySTLSequenceHolder<Matrices> targetHolders_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyMatrixSwarmPSA )


void MatrixProviderPtr_release( MatrixProvider *&pCamera );
/*
 *	Wrapper to let the C++ vector of matrices work as a vector/list in Python.
 */
namespace PySTLObjectAid
{
	/// Releaser is same for all PyObject's
	template <> struct Releaser< MatrixProvider * >
	{
		static void release( MatrixProvider *&pMatrix )
        {
        	MatrixProviderPtr_release( pMatrix );
        }
	};
}


#ifdef CODE_INLINE
#include "matrix_swarm_psa.ipp"
#endif

#endif


/* matrix_swarm_psa.hpp */
