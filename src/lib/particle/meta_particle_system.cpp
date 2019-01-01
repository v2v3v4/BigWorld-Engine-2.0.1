/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#ifdef EDITOR_ENABLED
#pragma warning(disable: 4355)
#endif//EDITOR_ENABLED

#include "meta_particle_system.hpp"
#include "particle/actions/particle_system_action.hpp"
#include "particle/actions/source_psa.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/auto_config.hpp"
#include "gizmo/meta_data.hpp"
#include "moo/resource_load_context.hpp"

DECLARE_DEBUG_COMPONENT2( "Particle", 0 )


#ifdef EDITOR_ENABLED
static AutoConfigString s_mpsMetaDataConfig( "editor/metaDataConfig/metaParticleSystem", "helpers/meta_data/meta_particle_system.xml" );
#endif

/**
 *	This method checks to see if the provided file is a particle system.
 *
 *	@param file	The file to the potential particle system.
 */
/*static*/ bool MetaParticleSystem::isParticleSystem( const std::string& file )
{
	BW_GUARD;
	DataSectionPtr ds = BWResource::openSection( file );
	if ( !ds )
		return false; // file is not a datasection or doesn't exist

	// hardcoding particle's section names because of a lack of better way at 
	// the moment:
	for( int i = 0; i < ds->countChildren(); ++i )
	{
		DataSectionPtr child = ds->openChild( i );
		// finding standard sections in the file
		if ( child->sectionName() == "seedTime" ||
			child->findChild( "serialiseVersionData" ) != NULL )
		{
			return true;
		}
		
		// finding alternate section as done in ParticleSystem::load
		DataSectionPtr firstDS = child->openFirstSection();
		if ( firstDS != NULL &&
			firstDS->findChild( "serialiseVersionData" ) != NULL )
		{
			return true;
		}
	}

	return false;
}


/**
 *	This is the constructor for MetaParticleSystem.
 */
MetaParticleSystem::MetaParticleSystem():
	pOwnWorld_( NULL ),
	attached_( false ),
	inWorld_( false )
#ifdef EDITOR_ENABLED
	, metaData_( this )
#endif//EDITOR_ENABLED
{
}


/**
 *	This is the destructor for ParticleSystem.
 */
MetaParticleSystem::~MetaParticleSystem()
{
	BW_GUARD;
	removeAllSystems();
}


/**
 *	This method clones a meta particle system, but does not copy the current
 *	state of the particles.
 *
 *	@return MetaParticleSystemPtr	Smart Pointer to the cloned system.
 */
MetaParticleSystemPtr MetaParticleSystem::clone() const
{
    BW_GUARD;
	MetaParticleSystemPtr result = new MetaParticleSystem();
    for (size_t i = 0; i < systems_.size(); ++i)
    {
        result->addSystem(systems_[i]->clone());
    }

#ifdef EDITOR_ENABLED
	result->metaData() = metaData();
#endif//EDITOR_ENABLED

    return result;
}


/**
 *	This static method returns all of the resources that will be required to
 *	create the meta particle system.  It is designed so that the resources
 *	can be loaded in the loading thread before constructing the system.
 *
 *	@param	pDS			The data section containing the meta particle system.
 *	@param	output		The set of output resources that are prerequisites for
 *						creation of the meta particle system.
 */
/* static */ void MetaParticleSystem::prerequisites( DataSectionPtr pDS, std::set<std::string>& output )
{
	BW_GUARD;
	if (pDS)
	{
		if ( pDS->readString( "serialiseVersionData", "" ) != "" )
		{
			//is a particle system
			ParticleSystem::prerequisites( pDS, output );
		}
		else
		{
			//is a meta-particle system	
			for(DataSectionIterator it = pDS->begin(); it != pDS->end(); it++)
			{
				DataSectionPtr pSystemDS = *it;
				std::string systemName = pDS->unsanitise(pSystemDS->sectionName());
				if (systemName != "seedTime")
				{
					ParticleSystem::prerequisites( pSystemDS, output );					
				}
			}
		}
	}	
}


