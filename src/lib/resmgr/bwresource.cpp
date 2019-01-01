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

#include "cstdmf/bwversion.hpp"
#include "cstdmf/bw_util.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/memory_trace.hpp"
#include "cstdmf/string_utils.hpp"

#include "bwresource.hpp"
#include "xml_section.hpp"
#include "data_section_cache.hpp"
#include "data_section_census.hpp"
#include "dir_section.hpp"
#include "zip_file_system.hpp"
#include "multi_file_system.hpp"

#ifndef CODE_INLINE
#include "bwresource.ipp"
#endif

#ifdef _WIN32
#include <direct.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <iomanip>
#include <strstream>
#include "cache_file_system.hpp"
#if BWCLIENT_AS_PYTHON_MODULE
#include <direct.h>
#endif
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif // _WIN32

#include <errno.h>
#include <memory>

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

#ifdef USE_MEMORY_TRACER
#define DEFAULT_CACHE_SIZE (0)
#else
#define DEFAULT_CACHE_SIZE (100 * 1024)
#endif


#define PATHS_FILE "paths.xml"
#define SOURCE_FILE "source.xml"
#define PACKED_EXTENSION "zip"
#define FOLDER_SEPERATOR '/'


#define RES_PATH_HELP												\
	"\n\nFor more information on how to correctly setup and run "	\
	"BigWorld client, please refer to the Client Installation "		\
	"Guide, in bigworld/doc directory.\n"


/// resmgr library Singleton

BW_SINGLETON_STORAGE( BWResource )

// File static values
namespace
{
// Stores whether BWResource has been initialised.
bool g_inited = false;

// Ideally, this pointer should not be used. It is a fallback if the singleton
// instance of BWResource was not created at the application level.
std::auto_ptr< BWResource > s_pBWResourceInstance;
}

typedef std::vector<std::string> STRINGVECTOR;


/**
 *  This local class contains the private implementation details for the 
 *  BWResource class.
 */
class BWResourceImpl
{
public:
	BWResourceImpl() :
		rootSection_( NULL ),
		fileSystem_( NULL ),
		nativeFileSystem_( NULL ),
		paths_(),
		defaultDrive_()
#if ENABLE_FILE_CASE_CHECKING		
		,checkCase_( false )
#endif // ENABLE_FILE_CASE_CHECKING
#ifdef _WIN32
		,appVersionData_( NULL )
#endif // _WIN32
	{
		nativeFileSystem_ = NativeFileSystem::create( "" );
	};


	~BWResourceImpl()
	{
		cleanUp( true );
		nativeFileSystem_ = NULL;
#ifdef _WIN32
		if ( appVersionData_ )
		{
			free( appVersionData_ );
			appVersionData_ = NULL;
		}
#endif // _WIN32
	};

	void cleanUp( bool final );

	// private interface taken from the now deprecated FilenameResolver
	bool loadPathsXML( const std::string& customAppFolder = "" );

	static bool isValidZipSize( uint64 size, const std::string & path );

	void						postAddPaths();


	static bool					hasDriveInPath( const std::string& path );
	static bool					pathIsRelative( const std::string& path );

	static std::string			formatSearchPath( const std::string& path, 
		const std::string& pathXml = "" );

#ifdef _WIN32
	bool loadPathsXMLInternal( const std::string& directory );

	static const std::string	getAppDirectory();

	// Windows Known Folders lookup methods
	static const std::string	getAppDataDirectory();
	static const std::string	getUserDocumentsDirectory();

	// Version info (VS_VERSION_INFO) access methods
	const std::string			getAppVersionString( 
		const std::string & stringName );
	void						getAppVersionData();
	const std::string			getAppVersionLanguage();
#endif // _WIN32

	// Member data
	DataSectionPtr				rootSection_;
	MultiFileSystemPtr			fileSystem_;
	FileSystemPtr				nativeFileSystem_;

	STRINGVECTOR				paths_;

    std::string                 defaultDrive_;
#if ENABLE_FILE_CASE_CHECKING
	bool						checkCase_;
#endif // ENABLE_FILE_CASE_CHECKING

#ifdef _WIN32
	static std::string          appDirectoryOverride_;
	void*						appVersionData_;
#endif // _WIN32
};


// function to compare case-insensitive in windows, case sensitive in unix
namespace
{
	int mf_pathcmp( const char* a, const char* b )
	{
#ifdef unix
		return strcmp( a, b );
#else
		return _stricmp( a, b );
#endif // unix
	}
};


void BWResourceImpl::cleanUp( bool final )
{
	rootSection_ = NULL;

	// Be careful here. If users hang on to DataSectionPtrs after
	// destruction of the cache, this could cause a problem at
	// shutdown time. Could make the cache a reference counted
	// object and make the DataSections use smart pointers to it,
	// if this is actually an issue.

	fileSystem_ = NULL;

	if (final)
	{
		DataSectionCache::fini();
	}
}


/**
 *	This static method finds out if a size value is bigger than the maximum Zip
 *	file size supported.
 *
 *	@param size		Size value to compare with the maximum supported size.
 *	@param path		Path of the file, used only for error reporting.
 *	@return	True if size is less or equal to the maximum supported size, false
 *			otherwise.
 */
