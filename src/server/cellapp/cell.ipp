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
#define INLINE
#endif

// -----------------------------------------------------------------------------
// Section: Cell
// -----------------------------------------------------------------------------

/**
 *	This method returns the number of real entities on this cell.
 */
INLINE
int Cell::numRealEntities() const
{
	return realEntities_.size();
}


/**
 *	This method returns a map of the real entities on this cell. The map is
 *	keyed by their IDs.
 */
INLINE
Cell::Entities & Cell::realEntities()
{
	return realEntities_;
}

// cell.ipp
