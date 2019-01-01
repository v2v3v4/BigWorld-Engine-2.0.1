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
#include "zip_file_system.hpp"
#include "zip/zlib.h"
#include "cstdmf/debug.hpp"
#include "cstdmf/bw_util.hpp"
#include "cstdmf/diary.hpp"

#include <time.h>

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

#include <algorithm>

// This is used to ensure that the path to the zip file fopened is correct
#define conformSlash(x) (x)

/**
 * This returns a string with all slashes normalised to single forward slashes
 */
inline std::string cleanSlashes( const std::string& filename )
{
	std::string result = filename;
	std::string::size_type slashpos;
	// Clean up any backslashes
	std::replace( result.begin(), result.end(), '\\', '/' );
	if ( ( slashpos = result.find( "//", 0 ) ) != std::string::npos )
	{
		// Clean up any duplicate slashes
		do
		{
			result.replace( slashpos, 2, "/" );
			slashpos = result.find( "//", slashpos );
		} while ( slashpos != std::string::npos );
	}
	// Get rid of any leading slashes
	if ( result[ 0 ] == '/' )
		result.erase( 0, 1 );

	// Get rid of any trailing slashes
	if ( !result.empty() && result[ result.length() - 1 ] == '/' )
	{
		result.erase( result.length() - 1, 1 );
	}
//	if ( result != filename )
//		TRACE_MSG( "cleanSlashes: %s => %s\n", filename.c_str(), result.c_str() );
	return result;
}

/**
 * This template function performs a slash-safe lookup of a filename
 * in the provided map. It assumes the slashes loaded into the map
 * are correct!
 *
 * In a non-consumer build, it will check for case mismatch and log it
 * as a warning. This will slow down missed lookups considerably.
 */
template< class T > inline typename std::map< std::string, T >::iterator fileMapLookup( std::map< std::string, T>& fileMap, const std::string& filename )
{
	// Don't handle bad-case lookups for now
	std::string mapKey = cleanSlashes( filename );
#if ENABLE_FILE_CASE_CHECKING
	typename std::map< std::string, T >::iterator fileIt = fileMap.find( mapKey );
	if ( fileIt != fileMap.end() )
		return fileIt;
	// Check for a case-mismatch. We can't use FilenameCaseChecker here
	// because it relies on the Windows Shell interface to find the right file
	std::string mapKeyLower = mapKey;
	std::transform( mapKeyLower.begin(), mapKeyLower.end(), mapKeyLower.begin(), ::tolower );
	for( fileIt = fileMap.begin(); fileIt != fileMap.end(); fileIt++ )
	{
		// Early out if we have a different filename length
		if ( mapKey.length() != fileIt->first.length() )
			continue;
		std::string candidateName = fileIt->first;
		std::transform( candidateName.begin(), candidateName.end(), candidateName.begin(), ::tolower );
		if ( mapKeyLower == candidateName )
		{
			// We found a match! Complain bitterly, but accept it.
			WARNING_MSG( "Case in filename %s does not match case in zip file %s\n", 
				mapKey.c_str(), fileIt->first.c_str() );
			return fileIt;
		}
	}
	return fileMap.end();
#else // ENABLE_FILE_CASE_CHECKING
	return fileMap.find( mapKey );
#endif // ENABLE_FILE_CASE_CHECKING
}


/**
 * This template function performs a slash-safe lookup of a filename
 * in the provided map. It assumes the slashes loaded into the map
 * are correct!
 *
 * In a non-consumer build, it will check for case mismatch and log it
 * as a warning. This will slow down missed lookups considerably.
 */
template< class T > inline typename std::map< std::string, T >::const_iterator fileMapLookupConst( const std::map< std::string, T>& fileMap, const std::string& filename )
{
	// Don't handle bad-case lookups for now
	std::string mapKey = cleanSlashes( filename );
#if ENABLE_FILE_CASE_CHECKING
	typename std::map< std::string, T >::const_iterator fileIt = fileMap.find( mapKey );
	if ( fileIt != fileMap.end() )
		return fileIt;
	std::string mapKeyLower = mapKey;
	std::transform( mapKeyLower.begin(), mapKeyLower.end(), mapKeyLower.begin(), ::tolower );
	for( fileIt = fileMap.begin(); fileIt != fileMap.end(); fileIt++ )
	{
		// Early out if we have a different filename length
		if ( mapKey.length() != fileIt->first.length() )
			continue;
		std::string candidateName = fileIt->first;
		std::transform( candidateName.begin(), candidateName.end(), candidateName.begin(), ::tolower );
		if ( mapKeyLower == candidateName )
		{
			// We found a match! Complain bitterly, but accept it.
			WARNING_MSG( "Case in filename %s does not match case in zip file %s\n", 
				mapKey.c_str(), fileIt->first.c_str() );
			return fileIt;
		}
	}
	return fileMap.end();
#else // ENABLE_FILE_CASE_CHECKING
	return fileMap.find( mapKey );
#endif // ENABLE_FILE_CASE_CHECKING
}


/**
 *	This enumeration contains constants related to Zip files.
 */
enum
{
	DIR_FOOTER_SIGNATURE 	= 0x06054b50,
	DIR_ENTRY_SIGNATURE		= 0x02014b50,
	LOCAL_HEADER_SIGNATURE	= 0x04034b50,
	DIGITAL_SIG_SIGNATURE	= 0x05054b50,

	MAX_FIELD_LENGTH		= 1024,

	METHOD_STORE			= 0,
	METHOD_DEFLATE			= 8,

	ZIP_HEADER_MAGIC		= 0x04034b50
};

/**
 *	This structure represents the footer at the end of a Zip file.
 *	Actually, a zip file may contain a comment after the footer,
 *	but for our purposes we can assume it will not.
 */
struct DirFooter
{
	uint32	signature PACKED;
	uint16	diskNumber PACKED;
	uint16	dirStartDisk PACKED;
	uint16	dirEntries PACKED;
	uint16	totalDirEntries PACKED;
	uint32	dirSize PACKED;
	uint32	dirOffset PACKED;
	uint16	commentLength PACKED;
};


/**
 *	Constructor.
 *
 *	@param zipFile		Path of the zip file to open
 */
ZipFileSystem::ZipFileSystem( const std::string& zipFile ) :
	pFile_( NULL ),
	path_( zipFile ),
	tag_( "" ),
	parentSystem_( NULL ),
	parentZip_( NULL ),
	offset_( 0 ),
	size_( 0 )
{
	SimpleMutexHolder mtx( mutex_ );
	FileHandleHolder fhh(this);
}


/**
 *	Constructor.
 *
 *	@param zipFile		Path of the zip file to open
 *	@param parentSystem	Parent system used for loading
 */
ZipFileSystem::ZipFileSystem( const std::string& zipFile,
								FileSystemPtr parentSystem ) :
	pFile_( NULL ),
	path_( zipFile ),
	tag_( "" ),
	parentSystem_( parentSystem ),
	parentZip_( NULL ),
	offset_( 0 ),
	size_( 0 )
{
	SimpleMutexHolder mtx( mutex_ );
	FileHandleHolder fhh(this);
}


/**
 *	Constructor.
 *
 *	Used for zip files nested in zip files.
 *
 *	@param zipFile		Path of the parent zip file to open
 *	@param parentZip	Parent zip system used for loading
 *	@param tag			name of the child zip file to load
 */
