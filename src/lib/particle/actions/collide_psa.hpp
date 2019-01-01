/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLLIDE_PSA_HPP
#define COLLIDE_PSA_HPP

#include "particle_system_action.hpp"

// Forward Class Declarations.
class ParticleSystem;

/**
 *	This class collides particles with the collision scene ( erk.. )
 */
class CollidePSA : public ParticleSystemAction
{
public:
	CollidePSA( float elasticity = 0.0f );

    ParticleSystemActionPtr clone() const;

	///	@name Overrides to the Particle System Action Interface.
	//@{
	void execute( ParticleSystem &particleSystem, float dTime );
	int typeID( void ) const;
	std::string nameID( void ) const;

	virtual size_t sizeInBytes() const { return sizeof(CollidePSA); }
	//@}

	///	@name Accessors to CollidePSA properties.
	//@{
	bool spriteBased() const;
	void spriteBased( bool s );

	float elasticity() const;
	void elasticity( float e );

	float minAddedRotation() const;
	void minAddedRotation( float e );

	float maxAddedRotation() const;
	void maxAddedRotation( float e );

	bool soundEnabled() const;
	void soundEnabled( bool e );

	const std::string& soundTag() const;
	void soundTag( const std::string& tag );

	int soundSrcIdx() const;
	void soundSrcIdx( int i );

	const std::string& soundProject() const;
	void soundProject( const std::string& project );

	const std::string& soundGroup() const;
	void soundGroup( const std::string& group );

	const std::string& soundName() const;
	void soundName( const std::string& sound );
	//@}

	static const std::string nameID_;

protected:
	void serialiseInternal(DataSectionPtr pSect, bool load);
	void updateSoundTag();

private:
	///	@name Auxiliary Variables for the CollidePSA.
	//@{
	static int typeID_;			///< TypeID of the CollidePSA.
	bool spriteBased_;
	float elasticity_;
	float minAddedRotation_;
	float maxAddedRotation_;
	int entityID_;
	std::string soundTag_;
	bool soundEnabled_;
	int soundSrcIdx_;

	std::string soundProject_;
	std::string soundGroup_;
	std::string soundName_;
	//@}
};

typedef SmartPointer<CollidePSA> CollidePSAPtr;


/*~ class Pixie.PyCollidePSA
 *
 *	PyCollidePSA is a PyParticleSystemAction that collides particles with the
 *	collision scene, based upon their position and velocity.
 *
 *	A new PyCollidePSA is created using Pixie.CollidePSA function.
 */
class PyCollidePSA : public PyParticleSystemAction
{
	Py_Header( PyCollidePSA, PyParticleSystemAction )
public:
	PyCollidePSA( CollidePSAPtr pAction, PyTypePlus *pType = &s_type_ );

	int typeID( void ) const;

	bool spriteBased() const;
	void spriteBased( bool s );

	float elasticity() const;
	void elasticity( float e );

	float minAddedRotation() const;
	void minAddedRotation( float e );

	float maxAddedRotation() const;
	void maxAddedRotation( float e );

	bool soundEnabled() const;
	void soundEnabled( bool e );

	const std::string& soundTag() const;
	void soundTag( const std::string& tag );

	int soundSrcIdx() const;
	void soundSrcIdx( int i );

	const std::string& soundProject() const;
	void soundProject( const std::string& bank );

	const std::string& soundGroup() const;
	void soundGroup( const std::string& group );

	const std::string& soundName() const;
	void soundName( const std::string& sound );

	///	@name Python Interface to the PyCollidePSA.
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, spriteBased, spriteBased );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, elasticity, elasticity );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, minAddedRotation, minAddedRotation );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, maxAddedRotation, maxAddedRotation );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, soundTag, soundTag );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, soundSrcIdx, soundSrcIdx );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, soundEnabled, soundEnabled );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, soundProject, soundProject );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, soundGroup, soundGroup );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, soundName, soundName );
	//@}
private:
	CollidePSAPtr pAction_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyCollidePSA )


#ifdef CODE_INLINE
#include "collide_psa.ipp"
#endif

#endif


/* collide_psa.hpp */
