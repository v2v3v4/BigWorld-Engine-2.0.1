/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SINK_PSA_HPP
#define SINK_PSA_HPP

#include "particle_system_action.hpp"
#include "math/boundbox.hpp"

/**
 *	This action destroys particles. The sink PSA can remove particles that
 *	are over a certain age. It can also remove particles that are over or
 *	under a certain speed.
 *
 *	All of its properties can be set simultaneously - that is, the sink PSA
 *	can be set to remove old particles and slow particles at the same time.
 */
class SinkPSA : public ParticleSystemAction
{
public:
	///	@name Constructor(s) and Destructor.
	//@{
	SinkPSA( float maximumAge = -1.0f, float minimumSpeed = -1.0f );	
	//@}

    ParticleSystemActionPtr clone() const;

	///	@name Accessors to SinkPSA properties.
	//@{
	float maximumAge( void ) const;
	void maximumAge( float value );

	float minimumSpeed( void ) const;
	void minimumSpeed( float timeInSeconds );

	bool outsideOnly( void ) const;
	void outsideOnly( bool outside );
	//@}

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(SinkPSA); }
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	bool isIndoors( const Vector3& pos, float radius ) const;

	///	@name Properties of the SinkPSA.
	//@{
	float maximumAge_;			///< Maximum age of particles in seconds.
	float minimumSpeed_;		///< Minimum speed in metres per seconds.
	bool outsideOnly_;			///< kill particles that travel indoors.
	void cacheHullInfo(const BoundingBox& wvbb);///< Optimisation for outsideOnly_
	void clearHullInfo();		///< Optimisation for outsideOnly_
	void accumulateOverlappers( const BoundingBox& bb, class Chunk* outsideChunk );
	std::set<class Chunk*> shells_;	///< Optimisation for outsideOnly_
	//@}

	///	@name Auxiliary Variables for the SinkPSA.
	//@{
	static int typeID_;			///< TypeID of the SinkPSA.
	//@}
};

typedef SmartPointer<SinkPSA> SinkPSAPtr;


/*~ class Pixie.PySinkPSA
 *
 *	PySinkPSA is a PyParticleSystemAction that destroys particles within
 *	a ParticleSystem. Particles can be destroyed if they are over a certain
 *	age or under or over a particular speed.
 *
 *	A new PySinkPSA is created using Pixie.SinkPSA function.
 */
class PySinkPSA : public PyParticleSystemAction
{
	Py_Header( PySinkPSA, PyParticleSystemAction )
public:
	PySinkPSA( SinkPSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	///	@name Accessors to PySinkPSA properties.
	//@{
	float maximumAge( void ) const;
	void maximumAge( float value );

	float minimumSpeed( void ) const;
	void minimumSpeed( float value );

	bool outsideOnly( void ) const;
	void outsideOnly( bool value );
	//@}

	///	@name Python Interface to the PySinkPSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, maximumAge, maximumAge )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, minimumSpeed, minimumSpeed )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, outsideOnly, outsideOnly )
	//@}
private:
	SinkPSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PySinkPSA )


#ifdef CODE_INLINE
#include "sink_psa.ipp"
#endif

#endif


/* sink_psa.hpp */
