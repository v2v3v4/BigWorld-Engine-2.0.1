/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_TREE_TYPE_HPP
#define SPEEDTREE_TREE_TYPE_HPP

#include "cstdmf/dogwatch.hpp"
#include "moo/effect_material.hpp"
#include "moo/effect_lighting_setter.hpp"
#include "math/boundbox.hpp"

#define NUM_EFFECTS 7
#include "speedtree_vertex_types.hpp"

class EnviroMinder;
class CSpeedTreeRT;
class CSpeedWind;

namespace speedtree {

typedef TreePartRenderData< BranchVertex >    BranchRenderData;
typedef TreePartRenderData< LeafVertex >      LeafRenderData;
typedef TreePartRenderData< BillboardVertex > BillboardRenderData;

/**
 *	Holds the actual data needed to render a specific type of tree. 
 *
 *	Although each instance of a speedtree in a scene will be stored
 *	in-memory as an instance of the SpeedTreeRenderer class, all 
 *	copies of a unique speedtree (loaded from a unique ".spt" file 
 *	or generated form a unique seed number) will be represented by 
 *	a single TSpeedTreeType object.
 */
class TSpeedTreeType
{
public:
	typedef SmartPointer< TSpeedTreeType > TreeTypePtr;
	typedef std::auto_ptr< CSpeedTreeRT > CSpeedTreeRTPtr;

	typedef std::vector< TSpeedTreeType* > RendererVector;
	typedef std::vector< RendererVector > RendererGroupVector;

	/**
	 *	Holds all data that can be cached to disk to save
	 *	time when generating the geometry of a tree (vertex
	 *	and index buffers plus bounding-boxses and leaf 
	 *	animation coefficients. Also implements the load() 
	 *	and save() functions.
	 */
	class TreeData
	{
	public:
		TreeData();
		
		bool save( const std::string & filename );
		bool load( const std::string & filename );

#ifdef OPT_BUFFERS
		void unload( );
#endif

		BoundingBox         boundingBox_;

		BranchRenderData    branches_;
		BranchRenderData    fronds_;
		LeafRenderData      leaves_;
		BillboardRenderData billboards_;

		float               leafRockScalar_;
		float               leafRustleScalar_;
	};

	/**
	 *	Holds LODing values and informations 
	 *	for all render parts of a tree.
	 */	
	struct LodData
	{
		int     branchLod_;
		float   branchAlpha_;
		bool    branchDraw_;

		int     frondLod_;
		float   frondAlpha_;
		bool    frondDraw_;

		int     leafLods_[2];
		float   leafAlphaValues_[2];
		int     leafLodCount_;
		bool    leafDraw_;

		float   billboardFadeValue_;
		bool    billboardDraw_;

		bool    model3dDraw_;
	};

	/**
	 *  Stores all the information required to draw a unique 
	 *	instance of a tree, in both immediate and batched modes.
	 *
	 *	This information includes world, view and projection 
	 *	transforms, combination of them, plus fog, log, lighing
	 *	and scalling information.
	 */
	class DrawData : public Aligned
	{
	public:

		static int s_count_;

		DrawData() :
		pDiffuseLights_( new Moo::LightContainer ),
		pSpecularLights_( new Moo::LightContainer )
		{
			s_count_+= sizeof(this);

			lodLevel_ = -1.f;
			initialised_ = false;

#if ENABLE_RESOURCE_COUNTERS
			RESOURCE_COUNTER_ADD( ResourceCounters::DescriptionPool("Speedtree/DrawData", (uint)ResourceCounters::SYSTEM),
				sizeof(*this) )
#endif
		}

		~DrawData()
		{
			s_count_-=sizeof(this);

#if ENABLE_RESOURCE_COUNTERS
			RESOURCE_COUNTER_SUB( ResourceCounters::DescriptionPool("Speedtree/DrawData", (uint)ResourceCounters::SYSTEM),
				sizeof(*this) )
#endif
		}

