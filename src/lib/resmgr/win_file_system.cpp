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
#include "bwresource.hpp"
#include "win_file_system.hpp"

#include "cstdmf/diary.hpp"
#include "cstdmf/string_utils.hpp"
#include "cstdmf/guard.hpp"

#include <windows.h>

#include <algorithm>


#define conformSlash(x) (x)

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )


long WinFileSystem::callsTo_getFileType_ = 0;
long WinFileSystem::callsTo_readDirectory_ = 0;
long WinFileSystem::callsTo_readFile_ = 0;
long WinFileSystem::callsTo_makeDirectory_ = 0;
long WinFileSystem::callsTo_writeFile_ = 0;
long WinFileSystem::callsTo_moveFileOrDirectory_ = 0;
long WinFileSystem::callsTo_eraseFileOrDirectory_ = 0;
long WinFileSystem::callsTo_posixFileOpen_ = 0;



/**
 *	This is the constructor.
 *
 *	@param basePath		Base path of the filesystem, including trailing '/'
 */
WinFileSystem::WinFileSystem(const std::string& basePath) :
	basePath_(basePath)
#if ENABLE_FILE_CASE_CHECKING
	,checkCase_(false)
#endif // ENABLE_FILE_CASE_CHECKING
{
	if (!basePath_.empty() && *basePath_.rbegin() != '/' && *basePath_.rbegin() != '\\')
	{
		basePath_ += '\\';
	}

	// TODO: This should be moved to another place, such as a global library
	// intialisation.
	static bool s_watchersInited = false;

	if (!s_watchersInited)
	{
		s_watchersInited = true;
		MF_WATCH(	"WinFileSystem/getFileType()",
					WinFileSystem::callsTo_getFileType_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::getFileType() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/readDirectory()",
					WinFileSystem::callsTo_readDirectory_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::readDirectory() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/readFile()",
					WinFileSystem::callsTo_readFile_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::readFile() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/makeDirectory()",
					WinFileSystem::callsTo_makeDirectory_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::makeDirectory() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/writeFile()",
					WinFileSystem::callsTo_writeFile_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::writeFile() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/moveFileOrDirectory()",
					WinFileSystem::callsTo_moveFileOrDirectory_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::moveFileOrDirectory() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/eraseFileOrDirectory()",
					WinFileSystem::callsTo_eraseFileOrDirectory_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::eraseFileOrDirectory() since "
					"program start.");

		MF_WATCH(	"WinFileSystem/posixFileOpen()",
					WinFileSystem::callsTo_posixFileOpen_,
					Watcher::WT_READ_ONLY,
					"The number of calls to WinFileSystem::posixFileOpen() since "
					"program start.");
	}
}

/**
 *	This is the destructor.
 */
WinFileSystem::~WinFileSystem()
{
}


/**
 *	This method returns the file type of a given path within the file system.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return The type of file
 */
IFileSystem::FileType WinFileSystem::getFileType( const std::string & path,
	FileInfo * pFI ) const
{
	BW_GUARD;

	InterlockedIncrement( &callsTo_getFileType_ );

	BWResource::checkAccessFromCallingThread( path, "WinFileSystem::getFileType" );

	if ( path.empty() )
		return FT_NOT_FOUND;

	DiaryScribe ds( Diary::instance(), "ftype " + path );
	
	std::string filename = conformSlash(basePath_ + path);

#if ENABLE_FILE_CASE_CHECKING
	if (checkCase_)
		caseChecker_.check(filename);
#endif // ENABLE_FILE_CASE_CHECKING

	return WinFileSystem::getAbsoluteFileTypeInternal( filename, pFI );
}


/**
 *	This method returns the file type of a given path within the file system.
 *
 *	@param path		Absolute path to the file.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return The type of file
 */
IFileSystem::FileType WinFileSystem::getAbsoluteFileType( 
		const std::string & path, FileInfo * pFI )
{
	BWResource::checkAccessFromCallingThread( path, 
			"WinFileSystem::getAbsoluteFileType" );

	if ( path.empty() )
		return FT_NOT_FOUND;

	DiaryScribe ds( Diary::instance(), "ftypeAbs " + path );
	
	return WinFileSystem::getAbsoluteFileTypeInternal( conformSlash( path ), 
			pFI );
}


/**
 *	This method returns the file type of a given path within the file system.
 * 	Does not do slash normalisation, diary entries or check access from 
 * 	main thread.
 *
 *	@param path		Absolute path to the file.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return The type of file
 */
