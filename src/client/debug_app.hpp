/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEBUG_APP_HPP
#define DEBUG_APP_HPP


#include "cstdmf/main_loop_task.hpp"

class FrameRateGraph;
class PythonServer;
class VersionInfo;


/**
 *	Debug task
 */
class DebugApp : public MainLoopTask
{
public:
	DebugApp();
	~DebugApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

	static DebugApp instance;

	VersionInfo *		pVersionInfo_;
	FrameRateGraph *	pFrameRateGraph_;

private:

#if ENABLE_PYTHON_TELNET_SERVICE
	PythonServer *		pPyServer_;
#else
	PyObject *			pPyServer_;
#endif

	std::string			driverName_;
	std::string			driverDesc_;
	std::string			deviceName_;
	std::string			hostName_;

	float				dTime_;
	float				fps_;
	float				fpsAverage_;
	float				maxFps_;
	float				minFps_;
	float				fpsCache_[50];
	float				dTimeCache_[50];
	int					fpsIndex_;
	float				timeSinceFPSUpdate_;

public:
	float				slowTime_;
	bool				drawSpecialConsole_;
	bool				shouldBreakOnCritical_;
	bool				shouldAddTimePrefix_;
};


#endif // DEBUG_APP_HPP


// debug_app.hpp
