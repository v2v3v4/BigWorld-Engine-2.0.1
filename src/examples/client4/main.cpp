/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity.hpp"

#ifndef USE_OPENSSL
#define USE_OPENSSL
#endif

#include "cstdmf/dprintf.hpp"

#include "connection/loginapp_public_key.hpp"
#include "connection/rsa_stream_encoder.hpp"
#include "connection/server_connection.hpp"
#include "connection/server_message_handler.hpp"

#include "entitydef/constants.hpp"

#include "pyscript/py_import_paths.hpp"

#include "resmgr/bwresource.hpp"

#include <iostream>

#ifdef _WIN32
#include <winsock.h>
#include <conio.h>
#else // ! ifdef _WIN32
#include <signal.h>
#endif // ifdef _WIN32

DECLARE_DEBUG_COMPONENT2( "Eg", 0 )

EntityID g_playerID = 0;
SpaceID g_spaceID = 0;

void mySleep( double seconds )
{
#ifdef _WIN32
	Sleep( int( seconds * 1000.0 ) );
#else
	usleep( int( seconds * 1000000.0 ) );
#endif
}


bool g_ctrlC = false;
ServerConnection * g_serverConnection;

void sigint(int)
{
	std::cout << "sigint called" << std::endl;
	g_ctrlC = true;
}

bool exitCondition()
{
#ifdef _WIN32
	while (_kbhit())
	{
		char key = _getch();
		if (key == 'q' || key == 'Q')
			g_ctrlC = true;
	}
#else
	// std::cout << "ctrl-c: " << g_ctrlC << std::endl;
#endif

	return g_ctrlC;
}


/**
 *	This class is used to handle messages from the server.
 */
class MyServerMessageHandler : public ServerMessageHandler
{
public:
	MyServerMessageHandler( ServerConnection & serverConnection ) :
		serverConnection_( serverConnection )
	{
	}

	/**
	 *	This method is called when the player is created on the base.
	 */
	virtual void onBasePlayerCreate( EntityID id, EntityTypeID type,
		BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onBasePlayerCreate: id = "
			<< id << std::endl;
		g_playerID = id;
		g_spaceID = 0;
		EntityType * pType = EntityType::find( type );
		MF_ASSERT( pType );
		Entity::entities_[ id ] = pType->newEntity(
			id, Vector3::zero(), 0, 0, 0, data, /*isBasePlayer:*/true );
	}

	/**
	 *	This method is called when the player is created on the cell.
	 */
	virtual void onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch, float roll, BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onCellPlayerCreate: id = "
			<< id << std::endl;

		MF_ASSERT( g_playerID == id );
		Entity * pPlayer = Entity::entities_[ id ];

