/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	This file implements a simple program that converts resources to relesae.
 */


#include "cdata_packer.hpp"
#include "msg_handler.hpp"
#include "packer_helper.hpp"
#include "packers.hpp"
#include "xml_packer.hpp"

#include "cstdmf/bw_util.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/dprintf.hpp"

#ifndef MF_SERVER
#include "moo/init.hpp"
#endif // MF_SERVER

#include "resmgr/bwresource.hpp"

#include <signal.h>

DECLARE_DEBUG_COMPONENT( 0 )

#ifdef WIN32
#define MAX_FILEPATH MAX_PATH
#else
#define MAX_FILEPATH PATH_MAX
#endif


// Packer tokens to ensure that they get compiled
extern int XmlPacker_token;
extern int ImagePacker_token;
extern int FxPacker_token;
extern int ChunkPacker_token;
extern int CDataPacker_token;
extern int ModelAnimPacker_token;
static int s_chunkTokenSet = 0
	| XmlPacker_token
	| ImagePacker_token
	| FxPacker_token
	| ChunkPacker_token
	| CDataPacker_token
	| ModelAnimPacker_token
	;


#ifdef _WIN32
char * basename( char * name )
{
	if ( !name )
		return NULL;

	char * ret = strrchr( name, '\\' );
	if ( ret )
		return ret + 1;
	return name;
}
#endif // _WIN32

void printUsage( char * exeFullName )
{
	char * exeFileName = basename( exeFullName );
	printf(
		"Compatible usage: %s [-r|--res search_paths] input_file [output_file [base_path]]\n"
		"Batch list usage: %s --list|-l asset_list_file\n"
		"    --in|-i input_path_root\n"
		"    --out|-o output_path_root\n"
		"    [--err|-e error_log_file]\n"
		"    [--res|-r search_paths]\n",
		exeFileName, exeFileName );
	printf( "\n"
		"'asset_list_file' is the file which contains all the files to process\n"
		"'input_path_root' is the root directory for the input files\n"
		"'output_path_root' is the root directory for the output files\n"
		"'error_log_file' is the error output file\n"
		"\n"
		"The batch list mode is the new, fast way of using res_packer\n"
		"The argument 'search_paths' is a semicolon-separated list of paths\n"
		"used when processing some assets types such as fonts, models, etc.\n"
		"If 'search_paths' is not specified, the file 'paths.xml' or the \n"
		"environment variable BW_RES_PATH will be used.\n"
		"\n"
		"In the old compatible mode, If an output file is specified, the input data\n"
		"is packed depending on the file type. If an output file is not specified,\n"
		"the input file string representation is printed.\n"
		"Some assets types also require that a correct base path is specified.\n"
		"For example, packing .font and .model assets require valid base_path.\n"
		"The base_path must point to the root path of the assets in the\n"
		"destination folder, so if packing the a file 'dragon.bmp' to the folder\n"
		"'/bw/finalgame/res/maps/monsters/dragon.bmp', and the root of the game\n"
		"will be '/bw/finalgame/res', then base_path must be '/bw/finalgame/res'\n"
		"If an incorrect base_path is specified, packing might fail or might\n"
		"generate invalid assets (for example, an invalid .font file)\n"
		"\n"
		"Full example of the old compatible form:\n"
		"    res_packer --res /bw/fantasydemo/res;/bw/bigworld/res\n"
		"    /bw/fantasydemo/res/models/my_model.model\n"
		"    /bw/fantasydemo/res_packed/models/my_model.model\n"
		"    /bw/fantasydemo/res_packed\n"
		"\n"
		"Full example of the batch list form:\n"
		"    res_packer --list assets.txt\n"
		"    --in /bw/fantasydemo/res\n"
		"    --out /bw/fantasydemo/res_packed\n"
		"    --err error.log\n"
		"    --res /bw/fantasydemo/res;/bw/bigworld/res\n"
		);
}


std::string removeTrailingSlash( const std::string & str )
{
	if (!str.empty())
	{
		std::string lastChar = str.substr( str.length() - 1 );
		if (lastChar == "/" || lastChar == "\\")
		{
			return str.substr( 0, str.length() - 1 );
		}
	}

	return str;
}


static bool s_quitRequested = false;