		Matrix  world_;
		Matrix  rotation_;
		Matrix  worldViewProj_;
		Matrix  worldView_;
		Matrix  rotationInverse_;
#ifdef EDITOR_ENABLED
		Vector4 fogColour_;
		float   fogNear_;
		float   fogFar_;
#endif
		float	treeScale_;
		float	windOffset_;
		Moo::EffectLightingSetter::BatchCookie	lightingBatch_;
		Moo::LightContainerPtr					pDiffuseLights_;
		Moo::LightContainerPtr					pSpecularLights_;

		float lodLevel_;

		LodData lod_;

		bool initialised_;

		static int32 s_defaultLightingBatch_;
	};

	TSpeedTreeType();
	~TSpeedTreeType();

	static bool initFXFiles();
	static void fini();

	// tick
	static void tick( float dTime );
	static void update();
	static bool recycleTreeTypeObjects();
	static bool doSyncInit();
	static void clearDXResources();

	// Animation
	void updateWind();

	// Deferred drawing
	void deferTree( DrawData* drawData );
	void deleteDeferred( DrawData* drawData );

	void computeDrawData(
		const Matrix & world,
		float windOffset,
		uint32         batchCookie,
		DrawData     & o_drawData );

	void uploadInstanceRenderConstants(
		Moo::EffectMaterialPtr      effect, 		
		Moo::EffectLightingSetter * lightSetter,
		const DrawData            & drawData, 
		bool commit = true );

	void uploadCommonRenderConstants(
		Moo::EffectMaterialPtr      effect,
		bool commit = false );

	void computeLodData( LodData &o_lod );

	// Actual drawing
	static void drawDeferredTrees();
	static void drawOptBillboards();

	enum ETREE_PARTS
	{
		TPART_LEAVES=0,
		TPART_FRONDS,
		TPART_BILLBOARDS,
		TPART_COUNT
	};
	static void drawTreeParts( uint parts );
	static void drawTreePart( RendererVector& renderGroup, ETREE_PARTS part );

	void drawDeferredBranches();
	void drawDeferredFronds();
	void drawDeferredLeaves();
	void drawDeferredBillboards();

	void drawBranch( const DrawData & drawData );
	void drawFrond( const DrawData & drawData );
	void drawLeaf( Moo::EffectMaterialPtr effect, const DrawData & drawData );
	void drawBillboard( const DrawData & drawData );
	void clearDeferredTrees();

	// Rendering states
	static void prepareRenderContext( Moo::EffectMaterialPtr effect );

	void setCompositeRenderStates( Moo::EffectMaterialPtr effect ) const;
	void setBranchRenderStates() const;
	void setFrondRenderStates();
	void setLeafRenderStates();
	void setBillboardRenderStates();

	static bool beginPass( Moo::EffectMaterialPtr		effect,
							Moo::EffectLightingSetter * lightSetter );
		
	static void endPass( Moo::EffectMaterialPtr effect );

	// Models
	void setBranchModel() const;
	void setFrondModel() const;
	void setLeafModel() const;

	// Shaders constants
	static void saveWindInformation( EnviroMinder * envMinder );
	static void saveLightInformation( EnviroMinder * envMinder );

	void uploadWindMatrixConstants( Moo::EffectMaterialPtr effect );

	// Setup
	static TreeTypePtr getTreeTypeObject( const char * filename, uint seed );
	static const TreeData generateTreeData(
		CSpeedTreeRT      & speedTree,
		CSpeedWind	      & speedWind,
		const std::string & filename, 
		int                 seed,
		int                 windAnglesCount );

	void asyncInit();
	void syncInit();
	void releaseResources();

	static void updateRenderGroups();

	typedef std::vector<TSpeedTreeType *> InitVector;
	static InitVector s_syncInitList_;
	static SimpleMutex s_syncInitLock_;
	static SimpleMutex s_vertexListLock_;

	// Deferred trees
#ifdef OPT_BUFFERS
	typedef std::avector< DrawData* > DrawDataVector;
#else
	typedef std::avector< DrawData > DrawDataVector;
#endif
	DrawDataVector deferredTrees_;

