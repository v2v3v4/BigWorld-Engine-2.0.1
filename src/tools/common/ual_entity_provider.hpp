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
 *	This file declares all classes needed to implement the asset locator's
 *	entity providers.
 */


#ifndef __UAL_ENTITY_PROVIDER_HPP__
#define __UAL_ENTITY_PROVIDER_HPP__

#include "ual/ual_dialog.hpp"


///////////////////////////////////////////////////////////////////////////////
//	Entity UAL VFolder provider classes
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements the entity VFolder provider
 */
template <typename providerMap>
class EntityVFolderProvider : public VFolderProvider
{
public:
	explicit EntityVFolderProvider( const std::string& thumb );
	~EntityVFolderProvider();
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


///////////////////////////////////////////////////////////////////////////////
//	Entity UAL List provider classes
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements the entity list provider
 */
template <typename providerMap>
class EntityListProvider : public ListProvider
{
public:
	explicit EntityListProvider( const std::string& thumb );

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


///////////////////////////////////////////////////////////////////////////////
//	Entity UAL loader classes
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements the entity loader class, that creates the VFolder and
 *	list providers from a given section of the UAL's config file.
 */
template <typename providerMap>
class UalEntityVFolderLoader : public UalVFolderLoader
{
public:
	bool test( const std::string& sectionName )
	{ 
		return sectionName == providerMap::getFactoryName(); 
	}
	VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};

#endif // __UAL_ENTITY_PROVIDER_HPP__
