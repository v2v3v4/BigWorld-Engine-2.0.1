/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	This creates an ImageTexture.
 *
 *	@param width		Width of the texture.
 *	@param height		Height of the texture.
 *	@param format		Format of the texture.
 *  @param mipLevels	Number of mip-map levels.
 */
template<typename PIXELTYPE>
inline Moo::ImageTexture<PIXELTYPE>::ImageTexture
(
	uint32				width, 
	uint32				height, 			
	D3DFORMAT			format		/*= recommendedFormat()*/,
	uint32				mipLevels	/*= 0*/
):
	BaseTexture(),
	width_(width),
	height_(height),
	format_(format),
	textureMemoryUsed_(0),
	lockCount_(0),
	lockLevel_(0)
{
	BW_GUARD;
	DX::Device *device = Moo::rc().device();
	HRESULT hr = 
		device->CreateTexture
		(
			width,
			height,
			mipLevels,
			0,		// usage
			format,
			D3DPOOL_MANAGED,
			&texture_,
			NULL	// shared handle
		);
	MF_ASSERT_DEV(SUCCEEDED(hr));
	if (texture_ != NULL)
		textureMemoryUsed_ = BaseTexture::textureMemoryUsed(texture_.pComObject());
}


/**
 *	This is the ImageTexture destructor.
 */
template<typename PIXELTYPE>
inline Moo::ImageTexture<PIXELTYPE>::~ImageTexture()
{
}


/**
 *	This function gets the underlying D3D texture.
 *
 *  @returns			The underlying D3D texture.	
 */
template<typename PIXELTYPE>
inline DX::BaseTexture *Moo::ImageTexture<PIXELTYPE>::pTexture()
{
	return texture_.pComObject();
}



/**
 *	This function gets the width of the texture.
 *
 *  @returns			The width of the texture.
 */
template<typename PIXELTYPE>
inline uint32 Moo::ImageTexture<PIXELTYPE>::width() const
{
	return width_;
}


/**
 *	This function gets the height of the texture.
 *
 *  @returns			The height of the texture.
 */
template<typename PIXELTYPE>
inline uint32 Moo::ImageTexture<PIXELTYPE>::height() const
{
	return height_;
}


/**
 *	This function gets the format of the texture.
 *
 *  @returns			The format of the texture.
 */
template<typename PIXELTYPE>
inline D3DFORMAT Moo::ImageTexture<PIXELTYPE>::format() const
{
	return format_;
}


/**
 *	This function returns the size of the texture in memory.
 *
 *  @returns			The size of the texture in memory.
 */
template<typename PIXELTYPE>
inline uint32 Moo::ImageTexture<PIXELTYPE>::textureMemoryUsed()
{
	return textureMemoryUsed_;
}


/**
 *	This function locks the specified mip-map level for reading/writing.
 *  You should only lock one level at a time.
 *
 *  @param level		The level to lock.
 */
template<typename PIXELTYPE>
inline void Moo::ImageTexture<PIXELTYPE>::lock(uint32 level /*= 0*/)
{
	BW_GUARD;
	if (lockCount_++ == 0)
	{
		D3DLOCKED_RECT lockedRect;
		HRESULT hr = texture_->LockRect(level, &lockedRect, NULL, 0);
		MF_ASSERT_DEV(SUCCEEDED(hr));
		if( SUCCEEDED(hr) )
		{
			PixelType *pixels = reinterpret_cast<PixelType *>(lockedRect.pBits);
			image_.resize(width_, height_, pixels, false, lockedRect.Pitch);
		}
		lockLevel_ = level;
	}
	MF_ASSERT_DEV(lockLevel_ == level);
}


/**
 *	This gets the texture's pixels as an image.  This should be called between
 *	calls to lock/unlock.
 *
 *  @returns			The texture's pixels as an image.
 */
template<typename PIXELTYPE>
inline typename Moo::ImageTexture<PIXELTYPE>::ImageType &
Moo::ImageTexture<PIXELTYPE>::image()
{
	return image_;
}


/**
 *	This gets the texture's pixels as an image.  This should be called between
 *	calls to lock/unlock.
 *
 *  @returns			The texture's pixels as an image.
 */
template<typename PIXELTYPE>
inline typename Moo::ImageTexture<PIXELTYPE>::ImageType const &
Moo::ImageTexture<PIXELTYPE>::image() const
{
	return image_;
}


/**
 *	This unlocks the texture after being written.
 */
template<typename PIXELTYPE>
inline void Moo::ImageTexture<PIXELTYPE>::unlock()
{
	BW_GUARD;
	if (--lockCount_ == 0)
	{
		HRESULT hr = texture_->UnlockRect(lockLevel_);
		MF_ASSERT_DEV(SUCCEEDED(hr));
		
		if( SUCCEEDED(hr) )
			image_.clear();
	}
}


/**
 *	This function gets the recommended format for the texture.
 *
 *  @returns			D3DFMT_X8R8G8B8.
 */
template<typename PIXELTYPE>
/*static*/ inline D3DFORMAT Moo::ImageTexture<PIXELTYPE>::recommendedFormat()
{
	return D3DFMT_A8R8G8B8;
}


/** 
 *	This function gets the recommended texture format for ARGB image textures.
 *
 *  @returns			D3DFMT_A8R8G8B8
 */
/*static*/ inline D3DFORMAT Moo::ImageTextureARGB::recommendedFormat()
{
	return D3DFMT_A8R8G8B8;
}


/** 
 *	This function gets the recommended texture format for uint8 image textures.
 *
 *  @returns			D3DFMT_L8
 */
/*static*/ inline D3DFORMAT Moo::ImageTexture8::recommendedFormat()
{
	return D3DFMT_L8;
}


/** 
 *	This function gets the recommended texture format for uint16 image textures.
 *
 *  @returns			D3DFMT_L16
 */
/*static*/ inline D3DFORMAT Moo::ImageTexture16::recommendedFormat()
{
	return D3DFMT_L16;
}
