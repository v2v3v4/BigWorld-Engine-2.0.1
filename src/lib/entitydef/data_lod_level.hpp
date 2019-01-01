/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_LOD_LEVEL_HPP
#define DATA_LOD_LEVEL_HPP

#include "network/basictypes.hpp"
#include "resmgr/datasection.hpp"

#include <string>

/**
 *	This class is used by DataLoDLevels. If the priority goes below the low
 *	value, the consumer should more to a more detailed level. If the priority
 *	goes above the high value, we should move to a less detailed level.
 */
class DataLoDLevel
{
public:
	DataLoDLevel();

	float low() const					{ return low_; }
	float high() const					{ return high_; }

	float start() const					{ return start_; }
	float hyst() const					{ return hyst_; }

	void set( const std::string & label, float start, float hyst )
	{
		label_ = label;
		start_ = start;
		hyst_ = hyst;
	}

	const std::string & label() const	{ return label_; }

	void finalise( DataLoDLevel * pPrev, bool isLast );

	int index() const			{ return index_; }
	void index( int i )			{ index_ = i; }

	enum
	{
		OUTER_LEVEL = -2,
		NO_LEVEL = -1
	};

private:
	float low_;
	float high_;
	float start_;
	float hyst_;
	std::string label_;

	// Only used when starting up. It is used to translate detailLevel if the
	// detail levels were reordered because of a derived interface.
	int index_;
};


/**
 *	This class is used to store where the "Level of Detail" transitions occur.
 */
class DataLoDLevels
{
public:
	DataLoDLevels();
	bool addLevels( DataSectionPtr pSection );

	int size() const;
	const DataLoDLevel & getLevel( int i ) const	{ return level_[i]; }

	DataLoDLevel *  find( const std::string & label );
	bool findLevel( int & level, DataSectionPtr pSection ) const;

	bool needsMoreDetail( int level, float priority ) const;
	bool needsLessDetail( int level, float priority ) const;

private:
	// TODO: Reconsider what MAX_DATA_LOD_LEVELS needs to be.
	DataLoDLevel level_[ MAX_DATA_LOD_LEVELS + 1 ];

	int size_;
};


#ifdef CODE_INLINE
#include "data_lod_level.ipp"
#endif

#endif // DATA_LOD_LEVEL_HPP
