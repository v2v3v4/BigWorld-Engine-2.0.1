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

#include <algorithm>
#include <string>

#include "texture_manager.hpp"
#include "texture_compressor.hpp"
#include "animating_texture.hpp"
#include "render_context.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "sys_mem_texture.hpp"

#include "cstdmf/binaryfile.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_trace.hpp"
#include "resmgr/auto_config.hpp"
#include "managed_texture.hpp"
#include "resource_load_context.hpp"

#include "dds.h"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

static AutoConfigString s_notFoundBmp( "system/notFoundBmp" );

static AutoConfigString s_textureDetailLevels( "system/textureDetailLevels" );
#ifdef EDITOR_ENABLED
static AutoConfigString s_toolsTextureDetail( "editor/textureDetailLevels" );
#endif // EDITOR_ENABLED

// Helper functions
namespace
{
	std::string toLower(const std::string& input)
	{
		std::string output = input;
		for (uint i = 0; i < output.length(); i++)
		{
			if (output[i] >= 'A' &&
				output[i] <= 'Z')
			{
				output[i] = output[i] - 'A' + 'a';
			}
		}
		return output;
	}


	int calcMipSkip(int qualitySetting, int lodMode)
	{	
		int result = 0;

		switch (lodMode)
		{
			case 0: // disabled
				result = 0;
				break;
			case 1: // normal
				result = qualitySetting;
				break;
			case 2: // low bias
				result = std::min(qualitySetting, 1);
				break;
			case 3: // high bias
				result = std::max(qualitySetting-1, 0);
				break;
		}
		return result;
	}


	const char *textureExts[] = 
	{ 
		"dds", "bmp", "tga", "jpg", "png", "hdr", "pfm", "dib", "ppm", NULL
	};
}

namespace Moo
{

/**
 * Default constructor.
 */
TextureManager::TextureManager() :
	fullHouse_( false ),
	detailLevelsModTime_( 0 )
{
	BW_GUARD;
	initDetailLevels( BWResource::openSection( s_textureDetailLevels ) );
#ifdef EDITOR_ENABLED
	// Load the tools-specific detail level override.
	initDetailLevels(
		BWResource::openSection( s_toolsTextureDetail ),
		true /* accumulate */ );
#endif
	detailLevelsModTime_ = BWResource::modifiedTime( s_textureDetailLevels );
	
	// texture quality settings
	this->qualitySettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"TEXTURE_QUALITY", "Texture Quality", *this, 
			&TextureManager::setQualityOption, 
			0, true, false);
				
	this->qualitySettings_->addOption("HIGH", "High", true);
	this->qualitySettings_->addOption("MEDIUM", "Medium", true);
	this->qualitySettings_->addOption("LOW", "Low", true);
	Moo::GraphicsSetting::add(this->qualitySettings_);

	// texture compression settings
	this->compressionSettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"TEXTURE_COMPRESSION", "Texture Compression", *this, 
			&TextureManager::setCompressionOption, 
			0, true, false);
				
	this->compressionSettings_->addOption("ON", "On", true);
	this->compressionSettings_->addOption("OFF", "Off", true);
	Moo::GraphicsSetting::add(this->compressionSettings_);

	// texture filtering settings
	this->filterSettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"TEXTURE_FILTERING", "Texture Filtering", *this, 
			&TextureManager::setFilterOption, 
			0, false, false);
				
	this->filterSettings_->addOption("ANISOTROPIC_16X", "Anisotropic (16x)", Moo::rc().maxAnisotropy() >= 16);
	this->filterSettings_->addOption("ANISOTROPIC_8X",  "Anisotropic (8x)", Moo::rc().maxAnisotropy() >= 8);
	this->filterSettings_->addOption("ANISOTROPIC_4X",  "Anisotropic (4x)", Moo::rc().maxAnisotropy() >= 4);
	this->filterSettings_->addOption("ANISOTROPIC_2X",  "Anisotropic (2x)", Moo::rc().maxAnisotropy() >= 2);
	this->filterSettings_->addOption("TRILINEAR", "Trilinear", true);
	this->filterSettings_->addOption("BILINEAR", "Bilinear", true);
	this->filterSettings_->addOption("POINT", "Point", true);
	Moo::GraphicsSetting::add(this->filterSettings_);
	
	// saved settings may differ from the ones set 
	// in the constructor call (0). If that is the case,
	// both settings will be pending. Commit them now.
	Moo::GraphicsSetting::commitPending();
}

/**
 * Destructor.
 */
TextureManager::~TextureManager()
{
	BW_GUARD;
	while (!textures_.empty())
	{
		INFO_MSG( "-- Texture not properly deleted on exit: %s\n", textures_.begin()->first.c_str() );

		textures_.erase(textures_.begin());
	}
}

/**
 * Static instance accessor.
 */
TextureManager* TextureManager::instance()
{
	return pInstance_;
}


/**
 *	Static method to return the default texture used as a fallback for missing
 *	textures.
 */
/*static*/ const std::string & TextureManager::notFoundBmp()
{
	return s_notFoundBmp.value();
}


/**
 * Load all contained textures.
 */
HRESULT TextureManager::initTextures( )
{
	BW_GUARD;
	HRESULT res = S_OK;
	TextureMap::iterator it = textures_.begin();

	while( it != textures_.end() )
	{
		(it->second)->load();
		it++;
	}
	return res;
}

/**
 * Release all textures.
 */
HRESULT TextureManager::releaseTextures( )
{
	BW_GUARD;
	HRESULT res = S_OK;
	TextureMap::iterator it = textures_.begin();

	while( it != textures_.end() )
	{
		HRESULT hr;
		if( FAILED( hr = (it->second)->release() ) )
		{
			res = hr;
		}

		it++;
	}

	return res;
}

/**
 * Release all DX managed resources.
 */
void TextureManager::deleteManagedObjects( )
{
	BW_GUARD;
	DEBUG_MSG( "Moo::TextureManager - Used texture memory %d\n", textureMemoryUsed() );
	releaseTextures();
}

/**
 * Create all DX managed resources.
 */
