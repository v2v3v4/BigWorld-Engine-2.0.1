/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_MGR_HPP
#define SPACE_MGR_HPP

#include <commctrl.h>
#include <shtypes.h>
#include <vector>
#include <string>

struct MRUProvider
{
	virtual ~MRUProvider(){}
	virtual void set( const std::string& name, const std::string& value ) = 0;
	virtual const std::string get( const std::string& name ) const = 0;
};

class SpaceManager
{
	std::vector<std::string>::size_type maxMRUEntries_;
	std::vector<std::string> recentSpaces_;

	static int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );
	static std::wstring getFolderByPidl( LPITEMIDLIST pidl );
	static LPITEMIDLIST commonRoot();

	MRUProvider& mruProvider_;
public:
	SpaceManager( MRUProvider& mruProvider, std::vector<std::string>::size_type maxMRUEntries = 10 );
	void addSpaceIntoRecent( const std::string& space );
	void removeSpaceFromRecent( const std::string& space );
	std::vector<std::string>::size_type num() const;
	std::string entry( std::vector<std::string>::size_type index ) const;

	std::string browseForSpaces( HWND parent ) const;
};

#endif//SPACE_MGR_HPP
