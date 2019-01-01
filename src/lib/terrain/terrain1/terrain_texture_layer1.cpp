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
#include "terrain_texture_layer1.hpp"

#include "terrain_block1.hpp"
#include "../terrain_data.hpp"

#include "cstdmf/debug.hpp"

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2("Terrain", 0)

/**
 *	This is the TerrainTextureLayer1 constructor.
 *
 *  @param terrainBlock		The underlying TerrainBlock1.
 *	@param index			Texture layer index.
 */
TerrainTextureLayer1::TerrainTextureLayer1(
	CommonTerrainBlock1 &terrainBlock, uint32 index):
    terrainBlock_(&terrainBlock),
	index_(index)
{
}

/**
 *	This is the TerrainTextureLayer1 destructor.
 */
TerrainTextureLayer1::~TerrainTextureLayer1()
{
}


/**
 *	This gets the name of the texture used in the layer.
 *
 *  @returns	The name of the texture used to draw the layer.
 */
/*virtual*/ std::string TerrainTextureLayer1::textureName() const
{
    return textureName_;
}


/**
 *	This sets the texture used by the layer.  The layer is created using the
 *	default size.
 *
 *  @param filename		The name of the new texture.
 *  @returns			True if the texture could be set.
 */
/*virtual*/ bool TerrainTextureLayer1::textureName(std::string const &filename)
{
#ifndef MF_SERVER
	Moo::BaseTexturePtr tex =
		Moo::TextureManager::instance()->get( filename, 
					true, false, true, "Terrain/TextureLayer1/TextureResource" );

	bool isOkay = true;
	if (!tex)
	{
		// Make the texture manager load the fallback for it, but
		// print an error.
		tex = Moo::TextureManager::instance()->get(
					Moo::TextureManager::notFoundBmp(), 
					true, true, true, "Terrain/TextureLayer1/TextureResource" );
		isOkay = false;
	}
	texture(tex, filename);
    return isOkay;
#else
	textureName_ = filename;
	return true;
#endif
}


#ifndef MF_SERVER


/**
 *	This gets the texture used by the layer.
 *
 *  @returns	The texture used by the layer.
 */
/*virtual*/ Moo::BaseTexturePtr TerrainTextureLayer1::texture() const
{
    return pTexture_;
}


/**
 *	This sets the texture used by the layer.
 *
 *  @param texture	The new texture.
 *  @param textureName	The name of the new texture's resource.
 */
/*virtual*/ void
TerrainTextureLayer1::texture
(
	Moo::BaseTexturePtr  texture,
    std::string          const &textureName
)
{
    textureName_ = textureName;
    pTexture_    = texture;

    blends_.resize(
		terrainBlock_->verticesWidth(), terrainBlock_->verticesHeight(),
		0);

    TerrainTextureLayer::defaultUVProjections(uProjection_, vProjection_);
}
#endif


/**
 *	This is called to determine whether layers of this type support setting UV
 *	projections.
 *
 *  @returns		true.
 */
/*virtual*/ bool TerrainTextureLayer1::hasUVProjections() const
{
    return false;
}


/**
 *	This returns the u-projection of the layer.
 *
 *  @returns		The u-projection axis of the layer.
 */
/*virtual*/ Vector4 const &TerrainTextureLayer1::uProjection() const
{
	return uProjection_;
}


/**
 *	This is used to set the u-projection of the layer.
 *
 *  @param u		The new u axis.
 */
/*virtual*/ void TerrainTextureLayer1::uProjection(Vector4 const &u)
{
	CRITICAL_MSG( "This texture layer has no u/v projections\n" );
}


/**
 *	This returns the v-projection of the layer.
 *
 *  @returns		The v-projection axis of the layer.
 */
/*virtual*/ Vector4 const &TerrainTextureLayer1::vProjection() const
{
	return vProjection_;
}


/**
 *	This is used to set the v-projection of the layer.
 *
 *  @param v		The new v axis.
 */
/*virtual*/ void TerrainTextureLayer1::vProjection(Vector4 const &v)
{
	CRITICAL_MSG( "This texture layer has no u/v projections\n" );
}


/**
 *	This gets the width of the layer.
 *
 *  @returns		The width of the layer.
 */
/*virtual*/ uint32 TerrainTextureLayer1::width() const
{
    return blends_.width();
}


/**
 *	This gets the height of the layer.
 *
 *  @returns		The height of the layer.
 */
/*virtual*/ uint32 TerrainTextureLayer1::height() const
{
    return blends_.height();
}


#ifdef EDITOR_ENABLED

/**
 *	This is called before the layer's image is read or written to.
 *
 *  @param readOnly	Is the layer being locked for reading?
 *  @returns		True.
 */
/*virtual*/ bool TerrainTextureLayer1::lock(bool readOnly)
{
    return true;
}


/**
 *	This gets the layer's image.  This has to be called within a lock/unlock
 *	pair.
 *
 *  @returns		The layer's image.
 */
/*virtual*/ TerrainTextureLayer1::ImageType &TerrainTextureLayer1::image()
{
    return blends_;
}


/**
*	This is called when the layer's image is not needed for reading/writing.
*
*  @returns		True.
*/
/*virtual*/ bool TerrainTextureLayer1::unlock()
{
	return true;
}


