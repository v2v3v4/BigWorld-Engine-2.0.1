/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SEARCH_FIELD_HPP
#define SEARCH_FIELD_HPP



/**
 *	This class encapsulates a search control that emulates search boxes found
 *	in other apps, and contains a text input area, a magnifier glass icon that
 *	can be clicked, an idle text control that shows a message when no text has
 *	been input, and a "close" button that clears the search when clicked.
 */
class SearchField : public CStatic
{
public:
	static const int MAX_SEARCH_TEXT = 80;

	SearchField();
	virtual ~SearchField();

	bool init( LPTSTR filterResID, LPTSTR closeResID,
		const std::wstring & idleText, const std::wstring & filterToolTip,
		const std::wstring & searchToolTip );

	const std::wstring & idleText() const;
	void idleText( const std::wstring & idleText );

	void filtersImg( LPTSTR filterResID );

	void searchText( const std::wstring & text );
	const std::wstring searchText() const;

	BOOL PreTranslateMessage( MSG* msg );

protected:
	LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:

	/**
	 *	This internal class deals with displaying an idle text message when
	 *	there is no search text, using the appropriate colours, etc.
	 */
	class IdleTextCEdit : public CEdit
	{
	public:
		IdleTextCEdit();

		const std::wstring & idleText() const { return idleText_; }
		void idleText( const std::wstring& idleText );

	private:
		std::wstring idleText_;

		bool idle() const;

		afx_msg void OnPaint();
		afx_msg void OnSetFocus( CWnd* pOldWnd );
		afx_msg void OnKillFocus( CWnd* pNewWnd );
		DECLARE_MESSAGE_MAP()
	};


	CToolTipCtrl toolTip_;
	IdleTextCEdit search_;	// Text that appears when empty and not in focus.
	CStatic filters_;	// Clickable icon to the right of the search text.
	CStatic close_;		// Button that clears the search text when clicked.

	void resizeInternal( int cx, int cy );
};

#endif // SEARCH_FIELD_HPP