/*static*/
bool BWResourceImpl::isValidZipSize( uint64 size, const std::string & path )
{
	if (size > ZipFileSystem::MAX_ZIP_FILE_KBYTES * 1024)
	{
#ifdef MF_SERVER
		ERROR_MSG( "BWResource::addPaths: "
				"Resource path is not a valid '%s' file: %s"
				"\n\nOnly Zip files of %"PRIu64"GB or less are supported.",
#else
		CRITICAL_MSG( "Search path is not a valid '%s' file: %s."
				"\n\nOnly files of %"PRIu64"GB or less are supported."
				RES_PATH_HELP,
#endif
				PACKED_EXTENSION,
				path.c_str(),
				ZipFileSystem::MAX_ZIP_FILE_KBYTES/(1024 * 1024) );

		return false;
	}
	return true;
}


void BWResourceImpl::postAddPaths()
{
	if (!g_inited)
	{
		CRITICAL_MSG( "BWResourceImpl::postAddPaths: "
				"BWResource::init should have been called already.\n" );
		return;
	}

	if ( fileSystem_ )
	{
		// already called, so clean up first.
		cleanUp( false );
	}

	fileSystem_ = new MultiFileSystem();

	int pathIndex = 0;
	std::string path;

	path = BWResource::getPath( pathIndex++ );

	FileSystemPtr tempFS = NativeFileSystem::create( "" );

	// add an appropriate file system for each path
	while (!path.empty())
	{
		// check to see if the path is actually a Zip file
		std::string pakPath = path;
		FileSystemPtr baseFileSystem;

		// also check to see if the path contains a "res.zip" Zip file
		std::string pakPath2 = path;
		if (*pakPath2.rbegin() != '/') pakPath2.append( "/" );
		pakPath2 += "res.";
		pakPath2 += PACKED_EXTENSION;

		IFileSystem::FileInfo fileInfo;
		if ( BWResource::getExtension( pakPath ) == PACKED_EXTENSION &&
			tempFS->getFileType( pakPath, &fileInfo ) == IFileSystem::FT_FILE )
		{
			// it's a Zip file system, so add it
			INFO_MSG( "BWResource::BWResource: Path is package %s\n",
					pakPath.c_str() );
			if (isValidZipSize( fileInfo.size, pakPath ))
			{
				baseFileSystem = new ZipFileSystem( pakPath );
			}
		}
		else if ( tempFS->getFileType( pakPath2, &fileInfo ) == IFileSystem::FT_FILE )
		{
			// it's a Zip file system, so add it
			INFO_MSG( "BWResource::BWResource: Found package %s\n",
					pakPath2.c_str() );
			if (isValidZipSize( fileInfo.size, pakPath2 ))
			{
				baseFileSystem = new ZipFileSystem( pakPath2 );
			}
		}
		else  if ( tempFS->getFileType( path, NULL ) == IFileSystem::FT_DIRECTORY )
		{
			if (*path.rbegin() != '/') path.append( "/" );

			// it's a standard path, so add a native file system for it
			baseFileSystem = NativeFileSystem::create( path );
		}

#ifdef _WIN32
		BinaryPtr binBlock = baseFileSystem->readFile( SOURCE_FILE );

		if (binBlock && binBlock->len() != 0)
		{
			DataSectionPtr spRoot = XMLSection::createFromBinary( "root", binBlock );

			fileSystem_->addBaseFileSystem( new CacheFileSystem( path, spRoot ) );
		}
#endif

		fileSystem_->addBaseFileSystem( baseFileSystem );

		path = BWResource::getPath( pathIndex++ );
	}

	tempFS = NULL;

#ifdef EDITOR_ENABLED
	// add a default native file system, to handle absolute paths in the tools
	fileSystem_->addBaseFileSystem( NativeFileSystem::create( "" ) );
#endif

	char * cacheSizeStr = ::getenv( "BW_CACHE_SIZE" );
	int cacheSize = cacheSizeStr ? atoi(cacheSizeStr) : DEFAULT_CACHE_SIZE;

	DataSectionCache::instance()->setSize( cacheSize );
	DataSectionCache::instance()->clear();

	//new DataSectionCache( cacheSize );
	rootSection_ = new DirSection( "", fileSystem_ );

#if ENABLE_FILE_CASE_CHECKING
	if (checkCase_)
		fileSystem_->checkCaseOfPaths(checkCase_);
#endif // ENABLE_FILE_CASE_CHECKING

	fileSystem_->postResouceInitialised();
}




// -----------------------------------------------------------------------------
// Section: BWResource
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
BWResource::BWResource(): 
		Singleton< BWResource >()
{
	pimpl_ = new BWResourceImpl();
}

/**
 *	Destructor
 */
BWResource::~BWResource()
{
	delete pimpl_;
}

void BWResource::refreshPaths()
{
	pimpl_->postAddPaths();
}

DataSectionPtr BWResource::rootSection()
{
	return pimpl_->rootSection_;
}

MultiFileSystemPtr BWResource::fileSystem()
{
	return pimpl_->fileSystem_;
}

/**
 *	This method purges an item from the cache, given its relative path from
 *	the root section.
 *
 *	@param path			The path to the item to purge.
 *	@param recursive	Set this to true to perform a full recursive purge.
 *	@param spSection	If not NULL, this should be a pointer to the data
 *						section at 'path'. This is used to improve efficiency
 *						and avoid a problem with duplicate child names.
 */
void BWResource::purge( const std::string & path, bool recursive,
		DataSectionPtr spSection )
{
	if (recursive)
	{
		if (!spSection)
			spSection = pimpl_->rootSection_->openSection( path );

		if (spSection)
		{
			DataSectionIterator it = spSection->begin();
			DataSectionIterator end = spSection->end();

			while (it != end)
			{
				this->purge( path + "/" + (*it)->sectionName(), true, *it );
				it++;
			}
		}
	}

	// remove it from the cache
	DataSectionCache::instance()->remove( path );

	// remove it from the census too
	DataSectionPtr pSection = DataSectionCensus::find( path );
	if (pSection)
	{
		 DataSectionCensus::del( pSection.get() );
	}
}

/**
 *	This method purges everything from the cache and census.
 */
void BWResource::purgeAll()
{
	DataSectionCache::instance()->clear();
	DataSectionCensus::clear();
}


/**
 *  This method saves the section with the given pathname.
 *
 *  @param resourceID     The relative path of the section.
 */
bool BWResource::save( const std::string & resourceID )
{
	DataSectionPtr ptr = DataSectionCensus::find( resourceID );

	if (!ptr)
	{
		WARNING_MSG( "BWResource::saveSection: %s not in census\n",
			resourceID.c_str() );
		return false;
	}

	return ptr->save();
}


/**
 *	Standard open section method.
 *	Always looks in the census first.
 */
DataSectionPtr BWResource::openSection( const std::string & resourceID,
										bool makeNewSection,
										DataSectionCreator* creator )
{
	BW_GUARD;

	DataSectionPtr pExisting = DataSectionCensus::find( resourceID );
	if (pExisting) return pExisting;

	return instance().rootSection()->openSection( resourceID, 
										makeNewSection, creator );
}


// Needs to be constucted like this for broken Linux thread local storage.
THREADLOCAL( bool ) g_doneLoading( false );

/**
 *	Causes the file system to issue warnings if
 *	files are loaded from the calling thread.
 *
 *	@param	watch	true if calling thread should be watched. False otherwise.
 */
void BWResource::watchAccessFromCallingThread(bool watch)
{
	g_doneLoading = watch;
}

/**
 *	Determines whether or not we are issuing warnings about loading
 *	from the calling thread.
 *
 *	@return True if we are watching access from the calling thread.
 */
bool BWResource::watchAccessFromCallingThread()
{
	return g_doneLoading;
}

/**
 *	Displays the proper warnings if
 *	files are loaded from the calling thread.
 */
void BWResource::checkAccessFromCallingThread(
	const std::string& path, const std::string& msg )
{
	BW_GUARD;

	if ( g_doneLoading )
	{
		WARNING_MSG( "%s: Accessing %s from non-loading thread.\n",
			msg.c_str(), path.c_str() );
	}
}

//
//
//	Static methods (taken from the now deprecated BWResource)
//
//

static std::string appDrive_;

#ifdef unix
// difference in .1 microsecond steps between FILETIME and time_t
const uint64 unixTimeBaseOffset = 116444736000000000ull;
const uint64 unixTimeBaseMultiplier = 10000000;

inline uint64 unixTimeToFileTime( time_t t )
{
	return uint64(t) * unixTimeBaseMultiplier + unixTimeBaseOffset;
}
#endif

#if !defined( _WIN32 )
// these are already defined in Win32
int __argc = 0;
char ** __argv = NULL;
#endif

#ifdef _WIN32

void BWResource::overrideAppDirectory(const std::string &dir)
{
	BWResourceImpl::appDirectoryOverride_ = dir;
}


/*static*/ const std::string BWResourceImpl::getAppDirectory()
{
	std::string appDirectory( "." );

	if (!BWResourceImpl::appDirectoryOverride_.empty())
	{
		appDirectory = BWResourceImpl::appDirectoryOverride_;
		BWUtil::sanitisePath( appDirectory );
	}
	else
	{
		#if BWCLIENT_AS_PYTHON_MODULE
			appDirectory = BWResource::getCurrentDirectory();
		#else
			appDirectory = BWUtil::executableDirectory();
		#endif

		#ifdef EDITOR_ENABLED
			appDrive_ = appDirectory[0];
			appDrive_ += ":";
		#endif
	}

	MF_ASSERT( !appDirectory.empty() );

	if (appDirectory[appDirectory.length() - 1] != '/')
	{
		appDirectory += "/";
	}

	return appDirectory;
}

namespace {

HRESULT getWindowsPath( int cisdl, std::string& out )
{
	wchar_t wappDataPath[MAX_PATH];
	HRESULT result= SHGetFolderPath( NULL, cisdl, NULL,
								SHGFP_TYPE_CURRENT, wappDataPath );
	if( SUCCEEDED( result ) )
	{	
		bw_wtoutf8( wappDataPath, out );
		std::replace( out.begin(), out.end(), '\\', '/' );
		out += "/";
		return result;
	}
	return result;
}

} // anonymous namespace

/*static*/ const std::string BWResourceImpl::getAppDataDirectory()
{
	std::string ret;
	HRESULT result = getWindowsPath( CSIDL_APPDATA|CSIDL_FLAG_CREATE, ret );
	if (FAILED(result))
	{	
		WARNING_MSG( "BWResourceImpl::getAppDataDirectory: Couldn't locate "
			"application data directory: 0x%08x\n", result );
	}
	return ret;
}


/*static*/ const std::string BWResourceImpl::getUserDocumentsDirectory()
{
	std::string ret;
	HRESULT result = getWindowsPath( CSIDL_MYDOCUMENTS|CSIDL_FLAG_CREATE, ret );
	if (FAILED(result))
	{	
		WARNING_MSG( "BWResourceImpl::userDocumentsDirectory: Couldn't locate "
			"My Documents: 0x%08x\n", result );
	}
	return ret;
}


const std::string BWResourceImpl::getAppVersionString(
	const std::string & stringName )
{
	getAppVersionData();
	if ( !appVersionData_ )
	{
		return "";
	}
	const std::string language = getAppVersionLanguage();
	if ( language == "" )
	{
		return "";
	}
	std::stringstream lookupPath;
	lookupPath << "\\StringFileInfo\\" << language << "\\" << stringName;

	std::wstring wlookupPath;
	bw_utf8tow( lookupPath.str(), wlookupPath );

	unsigned int resultLen;
	char* resultPtr;
	if ( VerQueryValue( appVersionData_, const_cast< wchar_t * >( wlookupPath.c_str() ),
		reinterpret_cast< void ** >( &resultPtr ), &resultLen ) == 0 )
	{
		// Bad resource, above name is bad, or no info for this entry.
		WARNING_MSG( "BWResourceImpl::getAppVersionString: "
			"VerQueryValue failed looking up '%s'\n", lookupPath.str().c_str() );
		return "";
	}
	std::string result( resultPtr, resultLen );
	return result;
}


const std::string BWResourceImpl::getAppVersionLanguage()
{
	getAppVersionData();
	if ( ! appVersionData_ )
	{
		return "";
	}

	// This comes from the MSDN page on VerQueryValue's sample code
	struct {
		uint16 lang;
		uint16 codepage;
	} *pTranslations;

	unsigned int translationsLen;

	if ( VerQueryValue( appVersionData_, L"\\VarFileInfo\\Translation",
		reinterpret_cast< void ** >( &pTranslations ), &translationsLen ) == 0 )
	{
		// Bad resource, above name is bad, or no info for this entry.
		WARNING_MSG( "BWResourceImpl::getAppVersionLanguage: "
			"VerQueryValue failed\n" );
		return "";
	}

	if ( translationsLen == 0 )
	{
		return "";
	}

	IF_NOT_MF_ASSERT_DEV( translationsLen % sizeof( *pTranslations ) == 0 )
	{
		translationsLen -= translationsLen % sizeof( *pTranslations );
	}

	std::stringstream result;
	result.setf( std::ios::hex, std::ios::basefield );
	result.setf( std::ios::right, std::ios::adjustfield );
	result.fill( '0' );

	uint16 currLang = LANGIDFROMLCID( GetThreadLocale() );
	for ( unsigned int i=0; i < ( translationsLen/sizeof( *pTranslations ) );
		i++ )
	{
		if ( pTranslations[ i ].lang == currLang )
		{
			result << std::setw( 4 ) << pTranslations[ i ].lang;
			result << std::setw( 4 ) << pTranslations[ i ].codepage;
			break;
		}
	}
	if ( result.str().empty() )
	{
		// Fallback to first language listed.
		result << std::setw( 4 ) << pTranslations[ 0 ].lang;
		result << std::setw( 4 ) << pTranslations[ 0 ].codepage;
	}

	// TODO: Does this need to be cached somewhere?
	return result.str();

}

void BWResourceImpl::getAppVersionData()
{
	if ( appVersionData_ )
	{
		return;
	}

	wchar_t wfileNameBuffer[MAX_PATH];
	int result =
		GetModuleFileName( NULL, wfileNameBuffer, ARRAY_SIZE( wfileNameBuffer ) );
	if ( result == 0 )
	{
		WARNING_MSG( "BWResourceImpl::getAppVersionData: GetModuleFileName "
			"failed: 0x%08x\n", GetLastError() );
		return;
	}
	if ( result == MAX_PATH )
	{
		// MAX_PATH means truncation which should never happen...
		WARNING_MSG( "BWResourceImpl::getAppVersionData: GetModuleFileName "
			"unexpectedly wanted to return more than %u bytes.\n", MAX_PATH );
		return;
	}

	int versionSize;
	DWORD scratchValue;
	versionSize = GetFileVersionInfoSize( wfileNameBuffer, &scratchValue );
	if ( versionSize == 0 )
	{
		WARNING_MSG( "BWResourceImpl::getAppVersionData: "
			"GetFileVersionInfoSize failed: 0x%08x\n", GetLastError() );
		return;
	}

	// Using malloc/free because this is an opaque buffer, not a character array
	// or other usefully newable data.
	appVersionData_ = malloc( versionSize );
	if ( ! appVersionData_ )
	{
		ERROR_MSG( "BWResourceImpl::getAppVersionData: malloc failed!\n" );
		return;
	}
	// From here on, any failure path must free and clear appVersionData_
	if ( ! GetFileVersionInfo( wfileNameBuffer, NULL, versionSize,
		appVersionData_ ) )
	{
		WARNING_MSG( "BWResourceImpl::getAppVersionData: "
			"GetFileVersionInfo failed: 0x%08x\n", GetLastError() );
		free( appVersionData_ );
		appVersionData_ = NULL;
		return;
	}
}
#endif // _WIN32


/**
 *	Load the paths.xml file, or avoid loading it if possible.
 */
bool BWResourceImpl::loadPathsXML( const std::string& customAppFolder )
{
	if (paths_.empty())
	{
		char * basePath;

		// The use of the BW_RES_PATH environment variable is deprecated
		if ((basePath = ::getenv( "BW_RES_PATH" )) != NULL)
		{
			WARNING_MSG( "The use of the BW_RES_PATH environment variable is " \
					"deprecated post 1.8.1\n" );

			BWResource::addPaths( basePath, "from BW_RES_PATH env variable" );
		}
	}

	if (paths_.empty())
	{
#ifdef _WIN32
		bool loadedXML = false;

		// Try explicitly given app folder...
		if (!customAppFolder.empty())
		{
			loadedXML = loadPathsXMLInternal( customAppFolder );
		}

		// ...otherwise try CWD...
		if (!loadedXML)
		{
			loadedXML = loadPathsXMLInternal( BWResource::getCurrentDirectory() );
		}

		// ...otherwise try app directory.
		if (!loadedXML)
		{
			loadedXML = loadPathsXMLInternal( BWResource::appDirectory() );
		}
#else
		// Try reading ~/.bwmachined.conf
		char * pHomeDir = ::getenv( "HOME" );

		if (pHomeDir == NULL)
		{
			struct passwd * pPWEnt = ::getpwuid( ::getuid() );
			if (pPWEnt)
			{
				pHomeDir = pPWEnt->pw_dir;
			}
		}

		if (pHomeDir)
		{
			char buffer[1024];
			snprintf( buffer, sizeof(buffer), "%s/.bwmachined.conf", pHomeDir );
			FILE * pFile = bw_fopen( buffer, "r" );

			if (pFile)
			{
				while (paths_.empty() &&
						fgets( buffer, sizeof( buffer ), pFile ) != NULL)
				{
					// Find first non-whitespace character in buffer
					char *start = buffer;
					while (*start && isspace( *start ))
						start++;

					if (*start != '#' && *start != '\0')
					{
						if (sscanf( start, "%*[^;];%s", start ) == 1)
						{
							BWResource::addPaths( start,
									"from ~/.bwmachined.conf" );
						}
						else
						{
							WARNING_MSG( "BWResource::BWResource: "
								"Invalid line in ~/.bwmachined.conf - \n"
								"\t'%s'\n",
								buffer );
						}
					}
				}
				fclose( pFile );
			}
			else
			{
				WARNING_MSG( "BWResource::BWResource: Unable to open %s\n",
						buffer );
			}
		}

		if (paths_.empty())
		{
			ERROR_MSG( "BWResource::BWResource: No res path set.\n"
				"\tIs ~/.bwmachined.conf correct or 'BW_RES_PATH' environment "
					"variable set?\n" );
		}
#endif
	}

#if _WIN32
#ifndef _RELEASE
	if (paths_.empty())
	{
		CRITICAL_MSG(
			"BWResource::BWResource: "
			"No paths found in BW_RES_PATH environment variable "
			"and no paths.xml file found in the working directory. "
			RES_PATH_HELP );
	}
#endif // not _RELEASE
#endif // _WIN32

#ifdef MF_SERVER
#ifdef _WIN32
	COORD c = { 80, 2000 };
	SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), c );
