/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEVICE_APP_HPP
#define DEVICE_APP_HPP

#include <vector>

#include "pathed_filename.hpp"
#include "cstdmf/main_loop_task.hpp"


class ProgressDisplay;
class GUIProgressDisplay;
class ProgressTask;

struct MessageTimePrefix;


/**
 *	Device task
 */
class DeviceApp : public MainLoopTask
{
public:
	DeviceApp();
	~DeviceApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void inactiveTick( float dTime );
	virtual void draw();

	void deleteGUI();
	bool savePreferences();

private:
	float								dTime_;
	Vector3								bgColour_;
	bool								soundEnabled_;

	MessageTimePrefix*					messageTimePrefix_;

	PathedFilename						preferencesFilename_;

public:
	static DeviceApp					instance;
	static HINSTANCE					s_hInstance_;
	static HWND							s_hWnd_;
	static ProgressDisplay *			s_pProgress_;
	static GUIProgressDisplay *			s_pGUIProgress_;
	static ProgressTask *				s_pStartupProgTask_;
	static std::vector< PyModelPtr >	updateModels_;
};


#endif // DEVICE_APP_HPP


// device_app.hpp
