/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"


#define WIN32_LEAN_AND_MEAN
#define NT_PLUGIN
#include <windows.h>

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnPlugin.h>

#include "resmgr/bwresource.hpp"
#include "VisualFileTranslator.hpp"
#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/auto_config.hpp"

DECLARE_DEBUG_COMPONENT2( "VisualExporter", 0 );


// under WIN32 we need to export two functions
// under linux compiling with -shared does the job...
#ifdef WIN32
#define MLL_EXPORT __declspec(dllexport) 
#else
#define MLL_EXPORT
#endif

// this function is called when the plugin is loaded. It is
// used to register any file translators, mel commands, 
// custom nodes etc that your plugin is adding to Maya
MLL_EXPORT MStatus initializePlugin( MObject obj )
{
	//~ printf ("Initialise Plugin.\n");
	//~ fflush(stdout); // make any messages show up

	// attach a plugin function set to register our file
	// translator with Maya
	MFnPlugin plugin( obj, "Micro Forte", "2.0", "Any");

	
	// an error code
	MStatus stat;

	// register the file translator with Maya 
	stat = plugin.registerFileTranslator( "BigWorldAsset", 
											"none",
											VisualFileTranslator::creator,
											NULL,
											NULL ); 

	
	// check for errors
	if ( stat != MS::kSuccess )
	{
		stat.perror("Error - initializePlugin: registration failed"); 
		return stat;
	}

	// Initalise BigWorld

	
	// Add the tools res directory
	wchar_t currentDir[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, currentDir );
	std::wstring exporterPath = bw_acptow( plugin.loadPath().asChar() );
	SetCurrentDirectory( exporterPath.c_str() );

	// make sure tools are located in the bigworld directory structure not Maya
	std::string toolsPath = bw_wtoutf8( exporterPath );
	std::string::size_type pos = toolsPath.find_last_of( "\\/" );
	
	pos = toolsPath.find_last_of( "\\/" );
	if (pos != std::string::npos)
	{
		toolsPath = toolsPath.substr( 0, pos );
		// Check we aren't running from the 3dsmax directory
		// we really want to be in bigworld/tools/exporter
		if (toolsPath.find( "maya" ) != std::string::npos)
		{
			MessageBox( GetForegroundWindow(),
				L"Exporter must be run from bigworld/tools/exporter/***, "
				L"not copied into the 3dsmax plugins folder\n",
				L"Visual Exporter", MB_OK | MB_ICONEXCLAMATION );
			return MS::kFailure;
		}

		pos = toolsPath.find_last_of( "\\/" );
		if (pos != std::string::npos)
			toolsPath = toolsPath.substr( 0, pos );
	}
	
	BWResource::init( bw_wtoutf8( exporterPath ) + "\\", false );
	BWResource::instance().fileSystem()->addBaseFileSystem( NativeFileSystem::create("") );

	if( !AutoConfig::configureAllFrom("resources.xml") )
	{
		MessageBox( GetForegroundWindow(), L"Cannot find resources.xml!", L"Visual Exporter Error",
			MB_OK | MB_ICONERROR );
		return MS::kFailure;
	}

	ExportSettings::instance().setSettingFileName( bw_wtoutf8( exporterPath ) + "\\exporter.xml" );
	ExportSettings::instance().readSettings();

	SetCurrentDirectory( currentDir );

	return MS::kSuccess;
}

// This function is called when you un-load your plugin.
// it is used to de-register anything you have added to Maya
MLL_EXPORT MStatus uninitializePlugin( MObject obj )
{
	// shutdown BigWorld
	// FilenameResolver has no destroy

	fflush(stdout); // make any messages show up

	// attach a plugin function set to register our file
	// translator with Maya
	MFnPlugin plugin( obj );

	// an error code
	MStatus stat;

	// de-register the file translator with Maya 
	stat = plugin.deregisterFileTranslator("BigWorldAsset");

	if ( stat != MS::kSuccess )
	{
		stat.perror("Error - uninitializePlugin: deregister failed"); 
	}

	return stat;
} 

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}
