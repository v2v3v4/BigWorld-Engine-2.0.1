/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_CONVERTER_HPP
#define TERRAIN_CONVERTER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *	This class converts the old terrain blocks to new for a space,
 *	it converts the heights and texture blends.
 */
class TerrainConverter
{
public:
	TerrainConverter();
	~TerrainConverter();

	void init( const std::string& spacePath, DataSectionPtr pSpaceSection );

	bool convertSingle( int32 x, int32 z, bool reconvert = true );
	void convertAll( ProgressTask* progress, bool reconvert = true );
	void wipeAll();
	void wipeRect( int32 xStart, int32 xEnd, int32 zStart, int32 zEnd );
	void wipeSingle( int32 x, int32 z );


private:

	void blockLRU( std::string id, BlockConverterPtr pBlock );
	BlockConverterPtr blockConverter( int32 x, int32 z );

	typedef std::list<std::string> LRUBlocks;
	LRUBlocks lruBlocks_;
	
	typedef std::map< std::string, BlockConverterPtr > BlockMap;
	BlockMap cachedBlocks_;

	bool readSettings( DataSectionPtr pSpaceSection );

	TerrainConverter( const TerrainConverter& );
	TerrainConverter& operator=( const TerrainConverter& );

	int32	minX_;
	int32	maxX_;
	int32	minZ_;
	int32	maxZ_;

	std::string spacePath_;
	DataSectionPtr pSpaceSection_;

	Terrain::TerrainSettingsPtr pTerrainSettings_;

	bool	singleDir_;

};


/**
 *	This class handles the conversion of one terrain block worth of data.
 */
class BlockConverter : public ReferenceCount
{
public:
	BlockConverter() {};
	~BlockConverter() {};

	void init( DataSectionPtr pTerrainSection, Terrain::TerrainSettingsPtr pSettings );

	void initSrcHeights( BlockConverter* pSouthWest, BlockConverter* pSouth,
					 BlockConverter* pSouthEast, BlockConverter* pWest,
					 BlockConverter* pEast, BlockConverter* pNorthWest,
					 BlockConverter* pNorth, BlockConverter* pNorthEast );
	
	void convertHeights( DataSectionPtr pParentSection );
	void convertBlends( DataSectionPtr pParentSection );
	void convertShadows( DataSectionPtr pParentSection );
	void convertHoles( DataSectionPtr pParentSection );
	void convertLODs( DataSectionPtr pTerrainSection );
	void convertNormals( DataSectionPtr pTerrainSection );
		
private:
	void copyHeights( BlockConverterPtr pSrcBlock, uint32 srcX, uint32 srcZ, 
		uint32 x, uint32 z, uint32 width, uint32 height );

	uint32 blockSize_;

	std::vector<std::string>	textures_;
	std::vector< float >		heights_;
	std::vector< Vector4 >		blendValues_;
	std::vector< uint16 >		shadowValues_;
	std::vector< bool >			holes_;

	std::vector< float >		srcHeights_;

	float spacing_;
	uint32 width_;
	uint32 height_;
	uint32 detailWidth_;
	uint32 detailHeight_;

	uint32 srcStride_;
	uint32 srcHeight_;

	uint32 blocksWidth_;
	uint32 blocksHeight_;
	uint32 verticesWidth_;
	uint32 verticesHeight_;

	Terrain::TerrainSettingsPtr	pTerrainSettings_;
};


#endif // TERRAIN_CONVERTER_HPP
