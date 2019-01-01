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
 *	@file
 *
 *	This file defines the CacheFileSystem class.
 */	
#ifndef _CACHE_FILE_SYSTEM_HEADER
#define _CACHE_FILE_SYSTEM_HEADER

#include <set>
#include "file_system.hpp"
#include "multi_file_system.hpp"

/**
 *	This class provides an implementation of IFileSystem that
 *	reads from native source filesystems and cache read file in
 *	target folder.
 */	
class CacheFileSystem : public IFileSystem 
{
public:	
	CacheFileSystem( const std::string& targetPath, DataSectionPtr source );

	virtual FileType	getFileType(const std::string& path, FileInfo * pFI) const;
	virtual FileType	getFileTypeEx(const std::string& path, FileInfo * pFI);
	virtual std::string	eventTimeToString( uint64 eventTime );

	virtual Directory	readDirectory(const std::string& path);
	virtual BinaryPtr	readFile(const std::string& path);
	virtual BinaryPtr	readFile( const std::string & dirPath,
							uint index );

	virtual bool		makeDirectory(const std::string& path);
	virtual bool		writeFile(const std::string& path, 
							BinaryPtr pData, bool binary);
	virtual bool		moveFileOrDirectory( const std::string & oldPath,
							const std::string & newPath );
	virtual bool		eraseFileOrDirectory( const std::string & path );

	virtual FILE *		posixFileOpen( const std::string & path,
							const char * mode );

	virtual std::string	getAbsolutePath( const std::string& path ) const;

	virtual std::string correctCaseOfPath( const std::string &path ) const;

	virtual void		postResouceInitialised() {	cacheResourcesXML();	}

	virtual FileSystemPtr clone();

private:

	bool isTotallyIgnored( std::string path ) const;
	void removeIgnoredSections( std::string path ) const;
	void checkForReplace( std::string* pPath ) const;
	void cacheFile( std::string path ) const;
	void cacheResourcesXML() const;
	static bool ensureDirectoryExist( std::string path );
	static bool isDirectoryExist( std::string path );

	MultiFileSystemPtr			fileSystem_;
	std::string					targetPath_;

	typedef std::vector<std::string> IgnoredSections;
	typedef std::map<std::string, IgnoredSections> IgnoredItems;
	typedef std::set<std::string> IncludedItems;
	typedef std::map<std::string, std::string> Replaces;

	IgnoredItems	ignoredPrefix_;
	IgnoredItems	ignoredSuffix_;
	IncludedItems	includedItems_;
	Replaces		replaces_;
};

#endif
