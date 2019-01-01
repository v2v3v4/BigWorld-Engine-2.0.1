/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _ZIP_FILE_SYSTEM_HEADER
#define _ZIP_FILE_SYSTEM_HEADER

#include "file_system.hpp"
#include <map>
#include <string>
#include <stdio.h>
#include "cstdmf/concurrency.hpp"


#ifdef _WIN32
#define PACKED
#pragma pack(1)
#else
#define PACKED __attribute__((packed))
#endif

class ZipFileSystem;
typedef SmartPointer<ZipFileSystem> ZipFileSystemPtr;

/**
 *	This class provides an implementation of IFileSystem
 *	that reads from a zip file.	
 */	
class ZipFileSystem : public IFileSystem
{
public:
	// Maximum supported Zip file size is 2GB
	static const uint64 MAX_ZIP_FILE_KBYTES = 2 * 1024 * 1024;

	ZipFileSystem();
	ZipFileSystem( FileSystemPtr parentSystem );
	ZipFileSystem( const std::string& zipFile );
	ZipFileSystem( const std::string& zipFile, FileSystemPtr parent );
	ZipFileSystem( const std::string& zipFile,
					ZipFileSystemPtr parentSystem,
					const std::string& tag );
	~ZipFileSystem();

	// IFileSystem implementation
	virtual FileType	getFileType(const std::string& path,
							FileInfo * pFI = NULL ) const;
	virtual FileType	getFileTypeEx( const std::string & path,
							FileInfo * pFI = NULL );
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

	virtual FileSystemPtr	clone();

	std::string			fileName(const std::string& dirPath, int index);
	BinaryPtr			createZip( );
	void				clear() { closeZip(); }
	void				clear(const std::string& dirPath);
	int					fileIndex( const std::string& name );

	int	fileCount( const std::string& dirPath ) const;
	bool childNode() const { return (parentZip_ && tag_ != ""); }
	const std::string& tag() const { return tag_; }
	int fileSize( const std::string & dirPath, uint index ) const;
	FileSystemPtr parentSystem() const { return parentSystem_; }
	static bool zipTest( BinaryPtr pBinary );

	bool				dirTest(const std::string& name)
						{ return dirMap_.find( name) != dirMap_.end(); }
	void init(const std::string& zipFile, const std::string& tag,
		ZipFileSystemPtr parentZip );	
private:
	/**
	 *	This structure represents the header that appears directly before
	 *	every file within a zip file.
	 */
	struct LocalHeader
	{
		uint32	signature PACKED;
		uint16	extractorVersion PACKED;
		uint16	mask PACKED;
		uint16	compressionMethod PACKED;
		uint16	modifiedTime PACKED;
		uint16	modifiedDate PACKED;
		uint32	crc32 PACKED;
		uint32	compressedSize PACKED;
		uint32	uncompressedSize PACKED;
		uint16	filenameLength PACKED;
		uint16	extraFieldLength PACKED;
	};

	/**
	 *	This structure represents an entry in the directory table at the
	 *	end of a zip file.
	 */
	struct DirEntry
	{
		uint32	signature PACKED;
		uint16	creatorVersion PACKED;
		uint16	extractorVersion PACKED;
		uint16	mask PACKED;
		uint16	compressionMethod PACKED;
		uint16	modifiedTime PACKED;
		uint16	modifiedDate PACKED;
		uint32	crc32 PACKED;
		uint32	compressedSize PACKED;
		uint32	uncompressedSize PACKED;
		uint16	filenameLength PACKED;
		uint16	extraFieldLength PACKED;
		uint16	fileCommentLength PACKED;
		uint16	diskNumberStart PACKED;
		uint16	internalFileAttr PACKED;
		uint32	externalFileAttr PACKED;
		int32	localHeaderOffset PACKED;
	};

	/**
	 *	This class represents an individual file in the zip.
	 */
	class LocalFile
	{
	public:
		//TODO: move out inlines and comment
		LocalFile() : filename_(""), pData_(NULL), localOffset_(0),
					bCompressed_(false)
		{
			entry_.localHeaderOffset = -1;
		}
		LocalFile(const std::string& filename, BinaryPtr data) :
					filename_(filename), pData_(data), localOffset_(0),
					bCompressed_(false)
		{
			entry_.localHeaderOffset = -1;
			updateHeader();
		}

