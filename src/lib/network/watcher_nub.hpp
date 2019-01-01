/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WATCHER_NUB_HPP
#define WATCHER_NUB_HPP

#include "endpoint.hpp"
#include "interfaces.hpp"

#include "cstdmf/singleton.hpp"

class RemoteEndpoint;

class MemoryOStream;

namespace Mercury
{
class EventDispatcher;
}


/**
 * 	This structure is the network message used to register a watcher.
 *
 * 	@ingroup watcher
 */
struct WatcherRegistrationMsg
{
	int		version;	// currently 0
	int		uid;		// your uid
	int		message;	// WATCHER_...
	int		id;			// e.g. 14
	char	abrv[32];	// e.g. "cell14"
	char	name[64];	// e.g. "Cell 14"
};

#ifdef _WIN32
	#pragma warning(disable: 4200)
#endif

/**
 * 	This structure is used to send watcher data across the network.
 *
 * 	@ingroup watcher
 */
struct WatcherDataMsg
{
	int		message;
	int		count;
	char	string[0];
};


/**
 * 	This enumeration contains the possible watcher message IDs.
 *
 * 	@ingroup watcher
 */
enum WatcherMsg
{
	// Deprecated messages used by old http watcher process.
	// WATCHER_MSG_REGISTER = 0,
	// WATCHER_MSG_DEREGISTER = 1,
	// WATCHER_MSG_FLUSHCOMPONENTS = 2,

	WATCHER_MSG_GET = 16,
	WATCHER_MSG_SET = 17,
	WATCHER_MSG_TELL = 18,
	WATCHER_MSG_GET_WITH_DESC = 20,

	WATCHER_MSG_GET2 = 26,
	WATCHER_MSG_SET2 = 27,
	WATCHER_MSG_TELL2 = 28,
	WATCHER_MSG_SET2_TELL2 = 29,

	WATCHER_MSG_EXTENSION_START = 107
};


/* Packets:
	Reg and Dereg are just a WatcherRegistrationMsg Get and Set are a
	WatcherDataMsg followed by 'count' strings (for get) or string pairs (for
	set and tell). Every get/set packet is replied to with a tell packet.
*/

class WatcherPacketHandler;

/**
 *	This class is used to process requests that the WatcherNub has
 *	received. You need one of these.
 *
 * 	@ingroup watcher
 */
class WatcherRequestHandler
{
public:
	virtual ~WatcherRequestHandler() {};

	virtual void processExtensionMessage( int messageID,
				char * data, int dataLen, const Mercury::Address & addr ) = 0;
};


/**
 *	This class is the core of the Watcher. It is responsible for receiving
 *	watcher requests and sending replies.
 *
 * 	@ingroup watcher
 */
class WatcherNub :
		public Mercury::InputNotificationHandler,
		public Singleton< WatcherNub >
{
public:
	WatcherNub();
	~WatcherNub();

	bool init( const char * listeningInterface, uint16 listeningPort );

	int registerWatcher( int id, const char * abrv, const char * longName,
			const char * listeningInterface = NULL,
			uint16 listeningPort = 0 );
	int deregisterWatcher();

	void attachTo( Mercury::EventDispatcher & dispatcher );

	void setRequestHandler( WatcherRequestHandler * pWatcherRequestHandler );

	// read (or try to read) a request
	bool receiveUDPRequest();

	bool processRequest( char * packet, int len,
			const RemoteEndpoint & remoteEndpoint );

	Endpoint & udpSocket()			{ return udpSocket_; }
	Endpoint & tcpSocket()			{ return tcpSocket_; }

	// add to the reply packet - only callable from within the callback.
	bool addReply( const char * identifier, const char * desc,
			const char * value );

	bool addReply2( unsigned int seqNum, MemoryOStream & value );
	bool addDirectoryEntry( MemoryOStream & value );

	int handleInputNotification( int fd );

private:
	void processWatcherGetRequest( WatcherPacketHandler & packetHandler,
			const char * path, bool withDesc = false );

	void processWatcherGet2Request( WatcherPacketHandler & packetHandler,
			const char * path, uint32 seqNum );

	void processWatcherSetRequest( WatcherPacketHandler & packetHandler,
			const char * path, const char * valueString );

	void processWatcherSet2Request( WatcherPacketHandler & packetHandler,
			char *& packet );

	void notifyMachineGuard();

	bool bindSockets( uint16 listeningPort, u_int32_t ifaddr );

	int		id_;
	bool	registered_;

	WatcherRequestHandler * pExtensionHandler_;

	bool	insideReceiveRequest_;

	char	*requestPacket_;

	bool	isInitialised_;

	Endpoint	udpSocket_;
	Endpoint	tcpSocket_;

	char	abrv_[32];
	char	name_[64];

	Mercury::EventDispatcher * pDispatcher_;
};

#endif // WATCHER_NUB_HPP
