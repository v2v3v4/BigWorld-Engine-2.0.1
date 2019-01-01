/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_DATABASE_HPP
#define MYSQL_DATABASE_HPP

#include "mappings/entity_type_mappings.hpp"

#include "cstdmf/bgtask_manager.hpp"
#include "network/frequent_tasks.hpp"
#include "network/interfaces.hpp"

#include "dbmgr_lib/idatabase.hpp"

#include <memory>

class BillingSystem;
class BufferedEntityTasks;

class MySql;
class MySqlLockedConnection;

class EntityDefs;

namespace DBConfig
{
class ConnectionInfo;
}

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

/**
 *	This class is an implementation of IDatabase for the MySQL database.
 */
class MySqlDatabase : public IDatabase,
	public Mercury::FrequentTask
{
public:
	MySqlDatabase( Mercury::NetworkInterface & interface,
		Mercury::EventDispatcher & dispatcher );
	~MySqlDatabase();

	virtual bool startup( const EntityDefs&,
				Mercury::EventDispatcher & dispatcher, bool isFaultRecovery );
	virtual bool shutDown();

	// IDatabase
	virtual BillingSystem * createBillingSystem();

	virtual void getEntity( const EntityDBKey & entityKey,
							BinaryOStream * pStream,
							bool shouldGetBaseEntityLocation,
							const char * pPasswordOverride,
							IDatabase::IGetEntityHandler & handler );

	virtual void putEntity( const EntityKey & ekey,
							EntityID entityID,
							BinaryIStream * pStream,
							const EntityMailBoxRef * pBaseMailbox,
							bool removeBaseMailbox,
							UpdateAutoLoad updateAutoLoad,
							IPutEntityHandler& handler );

	virtual void delEntity( const EntityDBKey & ekey,
		EntityID entityID,
		IDatabase::IDelEntityHandler& handler );

	virtual void executeRawCommand( const std::string & command,
		IDatabase::IExecuteRawCommandHandler& handler );

	virtual void putIDs( int numIDs, const EntityID * ids );
	virtual void getIDs( int numIDs, IDatabase::IGetIDsHandler& handler );

	// Backing up spaces.
	virtual void writeSpaceData( BinaryIStream& spaceData );

	virtual bool getSpacesData( BinaryOStream& strm );
	virtual void autoLoadEntities( IEntityAutoLoader & autoLoader );

	virtual void setGameTime( GameTime gameTime );

	virtual void getBaseAppMgrInitData(
			IGetBaseAppMgrInitDataHandler& handler );

	// BaseApp death handler
	virtual void remapEntityMailboxes( const Mercury::Address& srcAddr,
			const BackupHash & destAddrs );

	// Secondary database entries
	virtual void addSecondaryDB( const IDatabase::SecondaryDBEntry& entry );
	virtual void updateSecondaryDBs( const BaseAppIDs& ids,
			IUpdateSecondaryDBshandler& handler );
	virtual void getSecondaryDBs( IDatabase::IGetSecondaryDBsHandler& handler );
	virtual uint32 numSecondaryDBs();
	virtual int clearSecondaryDBs();

	// DB locking
	virtual bool lockDB();
	virtual bool unlockDB();

	virtual void doTask();

	bool hasFatalConnectionError() const
									{ return reconnectTimerHandle_.isSet(); }

private:
	bool syncTablesToDefs() const;
	void startBackgroundThreads(
		const DBConfig::ConnectionInfo & connectionInfo );

	// BigWorld internal tables initialisation
	static void initSpecialBigWorldTables( MySql& connection,
			const EntityDefs& entityDefs );

private:
	void getAutoLoadSpacesFromEntities( SpaceIDSet & spaceIDs );

	BgTaskManager bgTaskManager_;

	int numConnections_;

	TimerHandle	reconnectTimerHandle_;
	size_t reconnectCount_;
	BufferedEntityTasks * pBufferedEntityTasks_;

	Mercury::NetworkInterface & interface_;
	Mercury::EventDispatcher & dispatcher_;

	const EntityDefs * pEntityDefs_;
	EntityTypeMappings entityTypeMappings_;

	MySqlLockedConnection * pConnection_;
};

#endif // MYSQL_DATABASE_HPP
