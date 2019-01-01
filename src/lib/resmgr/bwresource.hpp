/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWRESOURCE_HPP
#define BWRESOURCE_HPP

#include "datasection.hpp"
#include "file_system.hpp"

#include "cstdmf/singleton.hpp"
#include "cstdmf/string_utils.hpp"

class MultiFileSystem;
class DataSectionCache;
class BWResourceImpl;
class BWResolver;

typedef SmartPointer<MultiFileSystem> MultiFileSystemPtr;

// Separator character for BW_RES_PATH
#ifdef unix
#define BW_RES_PATH_SEPARATOR ":"
#else
#define BW_RES_PATH_SEPARATOR ";"
#endif

/**
 *	This class is intended to be used as a singleton. It supports functionality
 *	for working with files in BigWorld's resource path.
 *
 *	It is also useful for operating multiple resource system contexts,
 *	where it may be desirable for extra resources to be visible to some
 *	threads but not others. In that case multiple instances of BWResource
 *	are permitted.
 *
 *  This class now contains all functionality previously owned by the now
 *  deprecated FilenameResolver, so use this class instead.
 */
class BWResource : public Singleton< BWResource >
{
public:
	BWResource();
	~BWResource();

	// Init methods taken from the now deprecated FilenameResolver
	static bool					init( int argc, const char * argv[] );
	static bool					init( const std::string& fullPath, bool addAsPath = true );

	static void					fini();

	void						refreshPaths();

	/// Returns the root section
	DataSectionPtr 				rootSection();
	MultiFileSystemPtr			fileSystem();

	/// Remove a resource from the cache.
	void						purge( const std::string& resourceID,
									bool recurse = false,
		   							DataSectionPtr spSection = NULL );
	void						purgeAll();

	/// Saves a section.
	bool						save( const std::string & resourceID );

	/// Shortcut method for opening a section
	static DataSectionPtr		openSection( const std::string & resourceID,
									bool makeNewSection = false,
									DataSectionCreator* creator = NULL );

	// thread watching methods
	static void watchAccessFromCallingThread(bool watch);
	static bool watchAccessFromCallingThread();
	static void checkAccessFromCallingThread(
		const std::string& path,const std::string& msg );

	// Static methods taken from the now deprecated FilenameResolver
	static IFileSystem::FileType getFileType( const std::string& file, IFileSystem::FileInfo * pFI );
	static bool					isDir( const std::string& file );
	static bool					isFile( const std::string& file );
	static bool					fileExists( const std::string& file );
	static bool					fileAbsolutelyExists( const std::string& file );
	static bool					pathIsRelative( const std::string& path );
	static bool					searchPathsForFile( std::string& file );
	static bool					searchForFile( const std::string& directory,
		std::string& file );
	static const std::string	getFilename( const std::string& file );
	static const std::string	getFilePath( const std::string& file );
	static const std::string	getExtension( const std::string& file );
	static const std::string	removeExtension( const std::string& file );
	static const std::string	changeExtension( const std::string& file,
		const std::string& newExtension );
	static       std::string 	formatPath( const std::string& path );
	static const std::string	getDefaultPath( void );
	static const std::string	getPath( int index );
	static int					getPathNum();
	static std::string			getPathAsCommandLine( bool ignoreLastPath = false);
	static void					addPath( const std::string& path, int atIndex=-1 );
	static void					addPaths( const std::string& path, const char* desc, const std::string& pathXml = "" );
	void						delPath( int index );
	static std::string			getCurrentDirectory();
	static bool					setCurrentDirectory( const std::string &currentDirectory );
	static bool					ensurePathExists( const std::string& path );
	static bool					ensureAbsolutePathExists( const std::string& path );
	static bool					isValid( );
	static bool					isFileOlder( const std::string & oldFile,
		const std::string & newFile, int mustBeOlderThanSecs = 0 );
	static uint64				modifiedTime( const std::string & fileOrResource );
	static IFileSystem::FileType resolveToAbsolutePath( std::string& path );

#ifdef _WIN32
	static const std::string	appDirectory();
	static const std::string	appPath();
	static const std::string	appDataDirectory();
	static const std::string	userDocumentsDirectory();
	static const std::string	appCompanyName();
	static const std::string	appProductName();
	static const std::string	appProductVersion();
#endif

#ifdef EDITOR_ENABLED
	// file system specific methods, hidden from the BWClient because they can
	// break working with Zip file-systems for instance.
	static const std::string	findFile( const std::string& file );
	static const std::string	removeDrive( const std::string& file );
    static void                 defaultDrive( const std::string& drive );
	static const std::string	dissolveFilename( const std::string& file );
	static const std::string	resolveFilename( const std::string& file );
	static bool			        validPath( const std::string& file );

//-----------------------------------------------------------------------------
	// wide versions, editor-only
	static const std::wstring	getFilenameW( const std::string& file );
	static const std::wstring	getFilenameW( const std::wstring& file );

