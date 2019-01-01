/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VOLATILE_INFO_HPP
#define VOLATILE_INFO_HPP

#include "resmgr/datasection.hpp"

/**
 *	This class is used to describe what information of an entity changes
 *	frequently and should be sent frequently.
 */
class VolatileInfo
{
public:
	VolatileInfo( float positionPriority = -1.f, 
		float yawPriority = -1.f, 
		float pitchPriority = -1.f, 
		float rollPriority = -1.f );

	bool parse( DataSectionPtr pSection );

	bool shouldSendPosition() const	{ return positionPriority_ > 0.f; }
	int dirType( float priority ) const;

	bool isLessVolatileThan( const VolatileInfo & info ) const;
	bool isValid() const;
	bool hasVolatile( float priority ) const;

	// Overloaded operators that are declared outside the class:
	//bool operator==( const VolatileInfo & volatileInfo1, 
	//	const VolatileInfo & volatileInfo2 );
	//bool operator!=( const VolatileInfo & volatileInfo1, 
	//	const VolatileInfo & volatileInfo2 );
		
	static const float ALWAYS;

	float positionPriority() const 	{ return positionPriority_; }
	float yawPriority() const 		{ return yawPriority_; }
	float pitchPriority() const 	{ return pitchPriority_; }
	float rollPriority() const 		{ return rollPriority_; }

private:
	float asPriority( DataSectionPtr pSection ) const;

	float	positionPriority_;
	float	yawPriority_;
	float	pitchPriority_;
	float	rollPriority_;
};


bool operator==( const VolatileInfo & volatileInfo1, 
	const VolatileInfo & volatileInfo2 );

bool operator!=( const VolatileInfo & volatileInfo1, 
	const VolatileInfo & volatileInfo2 );

#ifdef CODE_INLINE
#include "volatile_info.ipp"
#endif

#endif // VOLATILE_INFO_HPP
