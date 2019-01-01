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
#include "texture_compressor.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/bwresource.hpp"


DECLARE_DEBUG_COMPONENT2( "Moo", 0 )


#ifndef CODE_INLINE
#include "texture_compressor.ipp"
#endif


#ifndef ReleasePpo
#define ReleasePpo(ppo)			\
	if (*(ppo) != NULL)			\
	{							\
		(*(ppo))->Release();	\
		*(ppo) = NULL;			\
	}							\
	else (VOID)0
#endif


// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------


/**
 *	This creates a texture compressor given a source texture to compress. Caller
 *	may optionally provide compression settings.
 *	
 *	@param src	The source texture to convert.
 *	@param fmt	The required destination format (optional, defaults to DXT5).
 *	@param numRequestedMipLevels The number of MIP levels to convert 
 *						(optional, defaults to zero).
 */
TextureCompressor::TextureCompressor(	DX::Texture*	src, 
										D3DFORMAT		fmt,
										uint32			numRequestedMipLevels ):
	fmtTo_( fmt ),
	pSrcTexture_( src ),
	numRequestedMipLevels_( numRequestedMipLevels )
{
}


/**
 *	Destructor.
 */
TextureCompressor::~TextureCompressor()
{
	pDestTexture_ = NULL;
}


// -----------------------------------------------------------------------------
// Section: Texture Compression Utility Routines.
// -----------------------------------------------------------------------------


/**
 *	This method converts the given source texture into the given
 *	texture format, and saves it to the given relative filename.
 *
 *	@param	filename	a resource tree relative filename.
 *
 *	@return True if there were no problems.
 */
bool TextureCompressor::save( const std::string & filename )
{
	BW_GUARD;
	bool result = false;

	//this call blts the source to destination texture, and changes formats.
	HRESULT hr = this->changeFormat( pSrcTexture_, pDestTexture_ );

	if ( SUCCEEDED( hr ) )
	{
		ID3DXBuffer* pBuffer = NULL;
		hr = D3DXSaveTextureToFileInMemory( &pBuffer,
			D3DXIFF_DDS, pDestTexture_.pComObject(), NULL );

		if ( SUCCEEDED( hr ) )
		{
			//DEBUG_MSG( "TextureCompressor : Saved DDS file %s\n", fullDDSName.c_str() );

			//have to release dest texture, given contract defined in this->changeFormat()
			pDestTexture_ = NULL;

			BinaryPtr bin = new BinaryBlock( pBuffer->GetBufferPointer(),
				pBuffer->GetBufferSize(), "BinaryBlock/TextureCompressor" );
			result = BWResource::instance().fileSystem()->writeFile(
				filename, bin, true );

			pBuffer->Release();
		}
	}

	//This FAILED section catches any problems in the above code.
	if ( FAILED( hr ) && !result )
	{
		ERROR_MSG( "Could not save file %s.  Error code %lx\n", filename.c_str(), hr );
	}

	return result;
}


/**
 *	This method converts the given source texture into the given
 *	texture format, and stows it in the given data section.
 *
 *	This method does not save the data section to disk.
 *
 *	@param	pSection	a Data Section smart pointer.
 *	@param	childTag	tag within the given section.
 *
 *	@return True if there were no problems.
 */
bool TextureCompressor::stow( DataSectionPtr pSection, const std::string & childTag )
{
	BW_GUARD;
	//Save a temporary DDS file
	const std::string tempName = "temp_texture_compressor.dds";
	if ( !this->save( tempName ) )
		return false;

	//Create a binary block copy of this dds file.
	DataSectionPtr pDDSFile = BWResource::openSection( tempName );
	BinaryPtr ddsData = pDDSFile->asBinary();
	BinaryPtr binaryBlock = 
		new BinaryBlock(ddsData->data(), ddsData->len(), "BinaryBlock/TextureCompressor");

	//Clean temporary file
	ddsData = NULL;
	pDDSFile = NULL;
	BWResource::instance().purge( tempName );
	BWResource::instance().fileSystem()->eraseFileOrDirectory( tempName );

	//Stick the DDS into a binary section, but don't save the file to disk.
	if (!childTag.empty())
	{
		pSection->delChild( childTag );
		DataSectionPtr textureSection = pSection->openSection( childTag, true );
		textureSection->setParent(pSection);
		textureSection->save();
		textureSection->setParent( NULL );

		if ( !pSection->writeBinary( childTag, binaryBlock ) )
		{
			CRITICAL_MSG( "TextureCompressor::stow: error while writing BinSection in \"%s\"\n", childTag.c_str());
			return false;
		}
	}
	else
	{
		pSection->setBinary(binaryBlock);
	}	

	return true;
}


