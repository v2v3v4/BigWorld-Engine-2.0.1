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
#include "terrain_hole_map1.hpp"

#include "../terrain_data.hpp"
#include "../terrain_height_map.hpp"
#include "terrain_block1.hpp"

using namespace Terrain;

/**
 *  This is the TerrainHoleMap1 default constructor.  This represents
 *  terrain without holes.
 *
 *  @param block        The block that the holes are for.
 */
/*explicit*/ TerrainHoleMap1::TerrainHoleMap1(CommonTerrainBlock1 &block):
    block_(&block),
    readOnly_(true),
    allHoles_(false),
    noHoles_(true),
    holesSize_(0)
{
}


/**
 *  This is the TerrainHoleMap1 destructor.
 */
/*virtual*/ TerrainHoleMap1::~TerrainHoleMap1()
{
}


/**
 *  This function returns true if there are no holes in the hole map.
 *
 *  @returns            True if there are no holes in the hole map.
 */
/*virtual*/ bool TerrainHoleMap1::noHoles() const
{
    return noHoles_;
}


/**
 *  This function returns true if the hole map is completely holes.
 */
/*virtual*/ bool TerrainHoleMap1::allHoles() const
{
    return allHoles_;
}


/**
 *  This function returns the width of the hole map.
 *
 *  @returns            The width of the hole map.
 */
/*virtual*/ uint32 TerrainHoleMap1::width() const
{
    return holesSize_;
}


/**
 *  This function returns the height of the hole map.
 *
 *  @returns            The height of the hole map.
 */
/*virtual*/ uint32 TerrainHoleMap1::height() const
{
    return holesSize_;
}



/**
 *  This function locks the hole map into memory.
 *
 *  @param readOnly     If true then the map is read but not written to.
 */
/*virtual*/ bool TerrainHoleMap1::lock( bool readOnly )
{
	readOnly_ = readOnly;
	if ( image_.isEmpty() )
	{
		image_.resize( holesSize_, holesSize_, allHoles_ );
	}
    return true;
}


/**
 *  This function returns the underlying image of the hole map.
 *
 *  @returns            The underlying image of the hole map.
 */
/*virtual*/ TerrainHoleMap1::ImageType &TerrainHoleMap1::image()
{
    return image_;
}


/**
 *  This function returns the underlying image of the hole map.
 *
 *  @returns            The underlying image of the hole map.
 */
/*virtual*/ TerrainHoleMap1::ImageType const &TerrainHoleMap1::image() const
{
    return image_;
}


/**
 *  This function unlocks the hole map.
 */
/*virtual*/ bool TerrainHoleMap1::unlock()
{
    BW_GUARD;
	if (!readOnly_)
    {
        recalcHoles(); // also clears image if all holes or no holes.
#ifdef EDITOR_ENABLED
		block_->setHeightMapDirty();
#endif // EDITOR_ENABLED
    }
    readOnly_ = true;
    return true;
}


/**
 *  This function saves the hole map.
 *
 *  @param dataSection  The /terrain DataSection in the cdata file.
 *
 *  @returns        true if successful, false otherwise
 */
/*virtual*/ bool TerrainHoleMap1::save(DataSectionPtr dataSection) const
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	bool* holes = (bool*)(data + block_->holesOffset());
	if ( allHoles_ || noHoles_ )
	{
		bool val = allHoles_ ? true : false;
		for ( uint32 i = 0; i < holesSize_*holesSize_; i++ )
			holes[i] = val;
	}
	else
	{
		for ( uint32 y = 0; y < holesSize_; y++ )
		{
			for ( uint32 x = 0; x < holesSize_; x++ )
			{
				*holes++ = image_.get( x, y );
			}
		}
	}

	return true;
}


/**
 *  This function loads an old-terrain hole map.
 *
 *  @param dataSection  The /terrain DataSection in the cdata file.
 *  @param error		If not null and an error occurs, returns the error string
 *
 *  @returns        true if successful, false otherwise
 */
/*virtual*/ bool TerrainHoleMap1::load(DataSectionPtr dataSection, std::string *error/* = NULL*/)
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	bool* holes = (bool*)(data + block_->holesOffset());

    allHoles_       = false;
    noHoles_        = true;
	holesSize_ = block_->blocksWidth();

    image_.resize(holesSize_, holesSize_);
	for ( uint32 y = 0; y < holesSize_; y++ )
	{
		for ( uint32 x = 0; x < holesSize_; x++ )
		{
			image_.set( x, y, *holes++ );
		}
	}

	recalcHoles();

	return true;
}


/**
 *  This function gets the block that the hole map is on.
 *
 *  @returns            The block that the hole map is on.
 */
/*virtual*/ BaseTerrainBlock &TerrainHoleMap1::baseTerrainBlock()
{
    return *block_;
}


/**
 *  This function gets the block that the hole map is on.
 *
 *  @returns            The block that the hole map is on.
 */
/*virtual*/ BaseTerrainBlock const &TerrainHoleMap1::baseTerrainBlock() const
{
    return *block_;
}


/**
 *  This function returns the size of the in the hole map.
 *
 *  @returns            The size of the in the hole map.
 */
uint32 TerrainHoleMap1::holesSize() const
{
    return holesSize_;
}


/**
 *  This recalculates allHoles_ and noHoles_ after unlocking and gets rid of
 *  the hole map if it is degenerate.
 */
void TerrainHoleMap1::recalcHoles()
{
    BW_GUARD;
	if (image_.isEmpty())
    {
        allHoles_ = false;
        noHoles_  = true;
    }
    else
    {
        uint32 w = image_.width();
        uint32 h = image_.height();

        allHoles_ = true;
        bool holes = false;

        for (uint32 y = 0; y < h; ++y)
        {
            for (uint32 x = 0; x < w; ++x)
            {
                bool v = image_.get(x, y);
                allHoles_ &= v;
                holes     |= v;
            }
        }
        noHoles_ = !holes;
        if (noHoles_ || allHoles_)
		{
		    image_.clear();
		}
    }
}

/**
 *  This method checks if there is a hole at the x/z position
 *	@param x x position
 *	@param z z position
 *	@return true if there is a hole
 */
bool TerrainHoleMap1::holeAt(float x, float z) const
{
	if (allHoles_) return true;
	if (noHoles_) return false;

	return image_.get( int32((x / BLOCK_SIZE_METRES) * float(width())),
		int32((z / BLOCK_SIZE_METRES) * float(height())) );
}
/**
 *  This method checks if there is a hole at the x/z rect
 *	@param xs x start position
 *	@param zs z start position
 *	@param xe x end position
 *	@param ze z end position
 *	@return true if the rect intersects a hole
 */
bool TerrainHoleMap1::holeAt( float xs, float zs, float xe, float ze ) const
{
	BW_GUARD;
	if (allHoles_) return true;
	if (noHoles_) return false;

	int32 xStart = int32(floorf((xs / BLOCK_SIZE_METRES) * float(width())));
	int32 xEnd = int32(floorf((xe / BLOCK_SIZE_METRES) * float(width())));

	int32 zStart = int32(floorf((zs / BLOCK_SIZE_METRES) * float(height())));
	int32 zEnd = int32(floorf((ze / BLOCK_SIZE_METRES) * float(height())));

	for (int32 z = zStart; z <= zEnd; z++)
	{
		for (int32 x = xStart; x <= xEnd; x++)
		{
			if (image_.get(x, z))
			{
				return true;
			}
		}
	}

	return false;
}