ZipFileSystem::ZipFileSystem( const std::string& zipFile,
							 ZipFileSystemPtr parentZip,
							 const std::string& tag ) :
	pFile_( NULL ),
	path_( zipFile ),
	tag_( tag ),
	parentSystem_( NULL ),
	parentZip_( parentZip ),
	offset_( 0 ),
	size_( 0 )
{
	if (parentZip->dirTest(tag))
		parentSystem_ = parentZip->parentSystem();
	else
	{
		// Get the offset for the internal file.
		parentZip->getFileOffset(tag, offset_, size_);
		parentSystem_ = parentZip->parentSystem();

		SimpleMutexHolder mtx( mutex_ );
		FileHandleHolder fhh(this);
	}
}


/**
 *	Constructor.
 *
 *	Empty zip system, doesnt load anything.
 *
 *	@param parentSystem	Parent system used for loading
 */
ZipFileSystem::ZipFileSystem( FileSystemPtr parentSystem ) :
	pFile_( NULL ),
	path_( "" ),
	tag_( "" ),
	parentSystem_( parentSystem ),
	parentZip_( NULL ),
	offset_( 0 ),
	size_( 0 )
{
}


/**
 *	This is the destructor.
 */
ZipFileSystem::~ZipFileSystem()
{
	SimpleMutexHolder mtx( mutex_ );
	this->closeZip();
}

/**
 *	Initialise some parameters for a nested zip file 
 *		(used in the conversion process)
 *
 *  @param zipFile  Name of the zip file.
 *	@param tag	 Name of the section to initialise.
 *	@param parentZip Parent zip file.
 */
void ZipFileSystem::init(	const std::string& zipFile,
							const std::string& tag,
							ZipFileSystemPtr parentZip )
{
	tag_ = tag;
	parentZip_ = parentZip;
	parentSystem_ = parentZip->parentSystem();

	if (zipFile != "")
	{
		path_ = zipFile;

		if (!parentZip->dirTest(tag))
		{
			// Get the offset for the internal file.
			parentZip->getFileOffset(tag, offset_, size_);	
			SimpleMutexHolder mtx( mutex_ );
			FileHandleHolder fhh(this);
		}
	}
}

ZipFileSystem::ZipFileSystem(  ) :
	pFile_( NULL ),
	path_( "" ),
	tag_( "" ),
	parentSystem_( NULL ),
	parentZip_( NULL ),
	offset_( 0 ),
	size_( 0 )	
{
}

/**
 *	Places the offset of the specified file into the passed references.
 *
 *	@param path	Name of the file
 *	@param fileOffset Output for the file offset.
 *	@param fileSize Output for the file size.
 */
void ZipFileSystem::getFileOffset(const std::string& path, uint32& fileOffset, uint32& fileSize)
{
	FileMap::iterator it = fileMapLookup( fileMap_, path );
	if (it == fileMap_.end())
		return;

	LocalFile& file = centralDirectory_[ it->second.first ];
	fileOffset = offset_ + file.dataOffset();
	fileSize = file.compressedSize();
}

/**
 *	Clear the specified directory.
 *
 *	@param dirPath	Name of the dir to clear
 */
void ZipFileSystem::clear(const std::string& dirPath)
{
	if (internalChildNode())
		return parentZip_->clear( parentZip_->tag() + "/" + dirPath );

	std::string path2 = dirPath;
	resolvePath(path2);
	if(pFile_)
	{
		fclose(pFile_);
		pFile_ = NULL;
	}
	Directory& dir = dirMap_[ path2 ].first;
	Directory::iterator it = dir.begin();		
	for (; it != dir.end(); it++)
	{
		std::string filename;
		if (path2 == "")
			filename = (*it);
		else
			filename = path2 + "/" + (*it);

		FileMap::iterator deleteMe = fileMapLookup( fileMap_, filename );
		MF_ASSERT( deleteMe != fileMap_.end() );
		if (deleteMe == fileMap_.end())
			continue;
		
		uint32 fileIndex = deleteMe->second.first;
		fileMap_.erase( deleteMe );
		centralDirectory_[ fileIndex ].clear();

		// Check if it's an encoded duplicate name..
		if (checkDuplicate(filename)) 
		{
			std::string decoded = decodeDuplicate(filename);

			FileDuplicatesMap::iterator it = fileMapLookup( duplicates_, decoded );
			if ( it != duplicates_.end() )
				duplicates_.erase( it );
		}
	}


	dirMap_[ path2 ].first.clear();
	dirMap_[ path2 ].second.clear();
}

/**
 *	This method opens a zipfile, and reads the directory.
 *	This will also populate the local directory for the zip.
 *
 *	@param path		Path of the zipfile.
 *
 *	@return True if successful.
 */
