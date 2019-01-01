/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_EXPLORER_HPP
#define TEXTURE_EXPLORER_HPP


/**
 *	This class utilitses the MF_WATCH macro system
 *	to implement a texture debugging tool on the Xbox.
 *
 *	This class is not supported on the PC build.
 */
class TextureExplorer
{
public:
	TextureExplorer();
	~TextureExplorer();
	void	tick();
	void	draw();
private:

	bool	enabled_;
	bool	fitToScreen_;
	bool	preserveAspect_;
	int		index_;
	int		currentIndex_;

	bool	doReload_;

	std::string textureName_;
	std::string texturePath_;
	Moo::BaseTexturePtr pTexture_;
	std::string format_;
	uint32	width_;
	uint32	height_;
	uint32	memoryUsage_;
	uint32	maxMipLevel_;

	bool	alpha_;
	Moo::Material*	pAlphaMaterial_;
	Moo::Material*	pColourMaterial_;

	typedef std::map< uint32, std::string > Formats;
	Formats surfaceFormats_;

	TextureExplorer( const TextureExplorer& );
	TextureExplorer& operator=( const TextureExplorer& );
};


#endif // TEXTURE_EXPLORER_HPP
