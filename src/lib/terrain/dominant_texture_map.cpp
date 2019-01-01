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
#include "dominant_texture_map.hpp"

#include "terrain_data.hpp"
#include "terrain_texture_layer.hpp"

#include "cstdmf/debug.hpp"
#include "physics2/material_kinds.hpp"

namespace Terrain
{

/**
 *  This is the DominantTextureMap default constructor.  This represents
 *  terrain without material information.
 *
 *  @param layers          Texture layers to extract texture info.
 *  @param sizeMultiplier  Multiplier to affect the size of the dominant texture map
 *                         relative to the size of the biggest layer.
 */
/*explicit*/ DominantTextureMap::DominantTextureMap(
	TextureLayers&		layers,
	float               sizeMultiplier /* = 0.5f*/) :
		image_( "Terrain/DominantTextureMap" )
{
	BW_GUARD;
	MF_ASSERT( layers.size() > 0 )

	// Use width and height of the biggest layer multiplied by 'sizeMultiplier'
	// as the width and height of the material ID map.
	uint32 width = 0;
	uint32 height = 0;
	for (uint32 i=0; i<layers.size(); i++)
	{
		width = std::max( width, uint32( layers[i]->width() * sizeMultiplier ) );
		height = std::max( height, uint32( layers[i]->height() * sizeMultiplier ) );
	}

	//Cache multipliers for each layer to convert indices
	//into parametric coordinates for the layer
	std::vector< std::pair< float, float > > multipliers;
	multipliers.reserve( layers.size() );

	for (uint32 i=0; i<layers.size(); i++)
	{
		float xMul = (layers[i]->width() / (float)width);
		float yMul = (layers[i]->height() / (float)height);
		multipliers.push_back( std::make_pair(xMul, yMul) );
	}

	image_.resize(width, height);

#ifdef EDITOR_ENABLED
	this->lock(false);
	for (uint32 i=0; i<layers.size(); i++)
	{
		layers[i]->lock(true);
	}
#endif

	std::vector<float> weights;
	weights.reserve(layers.size());

	for (uint32 i=0; i<layers.size(); i++)
	{
		const std::string& textureName = layers[i]->textureName();
		textureNames_.push_back( textureName );
		uint32 mKind = MaterialKinds::instance().get( textureName );
		DataSectionPtr ds = MaterialKinds::instance().userData(mKind);
		float weight = 1.f;
		if ( ds.hasObject() )
		{
			//read the weight for the material kind, if available.
			weight = ds->readFloat( "weight", weight );
			DataSectionPtr ts = MaterialKinds::instance().textureData(mKind,textureName);
			if (ts.hasObject())
			{
				//read the weight for the texture map itself, if available.
				weight = ts->readFloat( "weight", weight );
			}
		}
		weights.push_back( weight );
	}

	//For each sample point, find the dominant layer and
	//store the ID of that layer in the image_ map.
	for (uint32 y=0; y<height; y++)
	{
		MaterialIndex* pValues = image_.getRow(y);
		for (uint32 x=0; x<width; x++)
		{
			MaterialIndex index;
			float maxValue = -1.0f;

			for (uint32 i=0; i<layers.size(); i++)
			{
				//get bicubic interpolated blend value at x,y position.
				uint8 blendValue = layers[i]->image().getBicubic(
					(float)x * multipliers[i].first,
					(float)y * multipliers[i].second );
				//adjust for weighting, allow some textures to dominate others.
				float adjustedBlendValue = (float)blendValue * weights[i];
				if (adjustedBlendValue > maxValue)
				{
					index = (MaterialIndex)i;
					maxValue = adjustedBlendValue;
				}
			}

			pValues[x] = index;
		}
	}

#ifdef EDITOR_ENABLED
	for (uint32 i=0; i<layers.size(); i++)
	{
		layers[i]->unlock();
	}
	this->unlock();
#endif
}


/**
 *  This is the DominantTextureMap default constructor. Use this
 *	constructor if you want to use the load() interface afterwards. 
 *
 */
DominantTextureMap::DominantTextureMap() :
	image_( "Terrain/DominantTextureMap" )
{
}

/**
 *	This method returns the index into our material table for the
 *	dominant texture at the given x,z position relative to the
 *	terrain block.
 *
 *	@return				The material index at relative position x,z
 */
MaterialIndex DominantTextureMap::materialIndex( float x, float z ) const
{
	BW_GUARD;
	//this->lock(true);
	x = (x/BLOCK_SIZE_METRES) * (image_.width() - 1) + 0.5f;
	z = (z/BLOCK_SIZE_METRES) * (image_.height()- 1) + 0.5f;

	MaterialIndex id = image_.get( int(x), int(z) );
	//this->unlock();
	return id;
}


/**
 *	This method returns the dominant material kind at x,z position
 *	relative to the terrain block's position.
 *
 *	@return				The texture name at relative position x,z
 */
uint32 DominantTextureMap::materialKind( float x, float z ) const
{
	BW_GUARD;
	MaterialIndex id = this->materialIndex(x,z);
	MF_ASSERT( id < textureNames_.size() )
	return MaterialKinds::instance().get( textureNames_[id] );
}


/**
 *	This method returns the name of the dominant texture at x,z position
 *	relative to the terrain block's position.
 *
 *	@return				The texture name at relative position x,z
 */
const std::string& DominantTextureMap::texture( float x, float z ) const
{
	BW_GUARD;
	MaterialIndex id = this->materialIndex(x,z);
	MF_ASSERT( id < textureNames_.size() )
	return textureNames_[id];
}


/**
 *  Allows setting the image from a derived class.
 *
 *  @param newImage     New dominant texture map image
 */
void DominantTextureMap::image( const ImageType& newImage )
{
	image_ = newImage;
}


/**
 *  Allows accessing the texture names from a derived class.
 *
 *  @returns     New texture names
 */
const std::vector<std::string>& DominantTextureMap::textureNames() const
{
	return textureNames_;
}


/**
 *  Allows setting the texture names from a derived class.
 *
 *  @param names     New texture names
 */
void DominantTextureMap::textureNames( const std::vector<std::string>& names )
{
	textureNames_.assign( names.begin(), names.end() );
}


/**
 *  This function returns the width of the dominant texture map.
 *
 *  @returns            The width of the dominant texture map.
 */
/*virtual*/ uint32 DominantTextureMap::width() const
{
    return image_.width();
}


/**
 *  This function returns the height of the dominant texture map.
 *
 *  @returns            The height of the dominant texture map.
 */
/*virtual*/ uint32 DominantTextureMap::height() const
{
    return image_.height();
}


#ifdef EDITOR_ENABLED

/**
 *  This function locks the dominant texture map for reading and/or writing.
 */
/*virtual*/ bool DominantTextureMap::lock(bool readonly)
{
	return true;
}


/**
 *  This function returns the underlying image of the dominant texture map.
 *
 *  @returns            The underlying image of the dominant texture map.
 */
/*virtual*/ DominantTextureMap::ImageType &DominantTextureMap::image()
{
    return image_;
}


/**
*  This function unlocks the dominant texture map.
*/
/*virtual*/ bool DominantTextureMap::unlock()
{
	return true;
}


/**
*  This function saves the dominant texture map to a DataSection.
*
*  @returns            A DataSection that can be used to restore the
*                      dominant texture map with DominantTextureMap::Load.
*/
/*virtual*/ bool DominantTextureMap::save( DataSectionPtr ) const
{
	// TODO implement if needed.  Currently the dominant texture map is not saved
	// to disk by design, so we don't need this and shouldn't call this.
	MF_ASSERT(0);
	return false;
}

#endif // EDITOR_ENABLED
/**
 *  This function returns the underlying image of the dominant texture map.
 *
 *  @returns            The underlying image of the dominant texture map.
 */
/*virtual*/ DominantTextureMap::ImageType const &DominantTextureMap::image() const
{
    return image_;
}
}