#endif
#endif

	this->postAddPaths();

	return !paths_.empty();
}

#ifdef _WIN32
bool BWResourceImpl::loadPathsXMLInternal( const std::string& directory )
{
	#ifdef LOGGING
	dprintf( "BWResource: app directory is %s\n",
		directory.c_str() );
	dprintf( "BWResource: using %spaths.xml\n",
		directory.c_str() );
	std::ofstream os( "filename.log", std::ios::trunc );
	os << "app dir: " << directory << "\n";
	#endif

	// load the search paths
	std::string fullPathsFileName = directory + PATHS_FILE;
	if ( nativeFileSystem_->getFileType( fullPathsFileName ) != IFileSystem::FT_NOT_FOUND )
	{
		DEBUG_MSG( "Loading.. %s\n", fullPathsFileName.c_str() );
		BinaryPtr binBlock = nativeFileSystem_->readFile( fullPathsFileName );
		DataSectionPtr spRoot = XMLSection::createFromBinary( "root", binBlock );
		if ( spRoot )
		{
			// load the paths
			std::string path = "Paths";
			DataSectionPtr spSection = spRoot->openSection( path );

			STRINGVECTOR paths;
			if ( spSection )
				spSection->readStrings( "Path", paths );

			// Format the paths
			for (STRINGVECTOR::iterator i = paths.begin(); i != paths.end(); ++i)
			{
				BWResource::addPaths( *i, "from paths.xml", directory );
			}

			return true;
		}
		else
		{
			DEBUG_MSG( "Failed to open paths.xml" );
			return false;
		}
	}
	else
	{
		return false;
	}
}
#endif

