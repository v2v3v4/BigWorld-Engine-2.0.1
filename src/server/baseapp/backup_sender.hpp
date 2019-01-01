/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BACKUP_SENDER_HPP
#define BACKUP_SENDER_HPP

#include "server/backup_hash.hpp"

class Base;
class BaseApp;
class BaseAppMgrGateway;
class Bases;
namespace Mercury
{
class BundleSendingMap;
class ChannelOwner;
class NetworkInterface;
class ReplyMessageHandler;
}


/**
 *	This class is responsible for backing up the base entities to other
 *	BaseApps.
 */
class BackupSender
{
public:
	BackupSender( BaseApp & baseApp );

	void tick( const Bases & bases,
			   Mercury::NetworkInterface & networkInterface );

	Mercury::Address addressFor( EntityID entityID ) const
	{
		return entityToAppHash_.addressFor( entityID );
	}

	void addToStream( BinaryOStream & stream );
	void handleBaseAppDeath( const Mercury::Address & addr );
	void setBackupBaseApps( BinaryIStream & data,
	   Mercury::NetworkInterface & networkInterface );

	void restartBackupCycle( const Bases & bases );
	void startOffloading() { isOffloading_ = true; }
	bool autoBackupBase( Base & base,
					 Mercury::BundleSendingMap & bundles,
					 Mercury::ReplyMessageHandler * pHandler = NULL );
	bool backupBase( Base & base,
					 Mercury::BundleSendingMap & bundles,
					 Mercury::ReplyMessageHandler * pHandler = NULL );
private:
	void ackNewBackupHash();

	int					offloadPerTick_;

	// Fractional overflow of backup for next tick
	float				backupRemainder_;
	std::vector<EntityID> 	basesToBackUp_;

	BackupHash			entityToAppHash_;	// The up-to-date hash
	BackupHash			newEntityToAppHash_; // The hash we are trying to switch to.
	bool				isUsingNewBackup_;
	bool				isOffloading_;
	BaseApp	&			baseApp_;
};

#endif // BACKUP_SENDER_HPP
