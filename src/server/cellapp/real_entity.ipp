/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// real_entity.ipp


#ifdef CODE_INLINE
#define INLINE	inline
#else
#define INLINE
#endif

// -----------------------------------------------------------------------------
// Section: RealEntity
// -----------------------------------------------------------------------------


/**
 *	Constructor for the RealEntityMethod
 */
INLINE RealEntityMethod::RealEntityMethod( RealEntity * re, StaticGlue glueFn,
		PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pEntity_( &re->entity() ),
	glueFn_( glueFn )
{ }


// real_entity.ipp
