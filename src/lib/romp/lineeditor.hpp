/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINEEDITOR_HPP
#define LINEEDITOR_HPP

#include "input/input.hpp"
#include <map>

// Forward declarations
class XConsole;

/**
 *	This class handles key events to edit a line of text
 */
class LineEditor
{
public:
	typedef std::vector<std::wstring> StringVector;

	enum ProcessState
	{
		NOT_HANDLED,
		PROCESSED,
		RESULT_SET,
	};


	LineEditor(XConsole * console);
	~LineEditor();

	ProcessState processKeyEvent( KeyEvent event, std::wstring & resultString );

	const std::wstring& editString() const			{ return editString_; }
	void editString( const std::wstring & s );

	int cursorPosition() const						{ return cx_; }
	void cursorPosition( int pos );

	bool advancedEditing() const					{ return advancedEditing_; }
	void advancedEditing( bool enable )				{ advancedEditing_ = enable; }
	
	void tick( float dTime );
	void deactivate();

	const StringVector history() const;
	void setHistory(const StringVector & history);
	void insertHistory( const std::wstring& history );

	void lineLength( int length ) { lineLength_ = length; }	
	int lineLength() { return lineLength_; }

private:
	LineEditor(const LineEditor&);
	LineEditor& operator=(const LineEditor&);

	bool processAdvanceEditKeys( KeyEvent event );
	
	int insertChar( int pos, wchar_t c );
	void deleteChar( int pos );
	int curWordStart( int pos ) const;
	int curWordEnd( int pos ) const;
	
	std::wstring cutText( int start, int end );
	int pasteText( int pos, const std::wstring & text );
	
	void showHistory();

	std::wstring editString_;		// the currently edited string
	std::wstring clipBoard_;		// clip-board string

	int		cx_;
	bool	inOverwriteMode_;
	bool	advancedEditing_;
	char	lastChar_;

	std::vector<std::wstring>	 history_;
	int	historyShown_;
	
	int lineLength_;
	
	XConsole * console_;
};

#ifdef CODE_INLINE
#include "lineeditor.ipp"
#endif


#endif // LINEEDITOR_HPP