/**
 *	This method loads a meta particle system, given a filename and base
 *	directory.
 *	
 *	@param	filename			The filename for the particle system.
 *	@param	directory			Base directory path, prepended to filename.
 *
 *	@return bool				Success or failure of the load operation.
 */
bool MetaParticleSystem::load( const std::string& filename,
							  const std::string& directory )
{
	BW_GUARD;
	DataSectionPtr pDS = BWResource::openSection(directory + filename);
	return this->load(pDS, BWResource::removeExtension(filename));
}


/**
 *	This method loads a meta particle system, given a data section.
 *	
 *	@param	pDS					DataSection to load from. 
 *
 *	@return bool				Success or failure of the load operation.
 */
bool MetaParticleSystem::load(DataSectionPtr pDS, const std::string & name)
{
	BW_GUARD;
	if (!pDS)
		return false;

	bool ok = true;

	DiaryEntryPtr de = Diary::instance().add( "particle " + name );

	Moo::ScopedResourceLoadContext resLoadCtx( "particle " + BWResource::getFilename( name ) );

	// make sure empty
	removeAllSystems();

	if (pDS->openSection( "serialiseVersionData" ))
	{
		ParticleSystemPtr newSystem = new ParticleSystem();

		// this is in old format, there is only one system
		// read in the system properties
		ok = newSystem->load(pDS);
		newSystem->name(name);
		addSystem( newSystem );
	}
	else
	{
		// loop through the system descriptions
		for(DataSectionIterator it = pDS->begin(); it != pDS->end(); it++)
		{
			DataSectionPtr pSystemDS = *it;
			std::string systemName = pDS->unsanitise(pSystemDS->sectionName());
			if (systemName != "seedTime" && systemName != METADATA_SECTION_NAME)
			{
				ParticleSystemPtr newSystem = new ParticleSystem;

				// read in the system properties
				ok &= newSystem->serialise(pSystemDS, true, systemName);
				addSystem( newSystem );
			}
		}
	}

	// Call this as we may be attached before loading; we still want to
	// maintain the attachement to the constituent particle systems	
	MatrixLiaison* attachment = pOwnWorld_;
	if (attachment)
	{
		this->detach();
		this->attach( attachment );
	}

#ifdef EDITOR_ENABLED
	ok &= populateMetaData( pDS );
#endif//EDITOR_ENABLED

	de->stop();

	return ok;
}


#ifdef EDITOR_ENABLED

bool MetaParticleSystem::populateMetaData( DataSectionPtr ds )
{
	static MetaData::Desc dataDesc( s_mpsMetaDataConfig.value() );
	return metaData_.load( ds, dataDesc );
}

#endif//EDITOR_ENABLED

/**
 *	This method saves a meta particle system, to a given filename and base
 *	directory.
 *	
 *	@param	filename			The filename for the system.
 *	@param	directory			Base directory path, prepended to filename.
 *	@param	transient			Whether the save operation is transient. If
 *								set the false, the full visibility bounding
 *								box is recaculated for the save - this may
 *								take a few seconds.
 */
void MetaParticleSystem::save( const std::string& filename, 
    const std::string& directory, bool transient )
{
	BW_GUARD;
	// always save to the new format
	DataSectionPtr pDS = BWResource::openSection( directory + filename, true ); // create a new directory if need to

	// for now, just remember if there was a seed time in the file
	float seedTime = pDS->readFloat( "seedTime", -1.f );

	// delete children (as some may have been deleted by the user)
	pDS->delChildren();

	// loop through the systems defined
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		// save system properties in separate sections
		ParticleSystemPtr system = *it;
		std::string systemName = pDS->sanitise(system->name());

		DataSectionPtr systemDS = pDS->openSection(systemName, true);
		system->serialise(systemDS, false, systemName, transient);
	}

	if ( seedTime > -1.f )
	{
		pDS->writeFloat( "seedTime", seedTime );
	}

