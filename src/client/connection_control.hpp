/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONNECTION_CONTROL_HPP
#define CONNECTION_CONTROL_HPP

#include "pyscript/pyobject_plus.hpp"

#if ENABLE_WATCHERS
#include "network/endpoint.hpp"
#include "network/logger_message_forwarder.hpp"
#endif

#include <memory>

class RSAStreamEncoder;
class ServerConnection;
class LoginHandler;
typedef SmartPointer< LoginHandler > LoginHandlerPtr;


/**
 *	This class controls the connection to the BigWorld server.
 */
class ConnectionControl
{
public:
	void connect( const std::string & server,
		PyObject * loginParams,
		SmartPointer<PyObject> progressFn );
	void disconnect();

	// Should we have an object (instance of this class?) managing the connectn,
	// that stats are reported in and accessible from Python? Maybe.
	// But this explicit limited syntax will do for now 'tho.
	const char * server() const;
	void probe( const std::string & server,
		SmartPointer<PyObject> progressFn );

	void tick();

	static ConnectionControl & instance();

	static ServerConnection * serverConnection()
		{ return instance().pServerConnection_; }
	static bool connectedOrConnecting()
		{ return instance().connected_; }

private:
	ConnectionControl();
	~ConnectionControl();
	ConnectionControl( const ConnectionControl& );
	ConnectionControl& operator=( const ConnectionControl& );

	bool initLogOnParamsEncoder( std::string publicKeyPath );

	void disconnected();

	ServerConnection *			pServerConnection_;
	std::auto_ptr< RSAStreamEncoder >
								pLogOnParamsEncoder_;

	LoginHandlerPtr				pLoginInProgress_;
	bool						connected_;	// our idea of connectivity

	SmartPointer<PyObject>	progressFn_;
	uint32					initialPacketsIn_;
	std::string				server_;


#if ENABLE_WATCHERS
	Endpoint loggerMessageEndpoint_;
	LoggerMessageForwarder* pLoggerMessageForwarder_;
#endif

	// progressFn codes:
	//  0: initial checks
	//  1: login (will get disconnect if successful)
	//	2: data
	//	6: disconnected (last call)
	// progressFn params:
	//  <=0: failure
	//  1: success
};


#endif // CONNECTION_CONTROL_HPP
