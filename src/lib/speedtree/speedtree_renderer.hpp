/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_RENDERER_HPP
#define SPEEDTREE_RENDERER_HPP

// BW Tech Hearders
#include "math/vector3.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"

// Forward declarations
class Matrix;
class BSPTree;
class BoundingBox;
class EnviroMinder;
class ShadowCaster;
typedef SmartPointer<class DataSection> DataSectionPtr;

#include "speedtree_config_lite.hpp"
#if SPEEDTREE_SUPPORT
#include "speedtree_tree_type.hpp"
#endif

namespace speedtree {

class SpeedTreeSettings;

/**
 *	Single instance of a speedtree in the world. Each tree chunk item in the
 *	space will own an instance of SpeedTreeRenderer. SpeedTreeRenderer on its
 *	turn, will point to a shared TSpeedTreeType that holds the actual data
 *	needed to render the tree model.
 */ 
class SpeedTreeRenderer
{

public:
	SpeedTreeRenderer();
	~SpeedTreeRenderer();

	static void init();
	static void fini();
	
	void load(
		const char   * filename, 
		uint           seed,
		const Matrix&  world );

	static void tick( float dTime );
	
	static void beginFrame(
		EnviroMinder * envMinder, 
		ShadowCaster * caster = NULL );

	static void flush();
		
	static void endFrame();

#ifdef EDITOR_ENABLED
	static void enableLightLines( bool drawLightLines );

	void allowBatching( bool doAllow ) { allowBatching_ = doAllow; }
	bool allowBatching() const { return allowBatching_; }
#endif // EDITOR_ENABLED

	void draw( const Matrix & transform, uint32 batchCookie );
	void resetTransform( const Matrix& world, bool updateBB=true );

	const BoundingBox & boundingBox() const;
	const BSPTree & bsp() const;
	
	const char * filename() const;
	uint seed() const;

	static float lodMode( float newValue );
	static float maxLod( float newValue );
	static bool enviroMinderLighting( bool newValue );
	static bool drawTrees( bool newValue );

	#if SPEEDTREE_SUPPORT
	static bool depthOnlyPass() { return s_depthOnlyPass_; }
	#else
	static bool depthOnlyPass() { return 0; }
	#endif

	int numTris() const;
	int numPrims() const;

	
private:
	static void update();
	
	void drawRenderPass( const Matrix & transform, uint32 batchCookie );
	void drawShadowPass( const Matrix & transform, uint32 batchCookie );

	SmartPointer<class TSpeedTreeType>		treeType_;
	SmartPointer<class BillboardOptimiser>	bbOptimiser_;

	int treeID_;

	#if SPEEDTREE_SUPPORT
	TSpeedTreeType::DrawData* drawData_;
	#endif

	float windOffset_;

#ifdef EDITOR_ENABLED
	bool allowBatching_;
#endif // EDITOR_ENABLED

	static SpeedTreeSettings* s_settings_;

	static bool s_depthOnlyPass_;
};

} // namespace speedtree

#endif // SPEEDTREE_RENDERER_HPP
