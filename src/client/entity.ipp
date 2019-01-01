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
 *	Set just the position.
 */
INLINE void Entity::position( const Position3D & pos )
{
	BW_GUARD;
	position_ = pos;

	this->shuffle();
}


/**
 *	Set the position and associated data.
 */
INLINE void Entity::pos( const Position3D & localPos,
	const float * auxVolatile, int numAux,  const Vector3 & velocity )
{
	BW_GUARD;

	IF_NOT_MF_ASSERT_DEV( -100000.f < localPos.x && localPos.x < 100000.f &&
		-100000.f < localPos.z && localPos.z < 100000.f )
	{
		ERROR_MSG( "Entity::pos: Tried to move entity %u to bad "
			"translation: ( %f, %f, %f )\n",
			id_, localPos.x, localPos.y, localPos.z);
		// Don't try and be at an invalid position
		return;
	}

	position_ = localPos;
	Vector3 dir = Vector3::zero();
	for (int i=0; i<numAux; i++) dir[i] = auxVolatile[i];

	if (pVehicle_ != NULL)
	{
		pVehicle_->transformVehicleToCommon( position_, dir );
	}
	else if (pSpace_)
	{
		pSpace_->transformSpaceToCommon( position_, dir );
	}

	for (int i=0; i<numAux; i++)
	{
		auxVolatile_[i] = dir[i];
	}
	
	velocity_ = velocity;

	this->shuffle();
}


/**
 *	Return the primary model, is present and a model.
 */
INLINE PyModel * Entity::pPrimaryModel() const
{
	BW_GUARD;
	if (primaryEmbodiment_ && PyModel::Check( &**primaryEmbodiment_ ))
		return (PyModel*)&**primaryEmbodiment_;
	return NULL;
}

/**
 *	This method transforms a point in the coordinate system used by passengers
 *	that have us as their vehicle into the coordinate system of common space.
 */
INLINE
void Entity::transformVehicleToCommon( Vector3 & pos, Vector3 & dir ) const
{
	Matrix m;
	m.setRotate( auxVolatile_[0], auxVolatile_[1], auxVolatile_[2] );
	pos = m.applyVector( pos ) + position_;
	dir[0] += auxVolatile_[0];
	//dir[1] += auxVolatile_[1];
	//dir[2] += auxVolatile_[2];
}

/**
 *	This method transforms a point in the coordinate system of common space
 *	into the coordinate system used by passengers that have us as their vehicle.
 */
INLINE
void Entity::transformCommonToVehicle( Vector3 & pos, Vector3 & dir ) const
{
	Matrix m;
	m.setRotateInverse( auxVolatile_[0], auxVolatile_[1], auxVolatile_[2] );
	pos = m.applyVector( pos - position_ );
	dir[0] -= auxVolatile_[0];
	//dir[1] -= auxVolatile_[1];
	//dir[2] -= auxVolatile_[2];
}


/* entity.ipp */