#ifdef EDITOR_ENABLED
	metaData_.save( pDS );
#endif//EDITOR_ENABLED

	// save cache to file
	pDS->save();
}


/**
 * This method is used internally by the editor for undo/redo.
 */
void MetaParticleSystem::reSerialise( DataSectionPtr pDS, bool load, bool transient)
{
	BW_GUARD;
	if (load)
	{
		//remove systems that no longer exist.
		ParticleSystems::iterator iter = systems_.begin();
		while ( iter != systems_.end() )
		{
			ParticleSystemPtr system = *iter;
			std::string systemName = system->name();

			if ( !pDS->findChild(systemName) )
			{
				iter = systems_.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		//add new systems
		DataSectionIterator it;
		for (it = pDS->begin(); it != pDS->end(); it++)
		{
			DataSectionPtr pSystemDS = *it;
			std::string systemName = pSystemDS->sectionName();
			ParticleSystemPtr ps = this->system( systemName.c_str() );
			if (!ps)
			{
				ps = new ParticleSystem();
				this->addSystem( ps );
			}
			ps->serialise(pSystemDS, load, systemName, transient);
		}
	}
	else
	{
		for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
		{
			// save system properties in separate sections
			ParticleSystemPtr system = *it;
			std::string systemName = system->name();

			DataSectionPtr systemDS = pDS->openSection(systemName, !load);
			IF_NOT_MF_ASSERT_DEV(systemDS)
			{
				continue;
			}

			// TODO: make a reSerialise for each system component
			system->serialise(systemDS, load, systemName, transient);
		}
	}
}


/**
 *	This method adds a system to the meta system.
 *
 *	@param system		A pointer to the system to be added.
 */
void MetaParticleSystem::addSystem(ParticleSystemPtr system)
{
	BW_GUARD;
	if (attached_)
	{
		system->detach();
		system->attach(pOwnWorld_);
	}

	systems_.push_back(system);
}


/**
 *	This method insterts a system to the meta system at the given index.
 *
 *	@param idx			Index at which the system should be added.
 *	@param system		A pointer to the system to be added.
 */
void MetaParticleSystem::insertSystem(size_t idx, ParticleSystemPtr system)
{
	BW_GUARD;
	if (attached_)
	{
		system->detach();
		system->attach(pOwnWorld_);
	}

	systems_.insert(systems_.begin() + idx, system);
}


/**
 *	This method removes a system from the meta system.
 *
 *	@param system		A pointer to the system to be removed.
 */
void MetaParticleSystem::removeSystem(ParticleSystemPtr system)
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		if (*it == system)
		{
			ParticleSystemPtr system = *it;
			system->detach();			
			systems_.erase(it);
			return;
		}
	}
}


/**
 *	This method removes all systems from the meta system.
 */
void MetaParticleSystem::removeAllSystems()
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		ParticleSystemPtr system = *it;
		system->detach();		
	}
	systems_.clear();
}


/**
 *	This method retrieves a system by name from the meta system.
 *
 *	@param	name					Name of the requested particle system.
 *	@return ParticleSystemPtr		Smart Pointer to the requested particle
 *									system if found, or NULL.
 */
ParticleSystemPtr MetaParticleSystem::system(const char * name)
{
	BW_GUARD;
	const std::string systemName(name);

	for (ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		if ((*it)->name() == systemName)
			return *it;
	}

	return NULL;
}


/**
 *	This method retrieves a system by index from the meta system.
 *
 *	@param	index					Index of the requested particle system.
 *	@return ParticleSystemPtr		Smart Pointer to the requested particle
 *									system if in range, or NULL.
 */
ParticleSystemPtr MetaParticleSystem::systemFromIndex( size_t index )
{
	BW_GUARD;
	if( index < this->systems_.size() )
		return this->systems_[ index ];
	else
		return NULL;
}


/**
 *	This method counts the number of particles currently active in the meta
 *	system.
 *
 *	@return int		The number of particles currently active.
 */
