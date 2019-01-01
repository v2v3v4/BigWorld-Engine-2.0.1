/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef SPEEDTREE_THUMB_PROV_HPP
#define SPEEDTREE_THUMB_PROV_HPP

// Module Interface
#include "speedtree_config_lite.hpp"

#if SPEEDTREE_SUPPORT // -------------------------------------------------------

// Base class
#include "ual/thumbnail_manager.hpp"

namespace speedtree {

class SpeedTreeRenderer;

/**
 *	Implements a thumb nail provider for SPT files. This class is 
 *	automatically linked into all BW editors that link with speedtree.lib. 
 */
class SpeedTreeThumbProv : public ThumbnailProvider
{
	DECLARE_THUMBNAIL_PROVIDER()
	
public:
	SpeedTreeThumbProv() : tree_( 0 ) {};
	virtual bool isValid(const ThumbnailManager& manager, const std::wstring& file);
	virtual bool prepare(const ThumbnailManager& manager, const std::wstring & file);
	virtual bool render(const ThumbnailManager& manager, const std::wstring & file, Moo::RenderTarget * rt);
private:
	SpeedTreeRenderer* tree_;
};

} // namespace speedtree
#endif // SPEEDTREE_SUPPORT
#endif // SPEEDTREE_THUMB_PROV_HPP