void TextureManager::createManagedObjects( )
{
	BW_GUARD;
	initTextures();
}

/**
 * This method returns total texture memory used in bytes.
 */
uint32 TextureManager::textureMemoryUsed( ) const
{
	BW_GUARD;
	const std::string animated( ".texanim" );

	TextureMap::const_iterator it = textures_.begin();
	TextureMap::const_iterator end = textures_.end();
	uint32 tm = 0;
	while( it != end )
	{
		const std::string& id = it->second->resourceID();
		if (id.length() < animated.length() ||
            id.substr( id.length() - animated.length(), animated.length() ) != animated )
		{
			tm += (it->second)->textureMemoryUsed( );
		}
		++it;
	}
	return tm;
}


/**
 *	Set whether or not we have a full house
 *	(and thus cannot load any new textures)
 */
void TextureManager::fullHouse( bool noMoreEntries )
{
	fullHouse_ = noMoreEntries;
}



/**
 *	This method saves a dds file from a texture.
 *
 *	@param texture the DX::BaseTexture to save out (important has to be 32bit colour depth.
 *	@param ddsName the filename to save the file to.
 *	@param format the TF_format to save the file in.
 *	@param	numDestMipLevels	Number of Mip levels to save in the DDS.
 *
 *	@return true if the function succeeds.
 */
bool TextureManager::writeDDS( DX::BaseTexture* texture, 
							  const std::string& ddsName, D3DFORMAT format, 
							  int numDestMipLevels )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( texture )
	{
		return false;
	}
	bool ret = false;

	// write out the dds
	D3DRESOURCETYPE resType = texture->GetType();
	if (resType == D3DRTYPE_TEXTURE)
	{
		DX::Texture* tSrc = (DX::Texture*)texture;

		TextureCompressor tc( tSrc, format, numDestMipLevels );

		ret = tc.save( ddsName );
	}

	return ret;
}


#define FNM_ENTRY( x ) fnm.insert( std::make_pair( #x, D3DFMT_##x ) );

typedef StringHashMap<D3DFORMAT> FormatNameMap;
/**
 *	This static function returns a map that converts from a format name
 *	into a D3DFORMAT enum value.
 */
static const FormatNameMap & formatNameMap()
{
	static FormatNameMap fnm;
	if (!fnm.empty()) return fnm;

	/* See d3d9types.h */
	FNM_ENTRY( UNKNOWN )

	FNM_ENTRY( R8G8B8 )
	FNM_ENTRY( A8R8G8B8 )
	FNM_ENTRY( X8R8G8B8 )
	FNM_ENTRY( R5G6B5 )
	FNM_ENTRY( X1R5G5B5 )
	FNM_ENTRY( A1R5G5B5 )
	FNM_ENTRY( A4R4G4B4 )
	FNM_ENTRY( R3G3B2 )
	FNM_ENTRY( A8 )
	FNM_ENTRY( A8R3G3B2 )
	FNM_ENTRY( X4R4G4B4 )
	FNM_ENTRY( A2B10G10R10 )
	FNM_ENTRY( G16R16 )

	FNM_ENTRY( A8P8 )
	FNM_ENTRY( P8 )

	FNM_ENTRY( L8 )
	FNM_ENTRY( A8L8 )
	FNM_ENTRY( A4L4 )

	FNM_ENTRY( V8U8 )
	FNM_ENTRY( L6V5U5 )
	FNM_ENTRY( X8L8V8U8 )
	FNM_ENTRY( Q8W8V8U8 )
	FNM_ENTRY( V16U16 )
//	FNM_ENTRY( W11V11U10 )
	FNM_ENTRY( A2W10V10U10 )

	FNM_ENTRY( UYVY )
	FNM_ENTRY( YUY2 )
	FNM_ENTRY( DXT1 )
	FNM_ENTRY( DXT2 )
	FNM_ENTRY( DXT3 )
	FNM_ENTRY( DXT4 )
	FNM_ENTRY( DXT5 )

	FNM_ENTRY( D16_LOCKABLE )
	FNM_ENTRY( D32 )
	FNM_ENTRY( D15S1 )
	FNM_ENTRY( D24S8 )
	FNM_ENTRY( D16 )
	FNM_ENTRY( D24X8 )
	FNM_ENTRY( D24X4S4 )


	FNM_ENTRY( VERTEXDATA )
	FNM_ENTRY( INDEX16 )
	FNM_ENTRY( INDEX32 )
	return fnm;
}

#define DEFAULT_TGA_FORMAT D3DFMT_DXT3
#define DEFAULT_BMP_FORMAT D3DFMT_DXT1


/**
 *	This method inits the texture detail levels used by the texture manager.
 *	If no section or an empty section is passed in, default detail levels will
 *	be used.
 *	@param pDetailLevels the datasection to initialise the detail levels from.
 */
void TextureManager::initDetailLevels( DataSectionPtr pDetailLevels, bool accumulate /*= false*/ )
{
	BW_GUARD;
	if ( !accumulate )
		detailLevels_.clear();

	if (pDetailLevels)
	{
		TextureDetailLevels::iterator it = detailLevels_.begin();

		std::vector< DataSectionPtr > pSections;
		pDetailLevels->openSections( "detailLevel", pSections );
		for (uint32 i = 0; i < pSections.size(); i++)
		{
			TextureDetailLevelPtr pDetail = new TextureDetailLevel;
			pDetail->init( pSections[i] );
			it = detailLevels_.insert( it, pDetail );
			it++;
		}
	}
	if (!detailLevels_.size())
	{
		TextureDetailLevelPtr pDetail = new TextureDetailLevel;
		pDetail->postfixes().push_back( "norms.bmp" );
		pDetail->postfixes().push_back( "norms.tga" );
		detailLevels_.push_back( pDetail );
		pDetail = new TextureDetailLevel;
		pDetail->format( D3DFMT_DXT1 );
		pDetail->postfixes().push_back( "bmp" );
		detailLevels_.push_back( pDetail );
		pDetail = new TextureDetailLevel;
		pDetail->format( D3DFMT_DXT3 );
		pDetail->postfixes().push_back( "tga" );
		detailLevels_.push_back( pDetail );
	}
}

