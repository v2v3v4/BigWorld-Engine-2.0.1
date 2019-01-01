/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP


#include <list>
#include "cstdmf/concurrency.hpp"
#include "cstdmf/smartpointer.hpp"
#include "resmgr/datasection.hpp"
#include "device_callback.hpp"
#include "moo_dx.hpp"
#include "base_texture.hpp"
#include "graphics_settings.hpp"


namespace Moo
{

class BaseTexture;
typedef SmartPointer<BaseTexture> BaseTexturePtr;

class TextureDetailLevel;
typedef SmartPointer< TextureDetailLevel > TextureDetailLevelPtr;
typedef std::list< TextureDetailLevelPtr > TextureDetailLevels;


/**
 *	This singleton class keeps track of and loads ManagedTextures.
 */
class TextureManager : public Moo::DeviceCallback
{
public:
	typedef void (*ProgessFunc)(float);
	typedef std::map< std::string, BaseTexture * > TextureMap;

	~TextureManager();

	static TextureManager*	instance();

	static const std::string & notFoundBmp();

	HRESULT					initTextures( );
	HRESULT					releaseTextures( );

	void					deleteManagedObjects( );
	void					createManagedObjects( );

	uint32					textureMemoryUsed( ) const;
	
	void					fullHouse( bool noMoreEntries = true );

	static bool				writeDDS( DX::BaseTexture* texture, 
								const std::string& ddsName, D3DFORMAT format, int numDestMipLevels = 0 );
	static TextureDetailLevelPtr detailLevel( const std::string& originalName );
	static bool				convertToDDS( 
								const std::string& originalName, 
								const std::string& ddsName,
								      D3DFORMAT    format );
	void					initDetailLevels( DataSectionPtr pDetailLevels, bool accumulate = false );
	void					reloadAllTextures();
	void					recalculateDDSFiles();
	void					reloadMipMaps(ProgessFunc progFunc);
	void					setFormat( const std::string & fileName, D3DFORMAT format );


	BaseTexturePtr	get( const std::string & resourceID,
		bool allowAnimation = true, bool mustExist = true,
		bool loadIfMissing = true, const std::string& allocator = "texture/unknown texture" );
	BaseTexturePtr	get( DataSectionPtr data, const std::string & resourceID,
		bool mustExist = true, bool loadIfMissing = true, bool refresh = true,
		const std::string& allocator = "texture/unknown texture" );
	BaseTexturePtr	getUnique( 
		const std::string& resourceID,
		const std::string & sanitisedResourceID = "",
		bool allowAnimation = true, bool mustExist = true,
		const std::string& allocator = "texture/unknown texture");
	bool isTextureFile( const std::string & resourceID ) const;

	BaseTexturePtr	getSystemMemoryTexture( const std::string& resourceID );
	
	int configurableMipFilter() const;
	int configurableMinMagFilter() const;
	int configurableMaxAnisotropy() const;

	const TextureMap&		textureMap() const { return textures_; };

	static void init();
	static void fini();

	static const char ** validTextureExtensions();

private:
	TextureManager();
	TextureManager(const TextureManager&);
	TextureManager& operator=(const TextureManager&);

	static void add( BaseTexture * pTexture );
	static void del( BaseTexture * pTexture );
	void addInternal( BaseTexture * pTexture, std::string resourceID = "" );
	void delInternal( BaseTexture * pTexture );

	std::string prepareResourceName( const std::string& resourceName );
	std::string prepareResource( const std::string& resourceName, bool forceConvert = false );

	void checkLoadedResourceName( const std::string & requested, const std::string & found ) const;

	BaseTexturePtr find( const std::string & resourceID );
	void setQualityOption(int optionIndex);
	void setCompressionOption(int optionIndex);
	void setFilterOption(int);
	void reloadDirtyTextures();
	bool isCompressed(TextureDetailLevelPtr lod);

	TextureMap				textures_;
	SimpleMutex				texturesLock_;

    TextureDetailLevels		detailLevels_;

	bool					fullHouse_;
	uint64					detailLevelsModTime_;
	int						lodMode_;

	GraphicsSetting::GraphicsSettingPtr		qualitySettings_;
	GraphicsSetting::GraphicsSettingPtr		compressionSettings_;
	GraphicsSetting::GraphicsSettingPtr		filterSettings_;

	typedef std::pair<BaseTexture *, std::string> TextureStringPair;
	typedef std::vector<TextureStringPair> TextureStringVector;
	TextureStringVector dirtyTextures_;

	friend BaseTexture;
	static TextureManager*	pInstance_;
};

/**
 * This class represents the global texture detail filters. Each detail level
 * specifies a string that texture should match to have that detail level.
 * See bigworld\res\system\data\texture_detail_levels.xml for example detail
 * levels.
 */
class TextureDetailLevel : public SafeReferenceCount
{
public:
	TextureDetailLevel();
	~TextureDetailLevel();
	void		init( DataSectionPtr pSection );
	void		init( const std::string & fileName, D3DFORMAT format );
	bool		check( const std::string& resourceID );

	void		clear();

	D3DFORMAT	format() const { return format_; }
	void		format( D3DFORMAT format ) { format_ = format; }
	D3DFORMAT	formatCompressed() const { return formatCompressed_; }
	void		formatCompressed( D3DFORMAT formatCompressed ) { formatCompressed_ = formatCompressed; }
	uint32		lodMode() const { return lodMode_; }
	void		lodMode( uint32 lodMode ) { lodMode_ = lodMode; }
	void		mipCount( uint32 count ) { mipCount_ = count; }
	uint32		mipCount() const { return mipCount_; }
	void		mipSize( uint32 size ) { mipSize_ = size; }
	uint32		mipSize() const { return mipSize_; }
	void		horizontalMips( bool val ) { horizontalMips_ = val; }
	bool		horizontalMips( ) const { return horizontalMips_; }
	void		noResize( bool val ) { noResize_ = val; }
	bool		noResize( ) const { return noResize_; }
	void		noFilter( bool val ) { noFilter_ = val; }
	bool		noFilter( ) const { return noFilter_; }

	typedef std::vector< std::string > StringVector;

	StringVector& postfixes() { return postfixes_; }
	StringVector& prefixes() { return prefixes_; }
	StringVector& contains() { return contains_; }
private:

	StringVector	prefixes_;
	StringVector	postfixes_;
	StringVector	contains_;
	uint32			lodMode_;
	uint32			mipCount_;
	uint32			mipSize_;
	bool			horizontalMips_;
	bool			noResize_;
	bool			noFilter_;
	D3DFORMAT		format_;
	D3DFORMAT		formatCompressed_;
};

}

#endif // TEXTURE_MANAGER_HPP
