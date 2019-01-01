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
#include "worldeditor/import/terrain_utils.hpp"
#include "worldeditor/project/space_helpers.hpp"
#include "worldeditor/project/world_editord_connection.hpp"
#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/misc/sync_mode.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "terrain/terrain_data.hpp"


/**
 *  TerrainUtils::TerrainFormat constructor
 */
TerrainUtils::TerrainFormat::TerrainFormat() :
	poleSpacingX( 0.0f ),
	poleSpacingY( 0.0f ),
	widthM( 0.0f ),
	heightM( 0.0f ),
	polesWidth( 0 ),
	polesHeight( 0 ),
	visOffsX( 0 ),
	visOffsY( 0 ),
	blockWidth( 0 ),
	blockHeight( 0 )
{
}


/**
 *  This function returns the size of the terrain in grid units.
 *
 *  @param space            The space to get.  if this is empty then the 
 *                          current space is used.
 *  @param gridMinX         The minimum grid x unit.
 *  @param gridMinY         The minimum grid y unit.
 *  @param gridMaxX         The maximum grid x unit.
 *  @param gridMaxY         The maximum grid y unit.
 */
void TerrainUtils::terrainSize
(
    std::string             const &space_,
    int                     &gridMinX,
    int                     &gridMinY,
    int                     &gridMaxX,
    int                     &gridMaxY
)
{
	BW_GUARD;

    std::string space = space_;
    if (space.empty())
        space = WorldManager::instance().getCurrentSpace();

    // work out grid size
    DataSectionPtr pDS = BWResource::openSection(space + "/" + SPACE_SETTING_FILE_NAME );
    if (pDS)
    {
        gridMinX = pDS->readInt("bounds/minX",  1);
        gridMinY = pDS->readInt("bounds/minY",  1);
        gridMaxX = pDS->readInt("bounds/maxX", -1);
        gridMaxY = pDS->readInt("bounds/maxY", -1);
    }
}


TerrainUtils::TerrainGetInfo::TerrainGetInfo():
    chunk_(NULL),
    chunkTerrain_(NULL)
{
}


TerrainUtils::TerrainGetInfo::~TerrainGetInfo()
{
    chunk_        = NULL;
    chunkTerrain_ = NULL;
}


/**
 *  This gets the raw terrain height information for the given block.
 *
 *  @param gx               The grid x position.
 *  @param gy               The grid y position.
 *  @param terrainImage     The terrain image data.
 *  @param forceToMemory    Force loading the chunk to memory.
 *  @param getInfo          If you later set the terrain data then you need to
 *                          pass one of these here and one to setTerrain.
 *  @return                 True if the terrain was succesfully read.
 */
bool TerrainUtils::getTerrain
(
    int										gx,
    int										gy,
    Terrain::TerrainHeightMap::ImageType    &terrainImage,
    bool									forceToMemory,
    TerrainGetInfo							*getInfo        /*= NULL*/
)
{
	BW_GUARD;

    Terrain::EditorBaseTerrainBlockPtr terrainBlock = NULL;

    BinaryPtr result;

    // Get the name of the chunk:
    std::string chunkName;
    int16 wgx = (int16)gx;
    int16 wgy = (int16)gy;
    ::chunkID(chunkName, wgx, wgy);
    if (chunkName.empty())
        return false;
    if (getInfo != NULL)
        getInfo->chunkName_ = chunkName;

    // Force the chunk into memory if requested:
    if (forceToMemory)
    {
        ChunkManager::instance().loadChunkNow
        (
            chunkName,
            WorldManager::instance().geometryMapping()
        );
    }
    
    // Is the chunk in memory?    
    Chunk *chunk = 
        ChunkManager::instance().findChunkByName
        (
            chunkName,
            WorldManager::instance().geometryMapping()
        );
    if (chunk != NULL)
    {
		if (getInfo != NULL)
			getInfo->chunk_ = chunk;
		WorldManager::instance().workingChunk( chunk, true );
        ChunkTerrain *chunkTerrain = 
            ChunkTerrainCache::instance(*chunk).pTerrain();
        if (chunkTerrain != NULL)
        {
            terrainBlock = 
                static_cast<Terrain::EditorBaseTerrainBlock *>(chunkTerrain->block().getObject());
            if (getInfo != NULL)
            {
                
                getInfo->block_        = terrainBlock;
                getInfo->chunkTerrain_ = static_cast<EditorChunkTerrain *>(chunkTerrain);
            }
        }
    }

    // Load the terrain block ourselves if not already in memory:
    if (terrainBlock == NULL)
    {
		std::string spaceName = WorldManager::instance().getCurrentSpace();
		terrainBlock = EditorChunkTerrain::loadBlock(spaceName + '/' + chunkName + ".cdata/terrain", 
			NULL, chunk->transform().applyToOrigin() );
        if (getInfo != NULL)
            getInfo->block_ = terrainBlock;
    }

    // If the chunk is in memory, get the terrain data:
    if (terrainBlock != NULL)
    {
        if (getInfo != NULL)
            getInfo->block_ = terrainBlock;

        Terrain::TerrainHeightMap &thm = terrainBlock->heightMap();
        Terrain::TerrainHeightMapHolder holder(&thm, true);
        terrainImage = thm.image();
        return true;
    }
    else
    {
        return false;
    }
}


