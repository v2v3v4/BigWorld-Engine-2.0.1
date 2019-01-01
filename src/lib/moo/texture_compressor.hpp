/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_TEXTURE_COMPRESSOR_HPP
#define MOO_TEXTURE_COMPRESSOR_HPP

/**
 *	This class has the ability to compress a texture to any given D3DFORMAT.
 *	This class is typically used with limited scope.
 */
class TextureCompressor
{
public:
	TextureCompressor(	DX::Texture*	src, 
						D3DFORMAT		fmt						= D3DFMT_DXT5, 
						uint32			numRequestedMipLevels	= 0 );

	~TextureCompressor();

	const DX::Texture*	pSrcTexture() const;
	void pSrcTexture( DX::Texture* );

	const D3DFORMAT destinationFormat() const;
	void destinationFormat( D3DFORMAT );

	bool save( const std::string & filename );
	bool stow( DataSectionPtr pSection, const std::string & childTag = "" );
	bool convertTo( ComObjectWrap<DX::Texture>& destTexture );

private:
	HRESULT bltAllLevels( ComObjectWrap<DX::Texture>& srcTexture,
								ComObjectWrap<DX::Texture>& destTexture ) const;
	
	HRESULT changeFormat( ComObjectWrap<DX::Texture>& srcTexture,
								ComObjectWrap<DX::Texture>& destTexture ) const;

	TextureCompressor( const TextureCompressor& );
	TextureCompressor& operator=( const TextureCompressor& );

	D3DFORMAT					fmtTo_;
	ComObjectWrap<DX::Texture>	pSrcTexture_;
	ComObjectWrap<DX::Texture>	pDestTexture_;
	uint32						numRequestedMipLevels_;
};

#ifdef CODE_INLINE
#include "texture_compressor.ipp"
#endif

#endif // MOO_TEXTURE_COMPRESSOR_HPP
