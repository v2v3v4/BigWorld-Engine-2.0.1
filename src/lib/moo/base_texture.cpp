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

#include "cstdmf/memory_counter.hpp"

#include "base_texture.hpp"
#include "texture_manager.hpp"

#ifndef CODE_INLINE
#include "base_texture.ipp"
#endif

memoryCounterDefine( texture, Texture );

namespace Moo
{

BaseTexture::BaseTexture(const std::string& allocator)
{
	BW_GUARD;
	memoryCounterAdd( texture );
	memoryClaim( this );

#if ENABLE_RESOURCE_COUNTERS
	allocator_ = allocator;
#endif
}

BaseTexture::~BaseTexture()
{
	memoryCounterSub( texture );
	memoryClaim( this );
}

/**
 *	This method implements a stub accessor for the resource id
 *	of the texture.
 *	@return an empty string.
 */
const std::string& BaseTexture::resourceID( ) const
{
	static std::string s;
	return s;
}


/**
 *	This method calculates the amount of memory used by a texture in bytes.
 */
uint32 BaseTexture::textureMemoryUsed( DX::Texture* pTex )
{
	BW_GUARD;
	uint32 textureMemoryUsed = sizeof(DX::Texture);

	DX::Surface* surface;
	if( pTex && SUCCEEDED( pTex->GetSurfaceLevel( 0, &surface ) ) )
	{
		D3DSURFACE_DESC desc;
		surface->GetDesc( &desc );
		textureMemoryUsed = sizeof(DX::Surface) + DX::surfaceSize( desc );
		surface->Release();
		surface = NULL;

		for( uint32 i = 1; i < pTex->GetLevelCount(); i++ )
		{
			if( SUCCEEDED( pTex->GetSurfaceLevel( i, &surface ) ) )
			{
				surface->GetDesc( &desc );
				textureMemoryUsed += sizeof(DX::Surface) + DX::surfaceSize( desc );
				surface->Release();
			}
		}
	}

	return textureMemoryUsed;
}


/**
 *	This method adds this texture to the texture manager
 */
void BaseTexture::addToManager()
{
	BW_GUARD;
	TextureManager::add( this );
}


/**
 *	This method deletes this texture from the texture manager
 */
void BaseTexture::delFromManager()
{
	BW_GUARD;
	TextureManager::del( this );
	// could put this in BaseTexture's destructor but then the
	// virtual resourceID fn wouldn't work for the debug message :)
}

/**
 * This function removes file extension from given name, and a ".c" extension 
 * if it exists.
 */
const std::string removeTextureExtension(const std::string & resourceName)
{
	BW_GUARD;
	std::string baseName = BWResource::removeExtension(resourceName);
	if (BWResource::getExtension(baseName) == "c")
	{
		baseName = BWResource::removeExtension(baseName);
	}
	return baseName;
}


/**
 * This function removes a ".c" extension from a given texture name.
 */
const std::string canonicalTextureName(const std::string & resourceName)
{
	std::string baseName  = removeTextureExtension(resourceName);
	std::string extension = BWResource::getExtension(resourceName);
	return baseName + "." + extension;
}


}

// base_texture.cpp
