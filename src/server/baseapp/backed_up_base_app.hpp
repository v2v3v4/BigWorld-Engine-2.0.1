/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BACKED_UP_BASE_APP_HPP
#define BACKED_UP_BASE_APP_HPP

#include "cstdmf/stdmf.hpp"
#include "server/backup_hash_chain.hpp"

#include <map>

/**
 *	This class is used for storing information about BaseApps that are being
 *	backed up by this BaseApp.
 */

class BackedUpEntities
{
public:
	std::string & getDataFor( EntityID entityID )
	{
		return data_[ entityID ];
	}
	
	bool contains( EntityID entityID ) const
	{
		return data_.count( entityID ) != 0;
	}

	bool erase( EntityID entityID )
	{
		return data_.erase( entityID );
	}
	
	void clear()
	{
		data_.clear();
	}
	
	void restore();
	
	bool empty() const	{ return data_.empty(); }
	
	void restoreRemotely( const Mercury::Address & deadAddr, 
		  const BackupHashChain & backedUpHashChain );

protected:
	typedef	std::map< EntityID, std::string > Container;
	Container data_;
};


class BackedUpEntitiesWithHash : public BackedUpEntities
{
public:
	void init( uint32 index, const MiniBackupHash & hash,
			   const BackedUpEntitiesWithHash & current );
	
	void swap( BackedUpEntitiesWithHash & other )
	{
		uint32 tempInt = index_;
		index_ = other.index_;
		other.index_ = tempInt;
		
		MiniBackupHash tempHash = hash_;
		hash_ = other.hash_;
		other.hash_ = tempHash;
		
		data_.swap( other.data_ );
	}
	
private:
	uint32 index_;
	MiniBackupHash hash_;
};


class BackedUpBaseApp
{
public:
	BackedUpBaseApp() : 
		usingNew_( false ),
		canSwitchToNewBackup_( false )
	{}

	void startNewBackup( uint32 index, const MiniBackupHash & hash );

	std::string & getDataFor( EntityID entityID )
	{
		if (usingNew_)
			return newBackup_.getDataFor( entityID );
		else
			return currentBackup_.getDataFor( entityID );
	}

	bool erase( EntityID entityID )
	{
		if (usingNew_)
			return newBackup_.erase( entityID );
		else
			return currentBackup_.erase( entityID );
	}

	void switchToNewBackup();
	void discardNewBackup();

	bool canSwitchToNewBackup() const 
		{ return canSwitchToNewBackup_; }

	void canSwitchToNewBackup( bool value ) 
		{ canSwitchToNewBackup_ = value; }

	void restore();

private:

	BackedUpEntitiesWithHash currentBackup_; // Up-to-date backup.
	BackedUpEntitiesWithHash newBackup_;	// Backup we're trying to switch to.
	bool usingNew_;
	bool canSwitchToNewBackup_;
};

#endif // BACKED_UP_BASE_APP_HPP