bool ZipFileSystem::openZip(const std::string& path)
{
	std::string dirComponent, fileComponent;
	DirFooter footer;
	DirPair blankDir;
	char buf[MAX_FIELD_LENGTH];	
	unsigned long i;
	int pos;

	if(pFile_)
	{
		fclose(pFile_);
		pFile_ = NULL;
	}

	if (parentSystem_)
	{
		if(!(pFile_ = parentSystem_->posixFileOpen(path, "rb")))
		{
			return false;
		}
	}
	else
	{		
		if(!(pFile_ = bw_fopen(conformSlash(path).c_str(), "rb")))
		{
			return false;
		}
	}

	if (centralDirectory_.size())
		return true;

	DiaryScribe ds( Diary::instance(), "zopen " + path_ );

	// Add a directory node for the root directory.
	dirMap_[""] = blankDir;

	// Check if this is the original zip (parent)
	if (offset_==0)
	{
		fseek(pFile_, 0, SEEK_END);
		size_ = ftell( pFile_ );
	}

	if(fseek(pFile_, offset_ + size_-(int)sizeof(footer), SEEK_SET) != 0)
	{
		ERROR_MSG("ZipFileSystem::openZip Failed to seek to footer (opening %s)\n",
			path.c_str());
		this->closeZip();
		return false;
	}

	if(fread(&footer, sizeof(footer), 1, pFile_) != 1)
	{
		ERROR_MSG("ZipFileSystem::openZip Failed to read footer (opening %s)\n",
			path.c_str());
		this->closeZip();
		return false;
	}

	if(footer.signature != DIR_FOOTER_SIGNATURE)
	{
		ERROR_MSG("ZipFileSystem::openZip Invalid footer signature (opening %s)\n",
			path.c_str());
		this->closeZip();
		return false;
	}

	if(fseek(pFile_, offset_ + footer.dirOffset, SEEK_SET) != 0)
	{
		ERROR_MSG("ZipFileSystem::openZip Failed to seek to directory start (opening %s)\n",
			path.c_str());
		this->closeZip();
		return false;
	}

	// Initialise the directory.
	uint32 dirOffset = 0;
	BinaryPtr dirBlock = NULL;
	if (footer.dirSize)
	{
		centralDirectory_.resize(footer.totalDirEntries);
		dirBlock = new BinaryBlock( NULL,
										footer.dirSize, 
										"BinaryBlock/ZipDirectory");

		if(fread(dirBlock->cdata(), dirBlock->len(), 1, pFile_) != 1)
		{
			ERROR_MSG("ZipFileSystem::openZip Failed to read directory block (opening %s)\n",
				path.c_str());
			this->closeZip();
			return false;
		}
	}
	else
		return true; //just an empty zip

	for(i = 0; i < footer.totalDirEntries; i++)
	{
		LocalFile& localFile = centralDirectory_[i];
		DirEntry& entry = localFile.entry_;

		memcpy( &entry, dirBlock->cdata() + dirOffset, sizeof(entry) );
		dirOffset += sizeof(entry);
		
		if(entry.signature != DIR_ENTRY_SIGNATURE)
		{
			ERROR_MSG("ZipFileSystem::openZip Invalid directory signature (opening %s)\n",
				path.c_str());
			this->closeZip();
			return false;
		}

		if(entry.filenameLength > MAX_FIELD_LENGTH)
		{
			ERROR_MSG("ZipFileSystem::openZip Filename is too long (opening %s)\n",
				path.c_str());
			this->closeZip();
			return false;
		}

		if(entry.extraFieldLength > MAX_FIELD_LENGTH)
		{
			ERROR_MSG("ZipFileSystem::openZip Extra field is too long (opening %s)\n",
				path.c_str());
			this->closeZip();
			return false;
		}

		if(entry.fileCommentLength > MAX_FIELD_LENGTH)
		{
			ERROR_MSG("ZipFileSystem::openZip File comment is too long (opening %s)\n",
				path.c_str());
			this->closeZip();
			return false;
		}

		std::string& filename = localFile.filename_;
		memcpy( buf, dirBlock->cdata() + dirOffset, entry.filenameLength );
		dirOffset += entry.filenameLength;
		
		filename.assign(buf, entry.filenameLength);
		filename = cleanSlashes( filename );

		dirOffset += entry.extraFieldLength + entry.fileCommentLength;

		if(filename[filename.length() - 1] == '/')
		{
			filename.erase(filename.length() - 1);
			dirMap_[ filename ] = blankDir;
		}

		//if the file/folder name is not ascii, then check if it's in utf-8,
		//if it's not in utf-8, then it will not be displayed correctly.
		if (entry.filenameLength > 0)
		{
			//only check the leaf files and the leaf folders,
			//so do it backward.
			bool nonAscii = false;
			char* start = &(buf[0]);
			char* end = &(buf[entry.filenameLength-1]);
			if (*end == '/')
				end--;
			
			char* cur = end;
			while (cur >= start &&
				*cur != '/')//stop when first hit a '/' because we only want to check the leaves
			{
				if ((*cur & 0x80) !=0)//ascii code range [0,127]
				{
					nonAscii = true;
					break;
				}
				cur--;
			}

			//If general purpose(entry.mask) bit 11 is set, the filename and comment must support The Unicode Standard
			if (nonAscii && ((entry.mask & 0x800) == 0))//not ascii and not utf-8
			{				
				WARNING_MSG("ZipFileSystem::openZip the file name (%s) is neither ASCII nor UTF-8,"
					"it may not be handled correctly\n",filename.c_str());

			}
		}




		// Check if it's an encoded duplicate name..
		if (checkDuplicate(filename)) 
		{
			std::string decoded = decodeDuplicate(filename);

			// keep the name.. and add the decoded version to the duplicates list.
			if ( fileMapLookup( duplicates_, decoded ) == duplicates_.end() )
				duplicates_[decoded] = 1;
			else
				duplicates_[decoded]++;

		}
		// create the filename->directory index mapping
		fileMap_[ filename ].first = i;

		pos = filename.rfind('/');
		if(pos == -1)
		{
			dirComponent = "";
			fileComponent = filename;
		}
		else
		{
			dirComponent = filename.substr(0, pos);
			fileComponent = filename.substr(pos + 1);
		}

		DirMap::iterator dirIter = fileMapLookup( dirMap_, dirComponent );
		if(dirIter == dirMap_.end())
		{
			dirMap_[ dirComponent ] = blankDir;
			DirPair& pair = dirMap_[ dirComponent ];

			//TODO: should this be stored on the full path not the file component?
			if (checkDuplicate(fileComponent))
			{
				pair.first.push_back(fileComponent);
				pair.second.push_back(decodeDuplicate(fileComponent));
			}
			else
			{
				pair.first.push_back(fileComponent);
				pair.second.push_back(fileComponent);
			}

			fileMap_[  filename ].second = 0;
			

			if (fileMapLookup( fileMap_, dirComponent ) == fileMap_.end())
			{
				updateFile( ( dirComponent ) , NULL );
			}
		}
		else
		{
			// add the directory index
			fileMap_[ filename ].second = dirIter->second.first.size();

			if (checkDuplicate(fileComponent))
			{
				dirIter->second.first.push_back(fileComponent);
				dirIter->second.second.push_back(decodeDuplicate(fileComponent));
			}
			else
			{
				dirIter->second.first.push_back(fileComponent);
				dirIter->second.second.push_back(fileComponent);
			}
		}
	}


	return true;
}


/**
 *	This method determines if the input string is an encoded duplicate name.
 *	It is assumed that the file names (section names) will not contain spaces
 *	so that is used as the determining factor.
 *
 *	@param filename Name to test.
 *
 *	@return true if the name is a duplicate.
 */
bool ZipFileSystem::checkDuplicate(const std::string& filename)
{
	return ( filename.find_first_of(' ') != filename.npos );
}


/**
 *	This method decodes a file name, returning the original duplicate name..
 *
 *	@param filename The name to decode.
 *
 *	@return The decoded string.
 */
std::string ZipFileSystem::decodeDuplicate(const std::string& filename)
{
	int pos = filename.find_first_of(' ');
	return filename.substr(0, pos);
}


/**
 *	This method encodes a file name.
 *
 *	@param filename The name to encode.
 *	@param count The duplicate count.
 *
 *	@return The encoded string.
 */
std::string ZipFileSystem::encodeDuplicate(const std::string& filename, int count) const
{
	std::string encoded = filename;
	const int B_LEN = 256;	
	char buff[B_LEN];
	int len = bw_snprintf( buff, B_LEN, " %d", count );
	encoded.append( &buff[0], len );
	return encoded;
}

/**
 *	Builds a zip file and returns it as a binary block.
 *
 *	[local file header 1]
 *	[file data 1]
 *	. 
 *	.
 *	.
 *	[local file header n]
 *	[file data n]    
 *	[central directory]
 *	[end of central directory record]
 *
 *	@return The file structure in a binary block.
 */
BinaryPtr ZipFileSystem::createZip()
{
	if (childNode())
		return NULL;

	CentralDir::iterator it;
	uint32 offset = 0;

	// Determine the file size.
	uint32 totalFileSize = sizeof(DirFooter);
	it=centralDirectory_.begin();
	while (it!=centralDirectory_.end())
	{
		if ((it)->isValid())
		{
			totalFileSize += (it)->sizeOnDisk();
			++it;
		}
		else //TODO: note that modifying the central dir means the indices in file map are wrong...
			it = centralDirectory_.erase(it); // remove invalid/unitialised files
	}	
	BinaryPtr pBinary = new BinaryBlock( NULL,
										totalFileSize, 
										"BinaryBlock/ZipCreate");

	// Write the files
	for (it=centralDirectory_.begin(); it!=centralDirectory_.end(); it++)
	{
		(it)->writeFile(pBinary,offset);
	}

	MF_ASSERT( offset < (uint32)pBinary->len() );

	// Build the directory footer.
	DirFooter footer;	memset(&footer, 0, sizeof(DirFooter));	
	footer.signature = DIR_FOOTER_SIGNATURE;
	footer.dirOffset = offset;
	footer.dirEntries = centralDirectory_.size();
	footer.totalDirEntries = centralDirectory_.size();
	footer.commentLength = 0;

	// Write the central directory
	for (it=centralDirectory_.begin(); it!=centralDirectory_.end(); it++)
	{
		(it)->writeDirEntry(pBinary,offset);
	}
	
	MF_ASSERT( offset < (uint32)pBinary->len() );

	// Write directory footer
	footer.dirSize = (offset - footer.dirOffset);
	memcpy(pBinary->cdata()+offset, &footer, sizeof(footer));
	offset += sizeof(footer);

	MF_ASSERT( offset == (uint32)pBinary->len() );

	return pBinary;
}


/**
 *	This method returns the file name associated with the given index.
 *
 *	@param dirPath Directory to index into.
 *	@param index File index.
 *
 *	@return The file name (decoded)
 */
