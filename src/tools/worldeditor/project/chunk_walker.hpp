/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WALKER_HPP
#define CHUNK_WALKER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/project/space_information.hpp"
#include "worldeditor/project/grid_coord.hpp"


/** 
 *	Interface to a helper class that provides chunk names / grid positions
 *	in any appropriate order.  Used by the space map to update itself.
 */
class IChunkWalker
{
public:
	virtual void reset() = 0;
	virtual void spaceInformation( const SpaceInformation& info ) = 0;
	virtual bool nextTile( std::string& chunkName, int16& gridX, int16& gridZ ) = 0;
};


//This chunk walker creates tile information in a linear fashion.
class LinearChunkWalker : public IChunkWalker
{
public:
	LinearChunkWalker();
	void reset();
	void spaceInformation( const SpaceInformation& info );
	bool nextTile( std::string& chunkName, int16& gridX, int16& gridZ );
private:
	SpaceInformation info_;	
	uint16 x_;
	uint16 z_;
};

/** 
 * This chunk walker creates tile information in a bounded fashion.
 * That is, it linearly walks the tiles, but between the chunks bounded by 
 * (left,bottom) and (right,top).
 * This improves performance as we don't have to walk every chunk to import export
 */
class BoundedChunkWalker : public IChunkWalker
{
public:
	BoundedChunkWalker( int16 left, int16 bottom, int16 right, uint16 top);
	void reset();
	void spaceInformation( const SpaceInformation& info );
	bool nextTile( std::string& chunkName, int16& gridX, int16& gridZ );
private:
	SpaceInformation info_;	
	int16 x_;
	int16 z_;
	int16 left_;
	int16 bottom_;
	int16 right_;
	int16 top_;		
	bool isSet_;
};


//This chunk walker just gives up information that is added to it.
//Most often used by others just to store a list of chunks.
//It can serialise to xml.
class CacheChunkWalker : public IChunkWalker
{
public:
	CacheChunkWalker();
	void reset()	{};
	void spaceInformation( const SpaceInformation& info );
	bool nextTile( std::string& chunkName, int16& gridX, int16& gridZ );
	void add( Chunk* pChunk );
	void add( int16& gridX, int16& gridZ );
	void save( DataSectionPtr pSection );
	void save( DataSectionPtr pSection, std::vector<uint16>& modifiedTileNumbers );
	void load( DataSectionPtr pSection );
    bool added( const Chunk* pChunk ) const;
    bool added( int16 gridX, int16 gridZ ) const;
    bool erase( const Chunk* pChunk );
    bool erase( int16 gridX, int16 gridZ );
    size_t size() const;
private:
	std::vector<uint16>		 xs_;
	std::vector<uint16>		 zs_;
	SpaceInformation info_;
};


/**
 *	This class scans the space on disk for all thumbnail files,
 *	and returns chunks identified as relevant by derived classes.
 *
 *	This base class handles file searching and tile storage.
 *	Derived classes need only implement the isRelevant interface.
 */
class FileChunkWalker : public IChunkWalker
{
public:
	FileChunkWalker( uint16 numIrrelevantPerFind = 10 );
	virtual ~FileChunkWalker();
	void load();
	void save();
protected:
	void reset();
	void spaceInformation( const SpaceInformation& info );
	bool nextTile( std::string& chunkName, int16& gridX, int16& gridZ );	

	virtual bool isRelevant( const WIN32_FIND_DATA& fileInfo ) = 0;

	SpaceInformation info_;	
	std::string	reentryFolder_;
private:
	void findNextFile();
	void findFolders( const std::string& folder );
	void advanceFileHandle();
	void releaseFileHandle();

	void findReentryFolder();
	bool partialFolderMatch( const std::string& folder, const std::string& target );
	bool findFolder( const std::string& target );

	//disregard ., .., CVS folders
	bool isValidFolder( const WIN32_FIND_DATA& fileInfo );

	HANDLE fileHandle_;
	WIN32_FIND_DATA findFileData_;
	std::string root_;
	std::string space_;
	std::string folder_;
	bool searchingFiles_;
	std::vector<std::string> folders_;
	std::vector<uint16> xs_;
	std::vector<uint16>	zs_;
	uint16	numIrrelevantPerFind_;
};



//This chunk walker finds thumbnails with a modification date
//later than the modification date stored in the thumbnail
//timestamp cache.
//(saved-to-disk thumbnails that have not yet been drawn on the space map)
class ModifiedFileChunkWalker : public FileChunkWalker
{
public:
	ModifiedFileChunkWalker( uint16 numIrrelevantPerFind = 10 );
	~ModifiedFileChunkWalker();
	void spaceInformation( const SpaceInformation& info );	
private:
	bool isRelevant( const WIN32_FIND_DATA& fileInfo );	
};


#endif // CHUNK_WALKER_HPP
