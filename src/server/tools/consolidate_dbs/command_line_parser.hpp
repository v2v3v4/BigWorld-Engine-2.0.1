/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TOOLS__CONSOLIDATE_DBS__COMMAND_LINE_PARSER_HPP
#define TOOLS__CONSOLIDATE_DBS__COMMAND_LINE_PARSER_HPP

#include "dbmgr_mysql/connection_info.hpp"

#include <memory>
#include <string>
#include <vector>

class CommandLineParser
{
public:
	CommandLineParser();
	~CommandLineParser();

	bool init( int argc, char * const argv[] );
	
	bool isVerbose() const 
		{ return isVerbose_; }

	bool shouldClear() const
		{ return shouldClear_; }

	bool shouldIgnoreSqliteErrors() const
		{ return shouldIgnoreSqliteErrors_; }

	const std::string & resPath() const
		{ return resPath_; }

	bool hadNonOptionArgs() const 
		{ return hadNonOptionArgs_; }

	const DBConfig::ConnectionInfo & primaryDatabaseInfo() const 
		{ return primaryDatabaseInfo_; }

	typedef std::vector< std::string > SecondaryDatabases;
	const SecondaryDatabases & secondaryDatabases() const
		{ return secondaryDatabases_; }

	void printUsage() const;

private:
	bool parsePrimaryDatabaseInfo( const std::string & commandLineSpec );
	bool readSecondaryDatabaseList( const std::string & path );

// Member data

	bool isVerbose_;
	bool shouldClear_;
	bool shouldIgnoreSqliteErrors_;
	bool hadNonOptionArgs_;

	std::string resPath_;

	DBConfig::ConnectionInfo primaryDatabaseInfo_;
	SecondaryDatabases secondaryDatabases_;
};

#endif // TOOLS__CONSOLIDATE_DBS__COMMAND_LINE_PARSER_HPP
