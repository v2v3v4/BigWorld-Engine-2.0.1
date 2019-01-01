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
#include "horizon_shadow_map1.hpp"

#include "terrain_block1.hpp"
#include "../terrain_data.hpp"

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2( "Terrain", 0 );

/**
 *  This is the HorizonShadowMap1 constructor.
 *
 *  @param block        The underlying block.
 */
/*explicit*/ HorizonShadowMap1::HorizonShadowMap1(TerrainBlock1 &block):
    HorizonShadowMap(),
    block_(&block),
	readOnly_(true)
{
}


/**
 *  This function determines the shadow at the given point.
 *
 *  @param x            The x coordinate to get the shadow at.
 *  @param z            The z coordinate to get the shadow at.
 *  @returns            The shadow at x, z.
 */ 
/*virtual*/ HorizonShadowPixel 
HorizonShadowMap1::shadowAt(int32 x, int32 z) const
{
    return image_.get(x, z);
}


/**
 *  This function returns the width of the HorizonShadowMap1.
 *
 *  @returns            The width of the HorizonShadowMap1.
 */
/*virtual*/ uint32 HorizonShadowMap1::width() const
{
    return image_.width();
}


/**
 *  This function returns the height of the HorizonShadowMap1.
 *
 *  @returns            The height of the HorizonShadowMap1.
 */
/*virtual*/ uint32 HorizonShadowMap1::height() const
{
    return image_.height();
}


/**
 *  This function locks the HorizonShadowMap1 to memory and enables it for
 *  editing.
 *
 *  @param readOnly     This is a hint to the locking.  If true then
 *                      we are only reading values from the map.
 *  @returns            True if the lock is successful, false 
 *                      otherwise.
 */
/*virtual*/ bool HorizonShadowMap1::lock(bool readOnly)
{
    readOnly_ = readOnly;
    return true;
}


/**
 *  This function accesses the HorizonShadowMap1 as though it is an image.
 *  This function should only be called within a lock/unlock pair.
 *
 *  @returns            The HorizonShadowMap1 as an image.
 */
/*virtual*/ HorizonShadowMap1::ImageType &HorizonShadowMap1::image()
{
    return image_;
}


/**
 *  This function accesses the HorizonShadowMap1 as though it is an image.
 *  This function should only be called within a lock/unlock pair.
 *
 *  @returns            The HorizonShadowMap1 as an image.
 */
/*virtual*/ HorizonShadowMap1::ImageType const &HorizonShadowMap1::image() const
{
    return image_;
}


/**
 *  This function unlocks the HorizonShadowMap1.
 *
 *  @returns            True if the unlocking was successful.
 */
/*virtual*/ bool HorizonShadowMap1::unlock()
{
#ifdef EDITOR_ENABLED
	if ( !readOnly_ )
	{
		// calling this to force recreation of the managed objects
		block_->setHeightMapDirty();
	}
#endif
    readOnly_ = true;
    return true;
}


/**
 *  This function saves the underlying data to a DataSection that can
 *  be read back in via load.  You should not call this between
 *  lock/unlock pairs.
 *
 *  @param pSection  The /terrain DataSection in the cdata file.
 *
 *  @returns            True if the save was successful.
 */
/*virtual*/ bool HorizonShadowMap1::save( DataSectionPtr pSection ) const
{
	BW_GUARD;
	BinaryPtr pTerrainData = pSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	uint16* shadows = (uint16*)(data + block_->shadowsOffset());

	// Convert from HorizonShadowPixel back to old terrain uint16 format.
	// Must also add the border back, which is done taking advantage of
	// the image class clamping inside its 'get' method.
	for( uint32 y = 0; y < block_->height(); y++ )
	{
		for( uint32 x = 0; x < block_->width(); x++ )
		{
			// Note that the loops scan the whole block area, so must substract
			// the border when doing image::get, which will clamp out-of-bound
			// values.
			HorizonShadowPixel pixel = image_.get( x - 1, y - 1 );
			*shadows++ =
				uint16( ((pixel.west & 0xFF00) >> 8) | (pixel.east & 0xFF00) );
		}
	}
	return true;
}


/**
 *  This function loads the HorizonShadowMap1 from a DataSection that was 
 *  saved via HorizonShadowMap1::save.  You should not call this between
 *  lock/unlock pairs.
 *
 *  @param dataSection  The /terrain DataSection in the cdata file.
 *  @param error        If not NULL and an error occurs then this will
 *                      be set to a reason why the error occured.
 *  @returns            True if the load was successful.
 */
/*virtual*/ bool 
HorizonShadowMap1::load(DataSectionPtr dataSection, std::string *error/*= NULL*/)
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	uint16* shadows = (uint16*)(data + block_->shadowsOffset());

    image_.resize( block_->verticesWidth(), block_->verticesHeight() );
	// convert from uint16 to HorizonShadowPixel so the terrain tools work
	// transparently. Also skips the borders of the shadow map to reduce it
	// to the size of the vertices.
	shadows += block_->width(); // skip first line
	for( uint32 y = 0; y < image_.height(); y++ )
	{
		shadows++; // skip first pixel in the line
		for( uint32 x = 0; x < image_.width(); x++ )
		{
			HorizonShadowPixel pixel;
			// make most significant byte an 'east' value from 0 to 65535
			pixel.east = (*shadows) & 0xFF00;
			// make least significant byte a 'west' value from 0 to 65535
			pixel.west = ((*shadows) & 0xFF) << 8;
			image_.set( x, y, pixel );
			shadows++;
		}
		shadows++; // skip last pixel in the line
	}

	return true;
}


/**
 *  This function returns the BaseTerrainBlock that the HorizonShadowMap1 is
 *  associated with.
 *
 *  @returns            The BaseTerrainBlock of the HorizonShadowMap1.
 */
/*virtual*/ BaseTerrainBlock &HorizonShadowMap1::baseTerrainBlock()
{
    return *block_;
}


/**
 *  This function returns the BaseTerrainBlock that the HorizonShadowMap1 is
 *  associated with.
 *
 *  @returns            The BaseTerrainBlock of the HorizonShadowMap1.
 */
/*virtual*/ BaseTerrainBlock const &HorizonShadowMap1::baseTerrainBlock() const
{
    return *block_;
}