/**
 *	This method initialises the BWResource from command line arguments.
 */
bool BWResource::init( int argc, const char * argv[] )
{
	// This should be called before the BWResource is constructed.
	MF_ASSERT( !g_inited );
	g_inited = true;

	BWResource & instance = BWResource::makeInstance();

	for (int i = 0; i < argc - 1; i++)
	{
		if (strcmp( argv[i], "--res" ) == 0 ||
			strcmp( argv[i], "-r" ) == 0)
		{
			addPaths( argv[i + 1], "from command line" );
		}
		if (strcmp( argv[i], "-UID" ) == 0 ||
			strcmp( argv[i], "-uid" ) == 0)
		{
			INFO_MSG( "UID set to %s (from command line).\n", argv[ i+1 ] );
#ifndef _WIN32
			::setenv( "UID", argv[ i+1 ], true );
#else
			std::string addStr = "UID=";
			addStr += argv[ i+1 ];
			::_putenv( addStr.c_str() );
#endif
		}
	}

	bool res = instance.pimpl_->loadPathsXML();

	if (res)
	{
		BWResource::initCommon();
	}

	return res;
}

/**
 * Initialise BWResource to use the given path directly
 */
bool BWResource::init( const std::string& fullPath, bool addAsPath )
{
	// This should be called before the BWResource is constructed.
	MF_ASSERT( !g_inited );
	g_inited = true;

	BWResource & instance = BWResource::makeInstance();

	bool res = instance.pimpl_->loadPathsXML(fullPath);

	if (addAsPath)
	{
		BWResource::addPath( fullPath );
	}

	if (res)
	{
		BWResource::initCommon();
	}

	return res;
}


/**
 * This method is called by both init methods.
 */
void BWResource::initCommon()
{
	DataSectionPtr pVersion = BWResource::openSection( "version.xml" );

	if (pVersion)
	{
		BWVersion::majorNumber(
			pVersion->readInt( "major", BWVersion::majorNumber() ) );
		BWVersion::minorNumber(
			pVersion->readInt( "minor", BWVersion::minorNumber() ) );
		BWVersion::patchNumber(
			pVersion->readInt( "patch", BWVersion::patchNumber() ) );
	}

	INFO_MSG( "BWResource::initCommon: Version is %s\n",
			BWVersion::versionString().c_str() );
}


/**
 *	This method finalises the singleton instance.
 */
void BWResource::fini()
{
	s_pBWResourceInstance.reset();
	g_inited = false;
}


/**
 *	This method creates the singleton instance of BWResource.
 */
// TODO: This method should not be needed once all BWResource instances are made
// at the application level.
BWResource & BWResource::makeInstance()
{
	MF_ASSERT( s_pBWResourceInstance.get() == NULL );

	if (BWResource::pInstance())
	{
		return BWResource::instance();
	}
	else
	{
		DEBUG_MSG( "BWResource::makeInstance: "
			"BWResource was not created at the app level.\n"
			"Create a BWResource as a member of the application or main.\n" );
		s_pBWResourceInstance.reset( new BWResource );

		return *s_pBWResourceInstance;
	}
}


/**
 *	This method searches for a file in the current search path, recursing into
 *	sub-directories.
 *
 *	@param file the file to find
 *
 *	@return true if the file is found
 */
bool BWResource::searchPathsForFile( std::string& file )
{
	std::string filename = getFilename( file );
	STRINGVECTOR::iterator it  = instance().pimpl_->paths_.begin( );
	STRINGVECTOR::iterator end = instance().pimpl_->paths_.end( );
	bool found = false;

	// search for the file in the supplied paths
	while( !found && it != end )
	{
		found = searchForFile( formatPath((*it)), filename );
		it++;
	}

	if ( found )
		file = filename;

	return found;
}

/**
 *	Searches for a file given a start directory, recursing into sub-directories
 *
 *	@param directory start directory of search
 *	@param file the file to find
 *
 *	@return true if the file is found
 *
 */
bool BWResource::searchForFile( const std::string& directory,
		std::string& file )
{
	bool result = false;

	std::string searchFor = formatPath( directory ) + file;

	// search for file in the supplied directory
	if ( instance().fileSystem()->getFileType( searchFor, NULL ) ==
		IFileSystem::FT_FILE )
    {
		result = true;
		file = searchFor;
	}
	else
	{
		// if not found, get all directories and start to search them
		searchFor  = formatPath( directory );
		IFileSystem::Directory dirList =
			instance().fileSystem()->readDirectory( searchFor );

		for( IFileSystem::Directory::iterator d = dirList.begin();
			d != dirList.end() && !result; ++d )
		{
			searchFor  = formatPath( directory ) + (*d);

       		// check if it is a directory
			if ( instance().fileSystem()->getFileType( searchFor, NULL ) ==
				IFileSystem::FT_DIRECTORY )
			{
            	// recurse into the directory, and start it all again
				printf( "%s\n", searchFor.c_str( ) );

				result = searchForFile( searchFor, file );
			}
		}
	}

	return result;
}

#ifdef EDITOR_ENABLED

/**
 *	Finds a file by looking in the search paths
 *
 *	Path Search rules.
 *		If the files path has a drive letter in it, then only the filename is
 *		used. The files path must be relative and NOT start with a / or \ to use
 *		the search path.
 *
 *		If either of the top 2 conditions are true or the file is not found with
 *		its own path appended to the search paths then the file is searched for
 *		using just the filename.
 *
 *	@param file the files name
 *
 *	@return files full path and filename
 *
 */
