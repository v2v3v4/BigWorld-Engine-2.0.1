/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_MAP_RESIZER_HPP
#define TERRAIN_MAP_RESIZER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/base_chunk_space.hpp"
#include "resmgr/datasection.hpp"


/**
 *	This is a helper class to resize maps in a space's terrain
 */
class TerrainMapResizer
{
public:
	static const uint32 DONT_RESIZE = 0;

	/**
	 *  This struct makes it easier to specify the new sizes.
	 */
	struct MapSizeInfo
	{
		/**
		 *	Constructor
		 */
		MapSizeInfo();

		/**
		 *	This method makes sure the struct's values are OK.
		 *	@return true if the struct is valid, false otherwise.
		 */
		bool valid() const;

		uint32 heightMap_;	/// Height map size
		uint32 normalMap_;	/// Normal map size
		uint32 holeMap_;	/// Hole map size
		uint32 shadowMap_;	/// Shadow map size
		uint32 blendMap_;	/// Blend map size
	};

	TerrainMapResizer();
	~TerrainMapResizer();

	MapSizeInfo spaceSizes( DataSectionPtr pSpaceSettings ) const;

	bool resize( const std::string & space, const MapSizeInfo & newSizes,
		ProgressTask * pProgress = NULL );

private:
	Terrain::EditorTerrainBlock2Ptr loadBlock(
		const std::string & space, const std::string & postfix,
		int x, int z, std::string * pError = NULL );

	bool convert(
		const std::string & space, DataSectionPtr pSpaceSettings,
		const MapSizeInfo & oldSizes, const MapSizeInfo & newSizes,
		ProgressTask * pProgress = NULL );

	bool resizeBlends(
		Terrain::EditorTerrainBlock2Ptr pBlock, const std::string & space,
		int x, int z, uint32 size );

	bool changeTerrainSettings(
		DataSectionPtr pSpaceSettings, const MapSizeInfo & newSizes );

	void resizeLayer( Terrain::TerrainTextureLayer & layer, uint32 newSize );

	Terrain::TerrainSettingsPtr pTerrainSettings_;
};

#endif // TERRAIN_MAP_RESIZER_HPP
