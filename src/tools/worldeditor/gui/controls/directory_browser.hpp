/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DIRECTORY_BROWSER_HPP
#define DIRECTORY_BROWSER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "common/user_messages.hpp"
#include <afxtempl.h>


class DirectoryBrowser : public CListBox
{
	DECLARE_DYNAMIC(DirectoryBrowser)
	// Xiaoming Shi : long file names friendly, for bug 4695{
	CArray<CString> mFileNameIndex;
	// Xiaoming Shi : long file names friendly, for bug 4695}
public:
	DirectoryBrowser();
	virtual ~DirectoryBrowser();

	void initialise(const std::vector<CString>& path);

	void setStartDirectory(const CString& directory);

protected:
	// Generated message map functions
	//{{AFX_MSG(DirectoryBrowser)
	afx_msg void OnSectionChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CString> basePaths_;
	CString currentPath_;
	void add(const WIN32_FIND_DATA& fileData);
	void fill();

	CString startDirectory_;
};


#endif // DIRECTORY_BROWSER_HPP
