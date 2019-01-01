/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWMACHINED_HPP
#define BWMACHINED_HPP

#include "cstdmf/singleton.hpp"
#include "network/endpoint.hpp"
#include "server/server_info.hpp"
#include "cluster.hpp"
#include "listeners.hpp"
#include "usermap.hpp"
#include "common_machine_guard.hpp"

class BWMachined : public Singleton< BWMachined >
{
public:
	BWMachined();
	~BWMachined();

	bool readConfigFile();
	int run();

	static const char *STATE_FILE;
	void save();
	void load();

	Endpoint& ep() { return ep_; }
	Cluster& cluster() { return cluster_; }
	const char * timingMethod() const { return timingMethod_.c_str(); }

	void closeEndpoints();

	friend class Cluster;

private:
	void initArchitecture();
	void initNetworkInterfaces();

	// The word size of the host architecture (32 / 64)
	uint8 hostArchitecture_;

protected:
	bool findBroadcastInterface();
	void updateSystemInfo();
	void update();
	bool broadcastToListeners( ProcessMessage &pm, int type );
	void removeRegisteredProc( unsigned index );
	void sendSignal (const SignalMessage & sm);
	void handlePacket( Endpoint & ep, sockaddr_in &sin, MGMPacket &packet );
	bool handleMessage( sockaddr_in &sin, MachineGuardMessage &mgm,
		MGMPacket &replies );
	void readPacket( Endpoint & ep, TimeQueue64::TimeStamp & tickTime );

	bool readConfigFile( FILE * fileName );
	const char * findOption( 
		const char * optionName, const char * oldOptionName );

	// Time between statistics updates (in ms)
	static const TimeQueue64::TimeStamp UPDATE_INTERVAL = 1000;

	class IncomingPacket
	{
	public:
		IncomingPacket( BWMachined &machined,
			MGMPacket *pPacket, Endpoint &ep, sockaddr_in &sin );
		~IncomingPacket() {	delete pPacket_; }
		void handle();

	private:
		BWMachined &machined_;
		MGMPacket *pPacket_;
		Endpoint *pEp_;
		sockaddr_in sin_;
	};

	class PacketTimeoutHandler : public TimerHandler
	{
	public:
		void handleTimeout( TimerHandle handle, void *pUser );
		void onRelease( TimerHandle handle, void *pUser );
	};

	PacketTimeoutHandler packetTimeoutHandler_;

	class UpdateHandler : public TimerHandler
	{
	public:
		UpdateHandler( BWMachined &machined ) : machined_( machined ) {}
		void handleTimeout( TimerHandle handle, void *pUser );
		void onRelease( TimerHandle handle, void *pUser ) {}

	protected:
		BWMachined &machined_;
	};

	// The IP of the interface through which broadcast message are sent
	u_int32_t broadcastAddr_;
	Endpoint ep_;

	// Endpoint listening to 255.255.255.255 for broadcasts
	Endpoint epBroadcast_;

	// Endpoint listening to the localhost interface
	Endpoint epLocal_;

	Cluster cluster_;

	typedef std::map< std::string, Tags > TagsMap;
	TagsMap tags_;
	std::string timingMethod_;
	std::string architecture_;

	SystemInfo systemInfo_;
	std::vector< ProcessInfo > procs_;

	Listeners birthListeners_;
	Listeners deathListeners_;
	UserMap users_;

	// A global TimeQueue for managing all callbacks in this app
	TimeQueue64 callbacks_;

	// A ServerInfo object for querying performance information from the system
	ServerInfo* pServerInfo_;

public:
	// The total period (in ms) over which we would like broadcasts replied to
	static const TimeQueue64::TimeStamp STAGGER_REPLY_PERIOD = 100;

	TimeQueue64::TimeStamp timeStamp();

	// Convert a struct timeval into a time in millseconds (only use for small tv's)
	inline TimeQueue64::TimeStamp tvToTimeStamp( struct timeval &tv ) {
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}

	// Sets a time in millseconds into a struct timeval
	inline void timeStampToTV( TimeQueue64::TimeStamp ms, struct timeval &tv ) {
		tv.tv_sec = ms/1000;
		tv.tv_usec = (ms%1000)*1000;
	}

	TimeQueue64& callbacks() { return callbacks_; }
};

#endif
