/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BACKED_UP_BASE_APPS_HPP
#define BACKED_UP_BASE_APPS_HPP

#include "backed_up_base_app.hpp"
#include <map>

class BackedUpBaseApp;
class MiniBackupHash;

namespace Mercury
{
class UnpackedMessageHeader;
}

/**
 *	This class handles the backing up of other BaseApps by this BaseApp.
 */
class BackedUpBaseApps
{
public:
	void startAppBackup( const Mercury::Address & realBaseAppAddr,
		uint32 index, uint32 hashSize, uint32 prime, bool isInitial );

	void stopAppBackup( const Mercury::Address	& realBaseAppAddr,
		uint32 index, uint32 hashSize, uint32 prime, bool isPending );


	void backUpEntity( const Mercury::Address & srcAddr,
			EntityID entityID, BinaryIStream & data );

	void stopEntityBackup( const Mercury::Address & srcAddr,
			EntityID entityID );

	void handleBaseAppDeath( const Mercury::Address & deadAddr );

	void onloadedEntity( const Mercury::Address & srcAddr, 
						 EntityID entityID );

	bool isBackingUpOthers() const
	{ return !apps_.empty(); }
private:
	typedef std::map< Mercury::Address, BackedUpBaseApp > Container;
	Container apps_;
};

#endif // BACKED_UP_BASE_APPS_HPP
