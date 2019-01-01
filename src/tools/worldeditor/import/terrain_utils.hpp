/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_UTILS_HPP
#define TERRAIN_UTILS_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "moo/image.hpp"
#include "terrain/editor_base_terrain_block.hpp"
#include "terrain/terrain_height_map.hpp"


namespace TerrainUtils
{
    struct TerrainFormat
    {
		TerrainFormat();
        float           poleSpacingX;   // Spacing between poles in metres in x-direction
        float           poleSpacingY;   // Spacing between poles in metres in y-direction
        float           widthM;         // Width of block in metres
        float           heightM;        // Height of block in metres
        uint32          polesWidth;     // Number of poles across entire block in x-direction
        uint32          polesHeight;    // Number of poles across entire block in y-direction 
        uint32          visOffsX;       // X-offset into block for the first visible pixel
        uint32          visOffsY;       // Y-offset into block for the first visible pixel
        uint32          blockWidth;     // The width of the visible portion of a block
        uint32          blockHeight;    // The height of the visible portion of a block
    };

    void terrainSize
    (
        std::string                         const &space,
        int                                 &gridMinX,
        int                                 &gridMinY,
        int                                 &gridMaxX,
        int                                 &gridMaxY
    );

    struct TerrainGetInfo
    {
        Chunk                               *chunk_;
        std::string                         chunkName_;
        Terrain::EditorBaseTerrainBlockPtr  block_;
        EditorChunkTerrain                  *chunkTerrain_;

        TerrainGetInfo();
        ~TerrainGetInfo();
    };

    bool getTerrain
    (
        int										gx,
        int										gy,
        Terrain::TerrainHeightMap::ImageType    &terrainImage,
        bool									forceToMemory,
        TerrainGetInfo							*getInfo        = NULL
    );

    void setTerrain
    (
        int										gx,
        int										gy,
        Terrain::TerrainHeightMap::ImageType    const &terrainImage,
        TerrainGetInfo							const &getInfo,
        bool									forceToDisk
    );

	void patchEdges
	(
		int32									gx,
		int32									gy,
		uint32									gridMaxX,
		uint32									gridMaxY,
		TerrainGetInfo							&getInfo,
		Terrain::TerrainHeightMap::ImageType	&terrainImage
	);

    bool isEditable
    (
        int                                 gx,
        int                                 gy
    );

    float heightAtPos
    (
        float                               gx,
        float                               gy,
        bool                                forceLoad   = false
    );

	bool
	rectanglesIntersect
	(
        int  left1, int  top1, int  right1, int  bottom1,
        int  left2, int  top2, int  right2, int  bottom2,
        int &ileft, int &itop, int &iright, int &ibottom		
	);

	bool spaceSettings
	(
		std::string const &space,
		uint16      &gridWidth,
		uint16      &gridHeight,
		GridCoord   &localToWorld
	);
}


#endif // TERRAIN_UTILS_HPP
