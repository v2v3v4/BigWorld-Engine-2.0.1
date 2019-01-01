/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef META_PARTICLE_SYSTEM_HPP
#define META_PARTICLE_SYSTEM_HPP

// Standard MF Library Headers.
#include "particle_system.hpp"
#include "cstdmf/smartpointer.hpp"
#include "particle.hpp"
#include "math/boundbox.hpp"

#ifdef EDITOR_ENABLED
#include "gizmo/meta_data.hpp"
#endif//EDITOR_ENABLED

class MetaParticleSystem;
typedef SmartPointer<MetaParticleSystem> MetaParticleSystemPtr;


/**
 *	This class is a container of a number of ParticleSystems
 *
 *	The MetaParticleSystem can contain several ParticleSystem objects, 
 *	allowing them to function as a single object.
 */
class MetaParticleSystem : public ReferenceCount
{
public:
	/// A collection of ParticleSystem
	typedef std::vector<ParticleSystemPtr> ParticleSystems;

	/// @name Constructor(s) and Destructor.
	//@{
	MetaParticleSystem();
	~MetaParticleSystem();
	//@}

    // Create a deep copy (the particle systems are also cloned) of the 
    // metaparticle system.
    MetaParticleSystemPtr clone() const;	

	// Return the resources required for a MetaParticleSystem
	static void prerequisites( DataSectionPtr pSection, std::set<std::string>& output );
	static bool isParticleSystem( const std::string& file );

	/// @name General Operations on Particle Systems.
	//@{
	bool tick( float dTime );
	void draw( const Matrix & worldTransform, float lod );
	virtual void localBoundingBox( BoundingBox & bb );
	virtual void localVisibilityBoundingBox( BoundingBox & bb );
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world );
	virtual void worldVisibilityBoundingBox( BoundingBox & bb );
	virtual bool attach( MatrixLiaison * pOwnWorld );
	virtual void detach();	
	virtual void tossed( bool isOutside );
	virtual void enterWorld();
	virtual void leaveWorld();
	virtual void move( float dTime );

	const Matrix & worldTransform( void );
	
	bool load(DataSectionPtr pDS, const std::string & filename);
	bool load( const std::string& filename,
		const std::string& directory = "particles" );

	void save( const std::string& filename,
		const std::string& directory = "particles",
        bool transient = false ); // state saving in xml format
	void reSerialise( DataSectionPtr sect, bool load = true, 
        bool transient = false );

	void addSystem( ParticleSystemPtr newSystem );
    void insertSystem( size_t idx, ParticleSystemPtr newSystem );
	void removeSystem( ParticleSystemPtr system );
	void removeAllSystems( void );

	ParticleSystemPtr system( const char * name );
	ParticleSystemPtr systemFromIndex( size_t index );
	ParticleSystemPtr getFirstSystem() { return (systems_.at(0)); }
	ParticleSystems & systemSet() { return systems_; }

	int size( void );
	int capacity( void );

	void spawn( int num=1 );	// force all the sources to create their force particle set
	void clear();				// delegate clear to all particle systems

	void isStatic( bool s );
	void setDoUpdate();
	void setFirstUpdate();

	size_t sizeInBytes() const;
	//@}


	/// @name for the editor
	//@{
	void transformBoundingBox(const Matrix& trans);
	//@}

	// Gets the largest duration of a MetaParticleSystem -1 
	// if it goes on forever.
	float	duration() const;

	// Returns the number of particlesystems in the metaparticlesystem
	uint32 MetaParticleSystem::nSystems() const;

#ifdef EDITOR_ENABLED
	MetaData::MetaData&				metaData()			{	return metaData_;	}
	const MetaData::MetaData&		metaData()	const	{	return metaData_;	}
	bool							populateMetaData( DataSectionPtr ds );
#endif//EDITOR_ENABLED

private:
	ParticleSystems		systems_;

	/// Variables to make it look like this is a PyAttachment.  It is
	/// only used when we are wrapped by PyMetaParticleSystem
	MatrixLiaison	* pOwnWorld_;
	bool			attached_;
	bool			inWorld_;

#ifdef EDITOR_ENABLED
	MetaData::MetaData		metaData_;
#endif//EDITOR_ENABLED
};

typedef SmartPointer<MetaParticleSystem>	MetaParticleSystemPtr;


#endif // META_PARTICLE_SYSTEM_HPP

/* meta_particle_system.hpp */
