/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef LIST_FILE_PROVIDER_HPP
#define LIST_FILE_PROVIDER_HPP

#include "list_cache.hpp"
#include "filter_spec.hpp"
#include "smart_list_ctrl.hpp"
#include "atlimage.h"
#include "cstdmf/concurrency.hpp"


/**
 *	This class derives from ListProvider to implement a virtual list provider
 *	that scans for files.
 */
class ListFileProvider : public ListProvider
{
public:
	enum ListFileProvFlags
	{
		// Default behaviout
		LISTFILEPROV_DEFAULT = 0,
		
		// Don't recurse into subfolders
		LISTFILEPROV_DONTRECURSE = 1,

		// Don't filter out DDSs if a matching TGA/JPG/PNG/BMP file exists.
		LISTFILEPROV_DONTFILTERDDS = 2
	};

	explicit ListFileProvider( const std::wstring& thumbnailPostfix );
	virtual ~ListFileProvider();

	virtual void init(
		const std::wstring& type,
		const std::wstring& paths,
		const std::wstring& extensions,
		const std::wstring& includeFolders,
		const std::wstring& excludeFolders,
		int flags = LISTFILEPROV_DEFAULT );

	virtual void refresh();

	virtual bool finished();

	virtual int getNumItems();

	virtual	const AssetInfo getAssetInfo( int index );
	virtual void getThumbnail( ThumbnailManager& manager,
								int index, CImage& img, int w, int h,
								ThumbnailUpdater* updater );

	virtual void filterItems();

	// additional interface
	virtual void setExtensionsIcon( std::wstring extensions, HICON icon );

	// makes the thread sleep for 50 msecs each 'msec' it uses
	virtual void setThreadYieldMsec( int msec );
	virtual int getThreadYieldMsec();

	// if greater than 0, thread priority will be above normal
	// if less than 0, thread priority will be below normal
	virtual void setThreadPriority( int priority );
	virtual int getThreadPriority();

private:
	static const int VECTOR_BLOCK_SIZE = 1000;


	/**
	 *	This simple struct just associates a list of extensions to an image.
	 */
	struct ExtensionsIcons
	{
		std::vector<std::wstring> extensions;
		HICON icon;
	};


	/**
	 *	This internal class holds information about a file found during
	 *	scanning.
	 */
	class ListItem : public SafeReferenceCount
	{
	public:
		std::wstring fileName;		// actually the full path
		std::wstring dissolved;		// dissolved filename
		std::wstring title;			// file name only, no path
	};


	bool hasImages_;
	typedef SmartPointer<ListItem> ListItemPtr;
	std::vector<ListItemPtr> items_;
	std::vector<ListItemPtr> searchResults_;
	typedef std::vector<ListItemPtr>::iterator ItemsItr;
	std::vector<std::wstring> paths_;
	std::vector<std::wstring> extensions_;
	std::vector<std::wstring> includeFolders_;
	std::vector<std::wstring> excludeFolders_;
	std::vector<ExtensionsIcons> extensionsIcons_;
	std::wstring thumbnailPostfix_;
	int flags_;
	std::wstring type_;

	void clearItems();
	void clearThreadItems();

	HICON getExtensionIcons( const std::wstring& name );

	// load thread stuff
	SimpleThread* thread_;
	SimpleMutex mutex_;
	SimpleMutex threadMutex_;
	bool threadWorking_;
	std::vector<ListItemPtr> threadItems_;
	std::vector<ListItemPtr> tempItems_;
	clock_t flushClock_;
	int threadFlushMsec_;
	clock_t yieldClock_;
	int threadYieldMsec_;
	int threadPriority_;

	static bool s_comparator( const ListItemPtr& a, const ListItemPtr& b );
	static void s_startThread( void* provider );
	void startThread();
	void stopThread();
	void setThreadWorking( bool working );
	bool getThreadWorking();

	void fillFiles( const CString& path );
	void removeDuplicateFileNames();
	void removeRedundantDdsFiles();
	void flushThreadBuf();
	bool isFiltering();
};

typedef SmartPointer<ListFileProvider> ListFileProviderPtr;

#endif // LIST_FILE_PROVIDER_HPP