std::string ZipFileSystem::fileName( const std::string& dirPath, int index )
{
	if (internalChildNode())
		return parentZip_->fileName( parentZip_->tag() + "/" + dirPath, index );

	std::string path2 = dirPath;
	resolvePath(path2);
	MF_ASSERT_DEBUG( fileMapLookup( dirMap_, path2 ) != dirMap_.end() );
	return dirMap_[ path2 ].second[ index ];
}


/**
 *	This method returns the index associated with a file name.
 *	If the file has duplicate names, it will get the first entry available.
 *
 *	@param name Name of the file.
 *
 *	@return The index associated with the name, -1 if it doesnt exist.
 */
int ZipFileSystem::fileIndex( const std::string& name )
{
	if (internalChildNode())
		return parentZip_->fileIndex(tag_ + "/" + name);

	std::string path = name;
	resolveDuplicate(path);

	FileMap::iterator it = fileMapLookup( fileMap_, path );

	if (it != fileMap_.end())
		return it->second.second;
	else
		return -1;
}


bool ZipFileSystem::resolveDuplicate( std::string& path ) const
{
	FileDuplicatesMap::const_iterator dupe = fileMapLookupConst( duplicates_, path );
	if (dupe != duplicates_.end())
	{
		path = encodeDuplicate(path, 1);
		return true;
	}

	return false;
}

void ZipFileSystem::resolvePath( std::string& path, bool bExtraChecks ) const
{
	DiaryScribe ds( Diary::instance(), "zip_resolvePath " + path );
	if (path_ == "")
		return;	

	path = cleanSlashes( path );

	std::string dirPath = cleanSlashes( path_ );
	//TODO: review this special case.
	if (path == tag_)
	{
		path = "";
		return;
	}

	if (bExtraChecks)
	{
		size_t off = path.find( dirPath );
		if (off != std::string::npos)
		{
			path = path.substr(MF_MIN(off + dirPath.size()+1, path.size()), path.size());
		}
		if (path.size()>1 && path[path.size()-1] == '.')
			path = path.erase(path.size()-2,path.size());
	}
}

/**
 *	This method reads the file via the file index.
 *
 *	@param dirPath	Path of the file to read.
 *  @param index	The array index of the file to read in the dirPath list.
 *
 *	@return The file data.
 */
BinaryPtr ZipFileSystem::readFile(const std::string & dirPath, uint index)
{
	if (internalChildNode())
		return parentZip_->readFile( parentZip_->tag() + "/" + dirPath, index );

	BWResource::checkAccessFromCallingThread( dirPath, "ZipFileSystem::readFile" );
	SimpleMutexHolder mtx( mutex_ );

	std::string path2 = dirPath;
	resolvePath(path2);
	DirMap::iterator it = fileMapLookup( dirMap_, path2 );

	if(it != dirMap_.end() && index < it->second.first.size())
	{
		if (path2 == "")
			return readFileInternal( it->second.first[index] );
		else
			return readFileInternal( path2 + "/" + it->second.first[index] );
	}
	else
		return NULL;
}


/**
 *	This method reads the contents of a file through readFileInternal.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr ZipFileSystem::readFile(const std::string& path)
{
	if (internalChildNode())
		return parentZip_->readFile(tag_ + "/" + path);

	BWResource::checkAccessFromCallingThread( path, "ZipFileSystem::readFile" );

	SimpleMutexHolder mtx( mutex_ );
	std::string path2 = path;
	resolvePath(path2);
	return readFileInternal( path2 );
}


/**
 *	This method reads the contents of a file. It is not thread safe.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A BinaryBlock object containing the file data.
 */
BinaryPtr ZipFileSystem::readFileInternal(const std::string& inPath)
{
	char *pCompressed = NULL, *pUncompressed = NULL;
	std::string path = inPath;
	resolveDuplicate(path);
	FileMap::iterator it = fileMapLookup( fileMap_, path );
	BinaryPtr pBinaryBlock;
	int r;

	if (it == fileMap_.end())
		return static_cast<BinaryBlock *>( NULL );

	LocalFile& file = centralDirectory_[it->second.first];
	LocalHeader& hdr = file.header_;
	if (file.entry_.localHeaderOffset < 0) 
	{	//doesnt exist in the file (dummy entry for directory struct)
		return static_cast<BinaryBlock *>( NULL );
	}

	if (file.pData_)
		return file.pData_;

	FileHandleHolder fhh(this);
	if (!fhh.isValid())
	{
		ERROR_MSG("ZipFileSystem::readFile Failed open zip file (%s)\n",
			path.c_str());
		return static_cast<BinaryBlock *>( NULL );
	}

	DiaryScribe ds( Diary::instance(), "zread " + path_ );

	if(fseek(pFile_, offset_ + file.entry_.localHeaderOffset + sizeof(hdr) +
			 	file.entry_.extraFieldLength + file.entry_.filenameLength, SEEK_SET) != 0)
	{
		ERROR_MSG("ZipFileSystem::readFile Failed to seek past local header (%s in %s)\n",
			path.c_str(), path_.c_str());
		return static_cast<BinaryBlock *>( NULL );
	}
	if(file.entry_.compressionMethod != METHOD_STORE &&
		file.entry_.compressionMethod != METHOD_DEFLATE)
	{
		ERROR_MSG("ZipFileSystem::readFile Compression method %d not yet supported (%s in %s)\n",
			file.entry_.compressionMethod, path.c_str(), path_.c_str());
		return static_cast<BinaryBlock *>( NULL );
	}

	BinaryPtr pCompressedBin = new BinaryBlock( NULL, file.entry_.compressedSize, "BinaryBlock/ZipRead" );
	pCompressed = (char*)pCompressedBin->data();
	if(!pCompressed)
	{
		ERROR_MSG("ZipFileSystem::readFile Failed to alloc data buffer (%s in %s)\n",
			path.c_str(), path_.c_str());
		return static_cast<BinaryBlock *>( NULL );
	}

	if(fread(pCompressed, 1, file.entry_.compressedSize, pFile_) != file.entry_.compressedSize)
	{
		ERROR_MSG("ZipFileSystem::readFile Data read error (%s in %s)\n",
			path.c_str(), path_.c_str());
		return static_cast<BinaryBlock *>( NULL );
	}

	if(file.entry_.compressionMethod == METHOD_DEFLATE)
	{
		unsigned long uncompressedSize = file.entry_.uncompressedSize;
		z_stream zs;
		memset( &zs, 0, sizeof(zs) );

		BinaryPtr pUncompressedBin = new BinaryBlock( NULL, file.entry_.uncompressedSize, "BinaryBlock/ZipRead" );
		pUncompressed = (char*)pUncompressedBin->data();
		if(!pUncompressed)
		{
			ERROR_MSG("ZipFileSystem::readFile Failed to alloc data buffer (%s in %s)\n",
				path.c_str(), path_.c_str());
			return static_cast<BinaryBlock *>( NULL );
		}

		zs.zalloc = NULL;
		zs.zfree = NULL;

		// Note that we dont use the uncompress wrapper function in zlib,
		// because we need to pass in -MAX_WBITS to inflateInit2 as the
		// window size. This disables the zlib header, see zlib.h for
		// details. This is what we want, since zip files don't contain
		// zlib headers.

		if(inflateInit2(&zs, -MAX_WBITS) != Z_OK)
		{
			ERROR_MSG("ZipFileSystem::readFile inflateInit2 failed (%s in %s)\n",
				path.c_str(), path_.c_str());
			return static_cast<BinaryBlock *>( NULL );
		}

		zs.next_in = (unsigned char *)pCompressed;
		zs.avail_in = file.entry_.compressedSize;
		zs.next_out = (unsigned char *)pUncompressed;
		zs.avail_out = uncompressedSize;

		if((r = inflate(&zs, Z_FINISH)) != Z_STREAM_END)
		{
			ERROR_MSG("ZipFileSystem::readFile Decompression error %d (%s in %s)\n", r,
				path.c_str(), path_.c_str());
			inflateEnd(&zs);
			return static_cast<BinaryBlock *>( NULL );
		}

		inflateEnd(&zs);
		pBinaryBlock = pUncompressedBin;
	}
	else
	{
		pBinaryBlock = pCompressedBin;
	}

	if(pFile_)
	{
#ifdef EDITOR_ENABLED
		fclose(pFile_);
		pFile_ = NULL;
#endif
	}

	return pBinaryBlock;
}