/**
*	This is called to save the layer.
*
 *  @param dataSection	data section of the /terrain resource in the cdata file
*	@returns			true if successfully saved.
*/
/*virtual*/ bool TerrainTextureLayer1::save( DataSectionPtr dataSection ) const
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	uint32* packedBlends = (uint32*)(data + terrainBlock_->blendsOffset());
	for ( uint32 y = 0; y < terrainBlock_->height(); ++y )
	{
		for ( uint32 x = 0; x < terrainBlock_->width(); ++x )
		{
			// Note that the loops scan the whole block area, so must substract
			// the border when doing image::get, which will clamp out-of-bound
			// values.
			unsigned char blend = blends_.get( x - 1, y - 1 );
			// pack back the blends of this layer to the corresponding byte in the
			// packed blends
			switch ( index_ )
			{
			case 0:
				*packedBlends &= ~0x000000ff;
				*packedBlends++ |= (uint32)((blend / 2 + 128) & 0x000000ff);
				break;
			case 1:
				*packedBlends &= ~0x0000ff00;
				*packedBlends++ |= (uint32)(((blend / 2 + 128)<<8) & 0x0000ff00);
				break;
			case 2:
				*packedBlends &= ~0x00ff0000;
				*packedBlends++ |= (uint32)(((blend / 2 + 128)<<16) & 0x00ff0000);
				break;
			case 3:
				*packedBlends &= ~0xff000000;
				*packedBlends++ |= (uint32)((blend<<24) & 0xff000000);
				break;
			}
		}
	}
	MF_ASSERT( textureName_.length() < terrainBlock_->textureNameSize() );
	strcpy( (char*)(data + terrainBlock_->textureNameOffset( index_ )), textureName_.c_str() );
	return true;
}

#endif // EDITOR_ENABLED

/**
 *	This gets the layer's image.  This has to be called within a lock/unlock
 *	pair.
 *
 *  @returns		The layer's image.
 */
/*virtual*/ TerrainTextureLayer1::ImageType const &TerrainTextureLayer1::image() const
{
	return blends_;
}


/**
 *	This is called to load the layer.
 *
 *  @param dataSection	data section of the /terrain resource in the cdata file
 *  @param error		If an error occurs during loading then this is set to
 *						an error string.
 *  @returns			True if successfully loaded.
 */
/*virtual*/ bool TerrainTextureLayer1::load(
	DataSectionPtr dataSection, std::string *error /*= NULL*/)
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	// put the blends of layer i into a continous block of memory.
	std::vector<unsigned char> blends;
	blends.resize( terrainBlock_->numMapElements() );
	uint32* packedBlends = (uint32*)(data + terrainBlock_->blendsOffset());
	for( unsigned int c = 0; c < terrainBlock_->numMapElements(); c++ )
	{
		// convert the byte corresponding to layer 'index_' in the packedBlends
		// to the way blends are used by the new terrain, to allow consistent
		// editing.
		switch ( index_ )
		{
		case 0:
			blends[c] = (unsigned char)(((packedBlends[ c ] & 0x000000ff) - 128.f) * 2);
			break;
		case 1:
			blends[c] = (unsigned char)((((packedBlends[ c ] & 0x0000ff00)>>8) - 128.f) * 2);
			break;
		case 2:
			blends[c] = (unsigned char)((((packedBlends[ c ] & 0x00ff0000)>>16) - 128.f) * 2);
			break;
		case 3:
			blends[c] = (unsigned char)((packedBlends[ c ] & 0xff000000)>>24);
			break;
		}
	}
	std::string filename = (char*)(data + terrainBlock_->textureNameOffset( index_ ));
	if (!textureName( filename ))
	{
		std::string errorTail;
#ifdef EDITOR_ENABLED
		errorTail = " for terrain block '" + terrainBlock_->resourceName() + "'";
#endif // EDITOR_ENABLED

		std::string err = "Could not load terrain texture '" + filename + "'" + errorTail + "\n";
		if (error)
			*error = err;
		ERROR_MSG( err.c_str() );
		
#ifndef MF_SERVER
		if (!this->texture())
		{
			ERROR_MSG( "Could not create the fallback texture in place of the "
				"missing texture%s\n",
				errorTail.c_str() );
			return false;
		}
#endif // MF_SERVER
	}

	blends_.resize(
		terrainBlock_->verticesWidth(), terrainBlock_->verticesHeight(),
		0);
	const unsigned char* src = &blends.front();
	// skip the top line, because the old terrain stores blends including
	// the same border as the heights, to be consistent with terrain2.
	src += terrainBlock_->width();
	for ( uint32 y = 0; y < blends_.height(); ++y )
	{
		// skip the border column to the left
		src += 1;
		for ( uint32 x = 0; x < blends_.width(); ++x )
		{
			blends_.set( x, y, *src++ );
		}
		// skip the border column to the right
		src += 1;
	}
	return true;
}


/**
 *	This returns the underlying block.
 *
 *  @returns		The underlying block.
 */
/*virtual*/ BaseTerrainBlock &TerrainTextureLayer1::baseTerrainBlock()
{
    return *terrainBlock_;
}


/**
 *	This returns the underlying block.
 *
 *  @returns		The underlying block.
 */
/*virtual*/ BaseTerrainBlock const &TerrainTextureLayer1::baseTerrainBlock() const
{
    return *terrainBlock_;
}