/**
*	This method converts source texture to destination texture. If texture is
*	created, it is done so in managed pool. If conversion is unsuccessful (due 
*	to an internal DirectX issue) then destTexture is not modified.
*
*	@param destTexture	Empty texture or a texture to reuse.
*	@returns			true if conversion was successful, false otherwise.
*	
*/
bool TextureCompressor::convertTo( ComObjectWrap<DX::Texture>& destTexture )
{
	BW_GUARD;
	bool ok = false;

	if ( changeFormat( pSrcTexture_, destTexture ) == S_OK )
	{
		ok = true;
	}

	return ok;
}


/**
 *	This method changes the source texture to the desired texture format.
 *	The result is stored in pDestTexture. If pDestTexture is empty, then a new
 *	texture with the number of requested MIP levels in created.
 *
 *	@param srcTexture	Source texture to convert from.
 *	@param dstTexture	Destination texture. If empty, a new texture with given
 *						format and MIP levels is created.
 *
 *	@returns			HRESULT indicating the outcome of this operation.
 */
HRESULT TextureCompressor::changeFormat(ComObjectWrap<DX::Texture>& srcTexture,
										ComObjectWrap<DX::Texture>& dstTexture ) const
{
	BW_GUARD;
	if ( !srcTexture )
	{
		ERROR_MSG("TextureCompressor::changeFormat() - source texture not set.\n");
		return S_FALSE;
	}

	D3DSURFACE_DESC desc;
	HRESULT hr = srcTexture->GetLevelDesc( 0, &desc );
	
	D3DFORMAT dstFormat = fmtTo_;
	if (!Moo::rc().supportsTextureFormat( dstFormat ))
	{
		WARNING_MSG( "TextureCompressor: This device does not support "
			"the desired compressed texture format: %d ('%c%c%c%c')\n", dstFormat,
			char(dstFormat>>24), char(dstFormat>>16), char(dstFormat>>8), 
			char(dstFormat) );
		dstFormat = D3DFMT_A8R8G8B8;
	}

	if (SUCCEEDED( hr ))
	{
		// check format
		switch (dstFormat)
		{
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			if ((desc.Width & 0x3) || (desc.Height & 0x3))
			{
				WARNING_MSG( "TextureCompressor: DXT format only support width/height to be multiple of 4. Using uncompressed format instead.\n" );
				dstFormat = D3DFMT_A8R8G8B8;

			}
		}

		if ( !dstTexture )
		{
			// No destination texture given, create one. If we didn't specify 
			// how many destination MIP levels, just use default.
			dstTexture = Moo::rc().createTexture( desc.Width, desc.Height, 
				numRequestedMipLevels_, 0, dstFormat, D3DPOOL_MANAGED );

			if (!dstTexture)
			{
				return E_FAIL;
			}
		}
		
		// Copy from source to destination.
		hr = bltAllLevels( srcTexture, dstTexture );

        if (FAILED(hr))
            return hr;
	}

	return S_OK;
}


/**
 *	This method BLTs all mip map levels from the source to destination
 *	textures, using the texture format baked into the destination
 *	texture resource.
 *
 *	@param srcTexture	The source texture to blit from.
 *	@param dstTexture	The destination texture to blit to.
 *
 *	@return HRESULT indicating the outcome of this operation.
 */
