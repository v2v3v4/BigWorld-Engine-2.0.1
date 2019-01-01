/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_APP_HPP
#define BASE_APP_HPP

#include "network/channel_owner.hpp"
#include "server/backup_hash.hpp"

#include <set>

class BaseAppMgr;

class BaseApp: public Mercury::ChannelOwner
{
public:
	BaseApp( BaseAppMgr & baseAppMgr, const Mercury::Address & intAddr,
			const Mercury::Address & extAddr,
			int id );

	float load() const { return load_; }

	void updateLoad( float load, int numBases, int numProxies )
	{
		load_ = load;
		numBases_ = numBases;
		numProxies_ = numProxies;
	}

	bool hasTimedOut( uint64 currTime, uint64 timeoutPeriod,
		   uint64 timeSinceHeardAny ) const;

	const Mercury::Address & externalAddr() const { return externalAddr_; }

	int numBases() const	{ return numBases_; }
	int numProxies() const	{ return numProxies_; }

	BaseAppID id() const	{ return id_; }
	void id( int id )		{ id_ = id; }

	void addEntity();

	static WatcherPtr pWatcher();

	const BackupHash & backupHash() const { return backupHash_; }
	BackupHash & backupHash() { return backupHash_; }

	const BackupHash & newBackupHash() const { return newBackupHash_; }
	BackupHash & newBackupHash() { return newBackupHash_; }

	bool isRetiring() const 		{ return isRetiring_; }
	bool isOffloading() const		{ return isOffloading_; }

	// Message handlers
	void retireApp( const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void startBackup( const Mercury::Address & addr );
	void stopBackup( const Mercury::Address & addr );
	void checkToStartOffloading();

private:

	BaseAppMgr & 			baseAppMgr_;

	Mercury::Address		externalAddr_;
	BaseAppID				id_;

	float					load_;
	int						numBases_;
	int						numProxies_;

	BackupHash 				backupHash_;
	BackupHash 				newBackupHash_;

	bool					isRetiring_;
	bool					isOffloading_;

	typedef std::set< Mercury::Address > BackingUp;
	BackingUp				backingUp_;
};

#endif // BASE_APP_HPP
