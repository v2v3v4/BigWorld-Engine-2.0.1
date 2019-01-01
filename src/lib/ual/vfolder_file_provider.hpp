/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef VFOLDER_FILE_PROVIDER_HPP
#define VFOLDER_FILE_PROVIDER_HPP

#include <stack>
#include "folder_tree.hpp"


/// Files VFolder provider initialisation flags.
enum FILETREE_FLAGS
{
	FILETREE_SHOWSUBFOLDERS = 1,	// Show subfolders
	FILETREE_SHOWFILES = 2,			// Show files
	FILETREE_DONTRECURSE = 4		// Just one recursion level
};


/**
 *	This class derives from VFolderItemData and keeps information about a files
 *	VFolder item.
 */
class VFolderFileItemData : public VFolderItemData
{
public:
	// Files VFolder item type, whether it's a file or a folder.
	enum ItemDataType
	{
		ITEMDATA_FOLDER,
		ITEMDATA_FILE
	};

	VFolderFileItemData(
		VFolderProviderPtr provider,
		ItemDataType type,
		const AssetInfo& assetInfo,
		int group,
		bool expandable );
	virtual ~VFolderFileItemData();

	virtual bool isVFolder() const { return false; }

	virtual bool handleDuplicate( VFolderItemDataPtr data );

	virtual ItemDataType VFolderFileItemData::getItemType() { return type_; }

private:
	ItemDataType type_;
};


/**
 *	This class derives from VFolderProvider to implement a provider that scans
 *	a series of paths for files.
 */
class VFolderFileProvider : public VFolderProvider
{
public:
	// Files VFolder item group, whether it's a file or a folder.
	enum FileGroup
	{
		FILEGROUP_FOLDER = VFolderProvider::GROUP_FOLDER,
		FILEGROUP_FILE = VFolderProvider::GROUP_ITEM
	};

	VFolderFileProvider();
	VFolderFileProvider(
		const std::wstring& thumbnailPostfix,
		const std::wstring& type,
		const std::wstring& paths,
		const std::wstring& extensions,
		const std::wstring& includeFolders,
		const std::wstring& excludeFolders,
		int flags );
	virtual ~VFolderFileProvider();

	virtual void init(
		const std::wstring& type,
		const std::wstring& paths,
		const std::wstring& extensions,
		const std::wstring& includeFolders,
		const std::wstring& excludeFolders,
		int flags );

	virtual bool startEnumChildren( const VFolderItemDataPtr parent );
	virtual VFolderItemDataPtr getNextChild( ThumbnailManager& thumbnailManager, CImage& img );
	virtual void getThumbnail( ThumbnailManager& thumbnailManager, VFolderItemDataPtr data, CImage& img );
	virtual const std::wstring getDescriptiveText( VFolderItemDataPtr data, int numItems, bool finished );
	virtual bool getListProviderInfo(
		VFolderItemDataPtr data,
		std::wstring& retInitIdString,
		ListProviderPtr& retListProvider,
		bool& retItemClicked );

	virtual const std::wstring getType();
	virtual int getFlags();
	virtual const std::vector<std::wstring>& getPaths();
	virtual const std::vector<std::wstring>& getExtensions();
	virtual const std::vector<std::wstring>& getIncludeFolders();
	virtual const std::vector<std::wstring>& getExcludeFolders();
	virtual const std::wstring getPathsString();
	virtual const std::wstring getExtensionsString();
	virtual const std::wstring getIncludeFoldersString();
	virtual const std::wstring getExcludeFoldersString();

private:
	/**
	 *	This internal class simply stores information for scanning a list of
	 *	paths for files and folders.  This is needed because of the nature of 
	 *	VFolder providers, that are asked for one item at a time, so we need to
	 *	store where we're at.
	 */
	class FileFinder : public ReferenceCount
	{
	public:
		CFileFind files;
		std::vector<std::wstring> paths;
		std::vector<std::wstring>::iterator path;
		std::vector<std::wstring>::iterator pathEnd;
		bool eof;
	};

	std::wstring type_;
	int flags_;
	typedef SmartPointer<FileFinder> FileFinderPtr;
	std::stack<FileFinderPtr> finderStack_;
	std::wstring thumbnailPostfix_;
	std::vector<std::wstring> paths_;
	std::vector<std::wstring> extensions_;
	std::vector<std::wstring> includeFolders_;
	std::vector<std::wstring> excludeFolders_;

	void popFinderStack();
	void clearFinderStack();
	FileFinderPtr topFinderStack();
};


#endif // VFOLDER_FILE_PROVIDER_HPP