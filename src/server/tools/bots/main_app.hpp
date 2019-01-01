/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MAIN_APP_HPP
#define MAIN_APP_HPP

#include "Python.h"

#include "bots_config.hpp"
#include "space_data_manager.hpp"

#include "cstdmf/md5.hpp"
#include "cstdmf/singleton.hpp"

#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

#include "pyscript/script.hpp"

#include "server/server_app.hpp"

#include <memory>

class ClientApp;
class StreamEncoder;
class MovementController;
class MovementFactory;
class PythonServer;

class MainApp : public ServerApp,
		public TimerHandler,
		public Singleton< MainApp >
{
public:
	typedef BotsConfig Config;

	SERVER_APP_HEADER( Bots, bots )

	MainApp( Mercury::EventDispatcher & mainDispatcher, 
		Mercury::NetworkInterface & networkInterface );
	virtual		~MainApp();

	// ServerApp overrides
	virtual bool init( int argc, char * argv[] );

	// TimerHandler overrides
	virtual void handleTimeout( TimerHandle timerHandle, void * args );

	void addBot();
	void addBots( int num );
	void addBotsWithName( PyObjectPtr logInfoData );
	void delBots( int num );

	void delTaggedEntities( std::string tag );

	void updateMovement( std::string tag );
	void runPython( std::string tag );

	void loginMD5Digest( std::string quoteDigest );
	std::string loginMD5Digest() const	{ return loginDigest_.quote(); }

	MovementController * createDefaultMovementController( float & speed,
		Vector3 & position );
	MovementController * createMovementController( float & speed,
		Vector3 & position, const std::string & controllerType,
		const std::string & controllerData );

	static void addFactory( const std::string & name,
									MovementFactory & factory );


	// ---- Accessors ----
	StreamEncoder * pLogOnParamsEncoder()
		{ return pLogOnParamsEncoder_.get(); }

	const MD5::Digest digest() const
		{ return loginDigest_; }

	const double & localTime() const
		{ return localTime_; }

	double sendTimeReportThreshold() const
		{ return sendTimeReportThreshold_; }

	void sendTimeReportThreshold( double threshold );

	// ---- Script related Methods ----
	ClientApp * findApp( EntityID id ) const;
	void appsKeys( PyObject * pList ) const;
	void appsValues( PyObject * pList ) const;
	void appsItems( PyObject * pList ) const;

	// ---- Get personality module ----
	PyObjectPtr getPersonalityModule() const;

	SpaceDataManager & spaceDataManager()
		{ return spaceDataManager_; }

private:
	void parseCommandLine( int argc, char * argv[] );
	bool initScript();
	void initWatchers();

	SpaceDataManager spaceDataManager_;

	std::auto_ptr< StreamEncoder > 	pLogOnParamsEncoder_;

	typedef std::list< SmartPointer< ClientApp > > Bots;
	Bots bots_;

	double localTime_;
	TimerHandle timerHandle_;
	double sendTimeReportThreshold_;

	PythonServer *	pPythonServer_;

	Bots::iterator clientTickIndex_;
	MD5::Digest loginDigest_;

};

#endif // MAIN_APP_HPP