const std::string BWResource::findFile( const std::string& file )
{
	return BWResolver::findFile( file );
}

/**
 *	Resolves a file into an absolute filename based on the search paths
 *	@see findFile()
 *
 *	@param file the files name
 *
 *	@return the resolved filename
 *
 */
const std::string BWResource::resolveFilename( const std::string& file )
{
	return BWResolver::resolveFilename( file );
}

/**
 *	Dissolves a full filename (with drive and path information) to relative
 *	filename.
 *
 *	@param file the files name
 *
 *	@return the dissolved filename, "" if filename cannot be dissolved
 *
 */
const std::string BWResource::dissolveFilename( const std::string& file )
{
	return BWResolver::dissolveFilename( file );
}

/**
 *  Determines whether a dissolved filename is a valid BigWorld path or not.
 *
 *	@param file the files name
 *
 *	@return a boolean for whether the file name is valid
 */
bool BWResource::validPath( const std::string& file )
{
	if (file == "") return false;

	bool hasDriveName = (file.c_str()[1] == ':');
	bool hasNetworkName = (file.c_str()[0] == '/' && file.c_str()[1] == '/');
	hasNetworkName |= (file.c_str()[0] == '\\' && file.c_str()[1] == '\\');

	return !(hasDriveName || hasNetworkName);
}

/**
 *	Removes the drive specifier from the filename
 *
 *	@param file filename string
 *
 *	@return filename minus any drive specifier
 *
 */
const std::string BWResource::removeDrive( const std::string& file )
{
	return BWResolver::removeDrive( file );
}

void BWResource::defaultDrive( const std::string& drive )
{
	BWResolver::defaultDrive( drive );
}

#endif // EDITOR_ENABLED


#if ENABLE_FILE_CASE_CHECKING

/**
 *	This enables/disables checking of filename case.
 *
 *	@param enable	If true then the case of filenames are checked as used.  
 *					This is useful to check for problems with files used on
 *					both Windows and Linux, but comes at a cost in execution
 *					speed.
 */
void BWResource::checkCaseOfPaths(bool enable)
{
	BWResource::instance().pimpl_->checkCase_ = enable;
	BWResource::instance().fileSystem()->checkCaseOfPaths(enable);
}


/**
 *	This gets whether checking of filename's case is disabled or enabled.
 *
 *	@return			True if filename case checking is enabled, false if it's 
 *					disabled.
 */
bool BWResource::checkCaseOfPaths()
{
	return BWResource::instance().pimpl_->checkCase_;
}

#endif // ENABLE_FILE_CASE_CHECKING


/**
 *	This gets the name of a path on disk, correcting the case if necessary.
 *
 *	@param path		The path to get on disk.
 *	@return			The case-sensitive name used by the file system.  If the
 *					path does not exist then it is returned.
 */
std::string BWResource::correctCaseOfPath(std::string const &path)
{
	return BWResource::instance().fileSystem()->correctCaseOfPath(path);
}


/**
 *	Get the type and information of a file
 *
 *	@param pathname	The relative path name - relative to the rest paths.
 *	@param pFI		The file info structure used to retrive the information of
 *					the file. Set to NULL will 
 *
 *	@return			The type of input file
 */
IFileSystem::FileType BWResource::getFileType( const std::string& pathname, IFileSystem::FileInfo* pFI )
{
	return BWResource::instance().fileSystem()->getFileType(
		pathname.c_str(), pFI );
}


/**
 *	Tests if a path name is a directory
 *
 *	@param pathname The relative path name - relative to the rest paths.
 *
 *	@return true if it is a directory
 *
 */
bool BWResource::isDir( const std::string& pathname )
{
	return BWResource::instance().fileSystem()->getFileType(
		pathname.c_str(), NULL ) == IFileSystem::FT_DIRECTORY;
}

/**
 *	Tests if a path name is a file
 *
 *	@param pathname The relative path name - relative to the rest paths.
 *
 *	@return true if it is a file
 *
 */
bool BWResource::isFile( const std::string& pathname )
{
	return BWResource::instance().fileSystem()->getFileType(
		pathname.c_str(), NULL ) == IFileSystem::FT_FILE;
}


/**
 *	Tests if a file exists.
 *
 *	@param file The relative path to the file - relative to the rest paths.
 *
 *	@return true if the file exists
 *
 */
bool BWResource::fileExists( const std::string& file )
{
	return BWResource::instance().fileSystem()->getFileType(
		file.c_str(), NULL ) != IFileSystem::FT_NOT_FOUND;
}

/**
 *	Tests if a file exists.
 *
 *	@param file The absolute path to the file (not including drive letter).
 *
 *	@return true if the file exists
 *
 */
bool BWResource::fileAbsolutelyExists( const std::string& file )
{
	return BWResource::instance().pimpl_->nativeFileSystem_->getFileType(
		file.c_str(), NULL ) != IFileSystem::FT_NOT_FOUND;
}

/**
 *	Tests if the given path is absolute or relative
 *
 *	@param path The path to test.
 *
 *	@return true if the path is relative.
 *
 */
bool BWResource::pathIsRelative( const std::string& path )
{
	return BWResourceImpl::pathIsRelative( path );
}

/**
 *	Retrieves the filename of a file.
 *
 *	@param file file to get filename from.
 *
 *	@return filename string.
 *
 */
const std::string BWResource::getFilename( const std::string& file )
{
	std::string::size_type pos = file.find_last_of( "\\/" );

	if (pos != std::string::npos)
		return file.substr( pos + 1, file.length() );
	else
		return file;
}

/**
 *	Retrieves the path from a file.
 *
 *	@param file file to get path of.
 *
 *	@return path string
 *
 */
const std::string BWResource::getFilePath( const std::string & file )
{
	return BWUtil::getFilePath( file );
}

/**
 *	Retrieves the extension of a filename.
 *
 *	@param file filename string
 *
 *	@return extension string
 *
 */
const std::string BWResource::getExtension( const std::string& file )
{
	std::basic_string <char>::size_type pos = file.find_last_of( "." );
	if ( pos != std::string::npos )
		return file.substr( pos + 1 );

	// no extension
	return "";
}

/**
 *	Removes the extension from the filename
 *
 *	@param file filename string
 *
 *	@return filename minus any extension
 *
 */
const std::string BWResource::removeExtension( const std::string& file )
{
	std::basic_string <char>::size_type pos = file.find_last_of( "." );
	if ( pos != std::string::npos )
		return file.substr( 0, pos );

	// no extension
	return file;
}

/**
 *	Removes the extension from the filename and replaces it with the given
 *	extension
 *
 *	@param file filename string
 *	@param newExtension the new extension to give the file, including the
 *	initial .
 *
 *	@return file with its extension changed to newExtension
 *
 */
const std::string BWResource::changeExtension( const std::string& file,
													const std::string& newExtension )
{
	return removeExtension( file ) + newExtension;
}

/**
 *	This method makes certain that the path contains a trailing folder separator.
 *
 *	@param path 	The path string to be formatted.
 *
 *	@return The path with a trailing folder separator.
 */
std::string BWResource::formatPath( const std::string & path )
{
	return BWUtil::formatPath( path );
}

/**
 * Gets the current working directory
 *
 * @returns string containing the current working directory
 */
std::string BWResource::getCurrentDirectory()
{
#ifdef _WIN32
	// get the working directory
	wchar_t wbuffer[MAX_PATH];
	GetCurrentDirectory( ARRAY_SIZE( wbuffer ), wbuffer ); 	 

	std::string dir = bw_wtoutf8( wbuffer );
	dir.append( "/" );

	BWUtil::sanitisePath( dir );
	return dir;
#else
	// TODO:PM May want to implement non-Windows versions of these.
	return "";
#endif
}

