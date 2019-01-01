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

#include "moo/texture_exposer.hpp"

namespace Moo
{

/**
 *	Constructor. Selects lod level 0.
 */
TextureExposer::TextureExposer( Moo::BaseTexturePtr tx ) :
	pDXBaseTexture_( tx->pTexture() ),
	pDXTexture_( NULL ),
	rectLocked_( -1 )
{
    BW_GUARD;
	pDXBaseTexture_->QueryInterface( IID_IDirect3DTexture9, (void**)&pDXTexture_ );
	this->level( 0 );
}

/**
 *	Destructor
 */
TextureExposer::~TextureExposer()
{
	BW_GUARD;
	this->level( -1 );

	if (pDXTexture_ != NULL)
	{
		if (rectLocked_ >= 0)
		{
			pDXTexture_->UnlockRect( rectLocked_ );
			rectLocked_ = -1;
		}
		pDXTexture_->Release();
	}
}


/**
 *	Switch to the given lod level. Returns true on success, false
 *	on failure. Failure can be due to nonexistent texture, nonexistent
 *	surface for given level, or failure to lock that level's surface.
 */
bool TextureExposer::level( int lod )
{
	BW_GUARD;
	// release old
	if (rectLocked_ >= 0)
	{
		pDXTexture_->UnlockRect( rectLocked_ );
		rectLocked_ = -1;
	}

	levFormat_ = D3DFMT_UNKNOWN;
	levWidth_ = 0;
	levHeight_ = 0;

	levPitch_ = 0;
	levBits_ = NULL;


	// obtain new
	D3DSURFACE_DESC desc;
	if (lod < 0 || pDXTexture_ == NULL) return false;
	if (pDXTexture_->GetLevelDesc( lod, &desc ) != D3D_OK) return false;

	D3DLOCKED_RECT lockedRect;
	if (pDXTexture_->LockRect( lod, &lockedRect, NULL,
		D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY ) != D3D_OK) return false;

	levFormat_ = desc.Format;
	levWidth_ = desc.Width;
	levHeight_ = desc.Height;

	levPitch_ = lockedRect.Pitch;
	levBits_ = (char*)lockedRect.pBits;

	rectLocked_ = lod;

	return true;
}

}

// texture_exposer.cpp
