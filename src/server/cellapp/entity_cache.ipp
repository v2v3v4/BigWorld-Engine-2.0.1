/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// entity_cache.ipp

#ifdef CODE_INLINE
#define INLINE	inline
#else
#define INLINE
#endif

const double BW_MAX_PRIORITY = DBL_MAX;

// -----------------------------------------------------------------------------
// Section: EntityCache methods
// -----------------------------------------------------------------------------

/**
 *	Default constructor, for dummy entity caches only!
 *
 *	This should not be called if you want to put the EntityCache into the
 *	AoI map - EntityCacheMap's insert function is the only way to do that.
 */
INLINE EntityCache::EntityCache( EntityID dummyID ) :
	// These values are streamed off so we don't really need to set them here.
	lastEventNumber_( 0 ),
	lastVolatileUpdateNumber_( 0 ),
	detailLevel_( 0 ),
	idAlias_( NO_ID_ALIAS )
{
	lodEventNumbers_[0] = 0;

	this->dummyID( dummyID );
}


/**
 *	This contructor is only used in EntityCacheMap::find.
 */
INLINE EntityCache::EntityCache( const Entity * pEntity ) :
	pEntity_( pEntity )
{
	this->construct();
}


/**
 *	Destructor, called on dummy entity caches only!
 *
 *	Normal entity caches only get their base class's destructor called,
 *	but since the EntityPtr smart pointer is in the base class that's ok.
 */
INLINE EntityCache::~EntityCache()
{
}


/**
 *	Standard init method (called with EntityPtr already set)
 *
 *	This is called after a new EntityCache (whose ordinary constructor has
 *	not been called) has been inserted into an EntityCacheMap.
 */
INLINE void EntityCache::construct()
{
	flags_ = 0;
	updateSchemeID_ = 0;

	if (this->pEntity() != NULL)
	{
		lastEventNumber_ = this->pEntity()->lastEventNumber();
		lastVolatileUpdateNumber_ = this->pEntity()->volatileUpdateNumber() - 1;
		detailLevel_ = this->numLoDLevels();
		updateSchemeID_ = this->pEntity()->aoiUpdateSchemeID();
	}
	else
	{
		lastEventNumber_ = 0;
		lastVolatileUpdateNumber_= 0;
		detailLevel_ = 0;
	}

	idAlias_ = NO_ID_ALIAS;

	MF_ASSERT( detailLevel_ <= MAX_LOD_LEVELS );

	for (DetailLevel i = 0; i < detailLevel_; i++)
		lodEventNumbers_[i] = 0;
}




/**
 *	This method updates the priority associated with this cache.
 */
INLINE float EntityCache::updatePriority( const Vector3 & origin )
{
	// TODO: The use of a double precision floating for the priority
	//		values gives 52 bits for significant figures.
	//
	//		If we increment the priority values by 10000 a second, we will
	//		run into trouble in about 140 years (provided the avatar doesn't
	//		change cells in that time), so we probably won't need to reset
	//		the priority values.
	//
	//		At present, we are incrementing priority values at around
	//		1000 per second - which assumes that everyone is roughly 500
	//		metres from the avatar.
	//
	// PM:	One solution to this would be to use an integer value for the
	//		priority and use a comparison that wraps. e.g. (x - y) > 0. This
	//		would work if the range in the priorities never exceeds half the
	//		range of the integer type.

	float diffX = this->pEntity()->position().x - origin.x;
	float diffZ = this->pEntity()->position().z - origin.z;
	const Priority distSQ = diffX * diffX + diffZ * diffZ;

	float delta = AoIUpdateSchemes::apply( updateSchemeID_, sqrtf( distSQ ) );

	// MF_ASSERT( priority_ < BW_MAX_PRIORITY );
	priority_ += delta;
	// MF_ASSERT( priority_ < BW_MAX_PRIORITY );

	return distSQ;
}


/**
 *	This method returns the number of LoD levels associated with this cache.
 */
INLINE int EntityCache::numLoDLevels() const
{
	MF_ASSERT( this->pEntity() );
	return EntityCache::numLoDLevels( *this->pEntity() );
}

/**
 *	Static method to return number of lod levels for an entity
 */
INLINE int EntityCache::numLoDLevels( const Entity & e )
{
	return e.pType()->description().lodLevels().size();
}



/**
 *	This method returns the priority associated with the entity.
 */
INLINE EntityCache::Priority EntityCache::priority() const
{
	return priority_;
}


/**
 *	This method sets the priority associated with the entity.
 */
INLINE
void EntityCache::priority( Priority newPriority )
{
	MF_ASSERT( fabs(newPriority) < BW_MAX_PRIORITY );
	priority_ = newPriority;
}


/**
 *	This method returns the id of the entity that this cache should have been
 *	pointing to. This should only be called when we do not have an entity. It
 *	also means that the life of this entity cache will be short.
 */
INLINE
EntityID EntityCache::dummyID() const
{
	MF_ASSERT( !this->pEntity() );
	return dummyID_;
}


/**
 *	This method sets the id of the entity that this cache should have been
 *	pointing to. This should only be called when we do not have an entity. It
 *	also means that the life of this entity cache will be short.
 */
INLINE
void EntityCache::dummyID( EntityID dummyID )
{
	MF_ASSERT( !this->pEntity() );
	dummyID_ = dummyID;
}


/**
 *	This method sets the number of the last event that was considered when the
 *	associated entity was updated.
 */
INLINE
void EntityCache::lastEventNumber( EventNumber number )
{
	lastEventNumber_ = number;
}


/**
 *	This method returns the number of the last event that was considered when
 *	the associated entity was updated.
 */
INLINE
EventNumber EntityCache::lastEventNumber() const
{
	return lastEventNumber_;
}


/**
 *	This method sets the volatile number associated with the last update.
 */
INLINE
void EntityCache::lastVolatileUpdateNumber( VolatileNumber number )
{
	lastVolatileUpdateNumber_ = number;
}


/**
 *	This method sets the volatile number associated with the last update.
 */
INLINE
VolatileNumber EntityCache::lastVolatileUpdateNumber() const
{
	return lastVolatileUpdateNumber_;
}


/**
 *	This method sets the current detail level this cache is at.
 */
INLINE void EntityCache::detailLevel( DetailLevel detailLevel )
{
	detailLevel_ = detailLevel;
}


/**
 *	This method sets the current detail level this cache is at.
 */
INLINE DetailLevel EntityCache::detailLevel() const
{
	return detailLevel_;
}


/**
 *	This method returns the id alias that is used for this entity.
 */
INLINE IDAlias EntityCache::idAlias() const
{
	return idAlias_;
}


/**
 *	This method sets the id alias that is used for this entity.
 */
INLINE void EntityCache::idAlias( IDAlias idAlias )
{
	idAlias_ = idAlias;
}


/**
 *	This method is used to stamp a cache level with the last event number that
 *	sent from it.
 *
 *	@see lodEventNumber
 */
INLINE void EntityCache::lodEventNumber( int level, EventNumber eventNumber )
{
	MF_ASSERT( 0 <= level && level < this->numLoDLevels() );
	lodEventNumbers_[ level ] = eventNumber;
}


/**
 *	This method is used get the event number a cache level was last stamped
 *	with.
 *
 *	@see lodEventNumber
 */
INLINE EventNumber EntityCache::lodEventNumber( int level ) const
{
	MF_ASSERT( 0 <= level && level < this->numLoDLevels() );
	return lodEventNumbers_[ level ];
}

// entity_cache.ipp