/**
 *  This function sets raw terrain data.
 *
 *  @param gx               The grid x position.
 *  @param gy               The grid y position.
 *  @param terrainImage     The terrain image data.  This needs to be the
 *                          data passed back via getTerrain.
 *  @param getInfo          If you later set the terrain data then you need to
 *                          pass an empty one of these here.  This needs to be 
 *                          the data passed back via getTerrain.
 *  @param forceToDisk      Force saving the terrain to disk if it was not
 *							in memory.
 */
void TerrainUtils::setTerrain
(
    int                                 gx,
    int                                 gy,
    Terrain::TerrainHeightMap::ImageType    const &terrainImage,
    TerrainGetInfo                      const &getInfo,
    bool                                forceToDisk
)
{
	BW_GUARD;

    // Set the terrain:
    {
        Terrain::TerrainHeightMap &thm = getInfo.block_->heightMap();
        Terrain::TerrainHeightMapHolder holder(&thm, false);
        thm.image().blit(terrainImage);
    }
	getInfo.block_->rebuildNormalMap( Terrain::NMQ_NICE );

    if (forceToDisk && getInfo.chunkTerrain_ == NULL)
    {
		std::string space = WorldManager::instance().getCurrentSpace();
		std::string cdataName = space + '/' + getInfo.chunkName_ + ".cdata";
		Matrix xform = Matrix::identity;
		DataSectionPtr cData = BWResource::openSection( cdataName );

		xform.setTranslate(gx*Terrain::BLOCK_SIZE_METRES, 0.0f, 
							gy*Terrain::BLOCK_SIZE_METRES);
		getInfo.block_->rebuildLodTexture(xform);

		EditorChunkCache::UpdateFlags updateFlags( cData );

		updateFlags.shadow_ = 0;
		updateFlags.thumbnail_= 0;
		updateFlags.terrainLOD_ = !getInfo.block_->isLodTextureDirty();
		updateFlags.save();

		cData->deleteSection( "thumbnail.dds" );
        if ( !getInfo.block_->saveToCData( cData ) )
        {
        	ERROR_MSG( "Couldn't set terrain for chunk '%s'\n", getInfo.chunkName_.c_str() );
        }
    }

    if (getInfo.chunk_ != NULL)
    {
    	if (getInfo.chunkTerrain_ != NULL)
			getInfo.chunkTerrain_->onTerrainChanged();
		if (!forceToDisk || getInfo.chunkTerrain_ != NULL)
		{
			WorldManager::instance().changedTerrainBlock(getInfo.chunk_);
			WorldManager::instance().markTerrainShadowsDirty(getInfo.chunk_);
			WorldManager::instance().workingChunk( getInfo.chunk_, true );			
		}
    }
}


/**
 *	This function can be called before setTerrain to patch heights along the
 *	border of the space in regions that are not visible, but which can 
 *	contribute to, for example, normals.  It does this by extending the edge
 *	values into the non-visible regions.
 *
 *	@param gx				The x-position of the chunk.
 *	@param gy				The y-position of the chunk.
 *	@param gridMaxX			The width of the space.
 *	@param gridMaxY			The height of the space.
 *	@param getInfo			Information returned by getTerrain.
 *	@param terrainImage		The new terrain image that needs extending.
 */