/**
 *	Sets the texture format to be used when loading the specified texture.
 *	This format will override all information defined in a texformat file 
 *	or in the global texture_detail_levels file. Other fields, such as 
 *	formatCompressed, lodMode, will be set 
 *	to their default values. Call this function before loading the texture
 *	for the first time. The format will not be changed if the texture has
 *	already been loaded. 
 *
 *	Also, this function will not work if the texture has already been 
 *	converted into a DDS in the filesystem. For this reason, when using 
 *	this function, make sure this same format is used when batch converting 
 *	this texture map for deployment (in the absence of the original file,
 *	the format used to write the DDS will be the one used).
 *	
 *	@param	fileName	full pach to the texture whose format should be set
 *						set this will be used like the contains field of the 
 *						texture_detail_levels, potentially affecting more 
 *						files if the file name provided is not the fullpath.
 *
 *	@param	format		the texture format to be used.
 */
void TextureManager::setFormat( const std::string & fileName, D3DFORMAT format )
{
	BW_GUARD;
	TextureDetailLevelPtr pDetail = new TextureDetailLevel;
	pDetail->init(fileName, format);
	detailLevels_.insert(detailLevels_.begin(), pDetail);
}

/**
 *	This method reloads all the textures used from disk.
 */
void TextureManager::reloadAllTextures()
{
	BW_GUARD;
	TextureMap::iterator it = textures_.begin();
	TextureMap::iterator end = textures_.end();
	while (it != end)
	{
		BaseTexture *pTex = it->second;
		std::string resourceName = pTex->resourceID();
		if (BWResource::getExtension(resourceName) == "dds")
		{
			BWResource::instance().purge( pTex->resourceID() );
			resourceName = prepareResource( resourceName, true );			
		}		
		it++;
	}	
	
	it = textures_.begin();
	end = textures_.end();
	while (it != end)
	{
		BaseTexture *pTex = it->second;
		if ( pTex->resourceID() == s_notFoundBmp.value() )
		{
			pTex->reload( prepareResource( it->first ) );
		}
		else
		{
			pTex->reload();		
		}
		it++;
	}	
}

/**
 *	This method changes the current texture detail level of the game.
 *	First set the appropriate values in the appropriate xml file, then
 *	call this method at runtime to reload all textures.
 */
void TextureManager::recalculateDDSFiles()
{	
	BW_GUARD;
	//do this to force recalculation of dds files
	DataSectionPtr pSect = BWResource::openSection(s_textureDetailLevels);
	pSect->save();
	detailLevelsModTime_ = BWResource::modifiedTime(s_textureDetailLevels);
	initDetailLevels( pSect );
#ifdef EDITOR_ENABLED
	initDetailLevels(
		BWResource::openSection( s_toolsTextureDetail ),
		true /* accumulate */ );
#endif

	reloadAllTextures();

	//do this to force other objects to rebind to textures.
	//not needed, due to base texture reload method.
	//rc().changeMode( Moo::rc().modeIndex(), Moo::rc().windowed() );	
}


/**
 *	This static mehtod matches a detail level to the filename of a texture
 *	@param originalName the original name of the texture to get detail level info for.
 */
TextureDetailLevelPtr TextureManager::detailLevel( const std::string& originalName )
{
	BW_GUARD;
	TextureDetailLevelPtr pRet = new TextureDetailLevel;
	
	DataSectionPtr pFmtSect = BWResource::openSection(
		removeTextureExtension( originalName ) + ".texformat" );
	
	if (pFmtSect)
	{
		pRet->init( pFmtSect );
	}
	else
	{
		TextureDetailLevels& tdl = instance()->detailLevels_;
		TextureDetailLevels::iterator it = tdl.begin();
		while (it != tdl.end())
		{
			if ((*it)->check( originalName ))
			{
				return *it;
			}
			it++;
		}
	}
	return pRet;
}

/**
 *	This method converts a texture file to a .dds.
 *	If there is a .dds file there already, this method will get the 
 *	format from the current dds if not it will compress .tgas to dxt3 and .bmps to dxt1.
 *
 *	@param originalName the file to convert.
 *	@param ddsName the file to convert to.
 *	@param format	D3D surface format to use when performing the conversion.
 *
 *	@return true if the operation is successful.
 */
