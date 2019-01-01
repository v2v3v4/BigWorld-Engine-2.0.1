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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

#include "cstdmf/debug.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/bw_util.hpp"

#include "bwresource.hpp"
#include "unix_file_system.hpp"

#include "bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

/**
 *	This is the constructor.
 *
 *	@param basePath		Base path of the filesystem, including trailing '/'
 */
UnixFileSystem::UnixFileSystem(const std::string& basePath) :
	basePath_(basePath)
{
	if (!basePath_.empty() && *basePath_.rbegin() != '/' && *basePath_.rbegin() != '\\')
	{
		basePath_ += '/';
	}
}

/**
 *	This is the destructor.
 */
UnixFileSystem::~UnixFileSystem()
{
}


/**
 *	This method returns the file type of a given path within the file system.
 *
 *	@param path		Path relative to the base of the filesystem.
 *  @param pFI		Pointer to a FileInfo structure to fill with the file's
 *                  details such as size, modification time..etc.
 *
 *	@return The type of file as a FileType enum value.
 */
IFileSystem::FileType UnixFileSystem::getFileType( const std::string& path,
	FileInfo * pFI ) const
{
	BWResource::checkAccessFromCallingThread( path,
			"UnixFileSystem::getFileType" );

	return UnixFileSystem::getAbsoluteFileTypeInternal( basePath_ + path, pFI );
}


/**
 *	This method returns the file type of a absolute path.
 *
 *	@param	path	Absolute path.
 * 	@param	pFI		Output parameter. If non-NULL, will contain additional 
 * 					information about the file.	
 *
 *	@return The type of file
 */
IFileSystem::FileType UnixFileSystem::getAbsoluteFileType( 
		const std::string& path, FileInfo * pFI )
{
	BWResource::checkAccessFromCallingThread( path,
			"UnixFileSystem::getAbsoluteFileType" );

	return UnixFileSystem::getAbsoluteFileTypeInternal( path, pFI );	
}

/**
 *	This method returns the file type of a absolute path. Does not check
 * 	access from calling thread.
 *
 *	@param	path	Absolute path.
 * 	@param	pFI		Output parameter. If non-NULL, will contain additional 
 * 					information about the file.	
 *
 *	@return The type of file
 */
inline IFileSystem::FileType UnixFileSystem::getAbsoluteFileTypeInternal( 
		const std::string& path, FileInfo * pFI )
{
	struct stat s;

	FileType ft = FT_NOT_FOUND;

	BWConcurrency::startMainThreadIdling();
	bool good = stat(path.c_str(), &s) == 0;
	BWConcurrency::endMainThreadIdling();

	if (good)
	{
		if(S_ISREG(s.st_mode))
		{
			ft = FT_FILE;
		}
		if(S_ISDIR(s.st_mode))
		{
			ft = FT_DIRECTORY;
		}

		if (ft != FT_NOT_FOUND && pFI != NULL)
		{
			pFI->size = s.st_size;
			pFI->created = std::min( s.st_ctime, s.st_mtime );
			pFI->modified = s.st_mtime;
			pFI->accessed = s.st_atime;
		}
	}

	return ft;
}


/*
 *	Overvide from IFileSystem. Also returns whether the file is an archive.
 */
IFileSystem::FileType UnixFileSystem::getFileTypeEx( const std::string& path,
	FileInfo * pFI )
{
	FileType ft = this->UnixFileSystem::getFileType( path, pFI );

	if (ft == FT_FILE)
	{
		FILE* pFile = this->posixFileOpen( path, "rb" );
		if( pFile )
		{
			uint16 magic=0;
			int frr = fread( &magic, sizeof( magic ), 1, pFile );
			fclose(pFile);

			if ((frr == 1) && (magic == ZIP_MAGIC))
			{
				ft = FT_ARCHIVE;
			}
		}
	}

	return ft;
}


/**
 *	Convert the event time into a CVS/Entries style string
 */
std::string	UnixFileSystem::eventTimeToString( uint64 eventTime )
{
	struct tm etm;
	memset( &etm, 0, sizeof(etm) );

	time_t eventUnixTime = time_t(eventTime);
	gmtime_r( &eventUnixTime, &etm );

	char buf[32];
/*	sprintf( buf, "%s %s %02d %02d:%02d:%02d %d",
		g_daysOfWeek[etm.tm_wday], g_monthsOfYear[etm.tm_mon+1], etm.tm_mday,
		etm.tm_hour, etm.tm_min, etm.tm_sec, 1900 + etm.tm_year );			*/
	asctime_r( &etm, buf );
	if (buf[24] == '\n') buf[24] = 0;
	return buf;
}


/**
 *	This method reads the contents of a directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A vector containing all the filenames in the directory.
 */
IFileSystem::Directory UnixFileSystem::readDirectory(const std::string& path)
{
	BWResource::checkAccessFromCallingThread( path,
			"UnixFileSystem::readDirectory" );

	Directory dir;
	DIR* pDir;
	dirent* pEntry;

	BWConcurrency::startMainThreadIdling();

	pDir = opendir((basePath_ + path).c_str());
	if (pDir != NULL)
	{
		pEntry = readdir(pDir);

		while(pEntry)
		{
			std::string name = pEntry->d_name;

			if(name != "." && name != "..")
				dir.push_back(pEntry->d_name);

			pEntry = readdir(pDir);
		}

		closedir(pDir);
	}

	BWConcurrency::endMainThreadIdling();

	return dir;
}