/**
 *	This method closes the zip file and frees all resources associated
 *	with it.
 */
void ZipFileSystem::closeZip()
{
	if(pFile_)
	{
		fclose(pFile_);
		pFile_ = NULL;
	}

	duplicates_.clear();

	fileMap_.clear();
	dirMap_.clear();

	centralDirectory_.clear();
}


/**
 *	This method reads the contents of a directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return A vector containing all the filenames in the directory.
 */
IFileSystem::Directory ZipFileSystem::readDirectory(const std::string& path)
{
	if (internalChildNode())
		return parentZip_->readDirectory(tag_ + "/" + path);

	BWResource::checkAccessFromCallingThread( path, "ZipFileSystem::readDirectory" );

	SimpleMutexHolder mtx( mutex_ );
	std::string path2 = path;
	resolvePath(path2);
	DirMap::iterator it = fileMapLookup( dirMap_, path2 );

	if(it == dirMap_.end())
	{
		Directory blankDir;
		return blankDir;
	}
	return it->second.second;
}


/**
 *	This method gets the file type, and optionally additional info.
 *
 *	@param path		Absolute path.
 *	@param pFI		If not NULL, this is set to the type of this file.
 *
 *	@return File type enum.
 */
IFileSystem::FileType ZipFileSystem::getFileType(const std::string& path,
	FileInfo * pFI ) const
{
	if (internalChildNode())
		return parentZip_->getFileType(tag_ + "/" + path, pFI);

	BWResource::checkAccessFromCallingThread( path, "ZipFileSystem::getFileType" );

	SimpleMutexHolder mtx( mutex_ );
	std::string path2 = path;
	resolvePath(path2,true);
	FileMap::const_iterator ffound = fileMapLookupConst( fileMap_, path2 );
	if (ffound == fileMap_.end())
		return FT_NOT_FOUND;

	const LocalFile& file = centralDirectory_[ffound->second.first];
	const DirEntry& hdr = file.entry_;

	FileType ft = (fileMapLookupConst( dirMap_, path2 ) != dirMap_.end()) ?
		FT_DIRECTORY : FT_FILE;

	if (pFI != NULL)
	{
		pFI->size = hdr.uncompressedSize;
#ifdef _WIN32
		FILETIME _ft;
		DosDateTimeToFileTime(hdr.modifiedDate,hdr.modifiedTime,&_ft);
		uint64 alltime = uint64(_ft.dwHighDateTime) << 32 | uint64(_ft.dwLowDateTime);
#else
		uint32 alltime =
			uint32(hdr.modifiedDate) << 16 | uint32(hdr.modifiedTime);
#endif //_WIN32

		pFI->created = alltime;
		pFI->modified = alltime;
		pFI->accessed = alltime;
		// TODO: Look for NTFS/Unix extra fields
	}
	return ft;

}

IFileSystem::FileType ZipFileSystem::getFileTypeEx(const std::string& path,
	FileInfo * pFI )
{
	if (internalChildNode())
		return parentZip_->getFileTypeEx(tag_ + "/" + path, pFI);

	BWResource::checkAccessFromCallingThread( path, "ZipFileSystem::getFileTypeEx" );

	SimpleMutexHolder mtx( mutex_ );

	DiaryScribe ds( Diary::instance(), "zip_ftypeEx " + path_ );

	std::string path2 = path;
	resolvePath(path2,true);

	FileMap::iterator ffound;

	resolveDuplicate(path2);
	ffound = fileMapLookup( fileMap_, path2 );

	if (ffound == fileMap_.end())
		return FT_NOT_FOUND;

	LocalFile& file = centralDirectory_[ffound->second.first];
	DirEntry& hdr = file.entry_;

	FileType ft = FT_FILE;
	if (fileMapLookup( dirMap_, path2 ) != dirMap_.end())
		return FT_DIRECTORY;	

	if (pFI != NULL)
	{
		pFI->size = hdr.uncompressedSize;
		uint32 alltime =
			uint32(hdr.modifiedDate) << 16 | uint32(hdr.modifiedTime);
		pFI->created = alltime;
		pFI->modified = alltime;
		pFI->accessed = alltime;
		// TODO: Look for NTFS/Unix extra fields
	}

	
	// already loaded.
	if (file.pData_)
	{
		if ( hdr.compressionMethod == METHOD_DEFLATE )
		{
			uint32 magic=0;
			int result;
			z_stream zs;
			memset( &zs, 0, sizeof( zs ) );
			zs.zalloc = NULL;
			zs.zfree = NULL;
			if ( inflateInit2( &zs, -MAX_WBITS ) != Z_OK )
			{
				ERROR_MSG( "ZipFileSystem::getFileTypeEx: inflateInit2 failed (%s in %s)\n",
					path.c_str(), path_.c_str() );
				return FT_FILE; //return no file instead?
			}
			zs.next_in = reinterpret_cast< Bytef* >( file.pData_->cdata() );
			zs.avail_in = file.pData_->len();
			zs.next_out = reinterpret_cast< Bytef* >( &magic );
			zs.avail_out = sizeof( magic );
			while ( ( result = inflate( &zs, Z_SYNC_FLUSH ) ) == Z_OK && zs.avail_out != 0 )
				;// Empty while
			inflateEnd( &zs );
			if ( result != Z_OK )
			{
				ERROR_MSG( "ZipFileSystem::getFileTypeEx: Decompression error %d (%s in %s)\n",
					result, path.c_str(), path_.c_str() );
				return FT_FILE; //return no file instead?
			}
			if (magic == ZIP_HEADER_MAGIC)
				return FT_ARCHIVE;
		} else {
			// Uncompressed
			if (zipTest(file.pData_))
				return FT_ARCHIVE;
		}
	}
	else
	{
		uint32 magic=0;
		FileHandleHolder fhh(this);
		if (!fhh.isValid())
		{
			ERROR_MSG("ZipFileSystem::getFileType Failed open zip file (%s)\n",
				path.c_str());
			return FT_NOT_FOUND;
		}

		// Check if it's a zip file.
		fseek(pFile_, offset_ + file.dataOffset() , SEEK_SET);

		if (hdr.uncompressedSize == 0)
		{
			return FT_FILE;
		}
		else if ( hdr.compressionMethod == METHOD_DEFLATE )
		{
			int result;
			z_stream zs;
			memset( &zs, 0, sizeof( zs ) );
			zs.zalloc = NULL;
			zs.zfree = NULL;
			if ( inflateInit2( &zs, -MAX_WBITS ) != Z_OK )
			{
				ERROR_MSG( "ZipFileSystem::getFileTypeEx: inflateInit2 failed (%s in %s)\n",
					path.c_str(), path_.c_str() );
				return FT_FILE; //return no file instead?
			}
			uint16 input;
			zs.next_in = reinterpret_cast< Bytef* >( &input );
			zs.avail_in = sizeof( input );
			zs.next_out = reinterpret_cast< Bytef* >( &magic );
			zs.avail_out = sizeof( magic );
			int readRecords;
			//read elements until we get the first deflated uint32 (the header)
			while ( ( readRecords = fread( &input, sizeof(input), 1, pFile_ ) ) == 1
				&& ( result = inflate( &zs, Z_SYNC_FLUSH ) ) == Z_OK
				&& zs.avail_out != 0 )
			{
				if ( readRecords != 1 )
				{
					inflateEnd( &zs );
					return FT_FILE; //return no file instead?
				}
				if ( result != Z_OK )
				{
					inflateEnd( &zs );
					ERROR_MSG( "ZipFileSystem::getFileTypeEx: Decompression error %d (%s in %s)\n",
						result, path.c_str(), path_.c_str() );
					return FT_FILE; //return no file instead?

				}
				// We decompressed successfully, but still have not gotten enough data
				MF_ASSERT( zs.total_out < sizeof( magic ) );
				// Inflate is happily advancing and reducing the pointers and length.
				// That's good for the output, but we're reading into the same input again.
				zs.next_in = reinterpret_cast< Bytef* >( &input );
				zs.avail_in = sizeof( input );
			}
			inflateEnd( &zs );
		} else {
			int frr = 1;
			frr = fread( &magic, sizeof(magic), 1, pFile_ );
			if (!frr)
				return FT_FILE; //return no file instead?
		}

		
		if (magic == ZIP_HEADER_MAGIC)
		    {
				return FT_ARCHIVE;
		    }
	}
	return ft;
}