bool TextureManager::convertToDDS( 
	const std::string& originalName, 
	const std::string& ddsName,
	      D3DFORMAT    format)
{
	BW_GUARD;
	bool ret = false;
	if (rc().device())
	{
		BinaryPtr texture = BWResource::instance().rootSection()->readBinary( originalName );
		if (texture)
		{
			ComObjectWrap<DX::Texture> tex;

			TextureDetailLevelPtr pDetailLevel = TextureManager::detailLevel( originalName );

			if (pDetailLevel->mipCount() == 0)
			{
				// Create the texture and the mipmaps.
				tex = Moo::rc().createTextureFromFileInMemoryEx( 
					texture->data(), texture->len(),
					D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8,
					D3DPOOL_SYSTEMMEM, D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 
					D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 0, NULL, NULL );

                if (tex)
                {	               
					ret = writeDDS( tex.pComObject(), ddsName, format );

				    TRACE_MSG( "TextureManager : write DDS %s\n", ddsName.c_str() );
                }
			}
			else
			{
				tex = Moo::rc().createTextureFromFileInMemoryEx( 
					texture->data(), texture->len(),
					D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_A8R8G8B8,
					D3DPOOL_SCRATCH, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL );
					
				if (tex)
				{
					D3DSURFACE_DESC desc;
					tex->GetLevelDesc( 0, &desc );
					
					uint32 width = desc.Width;
					uint32 height = desc.Height;
					uint32 newWidth = width;
					uint32 newHeight = height;
					uint32 mipBase = pDetailLevel->mipSize();

					if (pDetailLevel->horizontalMips())
					{
						newWidth = mipBase != 0 
							? mipBase 
							: newWidth / 2;
						mipBase = newWidth;
					}
					else
					{
						newHeight = mipBase != 0 
							? mipBase 
							: newHeight / 2;
						mipBase = newHeight ;
					}

                    uint32 realMipCount = 0;
					uint32 levelWidth = newWidth;
					uint32 levelHeight = newHeight;
					uint32 levelOffset = 0;
					uint32 levelSize = mipBase;
					uint32 levelMax = pDetailLevel->horizontalMips() ? width : height;
					uint32 mipCount = pDetailLevel->mipCount() > 0 ?  pDetailLevel->mipCount() : 0xffffffff;
					while (levelWidth > 0 &&
						levelHeight > 0 &&
						realMipCount < pDetailLevel->mipCount() &&
						(levelSize + levelOffset) <= levelMax)
					{
						realMipCount++;
						levelHeight = levelHeight >> 1;
						levelWidth = levelWidth >> 1;
						levelOffset += levelSize;
						levelSize = levelSize >> 1;
					}
					
					ComObjectWrap<DX::Texture> newTexture;
					newTexture = rc().createTexture( newWidth, newHeight, realMipCount, 0, D3DFMT_A8R8G8B8,
						D3DPOOL_SCRATCH );
					if (newTexture)
					{
						uint32 levelWidth = newWidth;
						uint32 levelHeight = newHeight;
						uint32 levelBase = 0;
						bool horizontal = pDetailLevel->horizontalMips();

						DX::Surface* pSurfSrc = NULL;
						tex->GetSurfaceLevel(0, &pSurfSrc);
						RECT srcRect;
						for (uint32 iLevel = 0; iLevel < realMipCount; iLevel++)
						{
							srcRect.top = horizontal ? 0 : levelBase;
							srcRect.bottom = srcRect.top + levelHeight;
							srcRect.left = horizontal ? levelBase : 0;
							srcRect.right = srcRect.left + levelWidth;
							levelBase += mipBase;
							levelWidth = levelWidth >> 1;
							levelHeight = levelHeight >> 1;
							mipBase = mipBase >> 1;
							DX::Surface* pSurfDest = NULL;
							newTexture->GetSurfaceLevel(iLevel, &pSurfDest);
							D3DXLoadSurfaceFromSurface(pSurfDest, NULL, NULL,
								pSurfSrc, NULL, &srcRect, D3DX_FILTER_NONE, 0);
							pSurfDest->Release();
						}
						pSurfSrc->Release();
						ret = writeDDS( newTexture.pComObject(), ddsName, format, realMipCount );
						TRACE_MSG( "TextureManager : write DDS %s\n", ddsName.c_str() );
					}
				}
			}
		}
	}
	return ret;
}


/**
 *	This method gets the given texture resource.
 *
 *  @param name				The name of the resource texture to get.
 *  @param allowAnimation	Can the texture be animated?
 *  @param mustExist		Must the texture exist?
 *  @param loadIfMissing	Load the texture if missing?
 */
BaseTexturePtr TextureManager::get( const std::string& name,
	bool allowAnimation, bool mustExist, bool loadIfMissing,
	const std::string& allocator)
{
	BW_GUARD;
	#define DISABLE_TEXTURING 0
	#if DISABLE_TEXTURING

	std::string saniName      = prepareResourceName(name);
	TextureDetailLevelPtr lod = TextureManager::detailLevel(saniName);
	D3DFORMAT format          = lod->format();

	typedef std::map<D3DFORMAT, BaseTexturePtr> BaseTextureMap;
	static BaseTextureMap s_debugTextureMap;
	BaseTextureMap::const_iterator textIt = s_debugTextureMap.find(format);
	if (textIt != s_debugTextureMap.end())
	{
		return textIt->second;
	}
	#endif // DISABLE_TEXTURING

	BaseTexturePtr res = NULL;

	if (name.length() > 3)
	{
		if (allowAnimation)
		{
			res = this->find( removeTextureExtension( name ) + ".texanim" );
			if (res) return res;
		}
	}
	
	// check the cache directly
	res = this->find( name );
	if (res) return res;

#ifdef EDITOR_ENABLED
	std::string nameFound = prepareResourceName( name );
#endif // EDITOR_ENABLED

	// try it as an already loaded dds then
	std::string ddsName = removeTextureExtension( name ) + ".dds";
	res = this->find( ddsName );
	if (res)
	{
#ifdef EDITOR_ENABLED
		checkLoadedResourceName( name, nameFound ); 
#endif // EDITOR_ENABLED

		return res;
	}

	if (!loadIfMissing)
	{
		return NULL;
	}

	// if couldn't create the dds for this resource last time, then the
	// original resource should be in the cache already
#ifndef EDITOR_ENABLED
	std::string nameFound = prepareResourceName( name );
#endif // !EDITOR_ENABLED

	res = this->find( nameFound );
	if (res)
	{
#ifdef EDITOR_ENABLED
		checkLoadedResourceName( name, nameFound ); 
#endif // EDITOR_ENABLED
		return res;
	}

	std::string resourceName = prepareResource(name);

	// alright, we're ready to load something now

	// make sure we're allowed to load more stuff
	if (fullHouse_)
	{
		CRITICAL_MSG( "TextureManager::getTextureResource: "
			"Loading the texture '%s' into a full house!\n",
			resourceName.c_str() );
	}

	BaseTexturePtr result = NULL;
	result = this->getUnique(name, resourceName, allowAnimation, mustExist, allocator);		

#if !CONSUMER_CLIENT_BUILD
	if (result)
	{
		if (BWResource::getExtension( name ) == "dds")
		{
			// requested a DDS, so just make sure we loaded the DDS indeed.
			checkLoadedResourceName( name, resourceName ); 
		}
		else if (BWResource::getExtension( name ) != "texanim")
		{
			// requested a non-DDS, check as usual.
			checkLoadedResourceName( name, nameFound ); 
		}
	}
#endif // !CONSUMER_CLIENT_BUILD

	#if DISABLE_TEXTURING
		s_debugTextureMap.insert(std::make_pair(result->format(), result));
	#endif // DISABLE_TEXTURING

	return result;
}

