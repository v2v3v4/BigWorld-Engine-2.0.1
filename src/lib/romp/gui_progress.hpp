/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_PROGRESS_HPP
#define GUI_PROGRESS_HPP


#include "progress.hpp"
#include "ashes/simple_gui.hpp"

class SimpleGUIComponent;

/**
 * TODO: to be documented.
 */
class GUIProgressDisplay : public ProgressDisplay
{
public:
	GUIProgressDisplay( const std::string& guiName, ProgressCallback pCallback = NULL );
	virtual ~GUIProgressDisplay();

	SmartPointer< SimpleGUIComponent > gui();

	virtual void add( const std::string & str );
	virtual void append( const std::string & str );
	virtual bool draw( bool force = false );
protected:	
	virtual void add( ProgressTask & task, const std::string & name );

	SmartPointer< SimpleGUIComponent >	gui_;
};

#endif
