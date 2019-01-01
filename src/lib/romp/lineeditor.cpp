/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include <windows.h>

#include "lineeditor.hpp"
#include "xconsole.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "UI", 0 )


#ifndef CODE_INLINE
#include "lineeditor.ipp"
#endif


namespace { // anonymous

// Named constants
const int   MAX_HISTORY_ENTRIES		= 50;
const int   MAX_LINES_OFFSET        = 5;
const std::wstring SEPARATORS		= L" `~!@#$%^&*()_+-=[]{}\\|;:'\",./<>?";

// Helper functions
bool isSeparator( wchar_t character );

} // namespace anonymous

#define EMPTY_STR(str) \
		(str.empty() || str.find_first_not_of(32) == std::wstring::npos)

// -----------------------------------------------------------------------------
// section: LineEditor
// -----------------------------------------------------------------------------

LineEditor::LineEditor(XConsole * console)
:cx_( 0 ),
 inOverwriteMode_( false ),
 advancedEditing_( false ),
 lastChar_( 0 ),
 historyShown_( -1 ),
 console_(console)
{
	BW_GUARD;
	lineLength_ = console_->visibleWidth();
}

LineEditor::~LineEditor()
{
	BW_GUARD;
}



/**
 *	This method processes key down events
 */
LineEditor::ProcessState LineEditor::processKeyEvent(
	KeyEvent event, std::wstring& resultString )
{
	BW_GUARD;
	const KeyCode::Key eventKey = event.key();

	bool isResultSet = false;
	bool isHandled = false;

	if (event.isKeyDown())
	{
		wchar_t keyChar = L'\0';
		const wchar_t* charString = event.utf16Char();

		if (charString[0] >= 0x20)
		{
			keyChar = charString[0];
		}

		
		isHandled = this->processAdvanceEditKeys( event );
		if (!isHandled)
		{
			isHandled = true;
			switch (eventKey)
			{
			case KeyCode::KEY_RETURN:
			case KeyCode::KEY_NUMPADENTER:
				if (!event.isAltDown())
				{
					resultString = editString_;
					isResultSet = true;
					editString_ = L"";
					cx_ = 0;
					lastChar_ = 0;
				}
				else
				{
					isHandled = false;
				}
				break;
			case KeyCode::KEY_BACKSPACE:
				if ( cx_ )
				{
					cx_--;
					deleteChar( cx_ );
				}
				break;
			case KeyCode::KEY_DELETE:
				if ( cx_ < (int)editString_.length( ) )
					deleteChar( cx_ );
				break;

			case KeyCode::KEY_INSERT:
				inOverwriteMode_ = !inOverwriteMode_;
				break;

			case KeyCode::KEY_LEFTARROW:
				if ( cx_ > 0 )
					cx_--;
				break;

			case KeyCode::KEY_RIGHTARROW:
				if ( cx_ < (int)editString_.length() )
					cx_++;
				break;

			case KeyCode::KEY_UPARROW:
				if (history_.size() > 0) 
				{
					if (historyShown_ == -1)
					{
						history_.insert( history_.begin(), editString_ );
						historyShown_ = 1;
					}
					else 
					{
						if (!EMPTY_STR(editString_))
						{
							history_[historyShown_] = editString_;
						}
						++historyShown_;
					}
					showHistory();
				}
				break;

			case KeyCode::KEY_DOWNARROW:
				if (history_.size() > 0) 
				{
					if (historyShown_ == -1)
					{
						history_.insert( history_.begin(), editString_ );
						historyShown_ = history_.size() - 1;
					}
					else 
					{
						if (!EMPTY_STR(editString_))
						{
							history_[historyShown_] = editString_;
						}
						--historyShown_;
					}
					showHistory();
				}
				break;

			case KeyCode::KEY_HOME:
				cx_ = 0;
				break;

			case KeyCode::KEY_END:
				cx_ = editString_.length();
				break;

			default:
				isHandled = false;
				break;
			}
		}

		if (!isHandled && keyChar != 0)
		{
			isHandled = true;

			cx_ += this->insertChar( cx_, keyChar );

			lastChar_ = 0;
		}
		else if (event.isCtrlDown())
		{
			if (event.isKeyDown() && eventKey == KeyCode::KEY_U)
			{
				int assignmentLocation = editString_.find_last_of( L"=" );

				if (assignmentLocation > 0)
				{
					editString_ = editString_.substr( 0, assignmentLocation + 1 );
					cx_ = min( cx_, (int)editString_.length() );

					isHandled = true;
				}
			}
		}
	}
	
	// end of line, request processing
	if (isResultSet)
	{
		insertHistory( resultString );
		return RESULT_SET;
	}

	if (isHandled)
	{
		return PROCESSED;
	}

	return NOT_HANDLED;
}

/**
 * Sets contents of edit string.
 */