/**
 *	This method gets the given texture resource from a DataSection.
 *
 *  @param data				The DataSection to get the resource from.
 *  @param name				The name of the resource texture to get.
 *  @param mustExist		Must the texture exist?
 *  @param loadIfMissing	Load the texture if missing?
 *  @param refresh			Refresh the texture from the data section even if
 *							already in the cache?
 *	@param allocator		This parameter is unused.
 */
BaseTexturePtr TextureManager::get( DataSectionPtr data, 
	const std::string& name, bool mustExist, bool loadIfMissing, bool refresh,
	const std::string& allocator)
{
	BW_GUARD;
	BaseTexturePtr res;
	
	// Find the value in the cache, and if we are refreshing remove from the
	// cache.  If in the cache and not refreshing then return the cached value:
	res = find(name);
	if (res != NULL && refresh)
	{
		delInternal(res.getObject());
		res = NULL;
	}
	if (res != NULL)
		return res;

	// Make sure that we're allowed to load more stuff:
	if (fullHouse_)
	{
		CRITICAL_MSG( "TextureManager::getTextureResource: "
			"Loading the texture '%s' into a full house!\n",
			name.c_str() );
	}

	// Try loading the texture and add it to the cache:
	ManagedTexture *manTex = new ManagedTexture(data, name, mustExist);
	if (manTex->valid())
	{
		res = manTex;
		addInternal(manTex);
	}
	else
	{
		delete manTex; manTex = NULL;
	}
		
	return res;
}

/**
 *	This method gets a unique copy of a texture resource
 *	@param resourceID the resourceID of the texture
 *	@param sanitisedResourceID the optional sanitised resource id,
 *		the sanitised resource id is the resource id that is used 
 *		for loading the texture (i.e. the .dds version of the resource)
 *  @param allowAnimation can the texture be animated?
 *  @param mustExist must the texture exist?
 *	@param allocator the owner of this texture
 *	@return pointer to the loaded texture
 */
BaseTexturePtr TextureManager::getUnique(
	const std::string& resourceID, const std::string& sanitisedResourceID, 
	bool allowAnimation, bool mustExist, const std::string& allocator )
{

	BW_GUARD;
	// Make sure we have a proper resource id to load
	std::string sanitisedID = 
		sanitisedResourceID.length() ? sanitisedResourceID : resourceID;

	BaseTexturePtr res = NULL;

	// load the texture
	MEM_TRACE_BEGIN( sanitisedResourceName )
	TextureDetailLevelPtr lod = TextureManager::detailLevel(resourceID);

	if (allowAnimation)
	{
		std::string animTextureName = removeTextureExtension( sanitisedID ) + ".texanim";
		DataSectionPtr animSect = BWResource::instance().openSection(animTextureName);
		if (animSect.exists())
		{
			res = new AnimatingTexture( animTextureName );
			this->addInternal( &*res );
		}
	}
	if (!res.exists())
	{
		int lodMode = lod->lodMode();
		int qualitySetting = this->qualitySettings_->activeOption();
		ManagedTexture * pMT = new ManagedTexture(
			sanitisedID, mustExist, 
			calcMipSkip(qualitySetting, lodMode),
			lod->noResize(), lod->noFilter(), allocator );
			
		if (pMT->valid())
		{
			res = pMT;

			if ( res->resourceID() == s_notFoundBmp.value() )
				this->addInternal( pMT, sanitisedID );
			else
				this->addInternal( pMT );
		}
		else
		{
			delete pMT;
		}
	}

	MEM_TRACE_END()
	return res;
}

/** 
 *	This method does a quick check to see if the resource is a texture file.
 *	It does this by checking that the file exists and has a valid texture
 *	extension.
 *
 *	@param resourceID	The name of the texture to test.
 *
 *	@return				True if the resource is a texture, false otherwise.
 */
bool TextureManager::isTextureFile(const std::string & resourceID) const
{
	BW_GUARD;
	// The file must exist:
	if (!BWResource::fileExists(resourceID))
		return false;
	
	// Only recognised extensions are textures:
	std::string extension = toLower(BWResource::getExtension(resourceID));
	for (size_t i = 0; textureExts[i] != NULL; ++i)
	{
		if (extension == textureExts[i])
			return true;
	}

	// Extension of "texanim" is a special case:
	if (extension == "texanim")
		return true;

	return false;
}

/**
 *	This method loads a texture into a system memory resource.
 *	@param resourceID the name of the texture in the res tree.
 *	@return the pointer to the system memory base texture.
 */
BaseTexturePtr	TextureManager::getSystemMemoryTexture( const std::string& resourceID )
{
	BW_GUARD;
	std::string resourceName = toLower( resourceID );
	resourceName = prepareResource( resourceName );
	SysMemTexture* sysTexture = new SysMemTexture( resourceName );
    
	BaseTexturePtr pResult;
    if (!sysTexture->failedToLoad())
	{
		pResult = sysTexture;
	}
	else
	{
		delete sysTexture;
	}
	return pResult;
}

std::string TextureManager::prepareResourceName( const std::string& resourceName )
{
	BW_GUARD;	
#if ENABLE_DDS_GENERATION

	std::string ext      = BWResource::getExtension(resourceName);
	std::string baseName = removeTextureExtension(resourceName);
	std::string lext	 = ext;
	
	_strlwr( const_cast<char*>( lext.c_str() ) );

	if (lext == "texanim") 
	{
		lext = "dds";
	}

	// If the source texture is a dds or the source texture can not be found
	// Check if one with a different extension can be found
	if (lext == "dds" || !BWResource::fileExists(resourceName))
	{
		for (uint32 i = 0; textureExts[i] != NULL; i++)
		{
			if (strcmp(textureExts[i], "dds") != 0)
			{
				std::string fileName = baseName + "." + textureExts[i];
#if ENABLE_FILE_CASE_CHECKING
				// This ensures that the extension is in the correct case
				// the rest of the filename is discarded in this scope.
				fileName = BWResource::correctCaseOfPath(fileName);
#endif
				if (BWResource::fileExists(fileName))
				{
					ext = BWResource::getExtension(fileName);
					break;
				}
			}
		}
	}

	return baseName + "." + ext;

#else

	if (resourceName.rfind("dds") == resourceName.size() - 3)
		return resourceName;
	TextureDetailLevelPtr pDetailLevel = TextureManager::detailLevel(resourceName);
	if (pDetailLevel->noResize())
		return resourceName;
	else if (pDetailLevel->formatCompressed())
		return BWResource::removeExtension(resourceName) + ".c.dds";
	else
		return BWResource::removeExtension(resourceName) + ".dds";

#endif // ENABLE_DDS_GENERATION
}

