/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef IMAGE_TEXTURE_HPP
#define IMAGE_TEXTURE_HPP

#include "moo/base_texture.hpp"
#include "moo/image.hpp"
#include "moo/moo_math.hpp"
#include "moo/render_context.hpp"

namespace Moo
{
	template<typename PIXELTYPE>
	class ImageTexture : public BaseTexture
	{
	public:
		typedef PIXELTYPE			PixelType;
		typedef Image<PixelType>	ImageType;

		ImageTexture
		(
			uint32		width, 
			uint32		height, 			
			D3DFORMAT	format		= recommendedFormat(),
			uint32		mipLevels	= 0
		);
		~ImageTexture();

		DX::BaseTexture *pTexture();

		uint32 width() const;
		uint32 height() const;
		D3DFORMAT format() const;
		uint32 textureMemoryUsed();

		void lock(uint32 level = 0);
		ImageType &image();
		ImageType const &image() const;
		void unlock();

		static D3DFORMAT recommendedFormat();

	private:
		uint32						width_;
		uint32						height_;
		D3DFORMAT					format_;
		uint32						textureMemoryUsed_;
		ImageType					image_;
		ComObjectWrap<DX::Texture>	texture_;
		uint32						lockCount_;
		uint32						lockLevel_;
	};

	typedef ImageTexture<Moo::PackedColour>		ImageTextureARGB;
	typedef ImageTexture<uint8>					ImageTexture8;
	typedef ImageTexture<uint16>				ImageTexture16;

	typedef SmartPointer<ImageTextureARGB>		ImageTextureARGBPtr;
	typedef SmartPointer<ImageTexture8>			ImageTexture8Ptr;
	typedef SmartPointer<ImageTexture16>		ImageTexture16Ptr;
}


#include "moo/image_texture.ipp"


#endif // IMAGE_TEXTURE_HPP
