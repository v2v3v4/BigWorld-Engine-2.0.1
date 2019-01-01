/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef OFFLOADED_BACKUPS_HPP
#define OFFLOADED_BACKUPS_HPP

#include "backed_up_base_app.hpp"
#include "network/basictypes.hpp"
#include "server/backup_hash_chain.hpp"
#include <string>
#include <map>

/**
 *	This class is responsible for holding entity backup data for entities that
 *	have been offloaded to other BaseApps. During retirement, the BaseApp
 *	becomes the backup for any entities that it offloads to other BaseApps.
 */
class OffloadedBackups
{
public:
	OffloadedBackups();

	bool wasOffloaded( EntityID entityID ) const;

	void backUpEntity( const Mercury::Address & srcAddr,
					   BinaryIStream & data );
	void handleBaseAppDeath( const Mercury::Address & deadAddr,
							 const BackupHashChain & backedUpHashChain );
	void stopBackingUpEntity( EntityID entityID );
	bool isEmpty()	{ return apps_.empty(); }
private:
	typedef std::map< Mercury::Address, BackedUpEntities > Container;
	Container apps_;
};


#endif // OFFLOADED_BACKUPS_HPP
