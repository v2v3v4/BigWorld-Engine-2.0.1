/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_CAMERA_HPP
#define WORLD_EDITOR_CAMERA_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "common/base_camera.hpp"
#include "cstdmf/singleton.hpp"


class WorldEditorCamera :
			public Singleton< WorldEditorCamera >, public ReferenceCount
{
public:
	WorldEditorCamera();
	
	enum CameraType
	{
		CT_MouseLook	= 0,
		CT_Orthographic = 1
	};

	BaseCamera & currentCamera();
	BaseCamera const & currentCamera() const;
	BaseCamera & camera( CameraType type );

	void changeToCamera( CameraType type );

	void zoomExtents( ChunkItemPtr item, float sizeFactor = 1.2 );

	void respace( const Matrix& view );

private:
	Vector3 getCameraLocation() const;

private:
	typedef SmartPointer<BaseCamera> BaseCameraPtr;

	std::vector<BaseCameraPtr>				cameras_;
	CameraType								currentCameraType_;
};


#endif // WORLD_EDITOR_CAMERA_HPP
