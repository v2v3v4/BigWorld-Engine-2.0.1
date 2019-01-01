/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef	SPACE_DATA_MANAGER_HPP
#define SPACE_DATA_MANAGER_HPP

#include "common/space_data_types.hpp"

#include "cstdmf/singleton.hpp"

#include "network/basictypes.hpp"

#include <map>

class SpaceData
{
public:
	SpaceData( const SpaceID id );
	~SpaceData();

	struct DataEntryMapKey
	{
		SpaceEntryID	entryID;
		uint16			key;
		bool operator<( const DataEntryMapKey & other ) const
		{ return key < other.key ||
		(key == other.key && entryID < other.entryID); }
	};
	typedef std::map<DataEntryMapKey, std::string> DataEntryMap;
	typedef std::pair<uint16, const std::string * > DataValueReturn;

	enum EntryStatus
	{
		DATA_ADDED,
		DATA_MODIFIED,
		DATA_DELETED,
		DATA_UNCHANGED,
		DATA_UNKNOWN
	};

	SpaceData::EntryStatus dataEntry( const SpaceEntryID & entryID,
		uint16 key, const std::string & data );
	DataValueReturn dataRetrieveSpecific( const SpaceEntryID & entryID,
		uint16 key = uint16(-1) );

	const std::string * dataRetrieveFirst( uint16 key );
	DataEntryMap::const_iterator dataRetrieve( uint16 key );

private:
	DataEntryMap	dataEntries_;
	SpaceID			id_;
};


class SpaceDataManager
{
public:
	SpaceDataManager();
	~SpaceDataManager();

	SpaceData * findOrAddSpaceData( const SpaceID spaceID );

private:
	typedef std::map< SpaceID, SpaceData * > SpaceDataMap;
	SpaceDataMap	spaceData_;
};

#endif // SPACE_DATA_MANAGER_HPP
