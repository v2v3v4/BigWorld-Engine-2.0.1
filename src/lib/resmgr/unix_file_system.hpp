/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _UNIX_FILE_SYSTEM_HEADER
#define _UNIX_FILE_SYSTEM_HEADER

#include "file_system.hpp"

/**
 *	This class provides an implementation of IFileSystem that
 *	reads from a Unix filesystem.	
 */	
class UnixFileSystem : public IFileSystem
{
public:	
	UnixFileSystem(const std::string& basePath);
	~UnixFileSystem();

	virtual FileType getFileType(   const std::string& path, FileInfo * pFI ) const;
	virtual FileType getFileTypeEx( const std::string& path, FileInfo * pFI );

	virtual std::string	eventTimeToString( uint64 eventTime );

	virtual Directory	readDirectory(const std::string& path);
	virtual BinaryPtr	readFile(const std::string& path);
	virtual BinaryPtr	readFile(const std::string& dirPath, uint index);

	virtual bool		makeDirectory(const std::string& path);
	virtual bool		writeFile(const std::string& path, 
							BinaryPtr pData, bool binary);
	virtual bool		moveFileOrDirectory( const std::string & oldPath,
							const std::string & newPath );
	virtual bool		eraseFileOrDirectory( const std::string & path );

	virtual FILE *		posixFileOpen( const std::string & path,
							const char * mode );
	
	virtual std::string	getAbsolutePath( const std::string& path ) const;

	virtual FileSystemPtr clone();

	static 	FileType getAbsoluteFileType( const std::string & path, 
							FileInfo * pFI = NULL );
	
private:
	std::string			basePath_;

	static 	FileType getAbsoluteFileTypeInternal( const std::string & path, 
							FileInfo * pFI = NULL );
};

#endif
