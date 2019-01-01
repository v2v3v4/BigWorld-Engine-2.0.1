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

#include "script_app.hpp"

#include "action_matcher.hpp"
#include "app.hpp"
#include "app_config.hpp"
#include "connection_control.hpp"
#include "device_app.hpp"
#include "entity_manager.hpp"
#include "physics.hpp"
#include "portal_state_changer.hpp"
#include "script_bigworld.hpp"

#include "ashes/simple_gui.hpp"
#include "ashes/simple_gui_component.hpp"

#include "client/bw_winmain.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/bwversion.hpp"
#include "cstdmf/memory_trace.hpp"

#include "math/colour.hpp"

#include "pyscript/personality.hpp"
#include "pyscript/py_callback.hpp"

#include "romp/gui_progress.hpp"




ScriptApp ScriptApp::instance;

int ScriptApp_token = 1;



ScriptApp::ScriptApp()
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Script/App", NULL );
}


ScriptApp::~ScriptApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Script/App" );*/
}


bool ScriptApp::init()
{
	BW_GUARD;	
#if ENABLE_WATCHERS
	DEBUG_MSG( "ScriptApp::init: Initially using %d(~%d)KB\n",
		memUsed(), memoryAccountedFor() );
#endif
	MEM_TRACE_BEGIN( "ScriptApp::init" )

	if (!BigWorldClientScript::init( AppConfig::instance().pRoot() ))
	{
		criticalInitError( "App::init: BigWorldClientScript::init() failed!" );
		MEM_TRACE_END()

		return false;
	}


#if ENABLE_WATCHERS

	MF_WATCH( "Entities/Active Entities",	EntityManager::instance().entities(),
		static_cast<Entities::size_type (Entities::*)() const>( &Entities::size ),
		"Shows the number of entities in the players's AoI" );

	MapWatcher<Entities> * pESeq = new MapWatcher<Entities>(
			EntityManager::instance().entities() );
	pESeq->addChild( "*", new BaseDereferenceWatcher(
		&Entity::watcher() ) );
	Watcher::rootWatcher().addChild( "Entities/Entity Objects", pESeq );

	MF_WATCH( "Client Settings/Entity Collisions", ActionMatcher::globalEntityCollision_,
		Watcher::WT_READ_WRITE, "Enables entity - entity collision testing" );

	/*MF_WATCH( "Client Settings/Debug Action Matcher", ActionMatcher::debug_, Watcher::WT_READ_WRITE,
				"if this is set to true, action matcher will run in debug mode "
				"that means, every model that is using action matcher will has an string displayed on "
				"top of it shows the current parameters");*/

#endif // ENABLE_WATCHERS

	DataSectionPtr pConfigSection = AppConfig::instance().pRoot();
	SimpleGUI::init( pConfigSection );

	//Now the scripts are setup, we can load a progress bar
	DeviceApp::s_pGUIProgress_ = new GUIProgressDisplay( loadingScreenGUI, BWProcessOutstandingMessages );
	if (DeviceApp::s_pGUIProgress_->gui())
	{
		if (!DeviceApp::s_pGUIProgress_->gui()->script().exists())
		{
			SimpleGUI::instance().addSimpleComponent( 
				*DeviceApp::s_pGUIProgress_->gui() );
		}
		DeviceApp::s_pProgress_    = DeviceApp::s_pGUIProgress_;
	}
	else
	{
		delete DeviceApp::s_pGUIProgress_;
		DeviceApp::s_pGUIProgress_ = NULL;
		// Legacy Progress bar
		//NOTE : only reading the loading screen from ui/loadingScreen
		//for legacy support.  loading screen is not AutoConfig'd from
		//resources.xml in the system/loadingScreen entry.
		if (loadingScreenName.value() == "")
		{
			pConfigSection->readString( "ui/loadingScreen", "" );
		}
		Vector3 colour = pConfigSection->readVector3( "ui/loadingText", Vector3(255,255,255) );
		DeviceApp::s_pProgress_ = new ProgressDisplay( NULL, displayLoadingScreen, Colour::getUint32( colour, 255 ) );
	}

	char buf[ 256 ];
	bw_snprintf( buf, sizeof(buf), "Build: %s %s. Version: %s",
		configString, App::instance().compileTime(), BWVersion::versionString().c_str() );
	DeviceApp::s_pProgress_->add( buf );

	if (DeviceApp::s_pGUIProgress_)
	{
		DeviceApp::s_pStartupProgTask_ = new ProgressTask( DeviceApp::s_pProgress_, "App Startup", PROGRESS_TOTAL );
	}
	else
	{
		//subtract one from progress total because there is no personality script part to it.
		DeviceApp::s_pStartupProgTask_ = new ProgressTask( DeviceApp::s_pProgress_, "App Startup", PROGRESS_TOTAL - 1 );
	}
	bool ret = !g_bAppQuit && DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP);

	MEM_TRACE_END()

	return ret;
}


void ScriptApp::fini()
{
	BW_GUARD;	
#if ENABLE_WATCHERS
	Watcher::fini();
#endif
	DeviceApp::instance.deleteGUI();
	SimpleGUI::instance().clearSimpleComponents();
	BigWorldClientScript::fini();
	SimpleGUI::fini();
}


void ScriptApp::tick( float dTime )
{
	BW_GUARD;
	double totalTime = App::instance().getTime();

	static DogWatch	dwScript("Script");
	dwScript.start();

	BigWorldClientScript::tick();

	// tick all vector4 / matrix providers.  do this before scripts
	// are run, so they can access up-to-date values, and any new
	// providers created are not ticked immediately.
	ProviderStore::tick( dTime );

	// very first call any timers that have gone off
	// (including any completion procedures stored by action queues
	//  on their last tick)
	Script::tick( totalTime );

	static DogWatch	dwPhysics("Physics");
	dwPhysics.start();

	// TODO: There is a weird bug related to the following line of code. It
	//	shows up when total time is very large. Try it by setting
	//	App::totalTime_ to something larger than 600,000. It looks as though
	//	the arguments to the following call are rounded to floats. This causes
	//	rounding problems.

	// call all the wards ticks for controlled entities
	// very important to call this (i.e. filter input) before
	// the entitymanager's (i.e. filter output)
	// also note: physics may call callbacks of its own
	Physics::tickAll( totalTime, totalTime - dTime );

	dwPhysics.stop();

	static DogWatch dwEntity("Entity");
	dwEntity.start();

	// call all the entitymanager's ticks
	EntityManager::instance().tick( totalTime, totalTime - dTime );

	// tell the server anything we've been waiting to
	ServerConnection * pSC = ConnectionControl::serverConnection();
	if (pSC != NULL && pSC->online())
	{
		if (totalTime - pSC->lastSendTime() >= pSC->minSendInterval())
			pSC->send();
	}

	dwEntity.stop();

	// see if the player's environemnt has changed
	static int oldPlayerEnvironment = -1;
	int newPlayerEnvironment = !isPlayerOutside();

	if (oldPlayerEnvironment != newPlayerEnvironment)
	{
		Script::call(
			PyObject_GetAttrString( Personality::instance(),
				"onChangeEnvironments" ),
			Py_BuildValue( "(i)", newPlayerEnvironment ),
			"Personality onChangeEnvironments: " );

		oldPlayerEnvironment = newPlayerEnvironment;
	}

	PortalStateChanger::tick( dTime );

	dwScript.stop();
}

void ScriptApp::inactiveTick( float dTime )
{
	BW_GUARD;
	this->tick( dTime );
}


void ScriptApp::draw()
{

}


// script_app.cpp