extern const char * g_daysOfWeek[];
extern const char * g_monthsOfYear[];


/**
 *	Translate an event time from a zip file to a string
 */
std::string	ZipFileSystem::eventTimeToString( uint64 eventTime )
{
	uint16 zDate = uint16(eventTime>>16);
	uint16 zTime = uint16(eventTime);

	char buf[32];
	bw_snprintf( buf, sizeof(buf), "%s %s %02d %02d:%02d:%02d %d",
		"Unk", g_monthsOfYear[(zDate>>5)&15], zDate&31,
		zTime>>11, (zTime>>5)&15, (zTime&31)*2, 1980 + (zDate>>9) );
	return buf;
}


/**
 *	This method creates a new directory.
 *
 *	@param path		Path relative to the base of the filesystem.
 *
 *	@return True if successful
 */
bool ZipFileSystem::makeDirectory(const std::string& /*path*/)
{
	return false;
}


/**
 *	This method returns the size of this file on disk. Includes the directory
 *	entry size
 *
 *
 *	@return The file size
 */
uint32 ZipFileSystem::LocalFile::sizeOnDisk()
{
	return	sizeof(LocalHeader) 
			+ filename_.length()*2 
			+ pData_->len() 
			+ sizeof(DirEntry);
}


/**
 *	This method writes the file data to the a file stream.
 *
 *	@param pFile File stream to write to.
 *	@param offset Output for the accumulation of the file offset.
 *
 *	@return true if sucessful
 */
bool ZipFileSystem::LocalFile::writeFile( FILE* pFile, uint32& offset )
{
	localOffset_ = offset;
	offset += fwrite(&header_, 1, sizeof(header_), pFile);
	offset += fwrite(filename_.c_str(), 1, header_.filenameLength, pFile);
	offset += fwrite(pData_->data(), 1, pData_->len(), pFile);
	return true;
}


/**
 *	This method writes the file data to the a binary block.
 *
 *	@param dst Destination binary block.
 *	@param offset Output for the accumulation of the buffer offset.
 *
 *	@return true if sucessful
 */
bool ZipFileSystem::LocalFile::writeFile( BinaryPtr dst, uint32& offset )
{
	localOffset_ = offset;

	memcpy((dst->cdata() + offset), &header_, sizeof(header_));
	offset += sizeof(header_);

	memcpy((dst->cdata() + offset), filename_.c_str(), header_.filenameLength);
	offset += header_.filenameLength;

	memcpy((dst->cdata() + offset), pData_->data(), pData_->len());
	offset += pData_->len();

	return true;
}


/**
 *	This method writes the file directory entry to the a binary block.
 *
 *	@param dst Destination binary block.
 *	@param offset Output for the accumulation of the buffer offset.
 *
 *	@return true if sucessful
 */
bool ZipFileSystem::LocalFile::writeDirEntry( BinaryPtr dst, uint32& offset )
{
	memset(&entry_, 0, sizeof(DirEntry));
	entry_.signature = DIR_ENTRY_SIGNATURE;
	entry_.filenameLength = filename_.length(); //max = MAX_FIELD_LENGTH
	entry_.extraFieldLength = 0;
	entry_.fileCommentLength = 0;
	entry_.localHeaderOffset = localOffset_;
	entry_.extractorVersion = header_.extractorVersion;
	entry_.creatorVersion = header_.extractorVersion;
	entry_.modifiedDate = header_.modifiedDate;
	entry_.modifiedTime  = header_.modifiedTime;
	entry_.uncompressedSize = header_.uncompressedSize;
	entry_.compressedSize = header_.compressedSize;
	entry_.compressionMethod = header_.compressionMethod;
	entry_.crc32 = header_.crc32;

	memcpy((dst->cdata() + offset), &entry_, sizeof(entry_));
	offset += sizeof(entry_);
	memcpy((dst->cdata() + offset), filename_.c_str(), entry_.filenameLength);
	offset += entry_.filenameLength;

	return true;
}


/**
 *	This method writes the file directory entry to the a file stream.
 *
 *	@param pFile File stream to write to.
 *	@param offset Output for the accumulation of the file offset.
 *
 *	@return true if sucessful
 */
bool ZipFileSystem::LocalFile::writeDirEntry( FILE* pFile, uint32& offset )
{
	memset(&entry_, 0, sizeof(DirEntry));
	entry_.signature = DIR_ENTRY_SIGNATURE;
	entry_.filenameLength = filename_.length(); //max = MAX_FIELD_LENGTH
	entry_.extraFieldLength = 0;
	entry_.fileCommentLength = 0;
	entry_.localHeaderOffset = localOffset_;
	entry_.extractorVersion = header_.extractorVersion;
	entry_.creatorVersion = header_.extractorVersion;
	entry_.modifiedDate = header_.modifiedDate;
	entry_.modifiedTime  = header_.modifiedTime;
//	entry_.externalFileAttr = 0x0;
	entry_.uncompressedSize = header_.uncompressedSize;
	entry_.compressedSize = header_.compressedSize;
	entry_.compressionMethod = header_.compressionMethod;
	entry_.crc32 = header_.crc32;

	offset += fwrite(&entry_, 1, sizeof(entry_), pFile);	
	offset += fwrite(filename_.c_str(), 1, entry_.filenameLength, pFile);
	return true;
}


/**
 *	This static method will compress the passed data block
 *
 *	TODO: best to move this into BinaryBlock similar to "compress"
 *			it just doesn't contain the zlib headers and is under finer control
 *
 */