		MF_ASSERT( pPlayer );
		pPlayer->readCellPlayerData( data );
		g_spaceID = spaceID;
	}

	/**
	 *	This method is called when an entity enters the client's AOI.
	 */
	virtual void onEntityEnter( EntityID entityID, SpaceID spaceID, EntityID )
	{
		std::cout << "MyServerMessageHandler::enterEntity: entityID = "
			<< entityID << std::endl;
		if (entityID != g_playerID)
		{
			Entity::EntityMap::iterator iter = Entity::cachedEntities_.find( entityID );
			if (iter != Entity::cachedEntities_.end())
			{
				std::cout << "MyServerMessageHandler::enterEntity: entityID "
							<< entityID << " is in cache" << std::endl;
				serverConnection_.requestEntityUpdate( entityID, iter->second->cacheStamps() );
			}
			else
			{
				std::cout << "MyServerMessageHandler::enterEntity: entityID "
							<< entityID << " is new" << std::endl;
				serverConnection_.requestEntityUpdate( entityID );
			}
		}
	}

	/**
	 *	This method is called when an entity leaves the client's AOI.
	 */
	virtual void onEntityLeave( EntityID id, const CacheStamps & stamps )
	{
		std::cout << "MyServerMessageHandler::onEntityLeave: id = "
			<< id << std::endl;
		Entity::EntityMap::iterator iter = Entity::entities_.find( id );
		if (iter != Entity::entities_.end())
		{
			iter->second->cacheStamps( stamps );
			Entity::cachedEntities_[ id ] = iter->second;
			Entity::entities_.erase( iter );
		}
		else
		{
			// it should exist, but is not fatal if it doesn't.
			WARNING_MSG( "MyServerMessageHandler::onEntityLeave: "
					"cannot find object id = %u\n", id );
		}
	}

	/**
	 *	This method is called by the server in response to a
	 *	requestEntityUpdate.
	 */
	virtual void onEntityCreate( EntityID id, EntityTypeID type,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch,	float roll,
		BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onEntityCreate: id = "
			<< id << std::endl;
		// Make sure it doesn't already exist.
		Entity::EntityMap::iterator cachedIter = Entity::cachedEntities_.find( id );
		if ( cachedIter != Entity::cachedEntities_.end() )
		{
			std::cout << "MyServerMessageHandler::onEntityCreate: id "
					<< id << " is in cache" << std::endl;
			cachedIter->second->position( pos );
			cachedIter->second->updateProperties( data );
			Entity::entities_[ id ] = cachedIter->second;
			Entity::cachedEntities_.erase( cachedIter );
		}
		else if ( Entity::entities_.find( id ) != Entity::entities_.end() )
		{
			ERROR_MSG( "MyServerMessageHandler::onEntityCreate: "
				"object %u already exists\n", id );
		}
		else
		{
			EntityType * pEntityType = EntityType::find( type );
			if (pEntityType == NULL)
			{
				ERROR_MSG( "MyServerMessageHandler::onEntityCreate: "
					"object type %d doesn't exist for client\n", (int)type );
				if (!g_spaceID)
				{
					// HACK: force a quit
					INFO_MSG( "MyServerMessageHandler::onEntityCreate: "
						"faking ctrl-c to force a quit\n" );
					g_ctrlC = true;
				}
				return;
			}
			Vector3 vectPos (pos);
			Entity::entities_[ id ] = pEntityType->newEntity(
				id, vectPos, yaw, pitch, roll, data, /*isBasePlayer:*/false );
		}
	}

	/**
	 *	This method is called by the server to update some properties of
	 *	the given entity, while it is in our AoI.
	 */
	virtual void onEntityProperties( EntityID id, BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onEntityProperties: id = "
			<< id << std::endl;
		// unimplemented since this client does not support detail levels
		// (currently the only cause of this message)
		Entity::EntityMap::iterator iter = Entity::entities_.find( id );
		if( iter != Entity::entities_.end() )
		{
			iter->second->updateProperties( data );
		}
		else
		{
			std::cout << "Entity not found" << std::endl;
		}
	}

	/**
	 *	This method is called when the server sets a property on an entity.
	 */
	virtual void onEntityProperty( EntityID entityID, int propertyID,
		BinaryIStream & data )
	{
		/*
		std::cout << "MyServerMessageHandler::onEntityProperty: "
			<< "entityID = " << entityID
			<< ". propertyID = " << propertyID << std::endl;
		*/
		Entity::EntityMap::iterator iter = Entity::entities_.find( entityID );
		if ( iter != Entity::entities_.end() )
		{
			iter->second->handlePropertyChange( propertyID , data );
		}
		else
		{
			// this could be a message for an entity that has not yet been
			// loaded, or has already been unloaded.
		}
	}

	/**
	 *	This method is called when the server calls a method on an entity.
	 */
	virtual void onEntityMethod( EntityID entityID, int methodID,
		BinaryIStream & data )
	{
		/*
		std::cout << "MyServerMessageHandler::onEntityMethod: "
			"entityID = " << entityID <<
			". methodID = " << methodID << std::endl;
		*/
		Entity::EntityMap::iterator iter = Entity::entities_.find( entityID );
		if ( iter != Entity::entities_.end() )
		{
			iter->second->handleMethodCall( methodID, data );
		}
		else
		{
			// this could be a message for an entity that has not yet been
			// loaded, or has already been unloaded.
		}
	}

	/**
	 *	This method is called when the position of an entity changes.
	 */
	virtual void onEntityMove( EntityID entityID, SpaceID spaceID,
			EntityID vehicleID,
		const Position3D & pos,	float yaw, float pitch, float roll,
		bool isVolatile )
	{
		/*
		std::cout << "MyServerMessageHandler::positionUpdate: "
			<< "entityID = " << entityID
			<< ". pos = " << pos << std::endl;
		*/
		if (Entity::entities_.find( entityID ) != Entity::entities_.end())
		{
			Entity::entities_[ entityID ]->position( pos );
		}
	}

	/**
	 *	This method is called to set the current time of day.
	 */
	virtual void setTime( TimeStamp gameTime,
			float initialTimeOfDay, float gameSecondsPerSecond )
	{
		std::cout << "MyServerMessageHandler::setTime:" << std::endl;
	}

	/**
	 *	This method is called to inform us about new or change data associated
	 *	with a space.
	 */
	virtual void spaceData( SpaceID spaceID, SpaceEntryID entryID, uint16 key,
		const std::string & data )
	{
		std::cout << "MyServerMessageHandler::spaceData:" << std::endl;
	}

	/**
	 *	This method is called to inform us that a space has gone.
	 */
	virtual void spaceGone( SpaceID spaceID )
	{
		std::cout << "MyServerMessageHandler::spaceGone:" << std::endl;
	}

	/**
	 *	This method is called when the server tells us to reset all our
	 *	entities. The player entity may optionally be saved (but still should
	 *	not be considered to be in the world).
	 *
	 *	This can occur when the entity that the client is associated with
	 *	changes or when the current client destroys its cell entity.
	 */
	virtual void onEntitiesReset( bool keepPlayerOnBase )
	{
		std::cout << "onEntitiesReset: " << g_playerID << "," << keepPlayerOnBase << std::endl;
		if (!keepPlayerOnBase)
		{
			g_playerID = 0;
		}
		g_spaceID = 0;

		Entity::EntityMap::iterator iterToDel;
		Entity::EntityMap::iterator iter = Entity::entities_.begin();
		while (iter != Entity::entities_.end())
		{
			Entity * pCurr = iter->second;
			iterToDel = iter;
			++iter;

			if (pCurr->id() != g_playerID)
			{
				Py_DECREF( pCurr );
				Entity::entities_.erase(iterToDel);
			}
		}

		iter = Entity::cachedEntities_.begin();
		while (iter != Entity::cachedEntities_.begin())
		{
			Entity * pCurr = iter->second;
			++iter;
			Py_DECREF( pCurr );
		}
		Entity::cachedEntities_.clear();
	}

	/**
	 *	This method is called when the server provides us with the details
	 *	necessary to restore the client to a previous state.
	 */
	virtual void onRestoreClient( EntityID id,
		SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, const Direction3D & dir,
		BinaryIStream & data )
	{
		INFO_MSG( "MyServerMessageHandler::onRestoreClient: id=%u (%d)\n",
			id, data.remainingLength() );

		// Make sure the id matches our player ID
		IF_NOT_MF_ASSERT_DEV( id == g_playerID )
		{
			ERROR_MSG( "MyServerMessageHandler::onRestoreClient: " 
				"received ID (%u) does not match player ID (%u)!",
				id, g_playerID );
			return;
		}

		Entity *pPlayer = Entity::entities_[ id ];

		// Set position, but ignore space, vehicle and direction since they
		// aren't actually supported in this implementation of Entity
		pPlayer->position( pos );

		// Get player's property dictionary.  We pass STEAL_REFERENCE because
		// the PyObject_GetAttrString() already increments the refcount and we
		// don't want to increment it again during SmartPointer construction.
		PyObjectPtr pDict( PyObject_GetAttrString( pPlayer, "__dict__" ),
			PyObjectPtr::STEAL_REFERENCE );

		// Read the properties from the stream into the player's dictionary.
		const EntityDescription & entityDesc = pPlayer->type().description();
		entityDesc.readStreamToDict( data, EntityDescription::CLIENT_DATA,
			pDict.getObject() );
	}

