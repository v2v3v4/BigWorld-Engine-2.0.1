/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef UAL_VFOLDER_LOADER
#define UAL_VFOLDER_LOADER


#include "cstdmf/smartpointer.hpp"
#include "cstdmf/guard.hpp"
#include <string>
#include <vector>


// Forward declarations
class UalDialog;

class UalFolderData;
typedef SmartPointer<UalFolderData> UalFolderDataPtr;

class VFolder;
typedef SmartPointer<VFolder> VFolderPtr;

class VFolderProvider;
typedef SmartPointer<VFolderProvider> VFolderProviderPtr;

class DataSection;
typedef SmartPointer<DataSection> DataSectionPtr;


///////////////////////////////////////////////////////////////////////////////
//	UalVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This abstract class is the base class for all VFolder loader classes.  A
 *	VFolder loader handles loading information for a virtual folder in the
 *	Asset Browser from XML.  This information is then used for both the tree
 *	view virtual folder as well as the contents of the item list when that
 *	folder is clicked.
 */
class UalVFolderLoader : public ReferenceCount
{
public:
	virtual bool test( const std::string& sectionName ) = 0;
	virtual bool subVFolders() { return false; }
	virtual VFolderPtr load( UalDialog* dlg, DataSectionPtr section,
		VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree ) = 0;

protected:
	UalVFolderLoader() :
		icon_( 0 ),
		iconSel_( 0 ),
		show_( true )
	{
		BW_GUARD;
	}
	~UalVFolderLoader()
	{
		BW_GUARD;

		if( icon_ )	DestroyIcon( icon_ );
		if( iconSel_ )	DestroyIcon( iconSel_ );
	}

	std::wstring displayName_;
	HICON icon_;
	HICON iconSel_;
	bool show_;
	UalFolderDataPtr folderData_;

	void error( UalDialog* dlg, const std::string& msg );

	void beginLoad( UalDialog* dlg, DataSectionPtr section,
		DataSectionPtr customData, int defaultThumbSize );

	VFolderPtr endLoad( UalDialog* dlg, VFolderProviderPtr provider,
		VFolderPtr parent, bool expandable,
		bool addToFolderTree = true, bool subVFolders = false );

};
typedef SmartPointer<UalVFolderLoader> UalVFolderLoaderPtr;


///////////////////////////////////////////////////////////////////////////////
//	LoaderRegistry: VFolder loaders vector singleton class
///////////////////////////////////////////////////////////////////////////////
typedef std::vector<UalVFolderLoaderPtr> VFolderLoaders;

/**
 *	This class is a singleton that stores all the available VFolder loaders.
 */
class LoaderRegistry
{
public:
	static VFolderLoaders& LoaderRegistry::loaders()
	{
		BW_GUARD;

		static LoaderRegistry instance;
		return instance.vfolderLoaders_;
	}
	static UalVFolderLoaderPtr loader( const std::string& sectionName );
private:
	VFolderLoaders vfolderLoaders_;
};


///////////////////////////////////////////////////////////////////////////////
//	UalVFolderLoaderFactory
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class simply registers a VFolder loader into the registry.
 */
class UalVFolderLoaderFactory
{
public:
	UalVFolderLoaderFactory( UalVFolderLoaderPtr loader );
};


///////////////////////////////////////////////////////////////////////////////
//	UalFilesVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements a file-searching VFolder loader.
 */
class UalFilesVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "Files"; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );

protected:
	bool pathIsGood( const std::wstring& path );
};


///////////////////////////////////////////////////////////////////////////////
//	UalXmlVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements an XML list VFolder loader.
 */
class UalXmlVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "XmlList"; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};


///////////////////////////////////////////////////////////////////////////////
//	UalHistoryVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements a history XML list VFolder loader.
 */
class UalHistoryVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "History"; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};


///////////////////////////////////////////////////////////////////////////////
//	UalFavouritesVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements a favourites XML list VFolder loader.
 */
class UalFavouritesVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "Favourites"; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};


///////////////////////////////////////////////////////////////////////////////
//	UalMultiVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements a multi VFolder container VFolder loader, useful for
 *	merging the results of multiple VFolders into one.
 */
class UalMultiVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "MultiVFolder"; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};


///////////////////////////////////////////////////////////////////////////////
//	UalPlainVFolderLoader
///////////////////////////////////////////////////////////////////////////////

/**
 *	This class implements a plain VFolder loader that creates VFolders with
 *	no items, useful for nesting other VFolders.
 */
class UalPlainVFolderLoader : public UalVFolderLoader
{
public:
	virtual bool test( const std::string& sectionName )
		{ return sectionName == "VFolder"; }
	virtual bool subVFolders() { return true; }
	virtual VFolderPtr load( UalDialog* dlg,
		DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
		bool addToFolderTree );
};


#endif // UAL_VFOLDER_LOADER