std::string TextureManager::prepareResource( const std::string& resourceName, bool forceConvert /*= false*/ )
{
	BW_GUARD;	
#if ENABLE_DDS_GENERATION

	// we haven't loaded this texture yet.
	// 1) if the extension is dds we need to find the source texture
	//    else use the given resource.
	// 2) find the source file and check if the dds needs to be regenerated
	std::string baseName      = removeTextureExtension(resourceName);	
	std::string resName       = prepareResourceName( resourceName );
	std::string ext           = BWResource::getExtension(resName);
	TextureDetailLevelPtr lod = TextureManager::detailLevel(resName);

	if ( lod->noResize() )
		return resName;

	D3DFORMAT comprFormat     = lod->format();
	std::string comprSuffix   = "";
	if(isCompressed(lod))
	{
		comprSuffix = ".c";
		comprFormat = lod->formatCompressed();
	}

	std::string ddsName = baseName + comprSuffix + ".dds";
	std::string bakName = baseName + ".dds";
	
	std::string result;
	if (ext == "dds")
	{
		// if no original file was found for this texture map,
		// look for the dds with the correct compression settings. 
		// If this can't be found, use the plain dds.
		result = BWResource::fileExists(ddsName)
			? ddsName
			: bakName;
	}
	else if (ext == "texanim")
	{
		result = resName;
	}
	else
	{
		// if the original file is present, regenerate the dds if
		// needed (with the correct compression settings) and use it.
		std::string tfmName = baseName + ".texformat";
		
		// check if there is a .dds texture with the same name 
		// as the texture we are trying to load. if there is 
		// check  the timestamp of it, if it's older, reconvert. 
		// i.e.: if the dds doesn't exist or both (the res exists 
		// and it is not older by 15min or less than the dds)

		// update DDS if the 
		// dds file does not exist
		uint64 ddsTime = BWResource::modifiedTime( ddsName );
		bool needsUpdate = (ddsTime == 0);
		
		if (!needsUpdate)
		{
			// ok, update DDS if the original resource has been modified
			uint64 resTime = BWResource::modifiedTime( resName );
			if (resTime != 0)
			{
				needsUpdate = (ddsTime < resTime);
			
				if (!needsUpdate)
				{
					// ok, update DDS if the texture detail file has been modified
					if (detailLevelsModTime_ != 0) 
					{
						needsUpdate = (ddsTime < detailLevelsModTime_);
					}
					
					if (!needsUpdate)
					{
						// ok, update DDS if the .texformat file has been modified
						uint64 tfmTime = BWResource::modifiedTime( tfmName );
						if (tfmTime !=0) 
						{
							needsUpdate = (ddsTime < tfmTime);
						}
					}					
				}
			}
		}		

		if (forceConvert || needsUpdate)
		{
			// it does not exist or is not up 
			// to date, so convert it to a dds
			if (BWResource::fileExists(resName))
			{
				result = convertToDDS( resName, ddsName, comprFormat )
					? ddsName
					: resName;
			}
			else
			{
				// but original file does not exists will have to 
				// do with the fallback option (the plain old dds)
				result = bakName;
			}
		}
		else
		{
			result = ddsName;
		}
	}

	return result;

#else

	return prepareResourceName( resourceName );

#endif // ENABLE_DDS_GENERATION
}


void TextureManager::checkLoadedResourceName( const std::string & requested,
											  const std::string & found ) const
{
	if (requested != found)
	{
		WARNING_MSG( "Texture '%s'%s was not found, using '%s' instead.\n",
			requested.c_str(),
			ResourceLoadContext::formattedRequesterString().c_str(),
			found.c_str() );
	}
}


void TextureManager::del( BaseTexture * pTexture )
{
	BW_GUARD;
	if (pInstance_)
		pInstance_->delInternal( pTexture );
}


void TextureManager::add( BaseTexture * pTexture )
{
	BW_GUARD;
	if (pInstance_)
		pInstance_->addInternal( pTexture );
}


/**
 *	Add this texture to the map of those already loaded
 */
void TextureManager::addInternal( BaseTexture * pTexture, std::string resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( texturesLock_ );	

	if ( !resourceID.empty() )
	{
		if ( textures_.find( resourceID ) != textures_.end() )
		{
			WARNING_MSG( "TextureManager::addInternal - replacing %s\n", resourceID.c_str() );
		}
		textures_.insert( std::make_pair(resourceID, pTexture ) );
	}
	else
	{
		if ( textures_.find( pTexture->resourceID() ) != textures_.end() )
		{
			WARNING_MSG( "TextureManager::addInternal - replacing %s\n", pTexture->resourceID().c_str() );
		}
		textures_.insert( std::make_pair(pTexture->resourceID(), pTexture ) );
	}
}


/**
 *	Remove this texture from the map of those already loaded
 */
void TextureManager::delInternal( BaseTexture * pTexture )
{
	BW_GUARD;
	SimpleMutexHolder smh( texturesLock_ );

	for (TextureMap::iterator it = textures_.begin(); it != textures_.end(); it++)
	{
		if (it->second == pTexture)
		{
			//DEBUG_MSG( "TextureManager::del: %s\n", pTexture->resourceID().c_str() );
			textures_.erase( it );
			return;
		}
	}

	INFO_MSG( "TextureManager::del: "
		"Could not find texture '%s' at 0x%08X to delete it\n",
		pTexture->resourceID().c_str(), pTexture );
}



/**
 *	This is a helper class used to find a texture with the given name
 */