HRESULT TextureCompressor::bltAllLevels(ComObjectWrap<DX::Texture>& srcTexture,
										ComObjectWrap<DX::Texture>& dstTexture ) const
{
    BW_GUARD;
	DX::Texture*	pmiptexSrc = srcTexture.pComObject();
	DX::Texture*	pmiptexDest = dstTexture.pComObject();
	uint32			numSrcMipLevels = srcTexture->GetLevelCount();
	uint32			numDstMipLevels = dstTexture->GetLevelCount();

	HRESULT hr;

	// If we have more destination than source mip levels, don't copy the last 
	// mip level in the source chain, as this one will be copied to scratch 
	// memory before being used as the source for further mip creation
	if (numDstMipLevels > numSrcMipLevels)
	{
		--numSrcMipLevels;
	}

	DWORD iLevel = 0;

	for (; iLevel < numSrcMipLevels; iLevel++)
	{
		DX::Surface* psurfSrc = NULL;
		DX::Surface* psurfDest = NULL;
		hr = pmiptexSrc->GetSurfaceLevel(iLevel, &psurfSrc);
		hr = pmiptexDest->GetSurfaceLevel(iLevel, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL,
			psurfSrc, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
	}


	// Now, create further desired mip map levels
	if ( numDstMipLevels > numSrcMipLevels )
	{
		DX::Surface * psurfSrc[2] = { NULL, NULL };

		HRESULT hr = D3D_OK;

		DX::Surface* pOriginalSource = NULL;
		hr = pmiptexSrc->GetSurfaceLevel( iLevel, &pOriginalSource );
		D3DSURFACE_DESC desc;
		hr = pOriginalSource->GetDesc( &desc );

		D3DFORMAT intermediateFormat = desc.Format;

		// If our source format is one of the compressed formats, go via
		// an uncompressed format when creating the mipmaps
		if (intermediateFormat == D3DFMT_DXT1 && intermediateFormat == D3DFMT_DXT2 &&
			intermediateFormat == D3DFMT_DXT3 && intermediateFormat == D3DFMT_DXT4 &&
			intermediateFormat == D3DFMT_DXT5 )
		{
			intermediateFormat = D3DFMT_A8R8G8B8;
		}

		// Create the source surface to use in scratch memory,
		// and load it with the desired mipmap
		hr = Moo::rc().device()->CreateOffscreenPlainSurface( desc.Width, 
			desc.Height, intermediateFormat, D3DPOOL_SYSTEMMEM, &psurfSrc[0], NULL );
		hr = Moo::rc().device()->CreateOffscreenPlainSurface( desc.Width >> 1, 
			desc.Height >> 1, intermediateFormat, D3DPOOL_SYSTEMMEM, &psurfSrc[1], NULL );

		hr = D3DXLoadSurfaceFromSurface(psurfSrc[0], NULL, NULL,
			pOriginalSource, NULL, NULL, D3DX_FILTER_NONE, 0);
		pOriginalSource->Release();

		RECT curRect = { 0, 0, desc.Width, desc.Height };

		int curSurf = 0;

		// Generate the mipmaps from the lowest known mipmap in the source texture
		for ( uint32 level = iLevel; level < numDstMipLevels; level++ )
		{
			DX::Surface* psurfDest = NULL;
			hr = pmiptexDest->GetSurfaceLevel(level, &psurfDest);
			hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL,
				psurfSrc[ curSurf ], NULL, &curRect, D3DX_FILTER_NONE, 0);

			if (curRect.right >= 2 && curRect.bottom >= 2)
			{
				RECT nextRect = { 0, 0, curRect.right >> 1, curRect.bottom >> 1 };
				int nextSurf = 1 - curSurf;

				hr = D3DXLoadSurfaceFromSurface(psurfSrc[ nextSurf ], NULL, &nextRect,
					psurfSrc[ curSurf ], NULL, &curRect, D3DX_FILTER_BOX | D3DX_FILTER_MIRROR, 0);

				curRect = nextRect;
				curSurf = nextSurf;
			}

			ReleasePpo(&psurfDest);
		}

		ReleasePpo(&psurfSrc[0]);
		ReleasePpo(&psurfSrc[1]);
	}

	return S_OK;
}

// texture_compressor.cpp
