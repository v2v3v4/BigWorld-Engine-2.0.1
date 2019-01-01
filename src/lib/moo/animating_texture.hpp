/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ANIMATING_TEXTURE_HPP
#define ANIMATING_TEXTURE_HPP

#include <iostream>
#include "base_texture.hpp"

namespace Moo
{

/**
 *	Class representing an animated texture.
 *	Stored as a list of frames with associated timing.
 */
class AnimatingTexture : public BaseTexture
{
public:
	AnimatingTexture(	const std::string& resourceID = std::string(),
						const std::string& allocator = "texture/unknown animation texture" );
	~AnimatingTexture();

	void									open( const std::string& resourceID );

	DX::BaseTexture*						pTexture( );
	uint32									width( ) const;
	uint32									height( ) const;
	D3DFORMAT								format( ) const;
	uint32									textureMemoryUsed( );
	const std::string&						resourceID( ) const;

	static void								tick( float dTime );

	HRESULT									reload( );

	bool									isAnimated() { return true; }

private:
	void									calculateFrame( );
	std::string								resourceID_;
	std::vector< BaseTexturePtr >			textures_;
	float									fps_;
	uint64									lastTime_;
	float									animFrame_;
	uint32									frameTimestamp_;

	static uint64							currentTime_;

	AnimatingTexture(const AnimatingTexture&);
	AnimatingTexture& operator=(const AnimatingTexture&);

	friend std::ostream& operator<<(std::ostream&, const AnimatingTexture&);
};

}

#ifdef CODE_INLINE
#include "animating_texture.ipp"
#endif




#endif
/*animating_texture.hpp*/
