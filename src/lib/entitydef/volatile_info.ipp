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
#define INLINE    inline
#else
/// INLINE macro.
#define INLINE
#endif

/**
 *	This method returns what volatile direction info should be sent.
 *	@return 0 = Yaw, pitch and roll
 *			1 = Yaw, pitch
 *			2 = Yaw
 *			3 = No direction
 *
 *	@note This assumes that the VolatileInfo is valid. That is, the direction
 *		priorities are in descending order.
 */
INLINE int VolatileInfo::dirType( float priority ) const
{
	return (priority > yawPriority_) +
		(priority > pitchPriority_) +
		(priority > rollPriority_);
}


/**
 *	This method returns whether or not this object has volatile data to send.
 */
INLINE bool VolatileInfo::hasVolatile( float priority ) const
{
	return (priority < positionPriority_) || (priority < yawPriority_);
}


/**
 *	This function implements the equality operator for VolatileInfo.
 */
INLINE bool operator==( const VolatileInfo & volatileInfo1, 
		const VolatileInfo & volatileInfo2 )
{
	return volatileInfo1.positionPriority() == volatileInfo2.positionPriority() &&
		volatileInfo1.yawPriority() == volatileInfo2.yawPriority() &&
		volatileInfo1.pitchPriority() == volatileInfo2.pitchPriority() &&
		volatileInfo1.rollPriority() == volatileInfo2.rollPriority();
}


/**
 *	This function implements the inequality operator for VolatileInfo.
 */
INLINE bool operator!=( const VolatileInfo & volatileInfo1,
		const VolatileInfo & volatileInfo2 )
{
	return !(volatileInfo1 == volatileInfo2);
}

// volatile_info.ipp