private:
	ServerConnection & serverConnection_;
};

int main( int argc, char * argv[] )
{
	DebugFilter::shouldWriteToConsole( true );

#ifdef _WIN32
	std::cout << "Press 'q' to quit" << std::endl;
#endif

	BWResource::init(argc, (const char **)argv);

	PyImportPaths paths;
	paths.addPath( EntityDef::Constants::entitiesClientPath() );

	if (!Script::init( paths, "client" ))
	{
		std::cerr << "Could not initialise Python" << std::endl;
		return -1;
	}

	// Initialise the server connection.
	ServerConnection serverConnection;
	g_serverConnection = &serverConnection;
	MyServerMessageHandler handler( serverConnection );

	double localTime = 0.0;
	serverConnection.pTime( &localTime );

	// Initialise the entity descriptions.
	MD5::Digest digest;
	if (!EntityType::init( EntityDef::Constants::entitiesFile(), digest ))
	{
		std::cerr << "Could not initialise entity data" << std::endl;
		return -1;
	}
	serverConnection.digest( digest );

	const char * serverName = NULL;
	const char * userName = NULL;
	const char * password = "a";

	for (int i = 0; i < argc - 1; i++)
	{
		if (strcmp( "-server", argv[i] ) == 0)
		{
			serverName = argv[ ++i ];
			std::cout << "Server is " << serverName << std::endl;
		}
		else if (strcmp( "-user", argv[i] ) == 0)
		{
			userName = argv[ ++i ];
			std::cout << "Username is " << userName << std::endl;
		}
		else if (strcmp( "-password", argv[i] ) == 0)
		{
			password = argv[ ++i ];
		}
	}

	char inputServerName[ 128 ];
	char inputUserName[ 128 ];

	if (serverName == NULL)
	{
		std::cout << "Input server name: ";
		scanf( "%128s", inputServerName );
	}

	if (userName == NULL)
	{
		std::cout << "Input user name: ";
		scanf( "%128s", inputUserName );
	}

	// Setup the public key to encrypt client / server communication
	RSAStreamEncoder logOnParamsEncoder( /* keyIsPrivate: */ false );
#if defined( PLAYSTATION3 ) || defined( _XBOX360 )
	logOnParamsEncoder.initFromKeyString( g_loginAppPublicKey );
#else
	logOnParamsEncoder.initFromKeyPath( "loginapp.pubkey" );
#endif
	serverConnection.pLogOnParamsEncoder( &logOnParamsEncoder );

	// Log the player in
	LogOnStatus status =
		serverConnection.logOn( &handler,
		serverName ? serverName : inputServerName,
			userName ? userName : inputUserName,
			password );

#ifndef _WIN32
	signal(SIGINT,sigint);
#endif

	if (!status.succeeded())
	{
		std::cout << "Login failed with status " << status << std::endl;
		std::cout << serverConnection.errorMsg() << std::endl;
#ifdef _WIN32
		std::cout << "Press 'q' to quit" << std::endl;

		while (!exitCondition())
		{
			mySleep( 0.01 );
		}
#endif

		return 1;
	}

	std::cout << "Remote address is " <<
		serverConnection.addr().c_str() << std::endl;

	// Wait until the server has called createPlayer on us (entity made on base)
	while (!g_playerID && !exitCondition())
	{
		serverConnection.processInput();
		serverConnection.send();
		mySleep( 0.1 );
	}

	std::cout << "Our id is " << g_playerID << std::endl;

#if 0
	// Wait until the server has called createEntity on us (entity made on cell)
	while (!g_spaceID && !exitCondition())
	{
		serverConnection.processInput( handler );
		serverConnection.send();
		mySleep( 0.1 );
	}
#endif

	std::cout << "Our current space id is " << g_spaceID << std::endl;

	const double updateRate = 10.0;

	while (!exitCondition())
	{
		mySleep( (float)(1.0/updateRate) );
		localTime += 1.f/updateRate;

		serverConnection.send();
		serverConnection.processInput();

		// avatar gets to be called once per time-slice
		// find the entity we're interested in
		Entity * pPlayer = Entity::entities_[ g_playerID ];
		if (pPlayer != NULL)
		{
			PyObject * pResult = PyObject_CallMethod( pPlayer, "onTick",
				"d", serverConnection.serverTime( localTime ) );
			if (PyErr_Occurred())
				PyErr_Print();
			PY_ERROR_CHECK();
			Py_XDECREF(pResult);

			// The player may not be on a cell.
			if (g_spaceID != 0)
			{
				serverConnection.addMove( g_playerID, g_spaceID, 0,
					pPlayer->position(), 0, 0, 0, true, pPlayer->position() );
			}
		}
	}

	std::cout << "Exiting" << std::endl;

	Entity * pPlayer = Entity::entities_[ g_playerID ];
	if (pPlayer != NULL)
	{
		PyObject * pResult = PyObject_CallMethod( pPlayer, "onFinish", "" );
		PY_ERROR_CHECK();
		Py_XDECREF(pResult);
	}
	serverConnection.send();

	EntityType::fini();
	Script::fini();

	return 0;
}

// main.cpp
