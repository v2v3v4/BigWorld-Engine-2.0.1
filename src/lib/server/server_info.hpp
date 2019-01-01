/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_INFO_HPP
#define SERVER_INFO_HPP

#include "cstdmf/init_singleton.hpp"

/**
 * 	This class provides details about the server
 * 	hardware being run on
 */
class ServerInfo
{
public:
	/// Constructor
	ServerInfo();

	/// Returns the hostname of this machine
	const std::string & serverName() const { return serverName_; }


	/// Returns a textual description of this machine's CPU
	const std::string & cpuInfo() const { return cpuInfo_; }
	/// Returns a vector of CPU speeds for the CPU cores in this machine
	const std::vector<float>& cpuSpeeds() const { return cpuSpeeds_; }


	/// Returns a textual description of this machine's RAM
	const std::string & memInfo() const { return memInfo_; }
	/// Returns the number of bytes of total RAM
	const uint64 memTotal() const { return memTotal_; }
	/// Returns the number of bytes of process-used RAM
	const uint64 memUsed() const { return memUsed_; }
	/// Request an update of the RAM statistics
	void updateMem();

private:

#ifndef _WIN32 // WIN32PORT
//#else // _WIN32
	void fetchLinuxCpuInfo();
	void fetchLinuxMemInfo();
#endif // _WIN32

	std::string serverName_;

	std::string cpuInfo_;
	std::vector<float> cpuSpeeds_;

	std::string memInfo_;
	uint64 memTotal_;
	uint64 memUsed_;
};

#endif // SERVER_INFO_HPP
