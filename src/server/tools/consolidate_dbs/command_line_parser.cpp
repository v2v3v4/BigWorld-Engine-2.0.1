/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "command_line_parser.hpp"

#include "cstdmf/debug.hpp"

#include "dbmgr_mysql/db_config.hpp"

#include <getopt.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>

namespace // (anonymous)
{
const char * USAGE_MESSAGE =
	"\n"
	"Usage: consolidate_dbs [options]\n"
	"       consolidate_dbs [options] <primarydb_spec> <secondarydb_list> "
												"(when invoked from DBMgr)\n\n"
	"Options:\n"
	" --ignore-sqlite-errors    do not abort on sqlite read errors\n"
	" --clear                   clear unconsolidated secondary database info\n"
	" -v, --verbose             verbose output\n"
	" -h, --help                print usage\n";

} // end namespace (anonymous)

/**
 *	Constructor.
 */
CommandLineParser::CommandLineParser():
		isVerbose_( false ),
		shouldClear_( false ),
		shouldIgnoreSqliteErrors_( false ),
		hadNonOptionArgs_( false ),
		resPath_(),
		primaryDatabaseInfo_( DBConfig::connectionInfo() ),
		secondaryDatabases_()
{}


/**
 *	Destructor.
 */
CommandLineParser::~CommandLineParser()
{}


/**
 *	Initialise the parser with the command line arguments.
 */
bool CommandLineParser::init( int argc, char * const argv[] )
{
	bool ok = true;

	static const struct option s_longOptions[] =
	{
		{ "res", required_argument, NULL, 'r' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "help", no_argument, NULL, 'h' },
		{ "clear", no_argument, NULL, 'c' },
		{ "ignore-sqlite-errors", no_argument, NULL, 'i' },
		{ 0, 0, 0, 0 }
	};

	static const char * s_shortOptions = ":r:vh";

	int ch = 0;
	while (ok &&
			-1 !=  (ch = getopt_long( argc, argv, 
					s_shortOptions, s_longOptions, NULL )))
	{
		switch (ch)
		{
		case 'r':
			resPath_ = optarg;
			break;

		case 'v':
			isVerbose_ = true;
			break;

		case 'h':
			this->printUsage();
			ok = false;
			break;

		case 'c':
			shouldClear_ = true;
			break;

		case 'i':
			shouldIgnoreSqliteErrors_ = true;
			break;

		case ':':
			ERROR_MSG( "Missing argument for option -%c", optopt );
			ok = false;
			break;

		case '?':
		default:
			ERROR_MSG( "Unknown option -%c", optopt );
			ok = false;
			break;
		}
	}

	if (!ok)
	{
		return false;
	}

	// We currently only support specifying:
	// 1) Nothing on the command-line
	// OR
	// 2) The primary and a file containing all secondary database paths. 
	// We can't make the paths arguments as there can arbitrarily many and we
	// may exceed command line limit.

	if (optind == argc)
	{
		// No args, we're done!
		return true;
	}

	if (argc - optind < 2)
	{
		ERROR_MSG( "Primary database specified, but no path to secondary "
				"database list was specified\n" );
		this->printUsage();
		return false;
	}

	if (optind < argc && argc - optind > 2)
	{
		WARNING_MSG( "Extraneous arguments after secondary database list "
				"path ignored" );
	}

	hadNonOptionArgs_ = true;

	std::string primarySpec( argv[optind] );
	std::string secondaryDatabaseListPath( argv[optind + 1] );

	if (shouldClear_)
	{
		WARNING_MSG( "consolidate_dbs: The --clear option does not take "
				"additional arguments\n" );
		// Don't parse, we use the bw.xml for primary database info.
		return true;
	}

	return this->parsePrimaryDatabaseInfo( primarySpec ) &&
		this->readSecondaryDatabaseList( secondaryDatabaseListPath );
}


/**
 *	
 */
bool CommandLineParser::parsePrimaryDatabaseInfo( 
		const std::string & commandLineSpec )
{
	std::vector< std::string > components;
	std::string::size_type current = 0;
	std::string::size_type next = 0;
	while (std::string::npos != (next = commandLineSpec.find( ';', current )))
	{
		std::string component( commandLineSpec, current, next - current );
		components.push_back( component );
		current = next + 1;
	}

	std::string component( commandLineSpec, current, next );
	components.push_back( component );

	if (components.size() != 5)
	{
		ERROR_MSG( "Primary database argument must be in the form "
				"<host>;<port>;<username>;<password>;<database>\n" );
		return false;
	}

	primaryDatabaseInfo_.host = components[0];

	char * pStrtoulEnd = NULL;
	unsigned long port = strtoul( components[1].c_str(), &pStrtoulEnd, 0 );

	if (!pStrtoulEnd || *pStrtoulEnd != '\0')
	{
		ERROR_MSG( "Could not parse database port :%s\n", 
			components[1].c_str() );
		return false;
	}
	else if (port >= 1 << (sizeof( uint16 ) * 8))
	{
		ERROR_MSG( "Primary database port is out-of-range: %s\n", 
			components[1].c_str() );
		return false;
	}

	primaryDatabaseInfo_.port = uint16( port );

	primaryDatabaseInfo_.username = components[2];
	primaryDatabaseInfo_.password = components[3];
	primaryDatabaseInfo_.database = components[4];

	return true;
}


/**
 *	Read the list of secondary databases from the given list.
 *
 *	@param path 	The path to the list of secondary databases to
 *					consolidate into the primary.
 *
 * 	@return			True on success, otherwise false.
 */
bool CommandLineParser::readSecondaryDatabaseList( const std::string & path )
{
	std::ifstream listFile( path.c_str() );

	if (!listFile.is_open())
	{
		ERROR_MSG( "Could not open secondary database list at \"%s\"\n",
			path.c_str() );
		return false;
	}

	while (!listFile.eof())
	{
		std::string secondaryDatabase;
		std::getline( listFile, secondaryDatabase );

		if (secondaryDatabase.length() > 0)
		{
			secondaryDatabases_.push_back( secondaryDatabase );
		}
	}
	listFile.close();

	return true;
}


/**
 *	Prints the usage text to stdout.
 */
void CommandLineParser::printUsage() const
{
	puts( USAGE_MESSAGE );
}

// command_line_parser.cpp