void LineEditor::editString( const std::wstring & s )
{
	BW_GUARD;
	editString_ = s.length() < uint(this->lineLength()) - MAX_LINES_OFFSET
		? s
		: s.substr(0, this->lineLength() - MAX_LINES_OFFSET);

	cx_ = min( cx_, int(editString_.size()) );
}


/**
 * Sets current cursor position.
 */
void LineEditor::cursorPosition( int pos )					
{ 
	BW_GUARD;
	cx_ = min( pos, int(editString_.size()) );
}


/**
 *	Processes key events, handling advanced editing commands. Advance editing
 *	commands are ignored if the advancedEditing flag is set to false. Bellow
 *	are the supported commands:
 *
 *	CTRL + <-		Go to beginning of word
 *	CTRL + ->		Go to end of word
 *	CTRL + A		Go to beginning of line (same as HOME)
 *	CTRL + E		Go to end word (same as END)
 *	CTRL + D		Delete character to the right (same as DELETE)
 *	CTRL + H		Delete character to the left (same as BACKSPACE)
 *	CTRL + K		Delete line to right of cursor (copies to clipboard)
 *	CTRL + U		Delete line to left of cursor (copies to clipboard)
 *	CTRL + [R|DEL]	Delete word to right of cursor (copies to clipboard)
 *	CTRL + [W|BS]	Delete word to left of cursor (copies to clipboard)
 *	CTRL + [Y|INS]	Paste clipboard contents
 */
bool LineEditor::processAdvanceEditKeys( KeyEvent event )
{
	BW_GUARD;
	if (!this->advancedEditing_) 
	{
		return false;
	}

	bool isHandled = false;
	if (event.isCtrlDown()) 
	{
		isHandled = true;
		switch (event.key())
		{
		case KeyCode::KEY_LEFTARROW:
		case KeyCode::KEY_JOY2:	// dpad left
			cx_ = curWordStart( cx_ );
			break;

		case KeyCode::KEY_RIGHTARROW:
		case KeyCode::KEY_JOY3:	// dpad right
			cx_ = curWordEnd( cx_ );
			break;

		case KeyCode::KEY_A:
			cx_ = 0;
			break;

		case KeyCode::KEY_E:
			cx_ = editString_.length();
			break;

		case KeyCode::KEY_D:
			if ( cx_ < (int)editString_.length( ) )
			{
				deleteChar( cx_ );
			}
			break;

		case KeyCode::KEY_H:
			if ( cx_ > 0 )
			{
				cx_--;
				deleteChar( cx_ );
			}
			break;

		case KeyCode::KEY_K:
			clipBoard_ = cutText( cx_, editString_.length() );
			break;

		case KeyCode::KEY_U:
			clipBoard_ = cutText( 0, cx_ );
			cx_ = 0;
			break;

		case KeyCode::KEY_W:
		case KeyCode::KEY_BACKSPACE:
		{
			int wordStart = curWordStart( cx_ );
			clipBoard_ = cutText( wordStart, cx_ );
			cx_ = wordStart;
			break;
		}

		case KeyCode::KEY_R:
		case KeyCode::KEY_DELETE:
		{
			clipBoard_ = cutText( cx_, curWordEnd( cx_ ) );
			break;
		}

		case KeyCode::KEY_Y:
		case KeyCode::KEY_INSERT:
			cx_ = pasteText( cx_, clipBoard_ );
			break;

		default:
			isHandled = false;
			break;
		}
	}	
	return isHandled;
}



/**
 *	Gives the line editor a chance to process time 
 *	based operations, like key repeat an the like. 
 */
void LineEditor::tick( float dTime )
{
	BW_GUARD;
}


/**
 *	Deactivates the line editor. Clears the down keys map.
 */
void LineEditor::deactivate()
{
	BW_GUARD;
}


/**
 *	Retrieves the current command history list.
 */
const LineEditor::StringVector LineEditor::history() const
{
	BW_GUARD;
	struct encodeSpaces
	{
		static std::wstring doit(const std::wstring &input)
		{
			std::wstring output = input;
			std::wstring::size_type pos = 0;
			while ((pos = output.find(L"\\", pos)) != std::wstring::npos)
			{
				output.replace(pos, 1, L"\\c");
				pos += 2;
			}
			pos = 0;
			while ((pos = output.find(L" ", pos)) != std::wstring::npos)
			{
				output.replace(pos, 1, L"\\s");
				pos += 2;
			}			
			return output;
		}
	};

	// reverse history to make it more 
	// readable when output to text format
	StringVector result(this->history_.size());
	std::reverse_copy(this->history_.begin(), this->history_.end(), result.begin());
	std::transform(result.begin(), result.end(), result.begin(), &encodeSpaces::doit);
	return result;
}

/**
 *	Inserts the given string into the next spot in the history list.
 *	This is used when the user pressed enter to execute a line.
 */
