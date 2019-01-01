/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NODE_CLAMP_PSA_HPP
#define NODE_CLAMP_PSA_HPP

#include "particle_system_action.hpp"

/**
 *	This action clamps the particle to a node. The node clamp PSA effectively
 *	moves the particle by the same displacement done to the node in the same
 *	update.
 */
class NodeClampPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	NodeClampPSA();	
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(NodeClampPSA); }
	//@}

	///	 Accessors to NodeClampPSA properties.
	//@{
	bool fullyClamp() const;
	void fullyClamp( bool f );
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	///	@name Auxiliary Variables for the NodeClampPSA.
	//@{
	static int typeID_;			///< TypeID of the NodeClampPSA.

	Vector3 lastPositionOfPS_;	///< Previous position of the particle system.
	bool firstUpdate_;			///< If true, need to ignore lastPositionOfPS.
	bool fullyClamp_;			///< If true( default ), does not retain relative positions.
	//@}
};

typedef SmartPointer<NodeClampPSA> NodeClampPSAPtr;


/*~ class Pixie.PyNodeClampPSA
 *
 *	PyNodeClampPSA is a PyParticleSystemAction that locks a particle to the Node
 *	the particle system is attached to. Any motion or change in position of the
 *	Node is also appied to the particles in the particle system.
 *
 *	A new PyNodeClampPSA is created using Pixie.NodeClampPSA function.
 */
class PyNodeClampPSA : public PyParticleSystemAction
{
	Py_Header( PyNodeClampPSA, PyParticleSystemAction )
public:
	PyNodeClampPSA( NodeClampPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	 Accessors to PyNodeClampPSA properties.
	//@{
	bool fullyClamp() const;
	void fullyClamp( bool f );
	//@}

	///	@name Python Interface to the PyCollidePSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, fullyClamp, fullyClamp )
	
	PY_RO_ATTRIBUTE_DECLARE( typeID(), typeID )
	//@}
private:
	NodeClampPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyNodeClampPSA )


#ifdef CODE_INLINE
#include "node_clamp_psa.ipp"
#endif

#endif


/* node_clamp_psa.hpp */
