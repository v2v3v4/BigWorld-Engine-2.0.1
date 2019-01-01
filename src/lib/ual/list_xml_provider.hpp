/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef LIST_XML_PROVIDER_HPP
#define LIST_XML_PROVIDER_HPP

#include "list_cache.hpp"
#include "filter_spec.hpp"
#include "smart_list_ctrl.hpp"
#include "atlimage.h"


/**
 *	This class derives from ListProvider to implement an XML virtual list
 *	provider.
 */
class ListXmlProvider : public ListProvider
{
public:
	ListXmlProvider();
	virtual ~ListXmlProvider();

	virtual void init( const std::wstring& fname );

	virtual void refresh();

	virtual bool finished();

	virtual int getNumItems();

	virtual	const AssetInfo getAssetInfo( int index );
	virtual void getThumbnail( ThumbnailManager& manager,
								int index, CImage& img, int w, int h,
								ThumbnailUpdater* updater );

	virtual void filterItems();

	// additional interface
	bool errorLoading() { return errorLoading_; };

private:
	static const int VECTOR_BLOCK_SIZE = 1000;
	typedef SmartPointer<AssetInfo> ListItemPtr;
	std::vector<ListItemPtr> items_;
	std::vector<ListItemPtr> searchResults_;
	typedef std::vector<ListItemPtr>::iterator ItemsItr;
	std::wstring path_;
	bool errorLoading_;

	static bool s_comparator( const ListItemPtr& a, const ListItemPtr& b );

	void clearItems();

	void refreshPurge( bool purge );
};

typedef SmartPointer<ListXmlProvider> ListXmlProviderPtr;

#endif // LIST_XML_PROVIDER_HPP
