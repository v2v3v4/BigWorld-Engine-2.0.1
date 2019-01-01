/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ARCHIVER_HPP
#define ARCHIVER_HPP

#include "network/basictypes.hpp"

#include <vector>

class BaseAppMgrGateway;
class Bases;
class SqliteDatabase;

namespace Mercury
{
class ChannelOwner;
}

typedef Mercury::ChannelOwner DBMgr;

/**
 *	This class is responsible for periodically archiving this BaseApp's
 *	entities.
 */
class Archiver
{
public:
	Archiver();

	void tick( DBMgr & dbMgr, BaseAppMgrGateway & baseAppMgr,
		Bases & bases, SqliteDatabase * pSecondaryDB );

	void handleBaseAppDeath( const Mercury::Address & addr,
		Bases & bases, SqliteDatabase * pSecondaryDB );

	static bool isEnabled();

private:
	void restartArchiveCycle( Bases & bases );

	// Used for which tick in archivePeriodInTicks we are up to.
	int					archiveIndex_;
	std::vector<EntityID> 	basesToArchive_;

	Mercury::Address	deadBaseAppAddr_;
};

#endif // ARCHIVER_HPP
