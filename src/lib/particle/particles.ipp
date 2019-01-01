/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 *	Constructor.
 */
INLINE ContiguousParticles::ContiguousParticles():
	particleIdx_( 0 )
{
}


/**
 *	This method clears the container of particles, leaving its capacity
 *	unchanged.
 */
INLINE void ContiguousParticles::clear()
{
	particleIdx_ = 0;
	BASE_CONTAINER::clear();
}


/**
 *	This method adds a newly created particle to the container.  It will 
 *  not be added if adding the particle pushes the particle population beyond 
 *  the capacity of the system.
 *
 *	@param particle		The new particle to be added.
 *	@param isMesh		Whether the new particle is mesh-style.
 */
INLINE
void ContiguousParticles::addParticle( const Particle& particle, bool isMesh )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( size() < capacity() )
	{
		MF_EXIT( "size >= capacity - possible memory corruption?" );
	}
	
	IF_NOT_MF_ASSERT_DEV( !isMesh )
	{
		MF_EXIT( "can't use ContiguousParticle containers with mesh particles" );
	}

	this->BASE_CONTAINER::push_back( particle );
	this->BASE_CONTAINER::operator[](size()-1).index(particleIdx_);
	
	//Save this since we need it for the flare effect
	lastParticleAdded_ = &(this->BASE_CONTAINER::operator [](size()-1));
	
	particleIdx_ = (particleIdx_+1)%capacity();
}


/**
 *	This method removes a particle from the particle system. The particle
 *	to be removed must be referenced via an iterator.
 *
 *	@param particle		The iterator to the particle to be removed.
 */
INLINE
ContiguousParticles::iterator ContiguousParticles::removeParticle(
	ContiguousParticles::iterator particle )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( this->size() > 0 )
	{
		return end();
	}

	//doesn't matter if we copy over ourselves - this only happens if
	//we erase the last particle in the vector
	*particle = back();
	
	//Need to hold on to what used to be the last element.
	lastParticleAdded_ = &(*particle);

	pop_back();

	//the passed in iterator is now the next valid one!
	//note if anyone saved the end iterator it will now be invalid.
	return particle;
}


INLINE
size_t ContiguousParticles::index( Particles::iterator particle )
{
	return particle->index();
}


/**
 *	Constructor.
 */
INLINE FixedIndexParticles::FixedIndexParticles():
	size_(0),
	next_(0)
{
}


/**
 *	This method adds a newly created particle to the container.  It will 
 *  not be added if adding the particle pushes the particle population beyond
 *  the capacity of the system.
 *
 *	@param particle		The new particle to be added.
 *	@param isMesh		Whether the new particle is mesh-style.
 */
INLINE
void FixedIndexParticles::addParticle( const Particle& particle, bool isMesh )
{
	BW_GUARD;
	int available = BASE_CONTAINER::size();
	
	if (size_ < available)
	{
		Particles& store = *this;

		//TODO :  This search is O(n) worst-case.  Possibly fix by maintaining
		//a separate store of free particle indices (which has 2*capacity bytes
		//worst-case memory usage).
		//For constant sink-times, the search is always O(1)
		//For random sink-times, the worst case is O(n) but on average will much less
		while ( store[next_].isAlive() )
		{
			next_ = (next_+1) % available;
		}

		store[next_] = particle;
		
		if (!isMesh)
			this->BASE_CONTAINER::operator[](next_).index(next_);

		++size_;		
		lastParticleAdded_ = &store[next_];
		next_ = (next_+1)%available;
	}
}


/**
 *	This method removes a particle from the particle system. The particle
 *	to be removed must be referenced via an iterator.
 *
 *	@param particle		The iterator to the particle to be removed.
 */
INLINE
FixedIndexParticles::iterator FixedIndexParticles::removeParticle(
	FixedIndexParticles::iterator particle )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( this->size() > 0 )
	{
		return end();
	}
	
	--size_;
	particle->kill();
	return ++particle;
}


INLINE
size_t FixedIndexParticles::index( Particles::iterator particle )
{
	return (particle - this->begin());
}


/**
 *	This method clears the container of particles, leaving its capacity
 *	unchanged.
 */
INLINE void FixedIndexParticles::clear()
{
	BW_GUARD;
	iterator it = BASE_CONTAINER::begin();
	iterator end = BASE_CONTAINER::end();
	while (it != end)
	{
		it->kill();
		++it;			
	}
	size_ = next_ = 0;
}


/**
 *	This method clears the container of particles, and changes its capacity
 *	to the new size.  For FixedIndexParticles, reserve acts more like a
 *	resize, all particles are now present and we use the kill() and isAlive()
 *	on particles to represent presence in the container or not.  However
 *	the container internally keeps track of it's own 'size', representing the
 *	number of live particles present.
 *
 *	@param	amount	The new size of the container.
 */
INLINE void FixedIndexParticles::reserve( size_t amount )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( amount > 0 )
	{
		return;
	}
	
	Particle p;
	BASE_CONTAINER::clear();
	Particles::reserve( amount );
	BASE_CONTAINER::resize( amount );
	size_ = next_ = 0;	
	IF_NOT_MF_ASSERT_DEV( amount == BASE_CONTAINER::size() )
	{
		MF_EXIT( "resize container failed" );
	}
}	
