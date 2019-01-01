/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CRITICAL_MESSAGE_BOX_HPP
#define CRITICAL_MESSAGE_BOX_HPP

#include <map>
#include <vector>
#include <string>


// This class is used to displayed the critical message,
// it shouldn't be used anywhere else. MsgBox should be used
// as the common message dialog instead.
class CriticalMsgBox
{
public:
	CriticalMsgBox( const char* info, bool sendInfoBackToBW );
	~CriticalMsgBox();

	// return true if we should enter debugger
	bool doModal();

private:
	static CriticalMsgBox* instance_;
	static std::map<HWND, WNDPROC> itemsMap_;

	static INT_PTR CALLBACK staticDialogProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );
	static INT_PTR CALLBACK itemProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );

	HWND statusWindow_;
	HWND enterDebugger_;
	HWND exit_;

	std::wstring info_;
	bool sendInfoBackToBW_;

	void create( HWND hwnd );
	void kill( HWND hwnd );
	INT_PTR CALLBACK dialogProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );
	void copyContent() const;
	void finishSending( HWND hwnd, const wchar_t* status );

	HFONT font_;
};


#endif//CRITICAL_MESSAGE_BOX_HPP