IFileSystem::FileType WinFileSystem::getAbsoluteFileTypeInternal( 
		const std::string & path, FileInfo * pFI )
{
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	std::wstring wpath;
	bw_utf8tow( path, wpath );
	BOOL ok = GetFileAttributesEx(
		wpath.c_str(),
		GetFileExInfoStandard,
		&attrData );
	//OutputDebugString( (path + "\n").c_str() );

	if (!ok || attrData.dwFileAttributes == DWORD(-1))
		return FT_NOT_FOUND;

	if (pFI != NULL)
	{
		pFI->size = uint64(attrData.nFileSizeHigh)<<32 | attrData.nFileSizeLow;
		pFI->created = *(uint64*)&attrData.ftCreationTime;
		pFI->modified = *(uint64*)&attrData.ftLastWriteTime;
		pFI->accessed = *(uint64*)&attrData.ftLastAccessTime;
	}

	if (attrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FT_DIRECTORY;

	return FT_FILE;
}

//TODO: add this as a configurable list.
// The code linked to this #define is in fact working fine. It is currently
// disabled until it is determined whether it is benefitial to use or not.
//
//#define ZIP_TEST_EARLY_OUT

/**
 *	This method returns the file type of a given path within the file system.
 *	This extended version checks if the file is an archive.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return The type of file
 */
IFileSystem::FileType WinFileSystem::getFileTypeEx( const std::string & path,
	FileInfo * pFI )
{
	IFileSystem::FileType ft = getFileType(path, pFI);
	if (ft != FT_FILE)
		return ft;

	FILE* pFile;

	DiaryScribe ds( Diary::instance(), "ftypeEx " + path );

#ifdef ZIP_TEST_EARLY_OUT
	size_t pos = path.rfind( "." );
	if (pos != std::string::npos)
	{
		std::string ext = path.substr( pos + 1 );
		if ( !(ext == "cdata" || ext == "zip"))
			return ft;
	}
	else
		return ft;
#endif

	pFile = this->posixFileOpen( path, "rb" );
	if(!pFile)
		return FT_NOT_FOUND;

	fseek(pFile, 0, SEEK_SET); //needed?
	uint16 magic=0;
	int frr = 1;
	frr = fread( &magic, 2, 1, pFile );
	fclose(pFile);
	if (!frr)
		return FT_FILE; //return no file instead?

	if (magic == ZIP_MAGIC)
		return FT_ARCHIVE;

	return ft;
}

extern const char * g_daysOfWeek[];
extern const char * g_monthsOfYear[];

/**
 *	This method converts the given file event time to a string.
 *	The string is in the same format as would be stored in a CVS/Entries file.
 */
std::string	WinFileSystem::eventTimeToString( uint64 eventTime )
{
	SYSTEMTIME st;
	memset( &st, 0, sizeof(st) );
	FileTimeToSystemTime( (const FILETIME*)&eventTime, &st );
	char buf[32];
	bw_snprintf( buf, sizeof(buf), "%s %s %02d %02d:%02d:%02d %d",
		g_daysOfWeek[st.wDayOfWeek], g_monthsOfYear[st.wMonth], st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wYear );
	return buf;
}


/**
 *	This method reads the contents of a directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A vector containing all the filenames in the directory.
 */
IFileSystem::Directory WinFileSystem::readDirectory(const std::string& path)
{
	InterlockedIncrement( &callsTo_readDirectory_ );

	BWResource::checkAccessFromCallingThread( path, "WinFileSystem::readDirectory" );

	Directory dir;
	bool done = false;
	WIN32_FIND_DATA findData;

	std::string dirstr = conformSlash(basePath_ + path);
	if (!dirstr.empty() && *dirstr.rbegin() != '\\' && *dirstr.rbegin() != '/')
		dirstr += "\\*";
	else
		dirstr += "*";

	std::wstring wdirstr;
	bw_utf8tow( dirstr, wdirstr );
	HANDLE findHandle = FindFirstFile( wdirstr.c_str(), &findData );

	if(findHandle == INVALID_HANDLE_VALUE)
		return dir;
		
#if ENABLE_FILE_CASE_CHECKING
	if (checkCase_)
		caseChecker_.check(dirstr);
#endif // ENABLE_FILE_CASE_CHECKING

	while(!done)
	{
		std::string name;
		bw_wtoutf8( findData.cFileName, name );

		if(name != "." && name != "..")
			dir.push_back(name);

		if(!FindNextFile(findHandle, &findData))
			done = true;
	}

	FindClose(findHandle);
	return dir;
}

/**
 *	This method reads the contents of a file via index in a directory.
 *
 *	@param dirPath		Path relative to the base of the filesystem.
 *	@param index		Index into the directory.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr WinFileSystem::readFile(const std::string & dirPath, uint index)
{
	IFileSystem::Directory dir = readDirectory(dirPath);
	if (index<dir.size())
	{
		std::string fileName = dir[index];

		return readFile(dirPath + "/" + fileName);
	}
	else
		return NULL;
}

/**
 *	This method reads the contents of a file
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr WinFileSystem::readFile(const std::string& path)
{
	InterlockedIncrement( &callsTo_readFile_ );

	BWResource::checkAccessFromCallingThread( path, "WinFileSystem::readFile" );

	FILE* pFile;
	char* buf;
	int len;

	DiaryEntryPtr de1 = Diary::instance().add( "fopen " + path );
	pFile = this->posixFileOpen( path, "rb" );
	de1->stop();

	if(!pFile)
		return NULL;

	int res = fseek(pFile, 0, SEEK_END);
	if (res)
	{
		ERROR_MSG("WinFileSystem: fseek failed %s\n", path.c_str());
		fclose( pFile );
		return NULL;
	}

	len = ftell(pFile);
	res = fseek(pFile, 0, SEEK_SET);
	if (res)
	{
		ERROR_MSG("WinFileSystem: fseek failed %s\n", path.c_str());
		fclose( pFile );
		return NULL;
	}


	if(len <= 0)
	{
		ERROR_MSG("WinFileSystem: No data in %s\n", path.c_str());
		fclose( pFile );
		return NULL;
	}

	BinaryPtr bp = new BinaryBlock(NULL, len, "BinaryBlock/WinFileSystem");
	buf = (char*)bp->data();

	if(!buf)
	{
		ERROR_MSG("WinFileSystem: Failed to alloc %d bytes for '%s'\n", len, path.c_str());
		fclose( pFile );
		return NULL;
	}

	DiaryEntryPtr de2 = Diary::instance().add( "fread " + path );
	int frr = 1;
	int extra = 0;
	for (int sofa = 0; sofa < len; sofa += extra)
	{
		extra = std::min( len - sofa, 128*1024 );	// read in 128KB chunks
		frr = fread( buf+sofa, extra, 1, pFile );
		if (!frr) break;
	}
	de2->stop();
	if (!frr)
	{
		ERROR_MSG("WinFileSystem: Error reading from %s\n", path.c_str());
		fclose( pFile );
		return NULL;
	}

	fclose( pFile );

	return bp;
}

/**
 *	This method creates a new directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return True if successful
 */
bool WinFileSystem::makeDirectory(const std::string& path)
{
	InterlockedIncrement( &callsTo_makeDirectory_ );

	std::string conformedPath = conformSlash(basePath_ + path);
	std::wstring wpath;
	bw_utf8tow( conformedPath, wpath );
	return CreateDirectory(
		wpath.c_str(), NULL ) == TRUE;
}

/**
 *	This method writes a file to disk.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pData	Data to write to the file
 *	@param binary	Write as a binary file (Windows only)
 *	@return	True if successful
 */
bool WinFileSystem::writeFile( const std::string& path,
		BinaryPtr pData, bool binary )
{
	InterlockedIncrement( &callsTo_writeFile_ );

	FILE* pFile = this->posixFileOpen( path, binary?"wb":"w" );
	if (!pFile) return false;

	if (pData->len())
	{
		if (fwrite( pData->data(), pData->len(), 1, pFile ) != 1)
		{
			fclose( pFile );
			CRITICAL_MSG( "WinFileSystem: Error writing to %s. Disk full?\n",
				path.c_str() );
			// this->eraseFileOrDirectory( path );
			return false;
		}
	}

	fclose( pFile );
	return true;
}

/**
 *	This method opens a file using POSIX semantics.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param mode		The mode that the file should be opened in.
 *	@return	True if successful
 */
FILE * WinFileSystem::posixFileOpen( const std::string& path,
		const char * mode )
{
	InterlockedIncrement( &callsTo_posixFileOpen_ );

	//char mode[3] = { writeable?'w':'r', binary?'b':0, 0 };

	std::string fullPath = conformSlash(basePath_ + path);	

#if ENABLE_FILE_CASE_CHECKING
	if (checkCase_)
		caseChecker_.check(fullPath);
#endif // ENABLE_FILE_CASE_CHECKING

	FILE * pFile = bw_fopen( fullPath.c_str(), mode );

	if (!pFile || ferror(pFile))
	{
		// This case will happen often with multi 
		// res paths, don't flag it as an error
		// ERROR_MSG("WinFileSystem: Failed to open %s\n", fullPath.c_str());
		return NULL;
	}

#if ENABLE_FILE_CASE_CHECKING
	SimpleMutexHolder pathCacheMutexHolder( pathCacheMutex_ );

	if ( invalidPathCache_.count( fullPath ) )
	{
		// We already know this is an invalid path
		return pFile;
	}

	WIN32_FIND_DATA fileInfo;
	std::vector< std::string > paths;
	bw_tokenise( fullPath, "/\\", paths );
	std::string testPath = *paths.begin() + "/";
	for ( std::vector< std::string >::iterator it = paths.begin() + 1;
		it != paths.end() ; ++it )
	{
		const std::string & pathPart = *it;
		testPath += pathPart;

		if ( !validPathCache_.count( testPath ) )
		{
			std::wstring wtestPath;
			bw_utf8tow( testPath, wtestPath );
			HANDLE hFile = FindFirstFile( wtestPath.c_str(), &fileInfo );

			if ( hFile == INVALID_HANDLE_VALUE )
				break;

			FindClose( hFile );

			std::string ncFileName;
			bw_wtoutf8( fileInfo.cFileName, ncFileName );
			if ( ncFileName != pathPart )
			{
				WARNING_MSG( "WinFileSystem: Case mismatch opening %s (%s)\n",
					fullPath.c_str(), ncFileName.c_str() );

				invalidPathCache_.insert( fullPath );

				break;
			}
			else
			{
				validPathCache_.insert( testPath );
			}
		}

		testPath += "/";
	}

#endif // ENABLE_FILE_CASE_CHECKING

	return pFile;
}


/**
 *	This method resolves the base dependent path to a fully qualified
 * 	path.
 *
 *	@param path			Path relative to the base of the filesystem.
 *	@param checkExists	When true, returns the path if the file or directory
 * 						exists and returns an empty string if it doesn't. 
 * 						When false, returns the fully qualified path even
 * 						if the file or directory doesn't exist.
 *
 *	@return	The fully qualified path or empty string. See checkExists.
 */
std::string	WinFileSystem::getAbsolutePath( const std::string& path ) const
{
	return conformSlash(basePath_ + path);
}


/**
 *	This corrects the case of the given path by getting the path that the
 *	operating system has on disk.
 *
 *	@param path		The path to get the correct case of.
 *	@return			The path corrected for case to match what is stored on
 *					disk.  If the path does not exist then it is returned.
 */
std::string WinFileSystem::correctCaseOfPath(const std::string &path) const
{
	std::string fullPath = conformSlash(basePath_ + path);
	std::string corrected = caseChecker_.filenameOnDisk(fullPath);
	return corrected.substr(basePath_.length(), std::string::npos);
}


#if ENABLE_FILE_CASE_CHECKING


/**
 *	This enables or disables a filename case check.
 *
 *	@param enable	If true then files are checked for correct case in the
 *					filename, and if false then files are not checked.
 */
void WinFileSystem::checkCaseOfPaths(bool enable)
{
	checkCase_ = enable;
}


#endif // ENABLE_FILE_CASE_CHECKING


/**
 *	This method renames the file or directory specified by oldPath
 *	to be specified by newPath. oldPath and newPath need not be in common.
 *
 *	Returns true on success
 */
bool WinFileSystem::moveFileOrDirectory( const std::string & oldPath,
	const std::string & newPath )
{
	std::string oldFullPath = conformSlash(basePath_ + oldPath);
	std::string newFullPath = conformSlash(basePath_ + newPath);
	
#if ENABLE_FILE_CASE_CHECKING
	if (checkCase_)
		caseChecker_.check(oldFullPath);
#endif // ENABLE_FILE_CASE_CHECKING

	std::wstring woldFullPath;
	std::wstring wnewFullPath;
	bw_utf8tow( oldFullPath, woldFullPath );
	bw_utf8tow( newFullPath, wnewFullPath );

	return !!MoveFileEx(
		woldFullPath.c_str(),
		wnewFullPath.c_str(),
		MOVEFILE_REPLACE_EXISTING );
}


/**
 *	This method erases the file or directory specified by path.
 *	Directories need not be empty.
 *
 *	Returns true on success
 */
bool WinFileSystem::eraseFileOrDirectory( const std::string & path )
{
	FileType ft = this->getFileType( path, NULL );
	if (ft == FT_FILE)
	{
		std::string fullPath = conformSlash(basePath_+path);
#if ENABLE_FILE_CASE_CHECKING
		if (checkCase_)
		{
			caseChecker_.check(fullPath);
		}
#endif // ENABLE_FILE_CASE_CHECKING

		std::wstring wfullPath;
		bw_utf8tow( fullPath, wfullPath );
		return !!DeleteFile( wfullPath.c_str() );
	}
	else if (ft == FT_DIRECTORY)
	{
		Directory d = this->readDirectory( path );
		for (uint i = 0; i < d.size(); i++)
		{
			if (!this->eraseFileOrDirectory( path + "/" + d[i] ))
				return false;
		}
		std::string conformedPath = conformSlash(basePath_+path);
		std::wstring wconformedPath;
		bw_utf8tow( conformedPath, wconformedPath );
		return !!RemoveDirectory( wconformedPath.c_str() );
	}
	else
	{
		return false;
	}
}


/**
 *	This method returns a IFileSystem pointer to a copy of this object.
 *
 *	@return	a copy of this object
 */
FileSystemPtr WinFileSystem::clone()
{
	return new WinFileSystem( basePath_ );
}


// win_file_system.cpp
