/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef XCONSOLE_HPP
#define XCONSOLE_HPP

#include "cstdmf/stdmf.hpp"
#include "moo/device_callback.hpp"
#include "font.hpp"

#include <string>

const int MAX_CONSOLE_WIDTH  = 200;
const int MAX_CONSOLE_HEIGHT = 100;

class InputHandler;

/**
 *	This class is the base class for all consoles.
 */
class XConsole : Moo::DeviceCallback
{
public:
	///	@name Constructors and Destructors.
	//@{
	XConsole();
	virtual ~XConsole();
	//@}

	virtual bool init();
	virtual void update() {}
	virtual InputHandler * asInputHandler() { return NULL; }
	virtual void activate( bool isReactivate );
	virtual void deactivate() {}
	virtual void onScroll() {}

	virtual bool allowDarkBackground() const;

	// console functionality

	/// @name Print Methods
	//@{
	void print( const std::wstring &string );
	void print( const std::string &string );
	void formatPrint( const char * format, ... );
	//@}

	/// @name General Methods
	//@{
	void clear();
	virtual void draw( float dTime );
	void drawCursor( float dTime );
	void toggleFontScale();

	void scrollDown();
	void scrollUp();
	//@}

	/// @name Accessors
	//@{
	virtual void setConsoleColour( uint32 colour );
	uint32 consoleColour() const;
	void setScrolling( bool state );

	void lineColourOverride( int line, uint32 colour );
	void lineColourOverride( int line );
	bool lineColourRetrieve( int line, uint32 & rColour ) const;

	int scrollOffset() const;
	void scrollOffset( int offset );

	FontPtr font()					{ return font_; }
	int visibleWidth() const;
	int visibleHeight() const;

	const wchar_t * line( int line ) const;
	wchar_t * line( int line );
	void line( int line, const wchar_t* src, size_t len );
	//@}

	/// @name Cursor Methods
	//@{
	void setCursor( uint8 cx, uint8 cy );
	virtual void hideCursor();
	void showCursor();

	bool isCursorShowing() const;

	uint8 cursorX() const;
	uint8 cursorY() const;
	void cursorX( uint8 x );
	void cursorY( uint8 y );
	//@}

	Vector2 consoleOffset() { return offset_; }
	void consoleOffset( Vector2 off ) { offset_ = off; }
	virtual void visibleAreaChanged( int oldVisibleWidth, int oldVisibleHeight ) {}

protected:
	// Helper functions
	void fillLine( int line, wchar_t character );
	void drawCharacter( uint8 x, uint8 y );
	void newline();
	void setChar( int row, int column, wchar_t value );

	// Member variables
	FontPtr			font_;
	int             visibleWidth_;
	int             visibleHeight_;
	bool            fitToScreen_;

	typedef std::vector<FontPtr> FontVector;
	FontVector fonts_;

	// console buffer
	wchar_t			buffer_[MAX_CONSOLE_HEIGHT][MAX_CONSOLE_WIDTH];
	uint32			consoleColour_;
	bool			scrollable_;
	bool			showCursor_;
	bool			shouldWrap_;

	/**
	 * TODO: to be documented.
	 */
	struct LineOverride
	{
		bool			inUse;
		uint32			colour;
	};

	LineOverride lineColours_[MAX_CONSOLE_HEIGHT];
	virtual void createUnmanagedObjects();

	int				scrollOffset_;

	// cursor position
	uint8			cursorX_;
	uint8			cursorY_;

	Vector2			offset_;

private:
	XConsole(const XConsole&);
	XConsole& operator=(const XConsole&);

	void selectFontBestMatch();
	void updateVisibleArea();
};


#ifdef CODE_INLINE
	#include "xconsole.ipp"
#endif


#endif // XCONSOLE_HPP
