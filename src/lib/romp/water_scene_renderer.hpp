/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _WATER_SCENE_RENDERER_HPP
#define _WATER_SCENE_RENDERER_HPP

#include "romp/texture_renderer.hpp"
#include "math/planeeq.hpp"

class PyModel;
class Water;
class ChunkSpace;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;

namespace Moo
{
	class LightContainer;
	typedef SmartPointer<LightContainer>	LightContainerPtr;
};

class WaterSceneRenderer;

/**
 * TODO: to be documented.
 */
class WaterScene : public TextureSceneInterface
{
public:
	WaterScene(float height);
	~WaterScene();

	bool checkOwners() const;
	void addOwner( const Water* owner ) { owners_.push_back(owner); }
	void removeOwner( const Water* owner ) { owners_.remove(owner); }
	bool getVisibleChunks( std::vector<class Chunk*>& visibleChunks, bool& outsideVisible, std::vector<Vector3>& verts );
#ifdef EDITOR_ENABLED
	bool drawRed();
	bool highlight();
#endif

	bool recreate( );
	void deleteUnmanagedObjects();
	bool closeToWater( const Vector3& pos );
	float waterHeight() const { return waterHeight_; }
	const Vector4& cameraPos() const { return camPos_; }
	bool shouldReflect( const Vector3& pos, PyModel* model );
	void setCameraPos( const Vector4& val ) { camPos_ = val; }

	void dynamic( bool state );	
	void staggeredDynamic( bool state );
	bool dynamic() const { return dynamic_; }

	Moo::RenderTargetPtr reflectionTexture();

	virtual bool cachable() const { return true; }
	virtual bool shouldDraw() const;
	virtual void render( float dTime );

	//TODO: create some general purpose clip plane functionality
	PlaneEq currentClipPlane() 
	{ 
		return PlaneEq(Vector3(0,waterHeight(),0), Vector3(0,1,0));
	}

	bool eyeUnderWater() const;

private:
	typedef std::list< const Water* > OwnerList;

	bool								dynamic_;
	//bool								drawingReflection_;	
	float								waterHeight_;
	Vector4								camPos_;
	OwnerList							owners_;

	WaterSceneRenderer*					reflectionScene_;
//	WaterSceneRenderer*					refractionScene_;	
};


/**
 * TODO: to be documented.
 */
class WaterSceneRenderer : public TextureRenderer
{
public:
	WaterSceneRenderer( WaterScene* scene, bool reflect=false );
	~WaterSceneRenderer();

	virtual void renderSelf( float dTime );

	bool recreate( );
	void deleteUnmanagedObjects();

	void setTexture( Moo::RenderTargetPtr texture ) { pRT_ = texture; }
	Moo::RenderTargetPtr getTexture() { return pRT_; }

	virtual bool shouldDraw() const;

	static void setPlayerModel( PyModel* pModel ) { playerModel_ = pModel; }
	static PyModel* playerModel(  ) { return playerModel_; }

	static void incReflectionCount() { reflectionCount_++; }
	static uint32 reflectionCount() { return reflectionCount_; }
	void clearReflectionCount() { reflectionCount_=0; }

	static WaterScene* currentScene() { return currentScene_; }
	static float currentCamHeight() { return currentScene()->cameraPos().y; }

	float waterHeight() const { return scene_->waterHeight(); }

	WaterScene* scene() { return scene_; }

	bool shouldReflect( const Vector3& pos, PyModel* model );

	static void resetSceneCounter() { drawnSceneCount_=0; }

private:
	WaterSceneRenderer( const WaterSceneRenderer& );
	WaterSceneRenderer& operator=( const WaterSceneRenderer& );

	void createRenderTarget();

	//TODO:
//	void resetDrawState();
//	void setDrawState();

	void currentScene(WaterScene* scene){ currentScene_ = scene; }

	//TODO: remove useless vars	
	bool							reflection_;
	WaterScene*						scene_;

	std::string						sceneName_;

	static PyModel*					playerModel_;
	static uint32					reflectionCount_;
	static WaterScene*				currentScene_;
	static int						drawnSceneCount_;

public:
	bool							eyeUnderWater_;

	static ChunkSpacePtr			s_pSpace_;
	static Moo::LightContainerPtr	s_lighting_;

	static bool						s_drawTrees_;
	static bool						s_drawDynamics_;
	static bool						s_drawPlayer_;
	static uint						s_maxReflections_;
	static float					s_maxReflectionDistance_;
	static bool						s_simpleScene_;
	static bool						s_useClipPlane_;
	static float					s_textureSize_;
};

#endif // _WATER_SCENE_RENDERER_HPP
