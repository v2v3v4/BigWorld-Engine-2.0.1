/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

#include <map>
#include <vector>
#include <string>

class MsgBox
{
public:
	typedef bool (*TickFunction)( MsgBox* msgBox, bool initDialog );

	MsgBox( const std::wstring& caption, const std::wstring& text,
		const std::wstring& buttonText1 = L"", const std::wstring& buttonText2 = L"",
		const std::wstring& buttonText3 = L"", const std::wstring& buttonText4 = L"" );
	MsgBox( const std::wstring& caption, const std::wstring& text,
		const std::vector<std::wstring>& buttonTexts );
	~MsgBox();

	void status( const std::wstring& status );
	void disableButtons();
	void enableButtons();

	unsigned int doModal( HWND parent = NULL, bool topmost = false, unsigned int timeOutTicks = INFINITE,
		TickFunction tickFunction = NULL );
	void doModalless( HWND parent = NULL, unsigned int timeOutTicks = INFINITE, TickFunction tickFunction = NULL );
	unsigned int getResult() const;
	bool stillActive() const;

	static HWND getDefaultParent()
	{
		return defaultParent_;
	}

	static void setDefaultParent( HWND hwnd )
	{
		defaultParent_ = hwnd;
	}

	static const unsigned int TIME_OUT = INFINITE;

private:
	static std::map<HWND, MsgBox*> wndMap_;
	static std::map<const MsgBox*, HWND> msgMap_;
	static std::map<HWND, WNDPROC> buttonMap_;
	static HWND defaultParent_;

	static INT_PTR CALLBACK staticDialogProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );
	static INT_PTR CALLBACK buttonProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );

	std::wstring caption_;
	std::wstring text_;
	std::vector<std::wstring> buttons_;
	unsigned int result_;
	bool model_;
	bool topmost_;
	unsigned int timeOut_;

	std::wstring status_;
	HWND statusWindow_;
	HWND savedFocus_;

	void create( HWND hwnd );
	void kill( HWND hwnd );
	INT_PTR CALLBACK dialogProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l );
	void copyContent() const;

	HFONT font_;
	TickFunction tickFunction_;
};

#endif//MESSAGE_BOX_HPP
