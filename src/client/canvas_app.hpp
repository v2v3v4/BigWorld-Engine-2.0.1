/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CANVAS_APP_HPP
#define CANVAS_APP_HPP


#include "cstdmf/main_loop_task.hpp"
#include "adaptive_lod_controller.hpp"


class FlashBangEffect;
class Distortion;


/**
 *	Canvas task
 */
class CanvasApp : public MainLoopTask
{
public:
	typedef std::vector<std::wstring> StringVector;

	CanvasApp();
	~CanvasApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

	static CanvasApp instance;

	std::vector<Vector4ProviderPtr>		flashBangAnimations_;
	Distortion* distortion()			{ return distortion_; }	

	void updateDistortionBuffer();
	
	const StringVector pythonConsoleHistory() const;
	void setPythonConsoleHistory(const StringVector & history);

private:
	bool setPythonConsoleHistoryNow(const StringVector & history);	

	AdaptiveLODController	lodController_;
	float gammaCorrectionOutside_;
	float gammaCorrectionInside_;
	float gammaCorrectionSpeed_;

	FlashBangEffect *		flashBang_;
	Distortion *			distortion_;

	float	dTime_;
	
	StringVector history_;
public:
	EnviroMinder::DrawSelection	drawSkyCtrl_;

	void finishFilters();
};


#endif // CANVAS_APP_HPP


// canvas_app.hpp
