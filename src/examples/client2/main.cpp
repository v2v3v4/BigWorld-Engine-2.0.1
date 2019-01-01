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

extern void mainLoopAction( ServerConnection & serverConnection,
					  Entity * pPlayer );
extern void shutdownAction( ServerConnection & serverConnection,
					  Entity * pPlayer );

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

void sigint(int)
{
	std::cout << "sigint called" << std::endl;
	g_ctrlC = true;
}

bool exitCondition()
{
#ifdef _WIN32
	while (_kbhit() && !g_ctrlC)
	{
		char key = _getch();
		if (key == 'q' || key == 'Q')
			g_ctrlC = true;
	}
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
			serverConnection_.requestEntityUpdate( entityID );
		}
	}

	/**
	 *	This method is called when an entity leaves the client's AOI.
	 */
	virtual void onEntityLeave( EntityID id, const CacheStamps & stamps )
	{
		std::cout << "MyServerMessageHandler::onEntityLeave: id = "
			<< id << std::endl;
		Entity::entities_.erase( id ); // it should exist, but is not fatal if it doesn't.
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
		Entity *& rpEntity = Entity::entities_[ id ];

		MF_ASSERT( rpEntity == NULL );
		Vector3 vectPos (pos);

		EntityType * pEntityType = EntityType::find( type );

		if (pEntityType != NULL)
		{
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
	}

	/**
	 *	This method is called when the server sets a property on an entity.
	 */
	virtual void onEntityProperty( EntityID entityID, int propertyID,
		BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onEntityProperty: "
			<< "entityID = " << entityID
			<< ". propertyID = " << propertyID << std::endl;
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
		std::cout << "MyServerMessageHandler::onEntityMethod: "
			"entityID = " << entityID <<
			". methodID = " << methodID << std::endl;
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
		std::cout << "MyServerMessageHandler::onEntityMove: "
			"id = " << entityID << ". pos = " << pos << std::endl;

		if (Entity::entities_.find( entityID ) == Entity::entities_.end())
			std::cout << "ERROR: Unknown entity " << entityID << std::endl;
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

private:
	ServerConnection & serverConnection_;
};


int main( int argc, char * argv[] )
{
#ifdef _WIN32
	std::cout << "Press 'q' to quit" << std::endl;
#endif
	BWResource::init(argc, (const char**)argv);

	PyImportPaths importPaths;
	importPaths.addPath( "" );

	if (!Script::init( importPaths ))
	{
		std::cerr << "Could not initialise Python" << std::endl;
		return -1;
	}

	// Initialise the server connection.
	ServerConnection serverConnection;
	MyServerMessageHandler handler( serverConnection );

	double localTime = 0.0;
	serverConnection.pTime( &localTime );

	// Initialise the entity descriptions.
	MD5::Digest digest;
	EntityType::init( EntityDef::Constants::entitiesFile(), digest );
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
	signal( SIGINT, sigint );
#endif

	if (!status.succeeded())
	{
		std::cout << "Login failed with status " << status << std::endl;
		std::cout << serverConnection.errorMsg() << std::endl;
#ifdef _WIN32
		std::cout << "Press 'q' to quit" << std::endl;
#else
		std::cout << "Press 'Ctrl+C' to quit" << std::endl;
#endif

		while (!exitCondition())
		{
			mySleep( 0.01 );
		}

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

	// Wait until the server has called createEntity on us (entity made on cell)
	while (!g_spaceID && !exitCondition())
	{
		serverConnection.processInput();
		serverConnection.send();
		mySleep( 0.1 );
	}

	std::cout << "Our current space id is " << g_spaceID << std::endl;

	while (!exitCondition())
	{
		::mainLoopAction( serverConnection, Entity::entities_[ g_playerID ] );

		const float radius = 10.f;
		const float period = 10.f;
		const int updateRate = 12;
		const float timeAdjust = 2 * MATH_PI / period;

		const float angle = float(localTime) * timeAdjust;

		Vector3 position( radius * sinf( angle ), 0.f, radius * cosf( angle ) );

		serverConnection.addMove( g_playerID, g_spaceID, 0, position,
			angle + MATH_PI/2.f, 0.f, 0.f, true, position );

		mySleep( 1.0/updateRate );
		localTime += 1.f/updateRate;

		serverConnection.send();
		serverConnection.processInput();
	}

	if (g_playerID != 0)
	{
		::shutdownAction( serverConnection, Entity::entities_[ g_playerID ] );
	}

	EntityType::fini();
	Script::fini();

	return 0;
}

// main.cpp
