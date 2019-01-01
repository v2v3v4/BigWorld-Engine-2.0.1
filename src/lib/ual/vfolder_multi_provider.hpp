/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef VFOLDER_MULTI_PROVIDER_HPP
#define VFOLDER_MULTI_PROVIDER_HPP

#include "folder_tree.hpp"
#include "resmgr/datasection.hpp"


/**
 *	This class derives from VFolderProvider to implement a VFolder provider
 *	that manages one or more sub-providers, allowing multiple asset sources to
 *	be shown under one UAL folder.
 */
class VFolderMultiProvider : public VFolderProvider
{
public:
	VFolderMultiProvider();
	virtual ~VFolderMultiProvider();

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
	void addProvider( VFolderProviderPtr provider );

private:
	typedef std::vector<VFolderProviderPtr> ProvVec;
	ProvVec providers_;
	ProvVec::iterator iter_;
	VFolderItemData* parent_;
};


#endif // VFOLDER_MULTI_PROVIDER_HPP