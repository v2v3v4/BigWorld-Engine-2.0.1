/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef VFOLDER_XML_PROVIDER_HPP
#define VFOLDER_XML_PROVIDER_HPP

#include "folder_tree.hpp"
#include "resmgr/datasection.hpp"


/**
 *	This class derives from VFolderItemData and keeps information about an XML
 *	VFolder item.
 */
class VFolderXmlItemData : public VFolderItemData
{
public:
	VFolderXmlItemData(
		VFolderProviderPtr provider,
		const AssetInfo& assetInfo,
		int group,
		bool expandable,
		const std::wstring& thumb ) :
		VFolderItemData( provider, assetInfo, group, expandable ),
		thumb_( thumb )
	{}

	virtual bool isVFolder() const { return false; }
	virtual std::wstring thumb() { return thumb_; }

private:
	std::wstring thumb_;
};


/**
 *	This class derives from VFolderProvider to implement a provider that gets
 *	its items from an XML list.
 */
class VFolderXmlProvider : public VFolderProvider
{
public:
	// XML VFolder item group, only one at the moment.
	enum FileGroup
	{
		XMLGROUP_ITEM = VFolderProvider::GROUP_ITEM
	};

	VFolderXmlProvider();
	VFolderXmlProvider( const std::wstring& path );
	virtual ~VFolderXmlProvider();

	virtual void init( const std::wstring& path );

	virtual bool startEnumChildren( const VFolderItemDataPtr parent );
	virtual VFolderItemDataPtr getNextChild( ThumbnailManager& thumbnailManager, CImage& img );
	virtual void getThumbnail( ThumbnailManager& thumbnailManager, VFolderItemDataPtr data, CImage& img );

	virtual const std::wstring getDescriptiveText( VFolderItemDataPtr data, int numItems, bool finished );
	virtual bool getListProviderInfo(
		VFolderItemDataPtr data,
		std::wstring& retInitIdString,
		ListProviderPtr& retListProvider,
		bool& retItemClicked );

	// additional interface
	virtual std::wstring getPath();
	virtual bool getSort();
private:
	std::wstring path_;
	bool sort_;
	typedef SmartPointer<AssetInfo> ItemPtr;
	std::vector<ItemPtr> items_;
	std::vector<ItemPtr>::iterator itemsItr_;
};


#endif // VFOLDER_XML_PROVIDER_HPP