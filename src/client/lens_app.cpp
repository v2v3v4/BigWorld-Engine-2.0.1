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
#include "lens_app.hpp"


#include "romp/lens_effect_manager.hpp"
#include "romp/progress.hpp"


#include "app.hpp"
#include "canvas_app.hpp"
#include "device_app.hpp"

LensApp LensApp::instance;

int LensApp_token = 1;


LensApp::LensApp() : 
	dTime_( 0.f )
{ 
	BW_GUARD;
	MainLoopTasks::root().add( this, "Lens/App", NULL ); 
}

LensApp::~LensApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Lens/App" );*/ 
}

bool LensApp::init()
{
	BW_GUARD;
	LensEffectManager::init();
	return DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP);
}


void LensApp::fini()
{
	BW_GUARD;
	LensEffectManager::fini();
}


void LensApp::tick( float dTime )
{
	dTime_ = dTime;
}


void LensApp::draw()
{
	BW_GUARD;

	if(!::gWorldDrawEnabled)
		return;

	LensEffectManager::instance().tick( dTime_ );
	LensEffectManager::instance().draw();

	// Finish off the back buffer filters now
	CanvasApp::instance.finishFilters();
	// Note: I have moved this after drawFore, because I reckon the things
	// in there (seas, rain) prolly want to be affected by the filters too.

}


// lens_app.cpp
