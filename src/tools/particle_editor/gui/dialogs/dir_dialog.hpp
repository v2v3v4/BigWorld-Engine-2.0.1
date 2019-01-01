/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#error File no longer in use.  Allan 04/03/03

#pragma once


class DirDialog
{
public:
	DirDialog();
	virtual ~DirDialog();

	
	CString windowTitle_;				// window title text
	CString promptText_;				// inner window text

	CString fakeRootDirectory_;			// restrict choice to directories below
	CString startDirectory_;			// start off in directory


	BOOL doBrowse(CWnd *pwndParent = NULL);		// spawn the window


	CString userSelectedDirectory_;		// directory returned


private:
	static int __stdcall DirDialog::browseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};