/**
 * Sets the current working directory
 *
 * @param currentDirectory path to make the current directory
 *
 * @returns true if successful
 */
bool BWResource::setCurrentDirectory(
		const std::string &currentDirectory )
{
#if defined(_WIN32) 
	std::wstring cwd;
	bw_utf8tow( currentDirectory, cwd );
	return SetCurrentDirectory( cwd.c_str( ) ) ? true : false;
#else
	// TODO:PM May want to implement non-Windows versions of these.
	(void) currentDirectory;

	return false;
#endif
}


/**
 * Checks for the existants of a drive letter
 *
 * @returns true if a drive is supplied in the path
 */
bool BWResourceImpl::hasDriveInPath( const std::string& path )
{
	return
    	strchr( path.c_str(), ':' ) != NULL ||					// has a drive colon
    	( path.c_str()[0] == '\\' || path.c_str()[0] == '/' );	// starts with a root char
}

/**
 * Check is the path is relative and not absolute.
 *
 * @returns true if the path is relative
 */
bool BWResourceImpl::pathIsRelative( const std::string& path )
{
	// has no drive letter, and no root directory
	return !hasDriveInPath( path ) && ( path.c_str()[0] != '/' && path.c_str()[0] != '\\');
}

/**
 * Returns the first path in the paths.xml file
 *
 * @returns first path in paths.xml
 */
const std::string BWResource::getDefaultPath( void )
{
	return instance().pimpl_->paths_.empty() ? std::string( "" ) : instance().pimpl_->paths_[0];
}

/**
 * Returns the path in the paths.xml file at position index
 *
 * @returns path at position index in paths.xml
 */
const std::string BWResource::getPath( int index )
{
	if (!instance().pimpl_->paths_.empty() &&
			0 <= index && index < (int)instance().pimpl_->paths_.size() )
	{
		return instance().pimpl_->paths_[index];
	}
	else
		return std::string( "" );
}

int BWResource::getPathNum()
{
	return int( instance().pimpl_->paths_.size() );
}

std::string BWResource::getPathAsCommandLine( bool ignoreLastPath /* = false */ )
{
	std::string commandLine;
	int totalPathNum = getPathNum();
	// generally we should only have the game path & bigworld path.
	// so we remove the automatically appended path
	if( ignoreLastPath )
		--totalPathNum;
	if ( totalPathNum > 0 )
		commandLine = "--res \"";
	for( int i = 0; i < totalPathNum; ++i )
	{
		commandLine += getPath( i );
		if( i != totalPathNum - 1 )
			commandLine += ';';
	}
	if ( totalPathNum > 0 )
		commandLine += "\"";
	return commandLine;
}

/**
 * Ensures that the path exists
 *
 */
bool BWResource::ensurePathExists( const std::string& path )
{
	std::string nPath = path;
    std::replace( nPath.begin(), nPath.end(), '/', '\\' );

    if ( nPath.empty() )
		return false; // just to be safe

    if ( nPath[nPath.size()-1] != '\\' )
		nPath = getFilePath( nPath );

    std::replace( nPath.begin(), nPath.end(), '/', '\\' );

	// skip drive letter, if using absolute paths
	uint poff = 0;
	if ( nPath.size() > 1 && nPath[1] == ':' )
		poff = 2;

	while (1)
	{
		poff = nPath.find_first_of( '\\', poff + 1 );
		if (poff > nPath.length())
			break;

		std::string subpath = nPath.substr( 0, poff );
		instance().fileSystem()->makeDirectory( subpath.c_str() );
	}
	return true;
}

/**
 * Ensures that the absolute path exists on the system
 */
bool BWResource::ensureAbsolutePathExists( const std::string& path )
{
	std::string nPath = path;
    std::replace( nPath.begin(), nPath.end(), '/', '\\' );

    if ( nPath.empty() )
	{
		return false; // just to be safe
	}

    if ( nPath[nPath.size()-1] != '\\' )
		nPath = getFilePath( nPath );

    std::replace( nPath.begin(), nPath.end(), '/', '\\' );

	// skip drive letter, if using absolute paths
	uint poff = 0;
	if ( nPath.size() > 1 && nPath[1] == ':' )
		poff = 2;

	while (1)
	{
		poff = nPath.find_first_of( '\\', poff + 1 );
		if (poff > nPath.length())
			break;

		std::string subpath = nPath.substr( 0, poff );
#ifdef _WIN32
		if( _mkdir( subpath.c_str() ) == ENOENT)
#else
		if( mkdir( subpath.c_str(), 0 ) == -1 && errno == ENOENT)
#endif // _WIN32
		{
			return false;
		}

	}
	return true;
}

bool BWResource::isValid()
{
	return instance().pimpl_->paths_.size( ) != 0;
}

std::string BWResourceImpl::formatSearchPath( const std::string& path, const std::string& pathXml )
{
	std::string tmpPath( path.c_str() );

#ifdef _WIN32
	// get the applications path and load the config file from it
	std::string appDirectory;

	if (pathXml != "")
	{
		appDirectory = pathXml;
	}
	else
	{
		appDirectory = BWResourceImpl::getAppDirectory();
	}

	// Does it contain a drive letter?
	if (tmpPath.size() > 1 && tmpPath[1] == ':')
	{
		MF_ASSERT(isalpha(tmpPath[0]));
	}
	// Is it a relative tmpPath?
	else if (tmpPath.size() > 0 && tmpPath[0] != '/')
	{
		tmpPath = appDirectory + tmpPath;
		// make sure slashes are windows-style
		std::replace( tmpPath.begin(), tmpPath.end(), '/', '\\' );

		// canonicalise (that is, remove redundant relative path info)
		std::wstring wtmpPath;
		bw_utf8tow( tmpPath, wtmpPath );
		wchar_t wfullPath[MAX_PATH];
		if ( PathCanonicalize( wfullPath, wtmpPath.c_str() ) )
		{
			bw_wtoutf8( wfullPath, tmpPath );
		}
	}
	else
	{
		// make sure slashes are windows-style
		std::replace( tmpPath.begin(), tmpPath.end(), '/', '\\' );

		// get absolute path
		std::wstring wtmpPath;
		bw_utf8tow( tmpPath, wtmpPath );
		wchar_t wfullPath[MAX_PATH];
		wchar_t* wfilePart;
		if ( GetFullPathName( wtmpPath.c_str(), ARRAY_SIZE( wfullPath ), wfullPath, &wfilePart ) )
		{
			bw_wtoutf8( wfullPath, tmpPath );
		}
		#ifndef EDITOR_ENABLED
			// not editor enabled, but win32, so remove drive introduced by
			// GetFullPathName to be consistent.
			tmpPath = BWResolver::removeDrive( tmpPath );
		#endif //  EDITOR_ENABLED
	}
	#if ENABLE_FILE_CASE_CHECKING
		// Since the case of the path can sometimes be wrong, we make sure the case is correct
		// here, this is so that the case checking doesn't get the actual res path wrong, just
		// the resource string
		tmpPath = BWResource::instance().pimpl_->nativeFileSystem_->correctCaseOfPath( tmpPath );
	#endif
#endif // _WIN32

	// Forward slashes only
	std::replace( tmpPath.begin(), tmpPath.end(), '\\', '/' );

	// Trailing slashes aren't allowed
	if (tmpPath[tmpPath.size() - 1] == '/')
		tmpPath = tmpPath.substr( 0, tmpPath.size() - 1 );

	return tmpPath;
}

void BWResource::addPath( const std::string& path, int atIndex )
{
	std::string fpath = BWResourceImpl::formatSearchPath( path );

	BWResourceImpl* pimpl = instance().pimpl_;
	if (uint(atIndex) >= pimpl->paths_.size())
		atIndex = pimpl->paths_.size();
	pimpl->paths_.insert( pimpl->paths_.begin()+atIndex, fpath );

	INFO_MSG( "Added res path: \"%s\"\n", fpath.c_str() );

	if ( instance().fileSystem() )
		instance().refreshPaths(); // already created fileSystem_, so recreate them
}

