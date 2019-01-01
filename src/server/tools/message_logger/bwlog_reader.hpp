/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWLOG_READER_HPP
#define BWLOG_READER_HPP

#include "bwlog_common.hpp"
#include "hostnames.hpp"
#include "user_log_reader.hpp"

#include "cstdmf/stdmf.hpp"

#include <map>
#include <string>

typedef std::map< uint16, std::string > UsernamesMap;

class BWLogReader : public BWLogCommon
{
public:
	//BWLogReader();
	~BWLogReader();

	bool init( const char *root );
	bool onUserLogInit( uint16 uid, const std::string &username );


	/* Main Public Interface */
	const char *getLogDirectory() const;
	bool getComponentNames( LogComponentsVisitor &visitor ) const;
	bool getHostnames( HostnameVisitor &visitor ) const;
	FormatStringList getFormatStrings() const;
	const UsernamesMap & getUsernames() const;

	UserLogReaderPtr getUserLog( uint16 uid );

	uint32 getAddressFromHost( const char *hostname ) const;

	const LogStringInterpolator *getHandlerForLogEntry( const LogEntry &entry );

	bool refreshFileMaps();

private:
	UsernamesMap usernames_;

	// Both BWLogWriter and BWLogReader need a UserLog list, however they both
	// handle the descruction of these lists differently so are kept in their
	// respective subclasses.
	UserLogs userLogs_;
};

#endif // BWLOG_READER_HPP
