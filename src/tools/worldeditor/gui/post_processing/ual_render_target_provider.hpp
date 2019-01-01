/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef UAL_RENDER_TARGET_PROVIDER_HPP
#define UAL_RENDER_TARGET_PROVIDER_HPP


#include "ual/ual_dialog.hpp"


/**
 *	This class implements the render target VFolder provider
 */
class RenderTargetVFolderProvider : public VFolderProvider
{
public:
	explicit RenderTargetVFolderProvider( const std::string& thumb );
	~RenderTargetVFolderProvider();
	bool startEnumChildren( const VFolderItemDataPtr parent );
	VFolderItemDataPtr getNextChild( ThumbnailManager& manager, CImage& img );
	void getThumbnail( ThumbnailManager& manager, VFolderItemDataPtr data, CImage& img );
	const std::wstring getDescriptiveText( VFolderItemDataPtr data, int numItems, bool finished );
	bool getListProviderInfo(
		VFolderItemDataPtr data,
		std::wstring& retInitIdString,
		ListProviderPtr& retListProvider,
		bool& retItemClicked );

private:
	int index_;
	std::string thumb_;
	CImage img_;
};


/**
 *	This class implements the render target list provider
 */
class RenderTargetListProvider : public ListProvider
{
public:
	explicit RenderTargetListProvider( const std::string& thumb );
	~RenderTargetListProvider();

	void refresh();
	bool finished();
	int getNumItems();
	const AssetInfo getAssetInfo( int index );
	void getThumbnail( ThumbnailManager& manager,
		int index, CImage& img, int w, int h, ThumbnailUpdater* updater );
	void filterItems();

private:
	std::vector<AssetInfo> searchResults_;
	std::string thumb_;
	CImage img_;
};


/**
 *	This class implements the render target loader class, that creates the VFolder and
 *	list providers from a given section of the UAL's config file.
 */
class UalRenderTargetVFolderLoader : public UalVFolderLoader
{
public:
	bool test( const std::string& sectionName )
	{ 
		BW_GUARD;

		return sectionName == "PostProcessingRenderTargets";
	}
	VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};

#endif // UAL_RENDER_TARGET_PROVIDER_HPP
