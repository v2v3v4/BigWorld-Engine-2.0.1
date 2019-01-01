/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_MAP_TIMESTAMP_CACHE_HPP
#define SPACE_MAP_TIMESTAMP_CACHE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/project/space_information.hpp"


class SpaceMapTimestampCache
{
public:
	SpaceMapTimestampCache();	

	void spaceInformation( const SpaceInformation& info );

	void load();
	void save();

	void loadTemporaryCopy();
	void saveTemporaryCopy();
	
	//convert a FILETIME to our time format
	uint64 getUint64( const FILETIME& fileTime ) const
	{
		return *(uint64*)&fileTime;
	}

	//get now in our time format
	uint64 now() const;

	//retrieve the time in the cache for the tile number
	uint64	cacheTime( int16 gridX, int16 gridZ ) const;	

	//touch the cache for the chunk at gridX,Z (set it to now)
	void	touch( int16 gridX, int16 gridZ );

private:
	uint64	cacheModifyTime( uint32 tileNum ) const;
	void cacheModifyTime( uint32 tileNum, uint64 modTime );
	uint32 tileNum( int16 gridX, int16 gridZ ) const;

	std::string cacheName_;
	BinaryPtr	pLastModified_;
	SpaceInformation info_;
};


#endif // SPACE_MAP_TIMESTAMP_CACHE_HPP
