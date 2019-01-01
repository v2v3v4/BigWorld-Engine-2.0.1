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
#include "cache_file_system.hpp"
#include "auto_config.hpp"

#include "cstdmf/diary.hpp"
#include "cstdmf/string_utils.hpp"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "shlwapi.lib" )

#include <algorithm>


DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )


/**
 *	This is the constructor.
 *
 *	@param targetPath		target path to write cached files
 *	@param source			data section contains the source directories
 */
CacheFileSystem::CacheFileSystem( const std::string& targetPath,
								   DataSectionPtr source )
	: fileSystem_( new MultiFileSystem() ), targetPath_( targetPath )
{
	std::replace( targetPath_.begin(), targetPath_.end(), '/', '\\' );

	if (*targetPath_.rbegin() != '\\')
	{
		targetPath_ += '\\';
	}

	if (ensureDirectoryExist( targetPath_ ))
	{
		std::vector<DataSectionPtr> paths;

		source->openSections( "path", paths );

		for (std::vector<DataSectionPtr>::iterator iter = paths.begin();
			iter != paths.end(); ++iter)
		{
			std::string path = (*iter)->asString();

			if (!isDirectoryExist( path ))
			{// path is relative
				path = targetPath_ + path;
			}

			if (isDirectoryExist( path ))
			{
				char tmp[ MAX_PATH + 1 ];

				std::replace( path.begin(), path.end(), '/', '\\' );

				if ( PathCanonicalizeA( tmp, path.c_str() ))
				{
					fileSystem_->addBaseFileSystem( NativeFileSystem::create( tmp ) );

					INFO_MSG( "CacheFileSystem::CacheFileSystem: "
						"add source path %s\n", tmp );
				}
				else
				{
					CRITICAL_MSG( "CacheFileSystem::CacheFileSystem: "
						"Cannot find source path %s\n", tmp );
				}
			}
			else
			{
				CRITICAL_MSG( "CacheFileSystem::CacheFileSystem: "
					"Cannot find source path %s\n", path.c_str() );
			}
		}

		std::vector<DataSectionPtr> included;

		source->openSections( "included", included );

		for (std::vector<DataSectionPtr>::iterator iter = included.begin();
			iter != included.end(); ++iter)
		{
			DataSectionPtr include = *iter;

			if (!include->asString().empty())
			{
				includedItems_.insert( include->asString() );
			}
		}

		if (DataSectionPtr ignored = source->openSection( "ignored" ))
		{
			for (int i = 0; i < ignored->countChildren(); ++i)
			{
				DataSectionPtr ds = ignored->openChild( i );
				IgnoredSections sections;

				ds->readStrings( "section", sections );

				if (ds->sectionName() == "prefix")
				{
					ignoredPrefix_[ ds->asString() ] = sections;
				}
				else if (ds->sectionName() == "suffix")
				{
					ignoredSuffix_[ ds->asString() ] = sections;
				}
			}
		}

		std::vector<DataSectionPtr> replaces;

		source->openSections( "replace", replaces );

		for (std::vector<DataSectionPtr>::iterator iter = replaces.begin();
			iter != replaces.end(); ++iter)
		{
			DataSectionPtr replace = *iter;
			std::string with = replace->readString( "with" );

			if (!replace->asString().empty() && !with.empty())
			{
				replaces_[ replace->asString() ] = with;
			}
		}
	}
	else
	{
		CRITICAL_MSG( "CacheFileSystem::CacheFileSystem: "
			"Cannot create the target path %s\n", targetPath.c_str() );
	}
}


/**
 *	This method returns the file type of a given path within the file system.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return The type of file
 */