/**
 *	Add all the PATH_SEPERATOR separated paths in path to the search path
 */
void BWResource::addPaths( const std::string& path, const char* desc, const std::string& pathXml )
{
	std::string buf = path;
	std::vector< std::string > pathParts;
	bw_tokenise< std::string >( path, BW_RES_PATH_SEPARATOR, pathParts );

	for ( std::vector< std::string >::iterator it = pathParts.begin() ;
			it != pathParts.end() ; ++it )
	{
		const std::string & pathPart = *it;

		std::string formattedPath = BWResourceImpl::formatSearchPath( pathPart, pathXml );

#ifdef MF_SERVER
		// Convert the res path to an absolute string if it isn't already
		if (formattedPath.size() > 0 && formattedPath[0] != '/')
		{
			// get the executable path and load the config file from it
			std::string tmpPath = BWUtil::executableDirectory() + '/' + formattedPath + '\0';

			char buf[PATH_MAX + 1];
			memset( buf, 0, sizeof( buf ) );
			strncpy( buf, tmpPath.c_str(), tmpPath.size() );

			// canonicalise (that is, remove redundant relative path info)
			char *absPath = canonicalize_file_name( buf );
			if (absPath)
			{
				formattedPath = absPath;

				// malloc'ed in canonicalize_file_name() above.
				raw_free( absPath );
			}
		}
#endif

		if (instance().pimpl_->nativeFileSystem_->getFileType( formattedPath, 0 ) ==
				IFileSystem::FT_NOT_FOUND)
		{
#ifdef MF_SERVER
			ERROR_MSG( "BWResource::addPaths: "
						"Resource path does not exist: %s\n",
						formattedPath.c_str() );

			if (formattedPath.find( ';' ) != std::string::npos)
			{
				ERROR_MSG( "BWResource::addPaths: "
						"Note that the separator character is ':', not ';'\n" );
			}
#else
			CRITICAL_MSG( "Search path does not exist: %s"
						RES_PATH_HELP, formattedPath.c_str() );
#endif
		}
		else
		{
			instance().pimpl_->paths_.push_back( formattedPath );
			INFO_MSG( "Added res path: \"%s\" (%s).\n", formattedPath.c_str(), desc );
		}
	}

	#if defined( _EVALUATION )
    STRINGVECTOR::iterator it  = instance().pimpl_->paths_.begin( );
    STRINGVECTOR::iterator end = instance().pimpl_->paths_.end( );
    while ( it != end ) {
		if ( it->find( "bigworld\\res" ) != std::string::npos &&
			std::distance( it, end ) > 1 )
		{
			CRITICAL_MSG(
				"<bigworld/res> is not the last entry in the search path (%s)."
				"This is usually an error. Press 'No' to ignore and proceed "
				"(you may experience errors or other unexpected behaviour). %d"
				RES_PATH_HELP, path.c_str(), std::distance( it, end ) );
		}
		++it;
	}
	#endif

	if ( instance().fileSystem() )
		instance().refreshPaths(); // already created fileSystem_, so recreate them
}


/**
 * Remove the path in the paths.xml file at position index
 * used mainly for testing.
 */
void BWResource::delPath( int index )
{
	if (!instance().pimpl_->paths_.empty() &&
			0 <= index && index < (int)instance().pimpl_->paths_.size() )
	{
		instance().pimpl_->paths_.erase(instance().pimpl_->paths_.begin() + index);
	}
}


/**
 *	Compares the modification dates of two files, and returns
 *	true if the first file 'oldFile' is indeed older (by at least
 *	mustBeOlderThanSecs seconds) than the second file 'newFile'.
 *	Returns false on error.
 */
bool BWResource::isFileOlder( const std::string & oldFile,
	const std::string & newFile, int mustBeOlderThanSecs )
{
	bool result = false;

	uint64 ofChanged = modifiedTime( oldFile );
	uint64 nfChanged = modifiedTime( newFile );

	if (ofChanged != 0 && nfChanged != 0)
	{
		uint64	mbotFileTime = uint64(mustBeOlderThanSecs) * uint64(10000000);
		result = (ofChanged + mbotFileTime < nfChanged);
	}

	return result;
}


/**
 *	Returns the time the given file (or resource) was modified.
 *	Returns 0 on error (e.g. the file does not exist)
 *  Only files are handled for backwards compatibility.
 */
uint64 BWResource::modifiedTime( const std::string & fileOrResource )
{
#ifdef EDITOR_ENABLED
	std::string filename = BWResourceImpl::hasDriveInPath( fileOrResource ) ?
		fileOrResource : resolveFilename( fileOrResource );
#else
	std::string filename = fileOrResource;
#endif

	IFileSystem::FileInfo fi;
	if ( instance().fileSystem()->getFileType( filename, &fi ) == IFileSystem::FT_FILE )
		return fi.modified;

	return 0;
}

/**
 *	Resolves the given path to an absolute path. Loops through each filesystem
 * 	and resolves the path to the first one that contains the file.
 * 
 * 	If the path is already an absolute path (i.e. it starts with a slash or 
 * 	drive letter), it simply returns the type of the file.
 * 
 * 	To resolve the path to a full path without checking for the existence of 
 * 	the file, see MultiFileSystem::getAbsolutePath().
 * 
 * 	@param	Input: The path to resolve. Output: The resolved path. Does not
 * 			resolve the path if path does not exist in any file systems.
 * 
 * 	@return	The type of the file. 
 */
IFileSystem::FileType BWResource::resolveToAbsolutePath( 
		std::string& path )
{
	IFileSystem::FileType type;
	if (path.empty() || BWResourceImpl::pathIsRelative( path ))
	{
		type = instance().fileSystem()->resolveToAbsolutePath( path );
	}
	else
	{
		type = NativeFileSystem::getAbsoluteFileType( path );
	}

	// Ensure that it ends with a path separator if it is a directory
	if (type == IFileSystem::FT_DIRECTORY && (path.rbegin() != path.rend()) && 
			(*path.rbegin() != PATH_SEPARATOR))
	{
		path += PATH_SEPARATOR;
	}

	return type;
}

#ifdef _WIN32
/*static*/ const std::string BWResource::appDirectory()
{
	return BWResourceImpl::getAppDirectory();
}


/*static*/ const std::string BWResource::appPath()
{
#ifdef  BWCLIENT_AS_PYTHON_MODULE
	MF_ASSERT( false && "Should not get called in PyModuleHybrid." );
#endif
	return BWUtil::executablePath();
}


/**
 *	This method returns the path to an appropriate subdirectory of the current user's CSIDL_APPDATA,
 *  and ensures the method exists.
 *
 *
 *	@return	The path to the existant directory, terminated with a slash.
 *
 */
/*static*/ const std::string BWResource::appDataDirectory()
{
	return BWResourceImpl::getAppDataDirectory();
}

/**
 *	This method returns the path to an appropriate subdirectory of the current user's CSIDL_APPDATA,
 *  and ensures the method exists.
 *
 *	@return	The path to the existant directory, terminated with a slash, or "" if the directory
 *			could not be found or created.
 *
 */
/*static*/ const std::string BWResource::userDocumentsDirectory()
{
	return BWResourceImpl::getUserDocumentsDirectory();
}

/**
 *	This method returns the company name defined in this application's VS_VERSION_INFO structure
 *
 *	This method takes the string matching the current locale if possible, or the first found
 *  string otherwise.
 *
 *	@return	The company name from this application's VS_VERSION_INFO structure or "" if it could
 *			not be retrieved.
 *
 */
