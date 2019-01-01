/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

// texture_compressor.ipp


/**
 *	This method returns the source texture.
 */
INLINE const DX::Texture*	TextureCompressor::pSrcTexture() const
{
	return pSrcTexture_.pComObject();
}


/**
 *	This method returns the source texture.
 */
INLINE void TextureCompressor::pSrcTexture( DX::Texture* s)
{
	pSrcTexture_ = s;
}


/**
 *	This method returns the desired texture format.
 */
INLINE const D3DFORMAT TextureCompressor::destinationFormat() const
{
	return fmtTo_;
}


/**
 *	This method sets the desired texture format.
 */
INLINE void TextureCompressor::destinationFormat( D3DFORMAT fmtTo )
{
	fmtTo_ = fmtTo;
}

// texture_compressor.ipp
