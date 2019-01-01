/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_EXPOSER_HPP
#define TEXTURE_EXPOSER_HPP

#include "moo/base_texture.hpp"

namespace Moo
{

/**
 * This class locks a DX texture and caches information for a given LOD
 * level.
 */
class TextureExposer
{
public:
	TextureExposer( Moo::BaseTexturePtr tx );
	~TextureExposer();

	bool level( int lod );

	D3DFORMAT format() const		{ return levFormat_; }
	int width() const				{ return levWidth_; }
	int height() const				{ return levHeight_; }

	int pitch() const				{ return levPitch_; }
	const char * bits() const		{ return levBits_; }

private:
	DX::BaseTexture*	pDXBaseTexture_;
	DX::Texture*		pDXTexture_;
	int					rectLocked_;

	D3DFORMAT	levFormat_;
	int			levWidth_;
	int			levHeight_;
	int			levPitch_;
	char *		levBits_;
};


}


#endif // TEXTURE_EXPOSER_HPP
