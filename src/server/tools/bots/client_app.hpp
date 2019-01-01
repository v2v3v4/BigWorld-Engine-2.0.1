/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLIENT_APP_HPP
#define CLIENT_APP_HPP

#include "Python.h"

#include "entity.hpp"
#include "main_app.hpp"

#include "common/space_data_types.hpp"

#include "connection/server_connection.hpp"
#include "connection/server_message_handler.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <queue>

class MovementController;
/*~ class BigWorld.ClientApp 
 *  @components{ bots } 
 *  
 *  Instances of the ClientApp class represent simulated clients
 *  running on the Bots application.
 *
 *  An instance of ClientApp behaves exactly as a normal BigWorld
 *  client in terms of data exchange with BigWorld server. After a
 *  ClientApp is created, it performs standard login procedure with
 *  either randomn or specfied name/password. It creates a simplified
 *  version of player entity under instructions from the server. From
 *  here on the player entity ID becomes the ClientApp ID. The
 *  ClientApp receives all data within the player AOI, remote
 *  procedure calls from the server as a normal BigWorld client would.
 *  However, it only process limited amount of these data, as most
 *  data is ignored. The default behaviour of the ClientApp (while it
 *  is connected with a server) is to send new player entity positions
 *  to the server. The player entity motion is driven either randomnly
 *  or by the movement controller that the Bots application is
 *  using. Additional game logic can be implemented in Python script
 *  to make simulation closer to the scenarios found in the real world
 *  gameplay. The ClientApp can also simulate realistic inter-network
 *  environment where network delay and dropoff often occur.
 */

/**
 *	This class is used to handle messages from the server.
 */
class ClientApp : public PyObjectPlus,
	public ServerMessageHandler
{
	Py_Header( ClientApp, PyObjectPlus )

public:
	ClientApp( const std::string & name, const std::string & password,
		 const std::string & tag, PyTypeObject * pType = &ClientApp::s_type_ );
	virtual ~ClientApp();

	// ---- Python related ----
	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	// ---- Overrides from ServerMessageHandler ----
	virtual void onBasePlayerCreate( EntityID id, EntityTypeID type,
		BinaryIStream & data );

	virtual void onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data );

	virtual void onEntityEnter( EntityID id, SpaceID spaceID, EntityID );

	virtual void onEntityLeave( EntityID id, const CacheStamps & stamps );

	virtual void onEntityCreate( EntityID id, EntityTypeID type,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data );

	virtual void onEntityProperties( EntityID id, BinaryIStream & data );

	virtual void onEntityProperty( EntityID entityID, int propertyID,
		BinaryIStream & data );

	virtual void onEntityMethod( EntityID entityID, int methodID,
		BinaryIStream & data );

	virtual void onEntityControl( EntityID id, bool control );

	virtual void onEntityMove(
		EntityID entityID, SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, float yaw, float pitch, float roll,
		bool isVolatile );

	virtual void setTime( GameTime gameTime,
			float initialTimeOfDay, float gameSecondsPerSecond );

	virtual void spaceData( SpaceID spaceID, SpaceEntryID entryID, uint16 key,
		const std::string & data );

	virtual void spaceGone( SpaceID spaceID );

	virtual void onProxyData( uint16 proxyDataID, BinaryIStream & data );

	virtual void onEntitiesReset( bool keepPlayerOnBase );

	virtual void onStreamComplete( uint16 id, const std::string &desc,
		BinaryIStream &data );

	// ---- To be overriden ----
	virtual void addMove( double time );

	// ---- General interface ----
	bool tick( float dTime );

	void logOn();
	void logOff();					
	void dropConnection();
	void setConnectionLossRatio( float lossRatio );
	void setConnectionLatency( float latencyMin, float latencyMax );

	const std::string & tag() const			{ return tag_; }
	EntityID id() const						{ return playerID_; }

	bool setMovementController( const std::string & type,
			const std::string & data );
	void moveTo( const Position3D &pos );
	void faceTowards( const Position3D &pos );
	void snapTo( const Position3D &pos ) { position_ = pos; }
	bool isMoving() const { return pDest_ != NULL; };
	void stop();

	int addTimer( float interval, PyObjectPtr callback, bool repeat );
	void delTimer( int id );

	void destroy();

	const ServerConnection * getServerConnection() const	{ return &serverConnection_; }

	void connectionSendTimeReportThreshold( double threshold )
	{
		serverConnection_.sendTimeReportThreshold( threshold );
	}

	typedef std::map<EntityID, Entity *> EntityMap;
	const EntityMap & entities() const { return entities_; }