BinaryPtr UnixFileSystem::readFile(const std::string & dirPath, uint index)
{
	IFileSystem::Directory dir = readDirectory(dirPath);
	if (dir.size() && index>=0 && index<dir.size())
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
BinaryPtr UnixFileSystem::readFile(const std::string& path)
{
	BWResource::checkAccessFromCallingThread( path,
			"UnixFileSystem::readFile" );

	FILE* pFile;
	char* buf;
	int len;

	BWConcurrency::startMainThreadIdling();
	pFile = bw_fopen((basePath_ + path).c_str(), "rb");
	BWConcurrency::endMainThreadIdling();

	if(!pFile)
		return static_cast<BinaryBlock *>( NULL );

	fseek(pFile, 0, SEEK_END);
	len = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	if(len <= 0)
	{
		ERROR_MSG("UnixFileSystem: No data in %s\n", path.c_str());
		fclose( pFile );
		return static_cast<BinaryBlock *>( NULL );
	}

	BinaryPtr bp = new BinaryBlock( NULL, len, "BinaryBlock/UnixFileSystem" );
	buf = (char *) bp->data();

	BWConcurrency::startMainThreadIdling();
	bool bad = !fread( buf, len, 1, pFile );

	BWConcurrency::endMainThreadIdling();

	if (bad)
	{
		ERROR_MSG( "UnixFileSystem: Error reading from %s\n", path.c_str() );
		fclose( pFile );
		return static_cast<BinaryBlock *>( NULL );
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
bool UnixFileSystem::makeDirectory(const std::string& path)
{
	return mkdir((basePath_ + path).c_str(), 0770) == 0;
}

/**
 *	This method writes a file to disk.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pData	Data to write to the file
 *	@param binary	Write as a binary file (Windows only)
 *	@return	True if successful
 */
bool UnixFileSystem::writeFile(const std::string& path,
		BinaryPtr pData, bool /*binary*/)
{
	BWConcurrency::startMainThreadIdling();
	FILE* pFile = bw_fopen((basePath_ + path).c_str(), "w");
	BWConcurrency::endMainThreadIdling();

	if(!pFile)
	{
		return false;
	}

	if(pData->len())
	{
		BWConcurrency::startMainThreadIdling();
		bool bad = fwrite(pData->data(), pData->len(), 1, pFile) != 1;
		BWConcurrency::endMainThreadIdling();

		if (bad)
		{
			ERROR_MSG("UnixFileSystem: Error writing to %s\n", path.c_str());
			return false;
		}
	}

	fclose(pFile);
	return true;
}


/**
 *	This method opens a file using POSIX semantics.
 *
 *	@param path		Path relative to the base of the filesystem.
 *  @param mode		Mode to use to open the file with, eg: "r", "w", "a"
 *                  (see system documentation for the fopen function
 *                  for details).
 *
 *	@return	 A FILE pointer on success, NULL on error.
 */
FILE * UnixFileSystem::posixFileOpen( const std::string& path,
	const char * mode )
{
	BWConcurrency::startMainThreadIdling();
	//char mode[3] = { writeable?'w':'r', binary?'b':0, 0 };
	FILE * pFile = bw_fopen( (basePath_ + path).c_str(), mode );
	BWConcurrency::endMainThreadIdling();

	if (!pFile)
	{
		// This case will happen often with multi res paths, don't flag it as
		// an error
		//ERROR_MSG("WinFileSystem: Failed to open %s\n", path.c_str());
		return NULL;
	}

	return pFile;
}

/**
 *	This method resolves the base dependent path to a fully qualified
 * 	path.
 *
 *	@param path			Path relative to the base of the filesystem.
 *
 *	@return	The fully qualified path.
 */
std::string	UnixFileSystem::getAbsolutePath( const std::string& path ) const
{
	return basePath_ + path;
}


/**
 *	This method renames the file or directory specified by oldPath
 *	to be specified by newPath. oldPath and newPath need not be in common.
 *
 *	Returns true on success
 */
bool UnixFileSystem::moveFileOrDirectory( const std::string & oldPath,
	const std::string & newPath )
{
	return rename(
		(basePath_+oldPath).c_str(), (basePath_+newPath).c_str() ) == 0;
}


/**
 *	This method erases the given file or directory.
 *	Directories need not be empty.
 *
 *	Returns true on success
 */
bool UnixFileSystem::eraseFileOrDirectory( const std::string & path )
{
	FileType ft = this->getFileType( path, NULL );
	if (ft == FT_FILE)
	{
		return unlink( (basePath_+path).c_str() ) == 0;
	}
	else if (ft == FT_DIRECTORY)
	{
		Directory d = this->readDirectory( path );
		for (uint i = 0; i < d.size(); i++)
		{
			if (!this->eraseFileOrDirectory( path + "/" + d[i] ))
				return false;
		}
		return rmdir( (basePath_+path).c_str() ) == 0;
	}
	else
	{
		return false;
	}
}

/**
 *	This is a virtual copy constructor.
 *
 *	Returns a copy of this object.
 */

FileSystemPtr UnixFileSystem::clone()
{
	return new UnixFileSystem( basePath_ );
}

// unix_file_system.cpp
