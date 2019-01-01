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
 *	This file defines the WinFileSystem class.
 */	
#ifndef _WIN_FILE_SYSTEM_HEADER
#define _WIN_FILE_SYSTEM_HEADER

#include "file_system.hpp"
#include "filename_case_checker.hpp"

/**
 *	This class provides an implementation of IFileSystem that
 *	reads from a Windows filesystem.	
 */	
class WinFileSystem : public IFileSystem 
{
public:	
	WinFileSystem(const std::string& basePath);
	~WinFileSystem();

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

	virtual std::string correctCaseOfPath(const std::string &path) const;
#if ENABLE_FILE_CASE_CHECKING
	virtual void		checkCaseOfPaths(bool enable);
#endif // ENABLE_FILE_CASE_CHECKING
							
	virtual FileSystemPtr	clone();

	static 	FileType getAbsoluteFileType( const std::string & path, 
							FileInfo * pFI = NULL );
	
private:
	WinFileSystem( const WinFileSystem & other );
	WinFileSystem & operator=( const WinFileSystem & other );

	static 	FileType getAbsoluteFileTypeInternal( const std::string & path, 
							FileInfo * pFI = NULL );
	
	std::string					basePath_;
	mutable FilenameCaseChecker	caseChecker_;
#if ENABLE_FILE_CASE_CHECKING
	bool						checkCase_;
	SimpleMutex					pathCacheMutex_;
	std::set<std::string>		validPathCache_;
	std::set<std::string>		invalidPathCache_;
#endif // ENABLE_FILE_CASE_CHECKING

	static long callsTo_getFileType_;
	static long callsTo_readDirectory_;
	static long callsTo_readFile_;
	static long callsTo_makeDirectory_;
	static long callsTo_writeFile_;
	static long callsTo_moveFileOrDirectory_;
	static long callsTo_eraseFileOrDirectory_;
	static long callsTo_posixFileOpen_;

};

#endif