protected:
	ServerConnection serverConnection_;
	EntityMap entities_;

	SpaceID			spaceID_;
	EntityID		playerID_;
	EntityID		vehicleID_;

	LoginHandlerPtr pLoginInProgress_;
	bool			isDestroyed_;
	bool			isDormant_;

	std::string		userName_;
	std::string		userPasswd_;
	std::string		tag_;
	float			speed_;
	Vector3			position_;
	Direction3D		direction_;

	MovementController * pMovementController_;
	bool			autoMove_;
	Vector3			*pDest_;
	bool			hasPlayerControl_;

	// This stuff is to manage bot timers
	class TimerRec
	{
	public:
		TimerRec( float interval, PyObjectPtr &pFunc, bool repeat ) :
			id_( ID_TICKER++ ), interval_( interval ), pFunc_( pFunc ),
			repeat_( repeat )
		{
			// Go back to 0 on overflow, since negative return values from
			// addTimer() indicate failure
			if (ID_TICKER < 0)
				ID_TICKER = 0;

			startTime_ = MainApp::instance().localTime();
		}

		bool operator< ( const TimerRec &other ) const
		{
			return finTime() >= other.finTime();
		}

		bool elapsed() const
		{
			return finTime() <= MainApp::instance().localTime();
		}

		int id() const { return id_; }
		float interval() const { return interval_; }
		float finTime() const { return startTime_ + interval_; }
		bool repeat() const { return repeat_; }
		PyObject* func() const { return pFunc_.getObject(); }
		void restart() { startTime_ = MainApp::instance().localTime(); }

	private:
		static int ID_TICKER;

		int id_;
		float interval_;
		float startTime_;
		PyObjectPtr pFunc_;
		bool repeat_;
	};

	std::priority_queue< TimerRec > timerRecs_;
	std::list< int > deletedTimerRecs_;
	void processTimers();

	PY_RO_ATTRIBUTE_DECLARE( playerID_, id );
	PY_RO_ATTRIBUTE_DECLARE( spaceID_, spaceID );
	PY_RO_ATTRIBUTE_DECLARE( userName_, loginName );
	PY_RO_ATTRIBUTE_DECLARE( userPasswd_, loginPassword );
	PY_RO_ATTRIBUTE_DECLARE( serverConnection_.online(), isOnline );
	PY_RO_ATTRIBUTE_DECLARE( isDestroyed_, isDestroyed );
	PY_RO_ATTRIBUTE_DECLARE( this->isMoving(), isMoving );
	PY_RW_ATTRIBUTE_DECLARE( tag_, tag );
	PY_RW_ATTRIBUTE_DECLARE( speed_, speed );
	PY_RW_ATTRIBUTE_DECLARE( position_, position );
	PY_RW_ATTRIBUTE_DECLARE( direction_.yaw, yaw );
	PY_RW_ATTRIBUTE_DECLARE( direction_.pitch, pitch );
	PY_RW_ATTRIBUTE_DECLARE( direction_.roll, roll );
	PY_RW_ATTRIBUTE_DECLARE( autoMove_, autoMove );

	PY_AUTO_METHOD_DECLARE( RETVOID, logOn, END );
	PY_AUTO_METHOD_DECLARE( RETVOID, logOff, END );
	PY_AUTO_METHOD_DECLARE( RETVOID, dropConnection, END );
	PY_AUTO_METHOD_DECLARE( RETVOID, setConnectionLossRatio, ARG( float, END ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, setConnectionLatency, ARG( float, ARG( float, END ) ) );

	PY_AUTO_METHOD_DECLARE( RETOK, setMovementController,
		ARG( std::string, ARG( std::string, END ) ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, moveTo,
		ARG( Vector3, END ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, faceTowards,
		ARG( Vector3, END ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, snapTo,
		ARG( Vector3, END ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, stop, END );

	PY_AUTO_METHOD_DECLARE( RETDATA, addTimer,
		ARG( float, ARG( PyObjectPtr, ARG( bool, END ) ) ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, delTimer,
		ARG( int, END ) );

	PyObject * pEntities_;
	PY_RO_ATTRIBUTE_DECLARE( pEntities_, entities );
};

#endif // CLIENT_APP_HPP