/*static*/ BinaryPtr ZipFileSystem::LocalFile::compressData( const BinaryPtr pData )
{
	int r;
	z_stream zs;
	memset( &zs, 0, sizeof(zs) );

	BinaryPtr pCompressed = new BinaryBlock(NULL, pData->len()+1,
								"BinaryBlock/ZipUncompressedData");

	// Note that we dont use the compress wrapper function in zlib,
	// because we need to pass in -MAX_WBITS to deflateInit2 as the
	// window size. This disables the zlib header, see zlib.h for
	// details. This is what we want, since zip files don't contain
	// zlib headers.

	zs.next_in = (unsigned char *)pData->cdata();
	zs.avail_in = pData->len();
	zs.next_out = (unsigned char *)pCompressed->cdata();
	zs.avail_out = pCompressed->len();
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;

	int res = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, METHOD_DEFLATE,
								-MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	if (res != Z_OK)
	{
		ERROR_MSG("ZipFileSystem::LocalFile::compressData"
					"deflateInit2 failed %s\n",
					zs.msg ? zs.msg : "" );
		return static_cast<BinaryBlock *>( NULL );
	}	
	//	If the parameter flush is set to Z_FINISH, pending input is pro-
	//	cessed, pending output is flushed and deflate() returns with
	//	Z_STREAM_END if there was enough output space; if deflate() re-
	//	turns with Z_OK, this function must be called again with Z_FINISH
	//	and more output space (updated avail_out but no more input data,
	//	until it returns with Z_STREAM_END or an error. After deflate()
	//	has returned Z_STREAM_END, the only possible operations on the
	//	stream are deflateReset() or deflateEnd().
	r = deflate(&zs, Z_FINISH);
	if (r == Z_OK)
	{
		// compressed data is bigger than the uncompressed.... don't compress.
		deflateEnd(&zs);
		return static_cast<BinaryBlock *>( NULL );
	}
	else if(r != Z_STREAM_END)
	{
		ERROR_MSG("ZipFileSystem::writeFile Compression error %d \n", r );
		deflateEnd(&zs);
		return static_cast<BinaryBlock *>( NULL );
	}
	MF_ASSERT( zs.total_out <= (uint)pCompressed->len() );
	BinaryPtr pBlock = new BinaryBlock(pCompressed->cdata(), zs.total_out,
								"BinaryBlock/ZipCompressedData");
	(void)deflateEnd(&zs);
	return pBlock;
}


/**
 *	This method updates the local header for this file.
 *
 */
void ZipFileSystem::LocalFile::updateHeader( bool clear )
{
	if (clear)
		memset(&header_, 0, sizeof(LocalHeader));

	header_.signature = LOCAL_HEADER_SIGNATURE;
	header_.filenameLength = filename_.length();
	header_.extraFieldLength = 0;
	header_.extractorVersion = 10;
	
	if (!bCompressed_ && pData_)
	{
		header_.crc32 = crc32( header_.crc32, (unsigned char*)pData_->data(), pData_->len() );
		bCompressed_ = true;
		
		header_.uncompressedSize = pData_->len();
		BinaryPtr pCompressed = NULL;

		if (pData_->canZip() && !ZipFileSystem::zipTest( pData_ ))
		{
			pCompressed = LocalFile::compressData( pData_ );			
		}

		if (pCompressed)
		{
			header_.compressionMethod = METHOD_DEFLATE;
			pData_ = pCompressed;
		}
		else
			header_.compressionMethod = METHOD_STORE;

		header_.compressedSize = pData_->len();
	}

	// Fill the time stamps
	time_t tVal = time(NULL);
	tm* sys_T = localtime(&tVal);
	int day = sys_T->tm_mday;
	int month = sys_T->tm_mon+1;
	int year = 1900 + sys_T->tm_year;
	int sec = sys_T->tm_sec;
	int min = sys_T->tm_min;
	int hour = sys_T->tm_hour;
	if (year <= 1980) //lol
		year = 0;
	else
		year -= 1980;
	header_.modifiedDate = (uint16) (day + (month << 5) + (year << 9));
	header_.modifiedTime = (uint16) ((sec >> 1) + (min << 5) + (hour << 11));
}


/**
 *	This method updates the zip directory with the input file.
 *	Files with a duplicate name are encoded to avoid multiple file names 
 *	in the zip.
 *
 *	@param name File name.
 *	@param data Binary data for the file.
 *
 */
void ZipFileSystem::updateFile(const std::string& name, BinaryPtr data)
{
	std::string dirComponent, fileComponent;

	std::string realName = cleanSlashes( name );

	// Search the existing files.
	FileMap::iterator it = fileMapLookup( fileMap_, name );
	if (it == fileMap_.end()) // not found (add new)
	{
		// Check the duplicates map
		FileDuplicatesMap::iterator it2 = fileMapLookup( duplicates_, name );
		if (it2 != duplicates_.end())
		{
			// It's a duplicate, add it to the directory with an encoded name.
			duplicates_[name]++;
			realName = encodeDuplicate(realName, duplicates_[name]);
		}
		LocalFile file(realName, data);
		centralDirectory_.push_back(file);

		// Update the directory index mapping.
		fileMap_[ realName ].first = centralDirectory_.size()-1;
	}
	else // Found in the mapping already....register as a duplicate.
	{
		// copy the non-encoded name entry into a newly encoded entry..
		realName = encodeDuplicate(name, 1);
		// add an entry in the duplicates map for the non-encoded name.
		duplicates_[name] = 1;
		uint32 centralIndex = (it)->second.first;
		uint32 dirIndex = (it)->second.second;
		centralDirectory_[centralIndex].update(realName);

		std::string dir, fileName;
		int pos = realName.rfind('/');
		if(pos == -1)
		{
			dir = "";
			fileName = realName;
		}
		else
		{
			dir = realName.substr(0, pos);
			fileName = realName.substr(pos + 1);
		}
		dirMap_[ dir ].first[ dirIndex ] = fileName;

		// erase the map entry for the non-encoded name
		fileMap_.erase(it);
		fileMap_[ realName ].first = centralIndex;

		// Now for the actual file..
		realName = encodeDuplicate(name, 2);
		LocalFile file(realName, data);
		centralDirectory_.push_back(file);
		fileMap_[ realName ].first = centralDirectory_.size()-1;
		duplicates_[name]++;
	}
	DirPair blankDir;


	int pos = realName.rfind('/');
	if(pos == -1)
	{
		dirComponent = "";
		fileComponent = realName;
	}
	else
	{
		dirComponent = realName.substr(0, pos);
		fileComponent = realName.substr(pos + 1);
	}

	//update the directory listings
	DirMap::iterator dirIter = fileMapLookup( dirMap_, dirComponent );
	if (dirIter == dirMap_.end()) // no directory listing found
	{
		dirMap_[ dirComponent ] = blankDir;
		DirPair& pair = dirMap_[ dirComponent ];
		
		if (checkDuplicate(fileComponent))
		{
			pair.first.push_back(fileComponent);
			pair.second.push_back(decodeDuplicate(fileComponent));
		}
		else
		{
			pair.first.push_back(fileComponent);
			pair.second.push_back(fileComponent);
		}

		fileMap_[ realName ].second = 0;

		if (fileMapLookup( fileMap_, dirComponent ) == fileMap_.end())
		{
			if (dirComponent != "")
			{
				updateFile( ( dirComponent ) , NULL );
			}
		}
	}
	else
	{
		// add the directory index
		fileMap_[ realName ].second = dirIter->second.first.size();

		if (checkDuplicate(fileComponent))
		{
			dirIter->second.first.push_back(fileComponent);
			dirIter->second.second.push_back(decodeDuplicate(fileComponent));
		}
		else
		{
			dirIter->second.first.push_back(fileComponent);
			dirIter->second.second.push_back(fileComponent);
		}	
	}
}


/**
 *	This method writes a file into the zip structure. (not saved to disk)
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param pData	Data to write to the file
 *	@param binary	Write as a binary file (Windows only)
 *	@return	True if successful
 */
bool ZipFileSystem::writeFile(const std::string& path,
		BinaryPtr pData, bool binary)
{
	std::string filename=path;
	if (!pData)
		return true;
		
	resolvePath(filename);	

	// If this is a nested zip, pass along the write to the parent
	if (childNode())
		return parentZip_->writeFile( tag_ + "/" + path , pData, binary);

	// Add the file information to the directory
	updateFile(filename, pData);
	return true;
}