IFileSystem::FileType CacheFileSystem::getFileType( const std::string & path,
	FileInfo * pFI ) const
{
	return fileSystem_->getFileType( path, pFI );
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
IFileSystem::FileType CacheFileSystem::getFileTypeEx( const std::string & path,
	FileInfo * pFI )
{
	return fileSystem_->getFileTypeEx( path, pFI );
}

/**
 *	This method converts the given file event time to a string.
 *	The string is in the same format as would be stored in a CVS/Entries file.
 */
std::string	CacheFileSystem::eventTimeToString( uint64 eventTime )
{
	return fileSystem_->eventTimeToString( eventTime );
}


/**
 *	This method reads the contents of a directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A vector containing all the filenames in the directory.
 */
IFileSystem::Directory CacheFileSystem::readDirectory(const std::string& path)
{
	return fileSystem_->readDirectory( path );
}

/**
 *	This method reads the contents of a file via index in a directory.
 *
 *	@param dirPath		Path relative to the base of the filesystem.
 *	@param index		Index into the directory.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr CacheFileSystem::readFile(const std::string & dirPath, uint index)
{
	CRITICAL_MSG( "CacheFileSystem::readFile( const std::string, uint ) is not supported" );
	return NULL;
}

/**
 *	This method reads the contents of a file
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr CacheFileSystem::readFile(const std::string& path)
{
	cacheFile( path );
	return NULL;
}

/**
 *	This method creates a new directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return True if successful
 */
bool CacheFileSystem::makeDirectory(const std::string& path)
{
	return fileSystem_->makeDirectory( path );
}

/**
 *	This method writes a file to disk.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pData	Data to write to the file
 *	@param binary	Write as a binary file (Windows only)
 *	@return	True if successful
 */
bool CacheFileSystem::writeFile( const std::string& path,
		BinaryPtr pData, bool binary )
{
	return false;
}

/**
 *	This method opens a file using POSIX semantics.
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param mode		The mode that the file should be opened in.
 *	@return	True if successful
 */
FILE* CacheFileSystem::posixFileOpen( const std::string& path,
		const char * mode )
{
	cacheFile( path );
	return NULL;
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
std::string	CacheFileSystem::getAbsolutePath( const std::string& path ) const
{
	std::string resolvePath = path;

	if (fileSystem_->resolveToAbsolutePath( resolvePath ) != FT_NOT_FOUND)
	{
		return resolvePath;
	}

	return fileSystem_->getAbsolutePath( path );
}


/**
 *	This corrects the case of the given path by getting the path that the
 *	operating system has on disk.
 *
 *	@param path		The path to get the correct case of.
 *	@return			The path corrected for case to match what is stored on
 *					disk.  If the path does not exist then it is returned.
 */
std::string CacheFileSystem::correctCaseOfPath(const std::string &path) const
{
	return fileSystem_->correctCaseOfPath( path );
}


/**
 *	This method renames the file or directory specified by oldPath
 *	to be specified by newPath. oldPath and newPath need not be in common.
 *
 *	Returns true on success
 */
bool CacheFileSystem::moveFileOrDirectory( const std::string & oldPath,
	const std::string & newPath )
{
	return fileSystem_->moveFileOrDirectory( oldPath, newPath );
}


/**
 *	This method erases the file or directory specified by path.
 *	Directories need not be empty.
 *
 *	Returns true on success
 */
bool CacheFileSystem::eraseFileOrDirectory( const std::string & path )
{
	return fileSystem_->eraseFileOrDirectory( path );
}


/**
 *	This method returns a IFileSystem pointer to a copy of this object.
 *
 *	@return	a copy of this object
 */
FileSystemPtr CacheFileSystem::clone()
{
	return new CacheFileSystem( *this );
}


/**
 *	This method checks if the file is ignored
 */
bool CacheFileSystem::isTotallyIgnored( std::string path ) const
{
	_strlwr_s( &path[0], path.size() + 1 );

	for (IncludedItems::const_iterator iter = includedItems_.begin();
		iter != includedItems_.end(); ++iter)
	{
		if (strncmp( path.c_str(), iter->c_str(), iter->size() ) == 0)
		{
			return false;
		}
	}

	for (IgnoredItems::const_iterator iter = ignoredPrefix_.begin();
		iter != ignoredPrefix_.end(); ++iter)
	{
		if (iter->second.empty() &&
			strncmp( path.c_str(), iter->first.c_str(), iter->first.size() ) == 0)
		{
			return true;
		}
	}

	for (IgnoredItems::const_iterator iter = ignoredSuffix_.begin();
		iter != ignoredSuffix_.end(); ++iter)
	{
		if (iter->second.empty() && iter->first.size() <= path.size() &&
			strcmp( path.c_str() + path.size() - iter->first.size(), iter->first.c_str() ) == 0)
		{
			return true;
		}
	}

	return false;
}


/**
 *	This method removes the ignored sections from certain files
 */
void CacheFileSystem::removeIgnoredSections( std::string path ) const
{
	_strlwr_s( &path[0], path.size() + 1 );

	for (IgnoredItems::const_iterator iter = ignoredPrefix_.begin();
		iter != ignoredPrefix_.end(); ++iter)
	{
		if (!iter->second.empty() &&
			strncmp( path.c_str(), iter->first.c_str(), iter->first.size() ) == 0)
		{
			DataSectionPtr ds = BWResource::openSection( path );

			for (IgnoredSections::const_iterator it = iter->second.begin();
				it != iter->second.end(); ++it)
			{
				ds->delChild( *it );
			}

			ds->save();
		}
	}

	for (IgnoredItems::const_iterator iter = ignoredSuffix_.begin();
		iter != ignoredSuffix_.end(); ++iter)
	{
		if (!iter->second.empty() && iter->first.size() <= path.size() &&
			strcmp( path.c_str() + path.size() - iter->first.size(), iter->first.c_str() ) == 0)
		{
			DataSectionPtr ds = BWResource::openSection( path );

			for (IgnoredSections::const_iterator it = iter->second.begin();
				it != iter->second.end(); ++it)
			{
				ds->delChild( *it );
			}

			ds->save();
		}
	}
}


/**
 *	This method checks if this source path should be replaced
 */
void CacheFileSystem::checkForReplace( std::string* pPath ) const
{
	MF_ASSERT_DEV( pPath != NULL );

	std::string fileName = BWResource::getFilename( *pPath );
	Replaces::const_iterator iter = replaces_.find( fileName );

	if (iter != replaces_.end())
	{
		*pPath = BWResource::getFilePath( *pPath );
		std::replace( pPath->begin(), pPath->end(), '/', '\\' );
		*pPath += iter->second;
	}
}


/**
 *	This method tries to cache the file being accessed
 */
void CacheFileSystem::cacheFile( std::string path ) const
{
	static THREADLOCAL(bool) in = false;

	if (isTotallyIgnored( path ))
	{
		return;
	}

	if (!in)
	{
		in = true;

		std::replace( path.begin(), path.end(), '/', '\\' );

		std::string targetPath = targetPath_ + path;
		std::string originalPath = path;
		IFileSystem::FileType type = fileSystem_->resolveToAbsolutePath( path );

		if (type == FT_DIRECTORY)
		{
			ensureDirectoryExist( targetPath.c_str() );
		}
		else if (type == FT_FILE || type == FT_ARCHIVE)
		{
			bool needUpdate = BWResource::fileAbsolutelyExists( path );

			if (needUpdate)
			{
				if (BWResource::fileAbsolutelyExists( targetPath ))
				{
					FileInfo fiTarget;
					WinFileSystem::getAbsoluteFileType( targetPath, &fiTarget );

					FileInfo fiSource;
					fileSystem_->getFileType( originalPath, &fiSource );

					if (fiTarget.modified > fiSource.modified)
					{
						needUpdate = false;
					}
				}
			}

			if (needUpdate)
			{
				ensureDirectoryExist( BWResource::getFilePath( targetPath ) );

				checkForReplace( &path );

				if (CopyFileA( path.c_str(), targetPath.c_str(), FALSE ))
				{
					// TODO: make it more generic
					strlwr( &path[0] );

					if (path.size() > 4 && path.substr( path.size() - 4 ) == ".fxo")
					{
						path[path.size() - 6] = '0';
						path[path.size() - 5] = '0';
						targetPath[targetPath.size() - 6] = '0';
						targetPath[targetPath.size() - 5] = '0';
						CopyFileA( path.c_str(), targetPath.c_str(), FALSE );

						path[path.size() - 6] = '0';
						path[path.size() - 5] = '1';
						targetPath[targetPath.size() - 6] = '0';
						targetPath[targetPath.size() - 5] = '1';
						CopyFileA( path.c_str(), targetPath.c_str(), FALSE );

						path[path.size() - 6] = '1';
						path[path.size() - 5] = '0';
						targetPath[targetPath.size() - 6] = '1';
						targetPath[targetPath.size() - 5] = '0';
						CopyFileA( path.c_str(), targetPath.c_str(), FALSE );

						path[path.size() - 6] = '1';
						path[path.size() - 5] = '1';
						targetPath[targetPath.size() - 6] = '1';
						targetPath[targetPath.size() - 5] = '1';
						CopyFileA( path.c_str(), targetPath.c_str(), FALSE );
					}

					removeIgnoredSections( originalPath );
				}
				else
				{
					ERROR_MSG( "CacheFileSystem::cacheFile: "
						"failed to cache file from %s to %s\n",
						path.c_str(), targetPath.c_str() );
				}
			}
		}

		in = false;
	}
}


/**
 *	This method tries to cache the file being accessed
 */
void CacheFileSystem::cacheResourcesXML() const
{
	std::vector<BinaryPtr> bins;
	std::vector<DataSectionPtr> sections;

	fileSystem_->collateFiles( AutoConfig::s_resourcesXML, bins );

	if (!bins.empty())
	{
		DataSectionPtr resourceXML = BWResource::openSection(
			AutoConfig::s_resourcesXML, true );

		for (std::vector<BinaryPtr>::iterator it = bins.begin();
			it != bins.end(); ++it)
		{
			sections.push_back(
				DataSection::createAppropriateSection( "root", *it ) );
		}

		for (std::vector<DataSectionPtr>::reverse_iterator iter = sections.rbegin();
			iter != sections.rend(); ++iter )
		{
			std::vector<DataSectionPtr> dss;
			std::vector<std::string> paths;

			dss.push_back( *iter );
			paths.push_back( "" );

			while (!dss.empty())
			{
				DataSectionPtr ds = *dss.begin();
				std::string path = *paths.begin();
				std::string value = ds->asString();

				if (!value.empty())
				{
					resourceXML->writeString( path, value );
				}

				dss.erase( dss.begin() );
				paths.erase( paths.begin() );

				for (int i = 0; i < ds->countChildren(); ++i)
				{
					DataSectionPtr child = ds->openChild( i );

					dss.push_back( child );
					paths.push_back( path + child->sectionName() + '/' );
				}
			}
		}

		resourceXML->save();
	}
}

/**
 *	This method checks if a path is existing and will try to create it
 *	if it doesn't exists
 *
 *	@param path		absolute path to check
 *
 *	@return			true if existing or created
 */
bool CacheFileSystem::ensureDirectoryExist( std::string path )
{
	std::replace( path.begin(), path.end(), '/', '\\' );

	return isDirectoryExist( path ) ||
		SHCreateDirectoryExA( NULL, path.c_str(), NULL ) == ERROR_SUCCESS;
}


/**
 *	This method checks if a path is existing
 *
 *	@param path		absolute path to check
 *
 *	@return			true if existing
 */
bool CacheFileSystem::isDirectoryExist( std::string path )
{
	std::replace( path.begin(), path.end(), '/', '\\' );

	DWORD attr = GetFileAttributesA( path.c_str() );

	return attr != INVALID_FILE_ATTRIBUTES &&
		( attr & FILE_ATTRIBUTE_DIRECTORY );
}


// cache_file_system.cpp