	// Deferred tree lighting
	struct DeferredLighting
	{
		uint32					batchCookie_;
		Moo::LightContainerPtr	pDiffuse_;
		Moo::LightContainerPtr	pSpecular_;
	};	
	typedef std::vector<float> FloatVector;
	typedef std::avector<DeferredLighting> DeferredLightingVector;
	//static to share amongst different VisualCompounds.  Different
	//types of trees draw in the same chunk using the same lighting,
	//so no use in having one vector per tree type.
	static DeferredLightingVector	s_deferredLighting_;	

	// Tree definition data
	typedef std::auto_ptr< BSPTree > BSPTreePtr;
	CSpeedTreeRTPtr speedTree_;

	class WindAnimation
	{
	public:
		WindAnimation();
		~WindAnimation();

		bool init( const std::string& iniFile );
		void update();
		void apply( ComObjectWrap<ID3DXEffect> pEffect );
		
		bool hasLeaves( ) const { return bLeaves_; }
		void hasLeaves( bool value ) { bLeaves_ = value; }

		int matrixCount() const;		
		CSpeedWind* speedWind() { return speedWind_; }

		static WindAnimation* lastWindUsed(ID3DXEffect* pEffect);
		static void lastWindUsed(ID3DXEffect* pEffect, WindAnimation* pWind);
		
		float		lastTickTime_;
		int			leafAnglesCount_;
		FloatVector anglesTable_;
		bool		bLeaves_;
		ID3DXEffect* lastEffect_;

	private:
		CSpeedWind	*speedWind_;
		typedef std::map<ID3DXEffect*, WindAnimation*> UsedEffectMap; 
		static UsedEffectMap s_usedEffectMap_;
	};

	WindAnimation* currentWind() const { return s_currentWind_; }
	static void setWind( WindAnimation* wind, ComObjectWrap<ID3DXEffect> pEffect )
	{
		BW_GUARD;
		// TODO: don't set all the wind variables for every single tree type...
		// need to group the common stuff
		//if ( s_currentWind_ != wind )
		{
			s_currentWind_ = wind;
			if (s_currentWind_)
				s_currentWind_->apply( pEffect );
		}
	}
	WindAnimation*			pWind_;
	static WindAnimation	s_defaultWind_;
	static WindAnimation*	s_currentWind_;

	static WindAnimation* setupSpeedWind(
		CSpeedTreeRT      & speedTree, 
		const std::string & datapath,
		const std::string & defaultWindIni );


	// Tree data
	uint              seed_;
	std::string	      filename_;
	TreeData          treeData_;
			
	// bsp
	BSPTreePtr        bspTree_;

	int               bbTreeType_;
	// reference counting
	void incRef() const;
	void decRef() const;
	int  refCount() const;

	mutable int refCounter_;

	// Static tree
	// definitions map
	typedef std::map<std::string, TSpeedTreeType *> TreeTypesMap;
	static TreeTypesMap s_typesMap_;

	static bool            s_frameStarted_;
	static float           s_time_;

	// Render groups
	static RendererGroupVector s_renderGroups_;

	// Materials
	typedef Moo::EffectMaterialPtr	EffectPtr;

	static EffectPtr s_branchesEffect_;
	static EffectPtr s_leavesEffect_;
	static EffectPtr s_billboardsEffect_;
	static EffectPtr s_shadowsFX_;
	static EffectPtr s_depthOnlyBranchesFX_;
	static EffectPtr s_depthOnlyLeavesFX_;
	static EffectPtr s_depthOnlyBillboardsFX_;

	// Lighting
	typedef std::auto_ptr< Moo::EffectLightingSetter > LightSetterPtr;
	static LightSetterPtr s_branchesLightingSetter_;
	static LightSetterPtr s_leavesLightingSetter_;

	// Auxiliary static data
	static float s_windVelX_;
	static float s_windVelZ_;

	static ShadowCaster * s_curShadowCaster_;

