/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "space_data_manager.hpp"

BW_SINGLETON_STORAGE( SpaceDataManager )

SpaceDataManager::SpaceDataManager()
{}

SpaceDataManager::~SpaceDataManager()
{
	SpaceDataMap::iterator iSpaceData = spaceData_.begin();
	while (iSpaceData != spaceData_.end())
	{
		delete iSpaceData->second;
		++iSpaceData;
	}

	spaceData_.clear();
}

SpaceData::SpaceData( const SpaceID id ) :
	id_( id )
{}

SpaceData::~SpaceData()
{
	dataEntries_.clear();
}

SpaceData * SpaceDataManager::findOrAddSpaceData( const SpaceID spaceID )
{
	SpaceData * spaceData = NULL;

	if (spaceID < 0)
	{	// an invalid space id, make it equivalent to a deleted data entry
		ERROR_MSG( "Invalid space id %d\n", spaceID );
		return NULL;
	}

	SpaceDataMap::iterator sdemiter = spaceData_.find( spaceID );

	if (sdemiter == spaceData_.end())
	{
		INFO_MSG( "Create a new space data for space %d\n", spaceID );

		spaceData = new SpaceData( spaceID );
		spaceData_.insert( std::make_pair( spaceID, spaceData ));
	}
	else
	{
		spaceData = sdemiter->second;
	}
	return spaceData;
}

SpaceData::EntryStatus SpaceData::dataEntry( const SpaceEntryID & entryID, uint16 key,
							const std::string & data )
{
	// see if it's being added
	if (key != uint16(-1))
	{
		// make sure it is novel
		DataEntryMapKey demk;
		demk.entryID = entryID;
		demk.key = key;
		DataEntryMap::const_iterator iter = dataEntries_.find( demk );
		if (iter == dataEntries_.end())
		{
			dataEntries_.insert( std::make_pair( demk, data ) );
			// ok it's novel, so add it
			return DATA_ADDED;
		}
		else
		{
			if (iter->second == data)
				return DATA_UNCHANGED;
			else
				return DATA_MODIFIED;
		}
	}
	// ok it's being revoked then
	else
	{
		// make sure we have it (not best way to find I know)
		DataEntryMap::iterator it;
		for (it = dataEntries_.begin(); it != dataEntries_.end(); it++)
		{
			if (it->first.entryID == entryID) break;
		}
		if (it == dataEntries_.end())
			return DATA_UNKNOWN;

		// ok we have it, so remove it
		key = it->first.key;
		dataEntries_.erase( it );
		return DATA_DELETED;
	}
}

SpaceData::DataEntryMap::const_iterator SpaceData::dataRetrieve( uint16 key )
{
	DataEntryMapKey demk;
	demk.entryID.ip = 0;
	demk.entryID.port = 0;
	demk.entryID.salt = 0;
	demk.key = key;

	DataEntryMap::const_iterator found = dataEntries_.lower_bound( demk );
	if (found != dataEntries_.end() && found->first.key == key)
		return found;
	else
		return dataEntries_.end();
}

const std::string * SpaceData::dataRetrieveFirst( uint16 key )
{
	DataEntryMap::const_iterator found = this->dataRetrieve( key );

	return (found != dataEntries_.end()) ? &found->second : NULL;
}
