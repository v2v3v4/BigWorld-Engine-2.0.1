/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BRUSH_THUMB_PROVIDER_HPP
#define BRUSH_THUMB_PROVIDER_HPP


#include "ual/image_thumb_prov.hpp"


/**
 *	This class helps generate thumbnails for .brush files.
 */
class BrushThumbProvider : public ThumbnailProvider
{
public:
	virtual bool isValid( const ThumbnailManager& manager, const std::wstring& file );
	virtual bool prepare( const ThumbnailManager& manager, const std::wstring& file );
	virtual bool render( const ThumbnailManager& manager, const std::wstring& file, Moo::RenderTarget* rt );

protected:
	std::wstring getTextureFileForBrush( const std::wstring & file ) const;

private:
	DECLARE_THUMBNAIL_PROVIDER()

	ImageThumbProv		imageProvider_;
};


#endif // BRUSH_THUMB_PROVIDER_HPP
