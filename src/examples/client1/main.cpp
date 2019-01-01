/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#ifndef USE_OPENSSL
#define USE_OPENSSL
#endif

#include "connection/loginapp_public_key.hpp"
#include "connection/rsa_stream_encoder.hpp"
#include "connection/server_connection.hpp"
#include "connection/server_message_handler.hpp"

#include "resmgr/bwresource.hpp"

#include <iostream>

#ifdef _WIN32
#ifdef _XBOX360
#include <xtl.h>
#else
#include <winsock.h>
#endif
#include <conio.h>
#endif

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

bool exitCondition()
{
	static bool shouldExit = false;

#if defined( _WIN32 ) && !defined( _XBOX )
	while (_kbhit() && !shouldExit)
	{
		char key = _getch();
		if (key == 'q' || key == 'Q')
			shouldExit = true;
	}
#endif

	return shouldExit;
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
	 *	This method is called when the base part of the player has been created.
	 */
	virtual void onBasePlayerCreate( EntityID id, EntityTypeID type,
		BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onBasePlayerCreate: id = "
			<< id << std::endl;
		g_playerID = id;
		data.finish(); // We don't actually process the data in this example.
	}

	/**
	 *	This method is called when the cell part of the player has been created.
	 */
	virtual void onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch,	float roll,
		BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onCellPlayerCreate: id = "
			<< id << std::endl;
		g_spaceID = spaceID;
		data.finish(); // We don't actually process the data in this example.
	}

	/**
	 *	This method is called when an entity enters the client's AOI.
	 */
	virtual void onEntityEnter( EntityID id, SpaceID spaceID, EntityID )
	{
		std::cout << "MyServerMessageHandler::onEntityEnter: id = "
			<< id << std::endl;
		serverConnection_.requestEntityUpdate( id );
	}

	/**
	 *	This method is called when an entity leaves the client's AOI.
	 */
	virtual void onEntityLeave( EntityID id, const CacheStamps & stamps )
	{
		std::cout << "MyServerMessageHandler::onEntityLeave: id = "
			<< id << std::endl;
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
		data.finish(); // We don't actually process the data in this example.
	}

	/**
	 *	This method is called by the server to update some properties of
	 *	the given entity, while it is in our AoI.
	 */
	virtual void onEntityProperties( EntityID id, BinaryIStream & data )
	{
		std::cout << "MyServerMessageHandler::onEntityProperties: id = "
			<< id << std::endl;
		data.finish(); // We don't actually process the data in this example.
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
		data.finish(); // We don't actually process the data in this example.
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
		data.finish(); // We don't actually process the data in this example.
	}


	/**
	 *	This method is called when the position of an entity changes.
	 */
	virtual void onEntityMove( EntityID entityID, SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos,	float yaw, float pitch, float roll,
		bool isVolatile )
	{
#ifndef _XBOX360
		std::cout << "MyServerMessageHandler::onEntityMove: "
			"id = " << entityID << ". pos = " << pos << std::endl;
#endif
		if (entityID == g_playerID)
		{
			g_spaceID = spaceID;
			serverConnection_.addMove( entityID, spaceID, vehicleID, pos,
					yaw, pitch, roll, false, pos );
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

	virtual void spaceData( SpaceID spaceID, SpaceEntryID entryID, uint16 key,
		const std::string & data )
	{
		std::cout << "MyServerMessageHandler::spaceData:" << std::endl;
	}

	virtual void spaceGone( SpaceID spaceID )
	{
		std::cout << "MyServerMessageHandler::spaceGone:" << std::endl;
	}

private:
	ServerConnection & serverConnection_;
};


int main( int argc, char * argv[] )
{
	BWResource::init(argc, (const char**)argv);

	std::cout << "Press 'q' to quit" << std::endl;

	ServerConnection serverConnection;
	MyServerMessageHandler handler( serverConnection );

	double localTime = 0.f;
	serverConnection.pTime( &localTime );

	// No support for actually entering text on the 360, so just hardcode
	// connection parameters
#ifdef _XBOX360
	const char * serverName = "elwood.syd.microforte.com.au";
	const char * userName = "360client";
#else
	const char * serverName = NULL;
	const char * userName = NULL;
#endif

	const char * password = "aa";

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

	if (!status.succeeded())
	{
		std::cout << "Login failed with status " << status << std::endl;
		std::cout << serverConnection.errorMsg() << std::endl;
		std::cout << "Press 'q' to quit" << std::endl;

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
		const float radius = 10.f;
		const float period = 10.f;
		const int updateRate = 12;
		const float timeAdjust = 2 * MATH_PI / period;

		const float angle = float(localTime) * timeAdjust;

		Vector3 position( radius * sinf( angle ), 0.f, radius * cosf( angle ) );

		serverConnection.addMove( g_playerID, g_spaceID, 0, position,
			angle + MATH_PI/2.f, 0.f, 0.f, true, position );

#if 0
		std::cout
			<< "now = " << time << std::endl
			<< "lastGameTime() = " << serverConnection.lastGameTime() << std::endl
			<< "gameTime() = " << serverConnection.serverTime(time) << std::endl
			<< "lastMessageTime() = " << serverConnection.lastMessageTime() << std::endl;
#endif

		mySleep( 1.0/updateRate );
		localTime += 1.f/updateRate;

		serverConnection.send();
		serverConnection.processInput();
	}

	return 0;
}