	static const std::wstring	getExtensionW( const std::string& file );
	static const std::wstring	getExtensionW( const std::wstring& file );

	static const std::wstring	removeExtensionW( const std::string& file );
	static const std::wstring	removeExtensionW( const std::wstring& file );

	static bool			        validPathW( const std::wstring& file );
	static void                 defaultDriveW( const std::wstring& drive );

	static const std::wstring	findFileW( const std::string& file );
	static const std::wstring	removeDriveW( const std::string& file );
	static const std::wstring	dissolveFilenameW( const std::string& file );
	static const std::wstring	resolveFilenameW( const std::string& file );

	static const std::wstring	findFileW( const std::wstring& file );
	static const std::wstring	removeDriveW( const std::wstring& file );
	static const std::wstring	dissolveFilenameW( const std::wstring& file );
	static const std::wstring	resolveFilenameW( const std::wstring& file );

#endif // EDITOR_ENABLED

#if ENABLE_FILE_CASE_CHECKING
	// Methods to enable case sensitive checking.
	static void					checkCaseOfPaths(bool enable);
	static bool					checkCaseOfPaths();
#endif // ENABLE_FILE_CASE_CHECKING
	static std::string			correctCaseOfPath(std::string const &path);

	static void overrideAppDirectory(const std::string &dir);

public:
	/**
	 *	A utility class for temporarily changing the file access
	 *	watcher state for this thread, whilst automatically restoring
	 *	the old state when the class instance goes out of scope.
	 */
	class WatchAccessFromCallingThreadHolder
	{
	public:
		WatchAccessFromCallingThreadHolder( bool newState )
		{
			oldState_ = BWResource::watchAccessFromCallingThread();
			BWResource::watchAccessFromCallingThread( newState );
		}

		~WatchAccessFromCallingThreadHolder()
		{
			BWResource::watchAccessFromCallingThread( oldState_ );
		}

	private:
		bool oldState_;
	};


private:
	static void initCommon();
	static BWResource & makeInstance();

	friend std::ostream& operator<<(std::ostream&, const BWResource&);
	BWResourceImpl* pimpl_;
// to let BWResourceImpl use private methods
	friend class BWResourceImpl;
	friend class BWResolver;
};



/**
 *  -- BWResolver - AVOID USING THIS CLASS DIRECTLY --
 *  Helper class, implements the resolve/dissolve functionality, previously
 *  provided by the FilenameResolver.
 *  -- IMPORTANT NOTES --
 *  - New code should stay away from this class unless absolutely
 *  necesary, since it breaks relative path names and file system
 *  abstraction.
 *  - If needed, as for example in the tools, the preferred way to use this
 *  functionality is through the BWResource class, so it makes sure it is only
 *  in the tools with the EDITOR_ENABLED flag.
 */
class BWResolver
{
public:
	static const std::string	findFile( const std::string& file );
	static const std::string	removeDrive( const std::string& file );
    static void                 defaultDrive( const std::string& drive );
	static const std::string	dissolveFilename( const std::string& file );
	static const std::string	resolveFilename( const std::string& file );
};

#ifdef CODE_INLINE
#include "bwresource.ipp"
#endif

#endif // BWRESOURCE_HPP
