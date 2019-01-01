/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_PNG_HPP
#define MOO_PNG_HPP


#include "cstdmf/debug.hpp"
#include "moo/image.hpp"


namespace Moo
{
/**
 *	When compressing to a PNG image this is used to convey the image data.
 *  When decompressing it is set to the decompressed data.
 */
struct PNGImageData
{
	uint8			*data_;			// raw image data - this is never owned by PNGImageData
	uint32			width_;			// image width
	uint32			height_;		// image height
	uint32			bpp_;			// bits per pixel
	uint32			stride_;		// row stride in bytes
	bool			upsideDown_;	// is the image upside down in memory?

	PNGImageData();
};

BinaryPtr compressPNG
(
	PNGImageData		const &data, 
	std::string			const &resCounter = "unknown compressed png"
);

bool decompressPNG(BinaryPtr pngData, PNGImageData &imgData);

template<typename PIXELTYPE>
BinaryPtr 
compressImage
(
	Image<PIXELTYPE>	const &image, 
	std::string			const &resCounter = "unknown compressed image");

template<typename PIXELTYPE>
bool decompressImage(BinaryPtr pngData, Image<PIXELTYPE> &image);
}


/**
 *	This compresses an Image<> to an in-memory PNG.
 *
 *  @param image		The image to compress.  sizeof(PixelType) has to be
 *						1, 2, 4, 8, 16 or 32.
 *	@param resCounter	The description of the compressed image for resource
 *						counting.
 *  @returns			The compressed image.
 */
template<typename PIXELTYPE>
inline BinaryPtr Moo::compressImage
(
	Image<PIXELTYPE>	const &image,
	std::string			const &resCounter
)
{
	PNGImageData pngData;
	pngData.data_   	= (uint8*)image.getRow(0);
	pngData.width_  	= image.width();
	pngData.height_ 	= image.height();
	pngData.bpp_    	= sizeof(PIXELTYPE)*8;
	pngData.stride_ 	= image.rawRowSize();
	pngData.upsideDown_	= image.isUpsideDown();
	return compressPNG(pngData, resCounter);
}


/**
 *	This decompresses an in-memory PNG to an Image<>.  sizeof(PixelType) has
 *	to match the bpp of the in-memory PNG.
 *
 *  @param pngData	The compress in-memory png.
 *  @param image	This is set to the decompressed image.
 *  @returns		True if successfully decompressed.
 */
template<typename PIXELTYPE>
inline bool Moo::decompressImage(BinaryPtr pngData, Image<PIXELTYPE> &image)
{
	PNGImageData imgData;
	bool ok = decompressPNG(pngData, imgData);
	if (ok)
	{
		IF_NOT_MF_ASSERT_DEV(imgData.bpp_ == sizeof(PIXELTYPE)*8)
		{
			return false;
		}
		image = 
			Image<PIXELTYPE>
			(
				imgData.width_, 
				imgData.height_,
				(PIXELTYPE*)imgData.data_,
				true,		// owns
				0,			// stride = calculate for me
				false,		// not flipped
				image.allocator()
			);
	}
	return ok;
}

#endif // MOO_PNG_HPP
