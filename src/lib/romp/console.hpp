/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ROMP_CONSOLE_HPP
#define ROMP_CONSOLE_HPP

#include "cstdmf/config.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/dogwatch.hpp"

#include "resmgr/datasection.hpp"

#include "romp/xconsole.hpp"
#include "romp/lineeditor.hpp"

#include "ashes/text_gui_component.hpp"

#include "input/input.hpp"

#include <vector>
#include <string>


/**
 *	This class is a simple base class for consoles that want input.
 */
class InputConsole : public XConsole, InputHandler
{
private:
	/// This method returns this object as an input handler.
	InputHandler * asInputHandler() { return this; };
};


/**
 *	This class is used to show statistic information.
 */
class StatisticsConsole : public InputConsole
{
public:
	/**
	 * TODO: to be documented.
	 */
	class Handler
	{
	public:
		virtual void displayStatistics( XConsole & console ) = 0;
	};

	StatisticsConsole( Handler * pHandler );
	virtual bool init();

	virtual bool handleKeyEvent( const KeyEvent & event );

protected:
	virtual void update();

private:
	DogWatchManager::iterator followPath();

	/// the path to the selected manifestation
	std::vector<int>	path_;

	Handler * pHandler_;
};


/**
 *	This class is used to display resource usage statistics.
 */
class ResourceUsageConsole : public InputConsole
{
public:
	/**
	 * The resource counter must implement this interface in order to display the 
	 * resource usage statistics to the console.
	 */
	class Handler
	{
	public:
		virtual void displayResourceStatistics(XConsole & console) = 0;
		virtual void dumpToCSV(XConsole & console) = 0;
		virtual void cycleGranularity() = 0;
	};

	ResourceUsageConsole(ResourceUsageConsole::Handler * pHandler);

	virtual bool init();

	virtual bool handleKeyEvent(const KeyEvent & event);

private:
	virtual void update();

	ResourceUsageConsole::Handler * pHandler_;
};


/**
 *	This class is used to show histogram information.
 */
class HistogramConsole : public InputConsole
{
	bool Luminance_;
	bool R_;
	bool G_;
	bool B_;
	bool Background_;
	unsigned int topRatio_;

public:
	virtual bool allowDarkBackground() const;
	virtual bool init();
	virtual void activate( bool isReactivate );
	virtual void deactivate();

	virtual bool handleKeyEvent( const KeyEvent & event );
protected:
	virtual void update();
};

/**
 *	This class is used to show help information.
 */
class HelpConsole : public XConsole
{
public:
	HelpConsole( int screen );

	virtual bool init();

private:
	int		screen_;
};


/**
 *	This class extends the XConsole with editing capabilities.
 */
class EditConsole : public InputConsole
{
public:
	EditConsole();

	/// Get the current prompt
	const std::wstring & prompt() const				{ return prompt_; }
	/// Set the prompt
	void prompt( const std::wstring & prompt )		{ prompt_ = prompt; }

	/// Set the line number to draw the edit string on
	void	editLine( int lineNumber )				{ editLine_ = lineNumber; }
	/// Get the line number to draw the edit string on
	int		editLine()								{ return editLine_; }

	void	editCol( int col )						{ editCol_ = col; }

	int		editCol() const							{ return editCol_; }

	void	editColour( uint32 colour )			{ editColour_ = colour; }
	uint32 editColour()	const					{ return editColour_;	}

	// XConsole override
	virtual void	hideCursor();

	// InputHandler overrides
	virtual bool handleKeyEvent( const KeyEvent & event );
	virtual bool handleAxisEvent( const AxisEvent & event );

	virtual void draw( float dTime );
	virtual void visibleAreaChanged( int oldVisibleWidth, int oldVisibleHeight );

protected:
	// XConsole override
	virtual void update();
	virtual void deactivate();

	// Methods derived classes should override.

	/**
	 *	This method should be overridden by derived classes. It is called
	 *	whenever there is a line for this console to process.
	 *
	 *	@param line	A string containing the current line to be processed.
	 */
	virtual void processLine( const std::wstring & line ) = 0;

protected:
	virtual void createUnmanagedObjects();

	LineEditor	lineEditor_;	///< The line editor for this console.
	std::wstring prompt_;		///< The prompt to display.
	int			editLine_;		///< The line number of the line being edited.
	int			editCol_;		///< The col number of the line being edited.
	uint32		editColour_;	
};


#if ENABLE_WATCHERS
/**
 *	This is the debug console.
 */
class DebugConsole : public EditConsole
{
public:
	~DebugConsole();
	virtual bool init();

protected:
	virtual void update();
	virtual void processLine( const std::wstring & line );

	virtual bool handleKeyEvent( const KeyEvent& event );

	virtual void activate( bool isReactivate );
	virtual void deactivate();

	void incrementLine( float increment );
	void selectItem( int itemNum );
	void edit( const std::string & label, const std::string & valueStr );

public:
	static int highlightItem_;

private:
	std::string path_;
};
#endif /* ENABLE_WATCHERS */

/**
 *	This is the Python console.
 */
class PythonConsole : public EditConsole
{
public:
	PythonConsole();

	virtual bool init();
	virtual void draw( float dTime );
	virtual void update();
	
	const LineEditor::StringVector history() const;
	void setHistory(const LineEditor::StringVector & history);

protected:
	virtual bool handleKeyEvent( const KeyEvent & event );
	virtual void processLine( const std::wstring & line );

private:
	void completeLine( int compIndex );

	std::wstring multiline_;

	bool isMultiline_;
	int  autoCompIndex_;

	std::wstring lastLineStart_;
	std::wstring lastLineExcess_;
};


/**
 *	This class implements a console that displays text from a data section.
 */
class TextConsole : public InputConsole
{
public:
	TextConsole();
	bool init( DataSectionPtr pSection );

protected:
	virtual void activate( bool isReactivate );
	virtual void onScroll();
	virtual bool handleKeyEvent( const KeyEvent & event );

private:
	int numPages() const;
	void refresh();

	DataSectionPtr pSection_;
	int pageNum_;
};


#ifdef CODE_INLINE
	#include "console.ipp"
#endif


#endif // ROMP_CONSOLE_HPP