class FindTexture
{
public:
	FindTexture( const std::string& resourceID ) : resourceID_( resourceID ) { }
	~FindTexture() { }

	bool operator () ( BaseTexture * pTexture )
		{ return pTexture->resourceID() == resourceID_; }

	std::string resourceID_;
};


/**
 *	Find this texture in the map of those already loaded
 */
BaseTexturePtr TextureManager::find( const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( texturesLock_ );

	TextureMap::iterator it;

	it = textures_.find( resourceID );
	if (it == textures_.end())
		return NULL;

	return BaseTexturePtr( it->second, BaseTexturePtr::FALLIBLE );
	// if ref count was zero then someone is blocked on our
	// texturesLock_ mutex waiting to delete it, so we return NULL
}

/**
 * This function returns true if the texture is compressed
 */
bool TextureManager::isCompressed(TextureDetailLevelPtr lod)
{
	int comprSettings = this->compressionSettings_->activeOption();
	return comprSettings == 0 && lod->formatCompressed() != D3DFMT_UNKNOWN;
}

/**
 *	Sets the texture quality setting. Implicitly called 
 *	whenever the user changes the TEXTURE_QUALITY setting.
 *
 *	Texture quality can vary from 0 (highest) to 2 (lowest). Lowering texture
 *	quality is done by skipping the topmost mipmap levels (normally two in the 
 *	lowest quality setting, one in the medium and none in the highest). 
 *
 *	Artists can tweak how each texture responds to the quality setting using the 
 *	<lodMode> field in the ".texformat" file or global texture_detail_level.xml. 
 *	The table bellow shows the number of lods skipped according to the texture's
 *	lodMode and the current quality setting.
 *
 *					texture quality setting
 *	lodMode			0		1		2
 *	0 (disabled)	0		0		0
 *	1 (normal)		0		1		2
 *	2 (low bias)	0		1		1
 *	3 (high bias)	0		0		1
 */
void TextureManager::setQualityOption(int optionIndex)
{
	BW_GUARD;
	TextureMap::iterator it  = textures_.begin();
	TextureMap::iterator end = textures_.end();
	while (it != end)
	{
		BaseTexture *pTex = it->second;
		TextureDetailLevelPtr lod = TextureManager::detailLevel(pTex->resourceID());
		int lodMode    = lod->lodMode();
		int newMipSkip = calcMipSkip(optionIndex, lodMode);

		// check if texture is compressed
		std::string baseName      = removeTextureExtension( pTex->resourceID() );
		std::string comprSuffix   = "";
		if (isCompressed(lod))
		{
			comprSuffix = ".c";
		}
		std::string ddsName = baseName + comprSuffix + ".dds";
		
		if (lodMode != 2 && newMipSkip != pTex->mipSkip())
		{
			pTex->mipSkip(newMipSkip);
			this->dirtyTextures_.push_back(
				std::make_pair(pTex, ddsName));
		}
		it++;
	}		

	int pendingOption = 0;
	if (!Moo::GraphicsSetting::isPending(
			this->compressionSettings_, pendingOption))
	{
		this->reloadDirtyTextures();
	}
}

/**
 *	Sets the texture compression setting. Implicitly called 
 *	whenever the user changes the TEXTURE_COMPRESSION setting.
 *
 *	Only textures with <formatCompressed> specified in their ".texformat" 
 *	or the global texture_detail_level.xml file. The format specified in 
 *	formatCompressed will be used whenever texture compressio is enabled. 
 *	If texture compression is disabled or formatCompressed has not been 
 *	specified, <format> will be used.
 */
void TextureManager::setCompressionOption(int optionIndex)
{
	BW_GUARD;
	TextureMap::iterator it  = textures_.begin();
	TextureMap::iterator end = textures_.end();
	while (it != end)
	{
		BaseTexture *pTex = it->second;
		std::string resourceName = pTex->resourceID();
		TextureDetailLevelPtr lod = TextureManager::detailLevel(resourceName);
		if (lod->formatCompressed() != D3DFMT_UNKNOWN &&
			BWResource::getExtension(resourceName) == "dds")
		{
			resourceName = prepareResource(resourceName);			
			this->dirtyTextures_.push_back(std::make_pair(pTex, resourceName));
		}		
		it++;
	}
	
	int pendingOption = 0;
	if (!Moo::GraphicsSetting::isPending(
			this->qualitySettings_, pendingOption))
	{
		this->reloadDirtyTextures();
	}
}

/**
 *	Triggered by the graphics settings registry when the user changes 
 *	the active TEXTURE_FILTER option. This method is left empty as
 *	we get the option from the actual graphics setting object.
 */
void TextureManager::setFilterOption(int)
{}

/**
 *	Returns the currently selected mipmapping filter. 
 *	This is defined by the active TEXTURE_FILTER option.
 */
int TextureManager::configurableMipFilter() const
{
	BW_GUARD;
	static D3DTEXTUREFILTERTYPE s_mipFilters[] = {
		D3DTEXF_LINEAR, D3DTEXF_LINEAR,
		D3DTEXF_LINEAR, D3DTEXF_LINEAR,
		D3DTEXF_LINEAR, D3DTEXF_POINT, D3DTEXF_NONE};
		
	return s_mipFilters[this->filterSettings_->activeOption()];
}

/**
 *	Returns the currently selected min/mag mapping filter. 
 *	This is defined by the active TEXTURE_FILTER option.
 */
int TextureManager::configurableMinMagFilter() const
{
	BW_GUARD;
	static D3DTEXTUREFILTERTYPE s_minMagFilters[] = {
		D3DTEXF_ANISOTROPIC, D3DTEXF_ANISOTROPIC,
		D3DTEXF_ANISOTROPIC, D3DTEXF_ANISOTROPIC,
		D3DTEXF_LINEAR, D3DTEXF_POINT, D3DTEXF_POINT};
		
	return s_minMagFilters[this->filterSettings_->activeOption()];
}

/**
 *	Returns the currently selected max anisotropy value. 
 *	This is defined by the active TEXTURE_FILTER option.
 */
