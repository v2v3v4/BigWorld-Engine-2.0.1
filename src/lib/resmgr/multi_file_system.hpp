/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _MULTI_FILE_SYSTEM_HEADER
#define _MULTI_FILE_SYSTEM_HEADER

#include "file_system.hpp"

/**
 *	This class provides an implementation of IFileSystem that
 *	reads from several other IFileSystems.	
 */	
class MultiFileSystem : public IFileSystem 
{
public:
	MultiFileSystem();
	MultiFileSystem( const MultiFileSystem & other );
	MultiFileSystem & operator=( const MultiFileSystem & other );

	~MultiFileSystem();

	void				addBaseFileSystem( FileSystemPtr pFileSystem,
							int index = -1 );
	void				delBaseFileSystem( int index );

	virtual FileType	getFileType( const std::string& path,
							FileInfo * pFI = NULL ) const;
	virtual FileType	getFileTypeEx( const std::string& path,
							FileInfo * pFI = NULL );
	virtual std::string	eventTimeToString( uint64 eventTime );

	virtual Directory	readDirectory(const std::string& path);
	virtual BinaryPtr	readFile(const std::string& path);
	virtual BinaryPtr	readFile( const std::string & dirPath,
							uint index );

	virtual void		collateFiles(const std::string& path,
								std::vector<BinaryPtr>& ret);

	virtual bool		makeDirectory(const std::string& path);
	virtual bool		writeFile(const std::string& path, 
							BinaryPtr pData, bool binary);
	virtual bool		moveFileOrDirectory( const std::string & oldPath,
							const std::string & newPath );
	virtual bool		eraseFileOrDirectory( const std::string & path );

	virtual FILE *		posixFileOpen( const std::string & path,
							const char * mode );
	
	virtual std::string	getAbsolutePath( const std::string& path ) const;
	
	FileType 			resolveToAbsolutePath( std::string& path ) const;

	virtual std::string correctCaseOfPath(const std::string &path) const;
#if ENABLE_FILE_CASE_CHECKING
	virtual void		checkCaseOfPaths(bool enable);
#endif

	virtual void		postResouceInitialised();

	virtual FileSystemPtr	clone();

private:
	typedef std::vector<FileSystemPtr>	FileSystemVector;

	FileSystemVector	baseFileSystems_;

	void cleanUp();
	void copy( const MultiFileSystem & other  );
};
typedef SmartPointer<MultiFileSystem> MultiFileSystemPtr;

#endif // _MULTI_FILE_SYSTEM_HEADER