	static Matrix		s_invView_;
	static Matrix		s_view_;
	static Matrix		s_projection_;
	static Moo::Colour	s_sunDiffuse_;
	static Moo::Colour	s_sunAmbient_;
	static Vector3		s_sunDirection_;

	// Watched parameters
	static bool  s_enviroMinderLight_;
	static bool  s_drawBoundingBox_;
	static bool	 s_drawNormals_;
	static bool  s_batchRendering_;
	static bool  s_drawTrees_;
	static bool  s_drawLeaves_;
	static bool  s_drawBranches_;
	static bool  s_drawFronds_;
	static bool  s_drawBillboards_;
	static bool  s_texturedTrees_;
	static bool  s_playAnimation_;
	static float s_leafRockFar_;
	static float s_maxLod_;
	static float s_lodMode_;
	static float s_lodNear_;
	static float s_lodFar_;
	static float s_lod0Yardstick_;
	static float s_lod0Variance_;

	#if ENABLE_BB_OPTIMISER
		static bool  s_optimiseBillboards_;
	#endif

	static int s_speciesCount_;
	static int s_uniqueCount_;
	static int s_visibleCount_;
	static int s_deferredCount_;
	static int s_lastPassCount_;
	static int s_passNumCount_;
	static int s_totalCount_;

	static std::string s_fxFiles_[NUM_EFFECTS];
	static std::string s_speedWindFile_;

	static DogWatch s_globalWatch_;
	static DogWatch s_prepareWatch_;
	static DogWatch s_drawWatch_;
	static DogWatch s_primitivesWatch_;
};

//TODO: why the macros? replace with functions?
#ifdef OPT_BUFFERS
// Macros
#define FOREACH_TREETYPE_BEGIN(dogWatcher)                                    \
	{                                                                         \
	ScopedDogWatch watcherHolder(dogWatcher);                                 \
	typedef TSpeedTreeType::TreeTypesMap::iterator iterator;                  \
	iterator renderIt = TSpeedTreeType::s_typesMap_.begin();                   \
	iterator renderEnd = TSpeedTreeType::s_typesMap_.end();                    \
	while ( renderIt != renderEnd ) {                                           \
		TSpeedTreeType * RENDER = renderIt->second;

#define FOREACH_TREETYPE_END ++renderIt; }}

#define FOREACH_DEFEREDTREE_BEGIN                                             \
	{                                                                         \
	typedef TSpeedTreeType::DrawDataVector::const_iterator iterator;         \
	iterator treeIt = this->deferredTrees_.begin();                            \
	iterator treeEnd = this->deferredTrees_.end();                             \
	while ( treeIt != treeEnd )                                                 \
	{                                                                         \
		const DrawData & DRAWDATA = *(*treeIt);

#define FOREACH_DEFERREDTREE_END ++treeIt; }}

#else
// Macros
#define FOREACH_TREETYPE_BEGIN(dogWatcher)                                    \
	{                                                                         \
	ScopedDogWatch watcherHolder(dogWatcher);                                 \
	typedef TSpeedTreeType::TreeTypesMap::iterator iterator;                  \
	iterator renderIt = TSpeedTreeType::s_typesMap_.begin();                   \
	iterator renderEnd = TSpeedTreeType::s_typesMap_.end();                    \
	while ( renderIt != renderEnd ) {                                           \
		TSpeedTreeType * RENDER = renderIt->second;

#define FOREACH_TREETYPE_END ++renderIt; }}

#define FOREACH_DEFEREDTREE_BEGIN                                             \
	{                                                                         \
	typedef TSpeedTreeType::DrawDataVector::const_iterator iterator;         \
	iterator treeIt = this->deferredTrees_.begin();                            \
	iterator treeEnd = this->deferredTrees_.end();                             \
	while ( treeIt != treeEnd )                                                 \
	{                                                                         \
		const DrawData & DRAWDATA = (*treeIt);

#define FOREACH_DEFERREDTREE_END ++treeIt; }}

#endif

} // namespace speedtree


#endif //SPEEDTREE_TREE_TYPE_HPP