/*static*/ const std::string BWResource::appCompanyName()
{
	return instance().pimpl_->getAppVersionString( "CompanyName" );
}

/**
 *	This method returns the product name defined in this application's VS_VERSION_INFO structure
 *
 *	This method takes the string matching the current locale if possible, or the first found
 *  string otherwise.
 *
 *	@return	The product name from this application's VS_VERSION_INFO structure or "" if it could
 *			not be retrieved.
 *
 */
/*static*/ const std::string BWResource::appProductName()
{
	return instance().pimpl_->getAppVersionString( "ProductName" );
}


/**
 *	This method returns the Product Version from the application's VS_VERSION_INFO structure
 *
 *	@return	The product version from this application's VS_VERSION_INFO structure or "" if it could
 *			not be retrieved.
 *
 */
/*static*/const std::string BWResource::appProductVersion()
{
	return instance().pimpl_->getAppVersionString( "ProductVersion" );
}
#endif // _WIN32


std::ostream& operator<<(std::ostream& o, const BWResource& /*t*/)
{
	o << "BWResource\n";
	return o;
}



// -----------------------------------------------------------------------------
// Section: BWResolver
// -----------------------------------------------------------------------------

/**
 *	Finds a file by looking in the search paths
 *
 *	Path Search rules.
 *		If the files path has a drive letter in it, then only the filename is
 *		used. The files path must be relative and NOT start with a / or \ to use
 *		the search path.
 *
 *		If either of the top 2 conditions are true or the file is not found with
 *		its own path appended to the search paths then the file is searched for
 *		using just the filename.
 *
 *	@param file the files name
 *
 *	@return files full path and filename
 *
 */
const std::string BWResolver::findFile( const std::string& file )
{
	std::string tFile;
	bool found = false;

	// is the file relative and have no drive letter
	if ( !BWResourceImpl::hasDriveInPath( file ) && BWResourceImpl::pathIsRelative( file ) )
	{
		STRINGVECTOR::iterator it  = BWResource::instance().pimpl_->paths_.begin( );
		STRINGVECTOR::iterator end = BWResource::instance().pimpl_->paths_.end( );

		// search for the file in the supplied paths
		while( !found && it != end )
		{
			tFile  = BWResource::formatPath((*it)) + file;
			found = BWResource::fileAbsolutelyExists( tFile );
			it++;
		}

		// the file doesn't exist, let's see about its directory
		if (!found)
		{
			std::string dir = BWResource::getFilePath( file );

			STRINGVECTOR::iterator it = BWResource::instance().pimpl_->paths_.begin( );
            for (; it != end; ++it)
			{
				std::string path = std::string( BWResource::formatPath((*it)) );
				found = BWResource::fileAbsolutelyExists( path + dir + '.' );
				if (found)
				{
					tFile = path + file;
					break;
				}
			}
		}
	}
	else
	{

		// the file is already resolved!
		tFile = file;
		found = true;
	}

	if ( !found )
	{
    	tFile  = BWResource::formatPath( BWResource::getDefaultPath() ) + file;
    }

	return tFile;
}

/**
 *	Resolves a file into an absolute filename based on the search paths
 *	@see findFile()
 *
 *	@param file the files name
 *
 *	@return the resolved filename
 *
 */
const std::string BWResolver::resolveFilename( const std::string& file )
{
	std::string tmpFile = file;
    std::replace( tmpFile.begin(), tmpFile.end(), '\\', '/' );
    std::string foundFile = tmpFile;

    foundFile = findFile( tmpFile );

    if ( strchr( foundFile.c_str(), ':' ) == NULL && ( foundFile[0] == '/' || foundFile[0] == '\\' ) )
    	foundFile = appDrive_ + foundFile;

#ifdef LOGGING
    if( file != foundFile && !fileExists( foundFile ) )
    {
        dprintf( "BWResource - Invalid File: %s \n\tResolved to %s\n", file.c_str(), foundFile.c_str() );
	    std::ofstream os( "filename.log", std::ios::app | std::ios::out );
    	os << "Error resolving file: " << file << "\n";
    }
#endif

	return foundFile;
}

/**
 *	Dissolves a full filename (with drive and path information) to relative
 *	filename.
 *
 *	@param file the files name
 *
 *	@return the dissolved filename, "" if filename cannot be dissolved
 *
 */
const std::string BWResolver::dissolveFilename( const std::string& file )
{
	std::string newFile = file;
    std::replace( newFile.begin(), newFile.end(), '\\', '/' );

	std::string path;
	bool found = false;

	// is the filename valid
	// we can only dissolve an absolute path (has a root), and a drive specified
	// is optional.

    if ( !BWResourceImpl::hasDriveInPath( newFile ) && BWResourceImpl::pathIsRelative( newFile ) )
        return newFile;

    STRINGVECTOR::iterator it  = BWResource::instance().pimpl_->paths_.begin( );
    STRINGVECTOR::iterator end = BWResource::instance().pimpl_->paths_.end( );

    // search for the file in the supplied paths
    while( !found && it != end )
    {
        // get the search path
        path   = std::string( BWResource::formatPath((*it)) );

        // easiest test: is the search path a substring of the file
        if ( mf_pathcmp( path.c_str(), newFile.substr( 0, path.length( ) ).c_str() ) == 0 )
        {
            newFile = newFile.substr( path.length( ), file.length( ) );
            found = true;
        }

        if ( !found )
        {
            // is the only difference the drive specifiers?

#ifdef _WIN32
			// remove drive letters if in Win32
            std::string sPath = removeDrive( path );
			std::string fDriveLess = removeDrive( newFile );
#else
            std::string sPath = path;
			std::string fDriveLess = newFile;
#endif
            // remove the filenames
            sPath = BWResource::formatPath( sPath );
			std::string fPath = BWResource::getFilePath( fDriveLess );

            if ( ( fPath.length( ) > sPath.length() ) &&
				( mf_pathcmp( sPath.c_str(), fPath.substr( 0, sPath.length() ).c_str() ) == 0 ) )
            {
                newFile = fDriveLess.substr( sPath.length(), file.length() );
                found = true;
            }
        }

        // point to next search path
        it++;
    }

    if ( !found )
    {
        // there is still more we can do!
        // we can look for similar paths in the search path and the
        // file's path
        // eg. \demo\zerodata\scenes & m:\bigworld\zerodata\scenes
        // TODO: implement this feature if required

        // the file is invalid
#ifdef LOGGING
        dprintf( "BWResource - Invalid: %s\n", file.c_str() );
	    std::ofstream os( "filename.log", std::ios::app | std::ios::out );
    	os << "Error dissolving file: " << file << "\n";
        newFile = "";
#endif
    }


	return newFile;
}

/**
 *	Removes the drive specifier from the filename
 *
 *	@param file filename string
 *
 *	@return filename minus any drive specifier
 *
 */
const std::string BWResolver::removeDrive( const std::string& file )
{
	std::string tmpFile( file );

    int firstOf = tmpFile.find_first_of( ":" );
	if ( 0 <= firstOf && firstOf < (int)tmpFile.size( ) )
		tmpFile = tmpFile.substr( firstOf + 1 );

	return tmpFile;
}

void BWResolver::defaultDrive( const std::string& drive )
{
    STRINGVECTOR::iterator it  = BWResource::instance().pimpl_->paths_.begin( );
    STRINGVECTOR::iterator end = BWResource::instance().pimpl_->paths_.end( );

	BWResource::instance().pimpl_->defaultDrive_ = drive;

    // search for the file in the supplied paths
    while( it != end )
    {
        // get the search path
        std::string path = *it;

        if ( path.c_str()[1] != ':' )
        {
            path = BWResource::instance().pimpl_->defaultDrive_ + path;
            (*it) = path;
        }

        ++it;
    }
}

#ifdef _WIN32
std::string BWResourceImpl::appDirectoryOverride_ = "";
#endif

// bwresource.cpp