void LineEditor::insertHistory( const std::wstring& s )
{
	BW_GUARD;
	if (!EMPTY_STR(s))
	{
		if (history_.size() > 0 && historyShown_ != -1)
		{
			history_[ 0 ] = s;
		}
		else
		{
			history_.insert( history_.begin(), s );
		}
	}
	else
	{
		if (history_.size() > 0 && EMPTY_STR(history_[ 0 ]))
		{
			history_.erase(history_.begin());
		}
	}
	// clamp history 
	if (history_.size() > MAX_HISTORY_ENTRIES)
	{
		history_.erase( history_.end()-1 );
	}
	historyShown_ = -1;
}


/**
 *	Replaces the command history list.
 */
void LineEditor::setHistory(const StringVector & history)
{
	BW_GUARD;
	struct decodeSpaces
	{
		static std::wstring doit(const std::wstring &input)
		{
			std::wstring output = input;
			std::wstring::size_type pos = 0;
			while ((pos = output.find(L"\\s", pos)) != std::wstring::npos)
			{
				output.replace(pos, 2, L" ");
				++pos;
			}
			pos = 0;
			while ((pos = output.find(L"\\c", pos)) != std::wstring::npos)
			{
				output.replace(pos, 2, L"\\");
				++pos;
			}			
			return output;
		}
	};

	this->history_.resize(history.size());
	std::reverse_copy(history.begin(), history.end(), this->history_.begin());
	std::transform(
		this->history_.begin(), this->history_.end(), 
		this->history_.begin(), &decodeSpaces::doit);
		
	this->historyShown_ = -1;
}


/**
 *	Insert the character at pos
 *
 *	@return amount to increase pos by
 */
int LineEditor::insertChar( int pos, wchar_t c )
{
	BW_GUARD;
	if (editString_.length() >= uint(this->lineLength()) - MAX_LINES_OFFSET) 
	{
		return 0;
	}

	if (pos < (int)editString_.length())
	{
		if (inOverwriteMode_)
		{
			editString_[pos] = c;
		}
		else
		{
			editString_.insert( pos, 1, c );
		}
	}
	else	
	{
		editString_ += c;
	}

	return 1;
}


/**
 *	Delete the character at pos
 */
void LineEditor::deleteChar( int pos )
{
	BW_GUARD;
	if ( pos == editString_.length( ) )
		editString_ = editString_.substr( 0, pos - 1 );
	else
	{
		std::wstring preStr = editString_.substr( 0, pos );
		std::wstring postStr =
			editString_.substr( pos + 1, editString_.length( ) );

		editString_ = preStr + postStr;
	}
}


/**
 *	Returns index of start of current word
 */
int LineEditor::curWordStart( int pos ) const
{
	BW_GUARD;
	// go to start of separating block
	while (pos > 0 && isSeparator( editString_[ pos - 1 ] ))
	{
		--pos;
	}

	// go to start of word	
	while (pos > 0 && !isSeparator( editString_[ pos - 1 ] ))
	{
		--pos;
	}

	return pos;
}


/**
 *	Returns index of end of current word
 */
int LineEditor::curWordEnd( int pos ) const
{
	BW_GUARD;
	// go to end of word	
	while (uint( pos ) < editString_.length() && 
			!isSeparator( editString_[ pos ] ))
	{
		++pos;
	}

	// go to end of separating block
	while (uint( pos ) < editString_.length() && 
			isSeparator( editString_[ pos ] ))
	{
		++pos;
	}

	return pos;
}


/**
 *	Deletes the substring defined by the given indices and returns it.
 */
std::wstring LineEditor::cutText( int start, int end )
{
	BW_GUARD;
	std::wstring text = editString_.substr( start, end - start );
	editString_ = 
			editString_.substr( 0, start ) + 
			editString_.substr( end, editString_.length() - end ); 
	return text;
}


/**
 *	Pastes the given text into the editing string. Returns new cursor pos.
 */
int LineEditor::pasteText( int pos, const std::wstring & text )
{
	BW_GUARD;
	editString_ = 
			editString_.substr( 0, pos ) + text +
			editString_.substr( pos, editString_.length() - pos ); 
	return pos + text.length();
}

/**
 *	Show the line historyShown_ from history_
 */
void LineEditor::showHistory()
{
	BW_GUARD;
	if (historyShown_ < 0)
	{
		 historyShown_ = history_.size() - 1;
	}
	else if (historyShown_ == history_.size())
	{
		 historyShown_ = 0;
	}

	this->editString( history_[ historyShown_ ] );
	cx_ = editString_.length();
	lastChar_ = 0;
}


namespace { // anonymous

// Helper functions
bool isSeparator( wchar_t character )
{
	BW_GUARD;
	return std::find( 
		SEPARATORS.begin(), 
		SEPARATORS.end(), 
		character ) != SEPARATORS.end();
}

} // namespace anonymous

// lineeditor.cpp
