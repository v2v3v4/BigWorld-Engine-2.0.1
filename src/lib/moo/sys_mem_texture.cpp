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
#include "sys_mem_texture.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Moo
{

// -----------------------------------------------------------------------------
// Section: SysMemTexture
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
SysMemTexture::SysMemTexture( const std::string& resourceID, D3DFORMAT format ) :
	width_( 0 ),
	height_( 0 ),
	format_(format),
	resourceID_( canonicalTextureName(resourceID) ),
	qualifiedResourceID_( resourceID ),
	failedToLoad_( false )
{
	load();
}

HRESULT SysMemTexture::load()
{
	BW_GUARD;
	if (failedToLoad_)
		return E_FAIL;

	if (pTexture_.pComObject())
		return S_OK;

	if (rc().device() == NULL)
		return E_FAIL;

	HRESULT hr = E_FAIL;

	// Open the texture file resource.
	BinaryPtr texture = BWResource::instance().rootSection()->readBinary( qualifiedResourceID_ );
	if( texture.hasObject() )
	{
		ComObjectWrap<DX::Texture> tex;
		static PALETTEENTRY palette[256];

		// Create the texture and the mipmaps.
		tex = Moo::rc().createTextureFromFileInMemoryEx( texture->data(), texture->len(),
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, format_, 
			D3DPOOL_SYSTEMMEM, D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 
			D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 0, NULL, palette );

		if( tex )
		{
			DX::Surface* surface;
			if( SUCCEEDED( tex->GetSurfaceLevel( 0, &surface ) ) )
			{
				D3DSURFACE_DESC desc;
				surface->GetDesc( &desc );

				width_ = desc.Width;
				height_ = desc.Height;
				format_ = desc.Format;

				surface->Release();
				surface = NULL;
			}
			// Assign the texture to the smartpointer.
			pTexture_ = tex;
		}
		else
		{
			ERROR_MSG( "SysMemTexture::load(%s): Unknown texture format\n",
				qualifiedResourceID_.c_str() );
			failedToLoad_ = true;
		}
	}
	else
	{
		failedToLoad_ = true;
	}
	return hr;
}


}
