/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "hostnames.hpp"

#include "cstdmf/debug.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>


bool Hostnames::init( const char *root, const char *mode )
{
	const char *hostnamesPath = this->join( root, "hostnames" );
	return TextFileHandler::init( hostnamesPath, mode );
}


void Hostnames::flush()
{
	hostnames_.clear();
}


bool Hostnames::handleLine( const char *line )
{
	char hostIp[64];
	char hostName[1024];
	struct in_addr addr;

	if (sscanf( line, "%63s %1023s", hostIp, hostName ) != 2)
	{
		ERROR_MSG( "Hostnames::handleLine: Unable to read hostnames file "
			"entry (%s)\n", line );
		return false;
	}

	if (inet_aton( hostIp, &addr ) == 0)
	{
		ERROR_MSG( "Hostnames::handleLine: Unable to convert hostname "
			"entry '%s' to a valid IPv4 address\n", hostIp );
		return false;
	}
	else
	{
		hostnames_[ addr.s_addr ] = hostName;
		return true;
	}
}


/**
 * This method attempts to discover the hostname associated with the provided
 * IP address.
 *
 * If the hostname does not already exist in the hostname map a network
 * query will occur to try and resolve the host.
 *
 * @returns A hostname (or IP) of the address on success, NULL on error.
 */
const char * Hostnames::getHostByAddr( uint32 addr )
{
	HostnamesMap::iterator it = hostnames_.find( addr );
	if (it != hostnames_.end())
	{
		return it->second.c_str();
	}

	struct hostent *ent = gethostbyaddr( &addr, sizeof( addr ), AF_INET );
	const char *hostname;

	// Unable to resolve the hostname, store the IP address as a string
	if (ent == NULL)
	{
		const char *reason = NULL;
		hostname = inet_ntoa( (in_addr&)addr );

		switch (h_errno)
		{
			case HOST_NOT_FOUND:
				reason = "HOST_NOT_FOUND"; break;
			case NO_DATA:
				reason = "NO_DATA"; break;
			case NO_RECOVERY:
				reason = "NO_RECOVERY"; break;
			case TRY_AGAIN:
				reason = "TRY_AGAIN"; break;
			default:
				reason = "Unknown reason";
		}

		WARNING_MSG( "Hostnames::getHostByAddr: Unable to resolve hostname "
			"of %s (%s)\n", hostname, reason );
	}
	else
	{
		char *firstdot = strstr( ent->h_name, "." );
		if (firstdot != NULL)
		{
			*firstdot = '\0';
		}
		hostname = ent->h_name;
	}

	// Write the mapping to disk
	const char *ipstr = inet_ntoa( (in_addr&)addr );
	char line[ 2048 ];
	bw_snprintf( line, sizeof( line ), "%s %s", ipstr, hostname );
	if (!this->writeLine( line ))
	{
		CRITICAL_MSG( "Hostnames::getHostByAddr: "
			"Couldn't write hostname mapping for %s\n", line );
		return NULL;
	}

	hostnames_[ addr ] = hostname;
	return hostnames_[ addr ].c_str();
}


/**
 * This method retrieves the address associated with the specified hostname
 * from the hostname map.
 *
 * @returns IP address of the hostname on success, 0 if not known.
 */
uint32 Hostnames::getHostByName( const char *hostname ) const
{
	std::string hostString = hostname;
	HostnamesMap::const_iterator it = hostnames_.begin();

	while (it != hostnames_.end())
	{
		if (it->second == hostString)
		{
			return it->first;
		}

		++it;
	}

	return 0;
}


/**
 * This method invokes onHost on the visitor for all the hostname entries
 * stored in the hostname map.
 *
 * @returns true on success, false on error.
 */
bool Hostnames::visitAllWith( HostnameVisitor &visitor ) const
{
	HostnamesMap::const_iterator iter = hostnames_.begin();
	bool status = true;

	while ((iter != hostnames_.end()) && (status == true))
	{
		status = visitor.onHost( iter->first, iter->second );
		++iter;
	}

	return status;
}