int MetaParticleSystem::size( void )
{
	BW_GUARD;
	int numberOfParticles = 0;
	for( ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		if ( (*it)->enabled() )
			numberOfParticles += (*it)->size();
	}
	return numberOfParticles;
}


/**
 *	This method returns the meta system's particle capacity.
 *
 *	@return int		The capacity of the meta system, or the maximum number
 *					of particles that can be active.
 */
int MetaParticleSystem::capacity( void )
{
	BW_GUARD;
	int capacityOfParticles = 0;
	for( ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		capacityOfParticles += (*it)->capacity();
	}
	return capacityOfParticles;
}


/**
 *	This method returns the memory used by the meta particle system.
 *
 *	@return size_t	Size, in bytes, used by the meta particle system.
 */
size_t MetaParticleSystem::sizeInBytes( void ) const
{
	BW_GUARD;
	size_t footprint = 0;
	for( ParticleSystems::const_iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		if ( (*it)->enabled() )
			footprint += (*it)->sizeInBytes();
	}
	return footprint;
}


/**
 *	This method ticks the meta particle system.
 *
 *	@param dTime	Time since last update in seconds.
 */
bool MetaParticleSystem::tick( float dTime )
{
	BW_GUARD;
	bool updated = false;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		updated |= (*it)->tick(dTime);
	}
	return updated;
}


/**
 *	This methods tells the particle system to draw itself.
 *
 *	Particles are stored in world coordinates so we do not need
 *	the attachment transform which is passed in.
 *
 *	@param	world	Unused world matrix from the parent attachment.
 *	@param	lod		Adjusted distance from camera, for level-of-detail use.
 */
void MetaParticleSystem::draw( const Matrix &world, float lod )
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		if ( (*it)->enabled() )
			(*it)->draw(world, lod);
	}
}


/**
 *	This accumulates our bounding box into the given variable.
 *
 *	@param	bb		The resultant bounding box.
 */
void MetaParticleSystem::localBoundingBox( BoundingBox & bb )
{
	BW_GUARD;	
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->localBoundingBox( bb );
	}	
}


/**
 *	This accumulates our visibility bounding box into the given variable.
 *
 *	@param	bb		The resultant visibility bounding box.
 */
void MetaParticleSystem::localVisibilityBoundingBox( BoundingBox & bb )
{
	BW_GUARD;	
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->localVisibilityBoundingBox( bb );
	}	
}


/**
 *	This accumulates our bounding box into the given matrix.
 *
 *	@param	bb		The resultant bounding box.
 */
void MetaParticleSystem::worldBoundingBox( BoundingBox & bb, const Matrix& world )
{
	BW_GUARD;	
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->worldBoundingBox( bb, world );
	}	
}


/**
 *	This accumulates our visibility bounding box into the given variable.
 *
 *	@param	bb		The resultant visibility bounding box.
 */
void MetaParticleSystem::worldVisibilityBoundingBox( BoundingBox & bb )
{
	BW_GUARD;	
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->worldVisibilityBoundingBox( bb );
	}	
}


/**
 *	This method attaches the meta particle system to the world via a
 *	matrix liason.  We give ourselves a ground specifier.
 *
 *	@param	pOwnWorld	The matrix liason that will provide our world matrix.
 *
 *	@returns	true on success, false on error.
 */
bool MetaParticleSystem::attach( MatrixLiaison * pOwnWorld )
{
	BW_GUARD;
	if (attached_) return false;

	attached_ = true;
	pOwnWorld_ = pOwnWorld;
	
	bool ret = true;

	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		ret |= (*it)->attach( pOwnWorld );
	}

	return ret;
}


/**
 *	This method detaches the particle system from the existing
 *	matrix liason, undoing the attach operation.
 */
void MetaParticleSystem::detach()
{
	BW_GUARD;
	attached_ = false;
	pOwnWorld_ = NULL;

	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->detach();
	}
}


/**
 *	The method is called when the particle system has first entered the world.
 *	Make sure to convert back to world coordinates when entering world.
 */