/**
 *	This method moves a file around.
 *
 *	@param oldPath		The path to move from.
 *	@param newPath		The path to move to.
 *
 *	@return	True if successful
 */
bool ZipFileSystem::moveFileOrDirectory( const std::string & oldPath,
	const std::string & newPath )
{
	return false;
}


/**
 *	This method erases a file or directory.
 *
 *	@param filePath		Path relative to the base of the filesystem.
 *	@return	True if successful
 */
bool ZipFileSystem::eraseFileOrDirectory( const std::string & filePath )
{
	std::string filename=filePath;
	resolvePath(filename);

	// If this is a nested zip, pass along the write to the parent
	if (internalChildNode())
		return parentZip_->eraseFileOrDirectory( tag_ + "/" + filename );

	FileDuplicatesMap::iterator dupe = fileMapLookup( duplicates_, filename );
	if (dupe != duplicates_.end())
	{
		filename = encodeDuplicate(filename, dupe->second);
		//remove from the duplicates
		if (dupe->second > 1)
			dupe->second = dupe->second - 1;
		else
			duplicates_.erase( dupe );
	}
	std::string fileComponent, dirComponent;
	int pos = filename.rfind('/');
	if(pos == -1)
	{
		dirComponent = "";
		fileComponent = filename;
	}
	else
	{
		dirComponent = filename.substr(0, pos);
		fileComponent = filename.substr(pos + 1);
	}

	// Remove this file from all the directory / file mappings
	FileMap::iterator deleteMe = fileMapLookup( fileMap_, filename );
	if (deleteMe == fileMap_.end())
		return false;
	uint32 fileIndex = deleteMe->second.first;
	uint32 dirIndex = deleteMe->second.second;
	DirPair& dirPair = dirMap_[ dirComponent ];

	dirPair.first.erase(dirPair.first.begin() + dirIndex);
	dirPair.second.erase(dirPair.second.begin() + dirIndex);

	fileMap_.erase( deleteMe );
	centralDirectory_[ fileIndex ].clear();

	if (dirPair.first.size() != dirIndex ) //not the last place in the list.. rebuild
	{
		// Run through this directory and update the indices
		Directory::iterator it = dirPair.first.begin();
		for (;it != dirPair.first.end(); it++)
		{
			std::string fullName;
			if (dirComponent != "")
				fullName = dirComponent + "/" + (*it);
			else
				fullName = (*it);
			FileMap::iterator file = fileMapLookup( fileMap_, fullName );
			// should always exist.. or there's a problem elsewhere
			MF_ASSERT(file != fileMap_.end());
			file->second.second = (uint32) (it - dirPair.first.begin());
		}
	}			


	return true;
}


/**
 *	This method opens a file using POSIX semantics. It first extracts the file
 *  from the Zip file, and then saves it to a temporary file
 *
 *	@param path		Path relative to the base of the filesystem.
 *	@param mode		The mode that the file should be opened in.
 *	@return	True if successful
 */
FILE * ZipFileSystem::posixFileOpen( const std::string& path,
		const char * mode )
{
	if (parentSystem_)
		return parentSystem_->posixFileOpen(path,mode);

	SimpleMutexHolder mtx( mutex_ );
#ifdef _WIN32

	wchar_t wbuf[MAX_PATH+1];
	wchar_t wfname[MAX_PATH+1];

	BinaryPtr bin = readFileInternal( path );
	if ( !bin )
	{
		return NULL;
	}

	GetTempPath( MAX_PATH, wbuf );
	GetTempFileName( wbuf, L"bwt", 0, wfname );

	FILE * pFile = _wfopen( wfname, L"w+bDT" );

	if (!pFile)
	{
		ERROR_MSG("ZipFileSystem::posixFileOpen: Unable to create temp file (%s in %s)\n",
			path.c_str(), path_.c_str());
		return NULL;
	}

	fwrite( bin->cdata(), 1, bin->len(), pFile );
	fseek( pFile, 0, SEEK_SET );
	return pFile;

#else // _WIN32

	BinaryPtr bin = readFileInternal( path );
	if ( !bin )
	{
		return NULL;
	}

	// TODO: Unix guys should use fmemopen instead here.
	FILE * pFile = tmpfile();

	if (!pFile)
	{
		ERROR_MSG("ZipFileSystem::posixFileOpen: Unable to create temp file (%s in %s)\n",
			path.c_str(), path_.c_str());
		return NULL;
	}

	fwrite( bin->cdata(), 1, bin->len(), pFile );
	fseek( pFile, 0, SEEK_SET );
	return pFile;

#endif // _WIN32
}


/**
 *	This method resolves the base dependent path to a fully qualified
 * 	path.
 *
 *	@param path			Path relative to the base of the filesystem.
 *
 *	@return	The fully qualified path.
 */
std::string	ZipFileSystem::getAbsolutePath( const std::string& path ) const
{
	// Absolute paths to a file inside a zip file is meaningless to almost any
	// application. Returning an empty string means that 
	// MultiFileSystem::getAbsolutePath() will skip over us and give an 
	// absolute path to a real file system.
	return std::string();
}


/**
 *	This method returns a IFileSystem pointer to a copy of this object.
 *
 *	@return	a copy of this object
 */
FileSystemPtr ZipFileSystem::clone()
{
	return new ZipFileSystem( path_ );
}


/**
 *	This method returns the size of the archive.
 *
 *	@return	the number of bytes this archive takes up.
 */
int ZipFileSystem::fileSize( const std::string & dirPath, uint index ) const
{
	if (internalChildNode())
		return parentZip_->fileSize( parentZip_->tag() + "/" + dirPath, index);

	std::string path2 = dirPath;
	resolvePath(path2);
	DirMap::const_iterator it = fileMapLookupConst( dirMap_, path2 );

	if (it != dirMap_.end() && index < it->second.first.size())
	{
		std::string fullName;
		if (path2 == "")
			fullName = it->second.first[index];
		else
			fullName = path2 + "/" + it->second.first[index];
		FileMap::const_iterator fileIt = fileMapLookupConst( fileMap_, fullName );
		if (fileIt != fileMap_.end())
		{
			const LocalFile& file = centralDirectory_[fileIt->second.first];
			BinaryPtr pDat = file.pData_;
			if (pDat) // modified file
			{
				return pDat->len();
			}
			else
			{
				return file.uncompressedSize();
			}
		}
	}
	return 0;
}

/**
 *	This method returns the size directory specified
 *
 *	@param dirPath Directory name
 *
 *	@return	the number of bytes this archive takes up.
 */
int	ZipFileSystem::fileCount( const std::string& dirPath ) const
{ 
	if (internalChildNode())
		return parentZip_->fileCount( parentZip_->tag() + "/" + dirPath );
	std::string path2;
	path2 = dirPath;		
	resolvePath(path2);
	DirMap::const_iterator dir = fileMapLookupConst( dirMap_, path2 );
	if (dir != dirMap_.end())
		return (int)dir->second.first.size();
	else		
		return 0;
}


/**
 *	This method tests a binary block to see if it is a zip file.
 *
 *	@param pBinary	Binary block to test.
 *
 *	@return	true if pBinary is a zip file.
 */
/*static*/ bool ZipFileSystem::zipTest( BinaryPtr pBinary )
{	
	uint32 magic;
	if (pBinary->len() > (int)sizeof( magic ))
	{
		magic = ((uint32*)pBinary->data())[0];
		if (magic == ZIP_HEADER_MAGIC)
		    return true;
	}
	return false;
}
	
// zip_file_system.cpp