void TerrainUtils::patchEdges
(
	int32								gx,
	int32								gy,
	uint32								gridMaxX,
	uint32								gridMaxY,
	TerrainGetInfo                      &getInfo,
	Terrain::TerrainHeightMap::ImageType	&terrainImage
)
{
	BW_GUARD;

	gx += gridMaxX/2;
	gy += gridMaxY/2;

	Terrain::TerrainHeightMap &thm = getInfo.block_->heightMap();

	uint32 x0 = 0;
	uint32 x1 = thm.xVisibleOffset();
	uint32 x2 = x1 + thm.blocksWidth();
	uint32 x3 = thm.polesWidth();

	uint32 y0 = 0;
	uint32 y1 = thm.zVisibleOffset();
	uint32 y2 = y1 + thm.blocksHeight();
	uint32 y3 = thm.polesHeight();

	// top
	if (gy == 0)
	{
		for (uint32 x = x0; x < x3; ++x)		
		{
			float height = terrainImage.get(x, y1);
			for (uint32 y = y0; y < y1; ++y)
			{
				terrainImage.set(x, y, height);
			}
		}	
	}

	// bottom
	if (gy == gridMaxY - 1)
	{
		for (uint32 x = x0; x < x3; ++x)		
		{
			float height = terrainImage.get(x, y2 - 1);
			for (uint32 y = y2; y < y3; ++y)
			{
				terrainImage.set(x, y, height);
			}
		}	
	}

	// left
	if (gx == 0)
	{
		for (uint32 y = y0; y < y3; ++y)				
		{
			float height = terrainImage.get(x1, y);
			for (uint32 x = x0; x < x1; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}	
	}

	// right
	if (gx == gridMaxX - 1)
	{
		for (uint32 y = y0; y < y3; ++y)				
		{
			float height = terrainImage.get(x2 - 1, y);
			for (uint32 x = x2; x < x3; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}	
	}

	// top-left
	if (gx == 0 && gy == 0)
	{
		float height = terrainImage.get(x1, y1);
		for (uint32 y = y0; y < y1; ++y)
		{
			for (uint32 x = x0; x < x1; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}	
	}

	// top-right
	if (gx == gridMaxX - 1 && gy == 0)
	{
		float height = terrainImage.get(x2 - 1, y1);
		for (uint32 y = y0; y < y1; ++y)
		{
			for (uint32 x = x2; x < x3; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}
	}

	// bottom-left
	if (gx == 0 && gy == gridMaxY - 1)
	{
		float height = terrainImage.get(x1, y2 - 1);
		for (uint32 y = y2; y < y3; ++y)
		{
			for (uint32 x = x0; x < x1; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}
	}

	// bottom-right
	if (gx == gridMaxX - 1 && gy == gridMaxY - 1)
	{
		float height = terrainImage.get(x2 - 1, y2 - 1);
		for (uint32 y = y2; y < y3; ++y)
		{
			for (uint32 x = x2; x < x3; ++x)
			{
				terrainImage.set(x, y, height);
			}
		}
	}
}


/**
 *  This function returns true if the terrain chunk at (gx, gy) is editable.
 *
 *  @param gx               The x grid coordinate.
 *  @param gy               The y grid coordinate.
 *  @returns                True if the chunk at (gx, gy) is locked, false
 *                          if it isn't locked.  The chunk does not have be to
 *							loaded yet.
 */
bool TerrainUtils::isEditable
(
    int                     gx,
    int                     gy
)
{
	BW_GUARD;
    // See if we are in a multi-user environment and connected:    
	return EditorChunk::outsideChunkWriteable(gx, gy, false);
}


/**
 *  This function returns the height at the given position.  It includes items
 *  on the ground.
 *
 *  @param gx               The x grid coordinate.
 *  @param gy               The y grid coordinate.
 *  @param forceLoad        Force loading the chunk and getting focus at this
 *                          point to do collisions.  The camera should be moved
 *                          near here.
 *  @returns                The height at the given position.
 */
float TerrainUtils::heightAtPos
(
    float                   gx,
    float                   gy,
    bool                    forceLoad   /*= false*/
)
{
	BW_GUARD;

    ChunkManager &chunkManager = ChunkManager::instance();

    if (forceLoad)
    {
        SyncMode chunkStopper;

        Vector3 chunkPos = Vector3(gx, 0.0f, gy);
        ChunkManager::instance().cameraSpace()->focus(chunkPos);
        std::string chunkName;
        int16 wgx = (int16)floor(gx/GRID_RESOLUTION);
        int16 wgy = (int16)floor(gy/GRID_RESOLUTION);
        ::chunkID(chunkName, wgx, wgy);
        if (!chunkName.empty())
        {
            chunkManager.loadChunkNow
            (
                chunkName,
                WorldManager::instance().geometryMapping()
            );
            chunkManager.checkLoadingChunks();
        }
        ChunkManager::instance().cameraSpace()->focus(chunkPos);
    }

    const float MAX_SEARCH_HEIGHT = 1000000.0f;
    Vector3 srcPos(gx, +MAX_SEARCH_HEIGHT, gy);
    Vector3 dstPos(gx, -MAX_SEARCH_HEIGHT, gy);
    
    float dist = 
        chunkManager.cameraSpace()->collide
        (
            srcPos, 
            dstPos, 
            ClosestObstacle::s_default
        );
    float result = 0.0;
    if (dist > 0.0)
        result = MAX_SEARCH_HEIGHT - dist;

    return result;
}


/**
 *	Test whether two rectangles intersect, and if they do return the
 *	common area.  The code assumes that left < right and bottom < top.
 *
 *  @param left1	Left coordinate of rectangle 1.
 *  @param top1  	Top coordinate of rectangle 1.
 *  @param right1	Right coordinate of rectangle 1.
 *  @param bottom1	Bottom coordinate of rectangle 1.
 *  @param left2	Left coordinate of rectangle 2.
 *  @param top2  	Top coordinate of rectangle 2.
 *  @param right2	Right coordinate of rectangle 2.
 *  @param bottom2	Bottom coordinate of rectangle 2.
 *  @param ileft	Left coordinate of the intersection rectangle.
 *  @param itop  	Top coordinate of the intersection rectangle.
 *  @param iright 	Right coordinate of the intersection rectangle.
 *  @param ibottom	Bottom coordinate of the intersection rectangle.
 *	@return			True if there was an intersection, false otherwise.
 */
bool
TerrainUtils::rectanglesIntersect
(
    int  left1, int  top1, int  right1, int  bottom1,
    int  left2, int  top2, int  right2, int  bottom2,
    int &ileft, int &itop, int &iright, int &ibottom
)
{
    if 
    (
        right1  < left2
        ||                     
        left1   > right2
        ||                     
        top1    < bottom2
        ||                     
        bottom1 > top2
    )
    {
        return false;
    }
    ileft   = std::max(left1  , left2  );
    itop    = std::min(top1   , top2   ); // Note that bottom < top
    iright  = std::min(right1 , right2 ); 
    ibottom = std::max(bottom1, bottom2);
    return true;
}


/**
 *	This gets the settings of the current space.
 *
 *  @param space		The name of the space to get.
 *  @param gridWidth	The width in chunks of the space.
 *  @param gridHeight	The height in chunks of the space.
 *  @param localToWorld	Coordinates that can be used to convert from grid
 *						coordinates to world coordinates.
 */
bool TerrainUtils::spaceSettings
(
    std::string const &space,
    uint16      &gridWidth,
    uint16      &gridHeight,
    GridCoord   &localToWorld
)
{
	BW_GUARD;

    DataSectionPtr pDS = BWResource::openSection(space + "/" + SPACE_SETTING_FILE_NAME );
    if (pDS)
    {
	    int minX = pDS->readInt("bounds/minX",  1);
	    int minY = pDS->readInt("bounds/minY",  1);
	    int maxX = pDS->readInt("bounds/maxX", -1);
	    int maxY = pDS->readInt("bounds/maxY", -1);

	    gridWidth  = maxX - minX + 1;
	    gridHeight = maxY - minY + 1;
	    localToWorld = GridCoord(minX, minY);

        return true;
    }
    else
    {
	    return false;
    }
}