int TextureManager::configurableMaxAnisotropy() const
{
	BW_GUARD;
	static int s_maxAnisotropy[] = {16, 8, 4, 2, 1, 1, 1};
	return s_maxAnisotropy[this->filterSettings_->activeOption()];
}

/**
 *	Reloads all textures in the dirty textures list. Textures are 
 *	added to the dirty list whenever the texture settings change,
 *	either quality or compression. 
 */
void TextureManager::reloadDirtyTextures()
{
	BW_GUARD;
	TextureStringVector::const_iterator texIt  = this->dirtyTextures_.begin();
	TextureStringVector::const_iterator texEnd = this->dirtyTextures_.end();
	while (texIt != texEnd)
	{
		this->delInternal(texIt->first);
		texIt->first->reload(texIt->second);
		this->addInternal(texIt->first);
		++texIt;
	}
	this->dirtyTextures_.clear();
}

/**
 * Returns true if string starts with substring.
 */
bool startsWith( const std::string& string, const std::string& substring )
{
	BW_GUARD;
	bool res = false;
	std::string::size_type length = substring.length();
	if (string.length() >= length)
	{
		res = substring == string.substr( 0 , length );
	}
	return res;
}

/**
 * Returns true if string ends with substring.
 */
bool endsWith( const std::string& string, const std::string& substring )
{
	BW_GUARD;
	bool res = false;
	std::string::size_type length = substring.length();
	std::string::size_type length2 = string.length();

	if (length2 >= length)
	{
		res = substring == string.substr( length2 - length, length );
	}
	return res;
}

/**
 * This function wraps std::string.find()
 */
bool containsString( const std::string& string, const std::string& substring )
{
	BW_GUARD;
	return string.find( substring ) != std::string::npos;
}

/**
 * A function templated on checker type.
 */
template<class Checker>
bool stringCheck( const std::string& mainString, 
				 const TextureDetailLevel::StringVector& subStrings,
				Checker checker )
{
	BW_GUARD;
	bool ret = !subStrings.size();
	TextureDetailLevel::StringVector::const_iterator it = subStrings.begin();
	while (it != subStrings.end() && ret == false)
	{
		if(checker( mainString, *it ))
			ret = true;
		it++;
	}
	return ret;
}

/**
 * Default constructor.
 */
TextureDetailLevel::TextureDetailLevel() : 
	format_( D3DFMT_A8R8G8B8 ),
	formatCompressed_( D3DFMT_UNKNOWN ),
	lodMode_( 0 ),
	mipCount_( 0 ),
	mipSize_( 0 ),
	horizontalMips_( false ),
	noResize_( false ),
	noFilter_( false )
{

}

/**
 * Destructor
 */
TextureDetailLevel::~TextureDetailLevel()
{

}

/**
 * Clear this object.
 */
void TextureDetailLevel::clear( )
{
	prefixes_.clear();
	postfixes_.clear();
	contains_.clear();
	lodMode_ = 0;
	format_ = D3DFMT_A8R8G8B8;
	formatCompressed_ = D3DFMT_UNKNOWN;
	mipCount_ = 0;
	mipSize_ = 0;
	horizontalMips_ = false;
	noResize_ = false;
	noFilter_ = false;
}


/**
 *	This method loads the matching and conversion criteria from
 *	a data section.
 */
void TextureDetailLevel::init( DataSectionPtr pSection )
{
	BW_GUARD;
	pSection->readStrings( "prefix", prefixes_ );
	pSection->readStrings( "postfix", postfixes_ );
	pSection->readStrings( "contains", contains_ );
	lodMode_ = pSection->readInt( "lodMode", lodMode_ );
	mipCount_ = pSection->readInt( "mipCount", mipCount_ );
	horizontalMips_ = pSection->readBool( "horizontalMips", horizontalMips_ );
	noResize_ = pSection->readBool( "noResize", noResize_ );	
	noFilter_ = pSection->readBool( "noFilter", noFilter_ );	
	mipSize_ = pSection->readInt( "mipSize", mipSize_ );
	
	std::string formatName = "A8R8G8B8";
	const FormatNameMap & fnm = formatNameMap();
	for (FormatNameMap::const_iterator fnmIt = fnm.begin(); fnmIt != fnm.end(); fnmIt++)
	{
		if (fnmIt->second == format_) formatName = fnmIt->first;
	}
	formatName = pSection->readString( "format", formatName );
	FormatNameMap::const_iterator it = fnm.find( formatName );
	if (it != fnm.end())
		format_ = it->second;

	formatName = "UNKNOWN";
	formatName = pSection->readString( "formatCompressed", formatName );
	it = fnm.find( formatName );
	if (it != fnm.end())
	{
		formatCompressed_ = it->second;
	}
}

/**
 *	Initialises this TextureDetailLevel object, matching the given file name to the
 *	provided texture format. All other parameters are left with their default values.
 */
void TextureDetailLevel::init( const std::string & fileName, D3DFORMAT format )
{
	BW_GUARD;
	contains_.push_back(fileName);
	format_ = format;
}

/**
 *	This method checks to see if a resource name matches this detail level
 *	@param resourceID the resourceID to check
 *	@return true if this detail level matches the resourceID
 */
bool TextureDetailLevel::check( const std::string& resourceID )
{
	BW_GUARD;
	bool match = false;
	match = stringCheck( resourceID, prefixes_, startsWith );
	if (match)
		match = stringCheck( resourceID, postfixes_, endsWith );
	if (match)
		match = stringCheck( resourceID, contains_, containsString );
	return match;
}

/**
 * Initialise the manager.
 */
void TextureManager::init()
{
	BW_GUARD;
	pInstance_ = new TextureManager;
}

/**
 * Finalise the manager.
 */
void TextureManager::fini()
{
	BW_GUARD;
	delete pInstance_;
	pInstance_ = NULL;
}


/**
 *	This gets a NULL-terminated list of valid textures.
 *
 *	@return			A NULL-terminated list of valid texture extensions.
 */
/*static*/ const char ** TextureManager::validTextureExtensions()
{
	return textureExts;
}


TextureManager* TextureManager::pInstance_ = NULL;

}
