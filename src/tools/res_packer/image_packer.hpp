/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __IMAGE_PACKER_HPP__
#define __IMAGE_PACKER_HPP__


#include "base_packer.hpp"
#include "packers.hpp"

#include <string>

/**
 *	This class converts all .bmp, .tga and .jpg images to .dds, using a
 *	.texformat file if it exists. It also copies .dds files that don't have
 *	a corresponding bmp, jpg or tga file.
 */
class ImagePacker : public BasePacker
{
private:
	enum Type {
		IMAGE,
		DDS
	};

public:
	ImagePacker() : type_( IMAGE ) {}

	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

private:
	DECLARE_PACKER()
	std::string src_;
	std::string dst_;
	Type type_;
};

#endif // __IMAGE_PACKER_HPP__
