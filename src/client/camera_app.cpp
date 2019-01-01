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
#include "camera_app.hpp"

#include "app.hpp"
#include "app_config.hpp"
#include "camera/annal.hpp"
#include "camera/camera_control.hpp"
#include "camera/projection_access.hpp"
#include "chunk/chunk_manager.hpp"
#include "client_camera.hpp"
#include "device_app.hpp"
#include "player.hpp"
#include "romp/progress.hpp"

CameraApp CameraApp::instance;

int CameraApp_token = 1;

CameraApp::CameraApp()
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Camera/App", NULL );
}


CameraApp::~CameraApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Camera/App" );*/
}


bool CameraApp::init()
{
	BW_GUARD;	
#if ENABLE_WATCHERS
	DEBUG_MSG( "CameraApp::init: Initially using %d(~%d)KB\n",
		memUsed(), memoryAccountedFor() );
#endif

	DataSectionPtr configSection = AppConfig::instance().pRoot();

	ClientSpeedProvider::initInstance();
	ClientCamera::initInstance( configSection );

	MF_WATCH( "Render/Near Plane",
		*ClientCamera::instance().projAccess(),
		MF_ACCESSORS( float, ProjectionAccess, nearPlane ), "Camera near plane." );

	MF_WATCH( "Render/Far Plane",
		*ClientCamera::instance().projAccess(),
		MF_ACCESSORS( float, ProjectionAccess, farPlane ),"Camera far plane." );

	MF_WATCH( "Render/Fov",
		*ClientCamera::instance().projAccess(),
		MF_ACCESSORS( float, ProjectionAccess, fov ), "Field of view." );


	// Set up the camera
	float farPlaneDistance =
		configSection->readFloat( "camera/farPlane", 250.f );
	Moo::Camera cam( 0.25f, farPlaneDistance, DEG_TO_RAD(60.f), 2.f );
	cam.aspectRatio(
		float(Moo::rc().screenWidth()) / float(Moo::rc().screenHeight()) );
	Moo::rc().camera( cam );

	// read in some ui settings
	CameraControl::strafeRate( configSection->readFloat(
		"ui/straferate", 100.0 ) );

	CameraControl::initDebugInfo();

	return DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP);
}


void CameraApp::fini()
{
	BW_GUARD;
	ClientCamera::fini();
}


void CameraApp::tick( float dTime )
{
	BW_GUARD;
	static DogWatch	dwCameras("Cameras");
	dwCameras.start();

	ClientCamera::instance().update( dTime );

	const BaseCameraPtr pCamera = ClientCamera::instance().camera();
	Matrix cameraMatrix = pCamera->view();

	extern Matrix* g_profilerAppMatrix;

	if (g_profilerAppMatrix)
	{
		cameraMatrix = *g_profilerAppMatrix;
	}

	// and apply the change
	Moo::rc().reset();
	Moo::rc().view( cameraMatrix );
	Moo::rc().updateProjectionMatrix();
	Moo::rc().updateViewTransforms();

	// tell the chunk manager about it too
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();

	if (pCamera && pCamera->spaceID() != 0)
		pSpace = ChunkManager::instance().space( pCamera->spaceID(), false );
	else if (Player::entity() && Player::entity()->pSpace())
		pSpace = Player::entity()->pSpace();

	if (pSpace)
	{
		ChunkManager::instance().camera( Moo::rc().invView(), pSpace );
	}
	else
	{
		ChunkManager::instance().camera( Matrix::identity, NULL );
	}

	static SpaceID lsid = -1;
	SpaceID tsid = pSpace ? pSpace->id() : -1;
	if (lsid != tsid)
	{
		lsid = tsid;

		INFO_MSG("CameraApp::tick: Camera space id is %d\n", lsid );
	}

	dwCameras.stop();
}

void CameraApp::inactiveTick( float dTime )
{
	BW_GUARD;
	this->tick( dTime );
}

void CameraApp::draw()
{

}



// camera_app.cpp
