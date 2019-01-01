/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef XML_DATABASE_HPP
#define XML_DATABASE_HPP

#include "dbmgr_lib/idatabase.hpp"

#include "cstdmf/timer_handler.hpp"
#include "resmgr/datasection.hpp"

#include <map>

class EntityDescriptionMap;
class EntityDefs;

/**
 *	This class implements the XML database functionality.
 */
class XMLDatabase : public IDatabase,
					public TimerHandler
{
public:
	XMLDatabase();
	~XMLDatabase();

	virtual bool startup( const EntityDefs&,
			Mercury::EventDispatcher & dispatcher,
			bool isFaultRecovery );
	virtual bool shutDown();

	// IDatabase
	virtual BillingSystem * createBillingSystem();

	virtual void getEntity( const EntityDBKey & entityKey,
		BinaryOStream * pStream,
		bool shouldGetBaseEntityLocation,
		const char * pPasswordOverride,
		IDatabase::IGetEntityHandler & handler );

	virtual void putEntity( const EntityKey & entityKey,
			EntityID entityID,
			BinaryIStream * pStream,
			const EntityMailBoxRef * pBaseMailbox,
			bool removeBaseMailbox,
			UpdateAutoLoad updateAutoLoad,
			IDatabase::IPutEntityHandler& handler );

	virtual void delEntity( const EntityDBKey & ekey,
		EntityID entityID,
		IDatabase::IDelEntityHandler& handler );

	virtual void  getBaseAppMgrInitData(
			IGetBaseAppMgrInitDataHandler& handler );

	virtual void executeRawCommand( const std::string & command,
		IExecuteRawCommandHandler& handler );

	virtual void putIDs( int count, const EntityID * ids );
	virtual void getIDs( int count, IGetIDsHandler& handler );
	virtual void writeSpaceData( BinaryIStream& spaceData );

	virtual bool getSpacesData( BinaryOStream& strm );
	virtual void autoLoadEntities( IEntityAutoLoader & autoLoader );

	virtual void remapEntityMailboxes( const Mercury::Address& srcAddr,
			const BackupHash & destAddrs );

	// Secondary database stuff.
	virtual void addSecondaryDB( const SecondaryDBEntry& entry );
	virtual void updateSecondaryDBs( const BaseAppIDs& ids,
			IUpdateSecondaryDBshandler& handler );
	virtual void getSecondaryDBs( IGetSecondaryDBsHandler& handler );
	virtual uint32 numSecondaryDBs();
	virtual int clearSecondaryDBs();

	virtual void handleTimeout( TimerHandle handle, void * arg );

	// DB locking
	virtual bool lockDB() 	{ return true; };	// Not implemented
	virtual bool unlockDB()	{ return true; };	// Not implemented

	DatabaseID findEntityByName( EntityTypeID, const std::string & name ) const;

private:
	void initEntityMap();

	bool deleteEntity( DatabaseID, EntityTypeID );

	void archive();
	void save();
	void timedSave();

	DatabaseID dbIDFromEntityID( EntityID id ) const;

	DataSectionPtr findOrCreateInfoSection( const std::string & name );

	void initAutoLoad();
	void addToAutoLoad( DatabaseID databaseID );
	void removeFromAutoLoad( DatabaseID databaseID );

	void getAutoLoadSpacesFromEntities( SpaceIDSet & spaceIDs );

	//typedef StringMap< DatabaseID > NameMap;
	  // StringMap cannot handle characters > 127, use std::map instead for
	  // wide string/unicode compatiblity
	typedef std::map< std::string, DatabaseID > NameMap;
	typedef std::vector< NameMap >				NameMapVec;
	typedef std::map< DatabaseID, DataSectionPtr > IDMap;

	DataSectionPtr	pDatabaseSection_;
	NameMapVec		nameToIdMaps_;
	IDMap			idToData_;

	typedef std::map< DatabaseID, DataSectionPtr > AutoLoadMap;
	AutoLoadMap 	autoLoadEntityMap_;

	/// Stores the maximum of the used player IDs. Used to allocate
	/// new IDs to new players if allowed.
	DatabaseID		maxID_;

	class ActiveSetEntry
	{
	public:
		ActiveSetEntry()
			{
				baseRef.addr.ip = 0;
				baseRef.addr.port = 0;
				baseRef.id = 0;
			}
		EntityMailBoxRef	baseRef;
	};

	typedef std::map< DatabaseID, ActiveSetEntry > ActiveSet;
	ActiveSet activeSet_;
	std::vector<EntityID> spareIDs_;
	EntityID nextID_;

	const EntityDefs*	pEntityDefs_;
	const EntityDefs*	pNewEntityDefs_;

	int numArchives_;

	TimerHandle	archiveTimer_;
	TimerHandle	saveTimer_;
};

#endif // XML_DATABASE_HPP
