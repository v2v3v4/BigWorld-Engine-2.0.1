/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROTO_MODULE_HPP
#define PROTO_MODULE_HPP

#include "moo/base_texture.hpp"
#include "appmgr/module.hpp"
#include "grid_coord.hpp"

#include "duplo/shadow_caster.hpp"

#include "utilities.hpp"

class ClosedCaptions;
class ParticleSystem;
class MaterialDrawOverride;

class MeModule : public FrameworkModule
{
public:
	MeModule();
	~MeModule();

	virtual bool init( DataSectionPtr pSection );

	virtual void onStart();
	virtual int  onStop();

	virtual bool updateState( float dTime );
	bool renderThumbnail( const std::string& fileName );
	virtual void render( float dTime );

	virtual bool handleKeyEvent( const KeyEvent & event );
	virtual bool handleMouseEvent( const MouseEvent & event );

	static MeModule& instance() { ASSERT(s_instance_); return *s_instance_; }

	float averageFPS() const { return averageFps_; }

	bool materialPreviewMode() const { return materialPreviewMode_; }
	void materialPreviewMode( bool on ) { materialPreviewMode_ = on; }

	typedef MeModule This;

	PY_MODULE_STATIC_METHOD_DECLARE( py_render )

private:
	MeModule( const MeModule& );
	MeModule& operator=( const MeModule& );
	
	void beginRender();
	void endRender();

	void renderChunks();
	void renderTerrain( float dTime = 0.f, bool shadowing = false );

	void setLights( bool checkForSparkles, bool useCustomLighting );

	bool initPyScript();
	bool finiPyScript();

	Moo::BaseTexturePtr mapTexture_;

	/** The last created MeModule */
	static MeModule* s_instance_;

	/** Camera position, X & Y specify position, Z specifies zoom */
	Vector3 viewPosition_;

	/**
	 * Where the cursor was when we started looking around,
	 * so we can restore it to here when done
	 */
	POINT lastCursorPosition_;

	/**
	 * Add to a GridCoord to transform it from a local (origin at 0,0) to a
	 * world (origin in middle of map) grid coord
	 */
	GridCoord localToWorld_;

	/** The start of the current selection */
	Vector2 selectionStart_;

	/** The currently selecting, with the mouse, rect */
	GridRect currentSelection_;

	/** Extent of the grid, in number of chunks. It starts at 0,0 */
	unsigned int gridWidth_;
	unsigned int gridHeight_;

	Vector2 currentGridPos();
	Vector3 gridPosToWorldPos( Vector2 gridPos );

	float averageFps_;

	PyObject * scriptDict_;

	SmartPointer<ClosedCaptions> cc_;

	ShadowCasterCommon* pShadowCommon_;

	ShadowCasterPtr pCaster_;

	MaterialDrawOverride* wireframeRenderer_;

	bool materialPreviewMode_;

	bool renderingThumbnail_;
	SmartPointer< Moo::RenderTarget > rt_;
};

#endif // PROTO_MODULE_HPP