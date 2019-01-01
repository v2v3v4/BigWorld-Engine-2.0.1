/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __WEB_INTEGRATION_HPP__
#define __WEB_INTEGRATION_HPP__

#include "Python.h"

#include "cstdmf/singleton.hpp"

#include "network/logger_message_forwarder.hpp"

#include <memory>
#include <string>

class BackupHashChain;
class BlockingDBLookUpHandler;
class EntityDescriptionMap;

namespace Mercury
{
class BlockingReplyHandler;
class EventDispatcher;
class NetworkInterface;
}


/**
 *	This class represents the web integration component.
 */
class WebIntegration : public Singleton< WebIntegration >
{
public:

	WebIntegration();
	virtual ~WebIntegration();

	bool init();

	Mercury::EventDispatcher & dispatcher();
	Mercury::NetworkInterface & interface();

	/**
	 *	Return the last known address of the DBMgr component.
	 *
	 *	@return reference to the last known address of the DBMgr
	 */
	const Mercury::Address & dbMgrAddr() const
	{ return dbMgrAddr_; }

	const Mercury::Address & getDBMgrAddr( bool forget = false );

	/**
	 *	Const-accessor for the entity description map.
	 */
	const EntityDescriptionMap & entityDescriptions() const
	{ return *pEntityDescriptions_; }

	void resetNetworkInterface( uint16 port );

	int logOn( const std::string & username, const std::string & password,
		bool allowAlreadyLoggedOn = true );

	PyObject * lookUpEntityByName( const std::string & entityTypeName,
		const std::string & entityName );

	PyObject * lookUpEntityByDBID( const std::string & entityTypeName,
		DatabaseID dbID );

	bool checkBaseAppHashHistory();

	const BackupHashChain & baseAppBackupHashChain() const 
	{ return *pBackupHashChain_; }

private: // private methods

	bool checkDBMgr();

	PyObject * lookUpEntityComplete( BlockingDBLookUpHandler & handler,
		Mercury::Bundle & bundle );

	bool getEntityTypeByName( const std::string & name,
		EntityTypeID & id );
	
	bool updateBaseAppHashHistory();

private:
	std::auto_ptr< Mercury::NetworkInterface >
							pInterface_;

	std::auto_ptr< Mercury::EventDispatcher >
							pDispatcher_;

	pid_t					createPid_;		// the pid of the process that
											// created the network interface
	Mercury::Address 		dbMgrAddr_;
	EntityDescriptionMap * 	pEntityDescriptions_;
	bool					hasInited_;

	std::auto_ptr< BackupHashChain >	pBackupHashChain_;

	// A superclass of Endpoint that automatically opens itself to a random
	// port and may switch to another random port with a simple function call.
	// Useful here because LoggerMessageForwarder uses a socket in its
	// constructor but both are static members of WebIntegration.
	class LoggerEndpoint : public Endpoint
	{
	public:
		LoggerEndpoint();
		bool switchSocket();
	};


	LoggerEndpoint loggerSocket_;
	SimpleLoggerMessageForwarder loggerMessageForwarder_;
};

#endif // __WEB_INTEGRATION_HPP__