		void update(const std::string& filename, BinaryPtr data)
		{
			filename_ = filename;
			pData_ = data;
			bCompressed_ = false;
			updateHeader();
		}
		void update(const std::string& filename)
		{
			filename_ = filename;
			updateHeader(false);
		}

		uint32 dataOffset() const
		{
			return entry_.localHeaderOffset + sizeof(LocalHeader) +
					entry_.filenameLength + entry_.extraFieldLength;
		}

		uint32 sizeOnDisk();
		bool isValid() const { return (pData_ != NULL);	}
		uint32 compressedSize() const { return entry_.compressedSize; }
		uint32 uncompressedSize() const { return entry_.uncompressedSize; }

		void clear()
		{
			memset(&header_, 0, sizeof(LocalHeader));
			pData_ = NULL;
			filename_ = "";
		}

		bool writeFile( FILE* pFile, uint32& offset );
		bool writeFile( BinaryPtr dst, uint32& offset );
		bool writeDirEntry( FILE* pFile, uint32& offset );
		bool writeDirEntry( BinaryPtr dst, uint32& offset );

		std::string		filename_;
		BinaryPtr		pData_;
		LocalHeader		header_;
		DirEntry		entry_;

		static BinaryPtr compressData( const BinaryPtr pData );
	private:
		uint32			localOffset_;
		bool			bCompressed_;

		void updateHeader( bool clear = true );
	};

	/**
	 *	Utility class that opens the zip file handle, and ensures it gets 
	 *	closed when this class goes out of scope.
	 */
	class FileHandleHolder
	{
	public:
		FileHandleHolder(ZipFileSystem* zfs) : valid_(false), zfs_(zfs)
		{
			MF_ASSERT(zfs_ != NULL);
			valid_ = zfs_->pFile_ != NULL || zfs->openZip(zfs->path_);
		}

		~FileHandleHolder()
		{
#ifdef EDITOR_ENABLED
			//we close the file every time if the editor is enabled to make sure 
			//we don't hold the cdata file (preventing the work of bwlockd - committing to the source control)
			if (zfs_->pFile_)
			{
				fclose(zfs_->pFile_);
				zfs_->pFile_ = NULL;
			}
#endif
		}

		bool isValid() const { return valid_; }

	private:
		bool valid_;
		ZipFileSystem* zfs_;
	};

	mutable SimpleMutex mutex_; // to make sure only one thread access the zlib stream

	// IndexPair: <central dir index, local directory index>
	typedef std::pair<uint32,uint32>			IndexPair; 
	typedef std::map<std::string, IndexPair >	FileMap;
	typedef std::map<std::string, uint32 >		FileDuplicatesMap;
	
	// DirPair: <internal name, external>
	typedef std::pair<Directory,Directory>		DirPair; 
	typedef std::map<std::string, DirPair >		DirMap;
	typedef std::vector<LocalFile>				CentralDir;

	FileDuplicatesMap	duplicates_;
	FileMap				fileMap_;
	DirMap				dirMap_;
	FILE*				pFile_;
	std::string			path_;
	std::string			tag_;
	FileSystemPtr		parentSystem_;
	ZipFileSystemPtr	parentZip_;	
	CentralDir			centralDirectory_;
	uint32				offset_;
	uint32				size_;

	ZipFileSystem( const ZipFileSystem & other );
	ZipFileSystem & operator=( const ZipFileSystem & other );

	bool				openZip(const std::string& path);	
	void				closeZip();
	virtual BinaryPtr	readFileInternal(const std::string& path);
	void				updateFile(const std::string& name, BinaryPtr data);
	bool				internalChildNode() const 
						{ return (childNode() && offset_ == 0); }

	bool				checkDuplicate(const std::string& filename);
	std::string			decodeDuplicate(const std::string& filename);
	std::string			encodeDuplicate(const std::string& filename, int count) const;

	void				getFileOffset(const std::string& path,
									uint32& offset, uint32& size);

	void resolvePath( std::string& path, bool bExtraChecks=false ) const;
	bool resolveDuplicate( std::string& path ) const;
};

#endif //_ZIP_FILE_SYSTEM_HEADER