void MetaParticleSystem::enterWorld()
{
	BW_GUARD;
	inWorld_ = true;

	for (uint32 i=0; i<systems_.size(); i++)
		systems_[i]->enterWorld();
}


/**
 *	The method is called when the particle system is leaving the world.
 *	Make sure to convert back to local coordinates when leaving world.
 */
void MetaParticleSystem::leaveWorld()
{
	BW_GUARD;
	inWorld_ = false;

	for (uint32 i=0; i<systems_.size(); i++)
		systems_[i]->leaveWorld();
}


/**
 *	This method is called when we are tossed into the world.
 *
 *	@param	isOutside	Boolean indicating whether we are being placed outside.
 */
void MetaParticleSystem::tossed( bool isOutside )
{
	BW_GUARD;
	for (uint32 i=0; i<systems_.size(); i++)
		systems_[i]->tossed( isOutside );
}


/**
 *	This method is called when the particle system is moved via a
 *	chunk item operation.
 *
 *	@param	dTime		Delta frame time.
 */
void MetaParticleSystem::move( float dTime )
{
	BW_GUARD;
	for (uint32 i=0; i<systems_.size(); i++)
		systems_[i]->move(dTime);
}


/**
 *	This method spawns a number of particles on-demand.
 *
 *	@param	num		The number of particles to spawn.
 */
void MetaParticleSystem::spawn( int num )
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		ParticleSystem::Actions & actions =  (*it)->actionSet();

		for (ParticleSystem::Actions::iterator actionIt = actions.begin();
				actionIt != actions.end();
				actionIt++)
		{
			if ((*actionIt)->typeID() == PSA_SOURCE_TYPE_ID)
			{
				SourcePSA * source = (SourcePSA *)((*actionIt).getObject());
				source->force(num);
			}
		}
	}
}


/**
 *	This method tells the meta particle system to remove all particles from
 *	itself.
 */
void MetaParticleSystem::clear()
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->clear();
	}
}


/**
 *	This method explicitly forces a meta particle system to undergo an update
 *	next tick.  Usually, particle systems are only updated when they are
 *	visible.  In some cases, it is necessary or desriable to force an update
 *	even when the system is off-screen.
 */
void MetaParticleSystem::setDoUpdate()
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->setDoUpdate();
	}
}


/**
 *	This method sets the first update flag on all particle systems.
 */
void MetaParticleSystem::setFirstUpdate()
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->setFirstUpdate();
	}
}


/*
 *	Advise the particle system that it will always be stationary.
 *	Stationary systems take less time calculating their
 *	bounding box than dynamic (moving) particle systems.
 *
 *	@param	s		Whether the system is stationary or not.
 */
void MetaParticleSystem::isStatic( bool s )
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->isStatic( s );
	}
}


/*
 *	Advise the particle system bbox that it has been moved
 *	Only required for static particle systems moved through the editor.
 */
void MetaParticleSystem::transformBoundingBox(const Matrix& trans)
{
	BW_GUARD;
	for(ParticleSystems::iterator it = systems_.begin(); it != systems_.end(); it++)
	{
		(*it)->transformBoundingBox( trans );
	}
}


/**
 *	This method returns the duration of the particle system.  The
 *	duration is only valid for particle systems containing at least
 *	one sink action that specifies a maximum particle age.
 *
 *	@return float	The duration of the particle system, or -1 if
 *					the particle system has no fixed duration.
 */
float MetaParticleSystem::duration() const
{
	BW_GUARD;
	float duration = -1;
	ParticleSystems::const_iterator it = systems_.begin();
	while( it != systems_.end())
	{
		duration = max( (*it)->duration(), duration );
		++it;
	}

	return duration;
}


/**
 *	This method returns the number of particle systems that make up the
 *	meta particle system.
 *
 *	@return uint32	The number of particle systems contained within.
 */
uint32 MetaParticleSystem::nSystems() const
{
	return systems_.size();
}


// meta_particle_system.cpp
