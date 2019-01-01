/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HOSTNAMES_HPP
#define HOSTNAMES_HPP

#include "text_file_handler.hpp"

#include "cstdmf/stdmf.hpp"

#include <map>
#include <string>

/**
 * This abstract class provides a template which can be used to retrieve
 * the hosts being stored in a Hostnames instance.
 */
class HostnameVisitor
{
public:
	virtual bool onHost( uint32 addr, const std::string &hostname ) = 0;
};


/**
 * This class handles the mapping between IP addresses and hostnames
 */
class Hostnames : public TextFileHandler
{
public:
	bool init( const char *root, const char *mode );

	virtual bool handleLine( const char *line );

	const char * getHostByAddr( uint32 addr );

	uint32 getHostByName( const char *hostname ) const;

	bool visitAllWith( HostnameVisitor &visitor ) const;

protected:
	virtual void flush();

private:
	typedef std::map< uint32, std::string > HostnamesMap;
	HostnamesMap hostnames_;
};

#endif // HOSTNAMES_HPP
