/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// managed_texture.hpp

#ifndef MANAGED_TEXTURE_HPP
#define MANAGED_TEXTURE_HPP

#include "base_texture.hpp"

class DataSection;
class BinaryBlock;
typedef SmartPointer<DataSection> DataSectionPtr;
typedef SmartPointer<BinaryBlock> BinaryPtr;

namespace Moo
{
typedef SmartPointer< class ManagedTexture > ManagedTexturePtr;

#define MANAGED_CUBEMAPS 1

/**
 * TODO: to be documented.
 */
class ManagedTexture : public BaseTexture
{
public:
	typedef ComObjectWrap< DX::BaseTexture > Texture;

	// Constructor that creates a blank lockable managed texture.  The
	// resourceID publishes the texture for external use.
	ManagedTexture( 
		const std::string& resourceID, uint32 w, uint32 h, int nLevels,
		DWORD usage, D3DFORMAT fmt, const std::string& allocator = 
		"texture/unknown managed texture" );
	void resize( uint32 w, uint32 h, int nLevels, DWORD usage, D3DFORMAT fmt );

	~ManagedTexture();

	DX::BaseTexture*	pTexture( );
	const std::string&	resourceID( ) const;

	/// Returns the width of the texture, not completely valid if the texture is
	/// a cubetexture, but what can you do...
	uint32				width( ) const;

	/// Returns the height of the texture, not completely valid if the texture is
	/// a cubetexture, but what can you do...
	uint32				height( ) const;

	D3DFORMAT			format( ) const;

	/// Returns the memory used by the texture.
	uint32				textureMemoryUsed( );

	bool				valid() const;

	static void			tick();

	virtual uint32		mipSkip() const { return this->mipSkip_; }
	virtual void		mipSkip( uint32 mipSkip ) { this->mipSkip_ = mipSkip; }

#if MANAGED_CUBEMAPS
	virtual bool		isCubeMap() { return cubemap_; }
#endif

	static bool					s_accErrs;
	static std::string			s_accErrStr;
	static void					accErrs( bool acc );
	static const std::string&	accErrStr();

	static uint32		totalFrameTexmem_;
private:

	uint32				width_;
	uint32				height_;
	D3DFORMAT			format_;
	uint32				mipSkip_;
	bool				loadedFromDisk_;

	bool				noResize_;
	bool				noFilter_;

	uint32				textureMemoryUsed_;
	uint32				originalTextureMemoryUsed_;

	ManagedTexture( 
		const std::string& resourceID, bool mustExist, int mipSkip = 0, bool noResize = false, 
		bool noFilter = false, const std::string& allocator = "texture/unknown managed texture" );
		
	ManagedTexture( 
		DataSectionPtr data, const std::string& resourceID, bool mustExist, 
		int mipSkip = 0, bool noResize = false, bool noFilter = false,
		const std::string& allocator = "texture/unknown managed texture" );

	HRESULT				reload( );
	HRESULT				reload(const std::string & resourceID);

	bool				load( bool mustExist );
	bool				loadBin( BinaryPtr texture, bool mustExist, bool skipHeader = false );
	virtual HRESULT		load()			{ return this->load( true ); }
	virtual HRESULT		release();

	DX::BaseTexture*			texture_;
	ComObjectWrap< DX::Texture > tex_;
	ComObjectWrap< DX::CubeTexture > cubeTex_;
	std::string			resourceID_;
	std::string			qualifiedResourceID_;
	bool				valid_;
	bool				failedToLoad_;

#if MANAGED_CUBEMAPS
	bool				cubemap_;
#endif

	uint32				localFrameTimestamp_;
	static uint32		frameTimestamp_;

	friend class TextureManager;
};

}

#ifdef CODE_INLINE
#include "managed_texture.ipp"
#endif

#endif
// managed_texture.hpp
