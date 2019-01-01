/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	Icon Thumbnail Provider (for files without preview, such as prefabs)
 */


#ifndef ICON_THUMB_PROV_HPP
#define ICON_THUMB_PROV_HPP

#include "thumbnail_manager.hpp"


/**
 *	This class implements an Icon Thumbnail Provider, for files without
 *	preview thumbnail, such as prefabs.
 */
class IconThumbProv : public ThumbnailProvider
{
public:
	IconThumbProv() : inited_( false ) {};
	virtual bool isValid( const ThumbnailManager& manager, const std::wstring& file );
	virtual bool needsCreate( const ThumbnailManager& manager, const std::wstring& file, std::wstring& thumb, int& size );
	virtual bool prepare( const ThumbnailManager& manager, const std::wstring& file );
	virtual bool render( const ThumbnailManager& manager, const std::wstring& file, Moo::RenderTarget* rt  );

private:
	bool inited_;
	std::wstring imageFile_;
	/**
	 *	This internal struct is used to match a file type with an icon.
	 */
	struct IconData
	{
		IconData(
			const std::wstring& e, const std::wstring& m, const std::wstring& i
			) :
		extension( e ),
		match( m ),
		image( i )
		{};
		std::wstring extension;
		std::wstring match;
		std::wstring image;
	};
	std::vector<IconData> iconData_;

	DECLARE_THUMBNAIL_PROVIDER()

	void init();
	std::wstring imageFile( const std::wstring& file );
};

#endif // ICON_THUMB_PROV_HPP