static void quitSignalHandler( int signo )
{
	if (signo == SIGINT
#ifdef WIN32
		|| signo == SIGBREAK
#endif // WIN32
		)
	{
		printf( "\n\nCTRL+%s was pressed. Finishing processing last asset.\n\n",
			(signo == SIGINT) ? "C" : "Break" );
		s_quitRequested = true;
		signal( signo, SIG_IGN );
	}
}


int main( int argc, char *argv[] )
{
	char * exeName = argv[0];
	char ** pArgs = argv + 1;
	int numArgs = argc - 1;
	bool result = true;

	bool hasAssetList = false;
	FILE* assetList = NULL;
	bool useErrorLog = false;
	FILE* errorLog = NULL;
	bool hasInPath = false;
	char inPath[256];
	bool hasOutPath = false;
	char outPath[256];

	if (signal( SIGINT, quitSignalHandler ) == SIG_ERR
#ifdef WIN32
		|| signal( SIGBREAK, quitSignalHandler ) == SIG_ERR
#endif // WIN32
		)
	{
		printf("Error: Could not set handler to SIGINT.\n");
	}

	// this class handles messages from the BW libs and sends them to cout
	MsgHandler msgHandler;

	DebugFilter::shouldWriteToConsole( true );

#ifdef _DEBUG
	printf( "\nres_packer arguments:\n" );
	for( int i = 0; i < argc; i++ )
	{
		printf( "  %d: '%s'\n", i, argv[i] );
	}
	printf( "\n" );
#endif

	bool processedFlag = true;

	*inPath = '\0';
	*outPath = '\0';

	while (processedFlag)
	{
		processedFlag = false;

		if ((numArgs > 1) &&
			(strcmp( pArgs[0],"--res" ) == 0 ||
				strcmp( pArgs[0],"-r" ) == 0 ))
		{
			// skip command-line-specified paths
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
		else if ((numArgs > 1) &&
				(strcmp( pArgs[0], "--list" ) == 0 ||
				strcmp( pArgs[0],"-l" ) == 0 ))
		{
			hasAssetList = true;
			assetList = bw_fopen( pArgs[1], "r" );
			if (assetList == NULL)
			{
				printf("Unable to open asset list \"%s\"\n", pArgs[1]);
				return EXIT_FAILURE;
			}
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
		else if ((numArgs > 1) &&
				(strcmp( pArgs[0], "--in" ) == 0 ||
				strcmp( pArgs[0],"-i" ) == 0 ))
		{
			hasInPath = true;
			strcpy( inPath, removeTrailingSlash( pArgs[1] ).c_str() );
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
		else if ((numArgs > 1) &&
				(strcmp( pArgs[0], "--out" ) == 0 ||
				strcmp( pArgs[0],"-o" ) == 0 ))
		{
			hasOutPath = true;
			strcpy( outPath, removeTrailingSlash( pArgs[1] ).c_str() );
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
		else if ((numArgs > 1) &&
				(strcmp( pArgs[0], "--err" ) == 0 ||
				strcmp( pArgs[0],"-e" ) == 0 ))
		{
			useErrorLog = true;
			errorLog = bw_fopen( pArgs[1], "w" );
			if (errorLog == NULL)
			{
				printf("Unable to open error log \"%s\"\n", pArgs[1]);
			}
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
		// NOTE: This option is currently not documented.
		else if ((numArgs > 0) &&
				(strcmp( pArgs[0], "--encrypt" ) == 0))
		{
			XmlPacker::shouldEncrypt( true );
			pArgs += 1;
			numArgs -= 1;
			processedFlag = true;
		}
		// NOTE: This option is currently not documented.
		else if ((numArgs > 1) &&
				(strcmp( pArgs[0], "--strip" ) == 0))
		{
			CDataPacker::addStripSection( pArgs[1] );
			pArgs += 2;
			numArgs -= 2;
			processedFlag = true;
		}
	}

	//Validate the command line arguments
	if ((hasAssetList && (!hasInPath || !hasOutPath || numArgs > 0)) || // list mode
		(!hasAssetList && (numArgs < 1 || numArgs > 3))) // compatible mode
	{
		printUsage( exeName );
		return EXIT_FAILURE;
	}

	PackerHelper::paths( inPath, outPath );
		
	if (hasAssetList)
	{
		// Batch list mode.
		PackerHelper::setCmdLine( argc, argv );

		PackerHelper::setBasePath( outPath );

		if ( !PackerHelper::initResources() )
		{
			printf("Unable to initialise the resource system.\n");
			return EXIT_FAILURE;
		}

		if ( !PackerHelper::initMoo() )
		{
			printf("Unable to initialise moo.\n");
			return EXIT_FAILURE;
		}

		char buf[ MAX_FILEPATH ];

		int fails = 0;
		while (fgets( buf, MAX_FILEPATH, assetList ) != 0 &&
				!s_quitRequested)
		{
			buf[ strlen(buf) - 1 ] = 0;
			std::string pInputName = std::string( inPath ) + "\\" + std::string( buf );
			std::string pOutputName = std::string( outPath ) + "\\" + std::string( buf );

			// reverse slashes
			std::replace( pInputName.begin(), pInputName.end(), '\\', '/' );
			std::replace( pOutputName.begin(), pOutputName.end(), '\\', '/' );

			BasePacker* packer = Packers::instance().find( pInputName, pOutputName );
			if (numArgs == 1)
			{
				if ( !packer )
					return EXIT_SUCCESS;	// file type not known, nothing to print

				if ( !packer->print() )
				{
					// failed printing, so there's something wrong with the file
					printf( "Error: Could not print file '%s'\n", pArgs[0] );
					printUsage( exeName );
					return EXIT_FAILURE;
				}
				return EXIT_SUCCESS;
			}

			printf( "Processing %s...", pInputName.c_str() );

			bool failed = false;
			if ( !packer )
			{
				// file type not known, so just copy it
				failed = !PackerHelper::copyFile( pInputName, pOutputName );
			}
			else
			{
				// file known, so try to process it
				failed = !packer->pack();
			}

			if (failed)
			{
				fails++;
				if (useErrorLog && (errorLog != NULL))
				{
					fprintf( errorLog, "%s\n", pInputName.c_str() );
				}
			}
			printf( " %s\n", failed ? "failed" : "succeeded" );
		}

		printf( "\nProcessing complete" );
		if (fails > 0)
		{
			printf( " - %d files failed: ", fails );
			if (useErrorLog)
				printf( "refer to error log" );
			else
				printf( "run with \"--err 'error_log_file'\" argument" );
			printf( " for more information" );
		}
		printf( ".\n\n" );

		//Close any files we have opened.
		if (assetList)
		{
			fclose( assetList );
		}
		if (errorLog)
		{
			fclose( errorLog );
		}
	}
	else
	{
		// Old compatible mode, only processing one file.
		std::string pInputName = pArgs[0];
		std::string pOutputName = numArgs == 1 ? "" : pArgs[1];

		PackerHelper::setCmdLine( argc, argv );

		if ( numArgs == 3 )
		{
			PackerHelper::setBasePath( pArgs[2] );
		}
		else
		{
			PackerHelper::setBasePath( BWResource::getFilePath( pOutputName ) );
		}

		if ( !PackerHelper::initResources() )
		{
			printf("Unable to initialise the resource system.\n");
			return EXIT_FAILURE;
		}

		if ( !PackerHelper::initMoo() )
		{
			printf("Unable to initialise moo.\n");
			return EXIT_FAILURE;
		}

		// reverse slashes
		std::replace( pInputName.begin(), pInputName.end(), '\\', '/' );
		std::replace( pOutputName.begin(), pOutputName.end(), '\\', '/' );

		BasePacker* packer = Packers::instance().find( pInputName, pOutputName );
		if (numArgs == 1)
		{
			if ( !packer )
				return EXIT_SUCCESS;	// file type not known, nothing to print

			if ( !packer->print() )
			{
				// failed printing, so there's something wrong with the file
				printf( "Error: Could not print file '%s'\n", pArgs[0] );
				printUsage( exeName );
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}

		printf( "Processing %s to %s...\n",
				pArgs[0], pArgs[1] );

		result = false;
		if ( !packer )
		{
			// file type not known, so just copy it
			result = PackerHelper::copyFile( pInputName, pOutputName );
		}
		else
		{
			// file known, so try to process it
			result = packer->pack();
		}

		result = result && !msgHandler.errorsOccurred();

		printf( "...%s\n", result ? "succeeded" : "failed" );
	}

	result = result && !s_quitRequested;

#ifndef MF_SERVER
	// Finialise moo
	Moo::fini();
#endif // MF_SERVER

	BWResource::fini();

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

// main.cpp
