/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/***
 *

THE SPEEDTREE INTEGRATION MODULE
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

The files, functions and data structures listed below (*) 
implement the core functionality of the SpeedTree integration 
module. Understanding what they do will provide insight on the
design behind this module. Please refer to their inlined doxigen 
documentation for detailed information of their inner workings.

(*) If you are trying to understand the overall design of this
    module, I suggest you to reading the documentation in the order 
    listed here.

>> Files:
	>>	speedtree_render.cpp
	>>	speedtree_util.hpp
	>>	speedtree_collision.cpp
	>>	billboard_optimiser.cpp

>> Functions:
	>>	getTreeTypeObject()
	>>	doSyncInit()
	>>	drawRenderPass()
	>>	computeDrawData()
	>>	drawDeferredTrees()

>> Structs
	>>	TSpeedTreeType
	>>	TreeData
	>>	LodData
	>>	DrawData

>> Possible Improvements (with comments):

	>>	Use vertex declarations instead of fvf.
	
	>>	Reduce number of vertex data. If you look at the fvf formats
		(speedtree_util.hpp and billboard_optimiser.?pp), you will 
		notice that there is a massive amount of data being passed
		on with each vertex. Maybe we should shave some data off 
		or compress it.	  
	  
	>>	Aggregate leaf and frond textures. There can be two beneffits
		of doing this: (1) less texture swaps between tree types and 
		(2) no need to keep the very enefficient original composite 
		map around. Problems: the aggregator, as it currently stands, 
		requires the original texture to recover from a lost device. 
		Also, a more sophisticated bathing may be required if more than
		one aggregated texture is require to store all loaded trees.
	
	>>	Improvements to the billboarding system. Currently, the billboard
		optimiser uses one single vertex buffer for each chunk. Although 
		this is fine for our current far distances, as the far plane 
		increases, the number of visible chunks explodes. One possible
		approach is to group chunks together into clusters, maybe in a 
		3x3 grid around the player. One easy way to prototype this idea
		would be mapping the bboptimiser objects by something other than
		the chunk address, something that would make adjacent chunks all
		fall into the same bboptimiser instance (maybe chunk coordinates
		div n, where n is the size of a cluster).
	
	>>	Detail maps. This is the one functionality missing from SpeedTreeRT 
		4.1. Have a chat with Adam for details.
	
	>>	Use hardware accelerated mesh instancing - See SpeedTreeRT API 
		reference (Section "Use of Mesh Instancing") for why the beneffits
		of using mesh instancing this is not clear cut.

 *
 ***/


// Module Interface
#include "speedtree_renderer.hpp"
#include "speedtree_config.hpp"

// BW Tech Hearders
#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/watcher.hpp"
#include "math/boundbox.hpp"
#include "physics2/bsp.hpp"

// Set DONT_USE_DUPLO to 1 to prevent all calls
// to the dublo lib. This avoids having to link
// against duplo (which, in turn, depends on a
// lot of other libs). It is useful for testing.
#define DONT_USE_DUPLO 0

DECLARE_DEBUG_COMPONENT2("SpeedTree", 0)

#if SPEEDTREE_SUPPORT // -------------------------------------------------------

// Speedtree Lib Headers
#include "speedtree_collision.hpp"
#include "billboard_optimiser.hpp"
#include "speedtree_util.hpp"

// BW Tech Hearders
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/smartpointer.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/data_section_census.hpp"
#include "moo/material.hpp"
#include "moo/render_context.hpp"
#include "moo/texture_manager.hpp"
#include "moo/effect_material.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/effect_lighting_setter.hpp"
#include "moo/vertex_buffer_wrapper.hpp"
#include "moo/index_buffer_wrapper.hpp"
#include "moo/effect_manager.hpp"
#include "moo/resource_load_context.hpp"
#include "romp/lod_settings.hpp"
#include "romp/enviro_minder.hpp"
#include "romp/time_of_day.hpp"
#include "romp/fog_controller.hpp"
#include "romp/geometrics.hpp"
#include "duplo/shadow_caster.hpp"
#include "romp/line_helper.hpp"

#include "speedtree_tree_type.hpp"

#if ENABLE_FILE_CASE_CHECKING
#include "resmgr/filename_case_checker.hpp"
#endif

// SpeedTree API
#include <SpeedTreeRT.h>
#include <SpeedTreeAllocator.h>

// DX Headers
#include <d3dx9math.h>

// STD Headers
#include <sstream>
#include <map>

// Import dx wrappers
using Moo::IndexBufferWrapper;
using Moo::VertexBufferWrapper;
using Moo::BaseTexturePtr;

PROFILER_DECLARE( SpeedTree_tick, "SpeedTree Tick" );
PROFILER_DECLARE( SpeedTree_beginFrame, "SpeedTree BeginFrame" );
PROFILER_DECLARE( SpeedTree_endFrame, "SpeedTree EndFrame" );
PROFILER_DECLARE( SpeedTree_drawTrees, "SpeedTree DrawTrees" );

PROFILER_DECLARE( SpeedTreeRenderer_drawRenderPass, "SpeedTreeRenderer DrawRenderPass" );

PROFILER_DECLARE( TSpeedTreeType_setEffectParam, "TSpeedTreeType SetEffectParam" );

PROFILER_DECLARE( TSpeedTreeType_drawBranch, "TSpeedTreeType DrawBranch" );
PROFILER_DECLARE( TSpeedTreeType_drawFrond, "TSpeedTreeType DrawFrond" );
PROFILER_DECLARE( TSpeedTreeType_drawLeaf, "TSpeedTreeType DrawLeaf" );
PROFILER_DECLARE( TSpeedTreeType_drawBillboard, "TSpeedTreeType DrawBillboard" );
PROFILER_DECLARE( TSpeedTreeType_drawBSPTree, "TSpeedTreeType DrawBSPTree" );

#include "speedtree_renderer_util.hpp"

namespace speedtree {

// Do we want to render depth only passes?
/*static*/ bool SpeedTreeRenderer::s_depthOnlyPass_ = false;

namespace { // anonymous

// Named contants
const float c_maxDTime        = 0.05f;

const AutoConfigString s_speedTreeXML("system/speedTreeXML");

static bool s_enableDrawPrim = true;

// Are we doing depth only now? Only if its the depth only phase and we have it on
static inline bool depthOnly()
{
	return Moo::rc().depthOnly() && speedtree::SpeedTreeRenderer::depthOnlyPass();
}

#ifdef EDITOR_ENABLED
bool s_enableLightLines_ = false;
#endif // EDITOR_ENABLED

} // namespace anonymous

#if ENABLE_RESOURCE_COUNTERS

class MFSpeedTreeAllocator: public CSpeedTreeAllocator
{
	virtual void* Alloc(size_t BlockSize, size_t Alignment = 16)
	{
		char* allocated_pointer = new char[ BlockSize + sizeof( size_t ) + sizeof( char* ) + Alignment - 1 ];
		char* p = allocated_pointer;
		p += sizeof( size_t ) + sizeof( char* );
		p = (char*)( ( (unsigned int)p + Alignment - 1 ) / Alignment * Alignment );
		*(size_t*)( (char*)p - sizeof( size_t ) - sizeof( char* ) )= BlockSize + Alignment - 1;
		*(char**)( (char*)p - sizeof( char* ) )= allocated_pointer;
		RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
			BlockSize + Alignment - 1 )
		return p;
	}
	virtual void Free(void* pBlock)
	{
		size_t size = *(size_t*)( (char*)pBlock - sizeof( size_t ) - sizeof( char* ) );
		delete[] *( (char**)pBlock - 1 );
		RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
			size )
	}
};

class MFSpeedTreeAllocatorSetter
{
public:
	MFSpeedTreeAllocatorSetter()
	{
		static MFSpeedTreeAllocator allocator;
		CSpeedTreeRT::SetAllocator( &allocator );
	}
}
MFSpeedTreeAllocatorSetter;

#endif

// ---------------------------------------------------------- Effect getters

static TSpeedTreeType::EffectPtr getBranchesEffect()
{
	return depthOnly() ?
		TSpeedTreeType::s_depthOnlyBranchesFX_ : TSpeedTreeType::s_branchesEffect_;
}


static TSpeedTreeType::EffectPtr getLeavesEffect()
{
	return depthOnly() ?
		TSpeedTreeType::s_depthOnlyLeavesFX_ : TSpeedTreeType::s_leavesEffect_;
}


static TSpeedTreeType::EffectPtr getBillboardsEffect()
{
	return depthOnly() ?
		TSpeedTreeType::s_depthOnlyBillboardsFX_ : TSpeedTreeType::s_billboardsEffect_;
}
// ---------------------------------------------------------- SpeedTreeSettings

/**
 *	SpeedTree setting management.
 */
class SpeedTreeSettings : private Moo::EffectManager::IListener
{
public:
	SpeedTreeSettings()
	{
		BW_GUARD;
		init();
	}
	~SpeedTreeSettings()
	{
		BW_GUARD;
		fini();
	}
	void refreshSettings() 
	{ 
		BW_GUARD;
		this->setQualityOption( qualitySettings_->activeOption()); 
	}
	void setQualityOption(int optionIndex);
	virtual void onSelectPSVersionCap(int psVerCap);
private:
	void init();
	void fini();
	Moo::GraphicsSetting::GraphicsSettingPtr qualitySettings_;
};


/**
 *	Construct/bind the setting objects
 */
void SpeedTreeSettings::init()
{
	BW_GUARD;
	// Speedtree quality options
	bool supportsHigh = Moo::rc().vsVersion() >= 0x300 && Moo::rc().psVersion() >= 0x300;
	bool supportsMed = Moo::rc().vsVersion() >= 0x200 && Moo::rc().psVersion() >= 0x200;

	qualitySettings_ = 
		Moo::makeCallbackGraphicsSetting(
		"SPEEDTREE_QUALITY", "Speedtree Quality", *this, 
			&SpeedTreeSettings::setQualityOption, 
			supportsHigh ? 0 : (supportsMed ? 1 : 0), false, false);

	qualitySettings_->addOption("VERYHIGH",  "Very High", supportsHigh);
	qualitySettings_->addOption("HIGH",  "High", supportsHigh);
	qualitySettings_->addOption("MEDIUM", "Medium", supportsMed);
	qualitySettings_->addOption("LOW", "Low", true);


	Moo::GraphicsSetting::add(qualitySettings_);
	this->setQualityOption(qualitySettings_->activeOption());

	Moo::EffectManager::instance().addListener( this );
}


/**
 *	Cleanup
 */
void SpeedTreeSettings::fini()
{
	BW_GUARD;
	qualitySettings_ = NULL;
	Moo::EffectManager::instance().delListener(this);
}


/**
 *	Apply the designated quality setting
 */
void SpeedTreeSettings::setQualityOption( int optionIndex )
{
	BW_GUARD;
	if (!TSpeedTreeType::s_leavesEffect_.hasObject() ||
		!TSpeedTreeType::s_branchesEffect_.hasObject() ||
		!TSpeedTreeType::s_billboardsEffect_.hasObject() ||
		!TSpeedTreeType::s_depthOnlyBillboardsFX_.hasObject() ||
		!TSpeedTreeType::s_depthOnlyBranchesFX_.hasObject() ||
		!TSpeedTreeType::s_depthOnlyLeavesFX_.hasObject()
		)
	{
		return;
	}

	// High
	switch(optionIndex)
	{
		case 0:
			//Very High
			TSpeedTreeType::s_leavesEffect_->hTechnique("leaves_30");
			TSpeedTreeType::s_branchesEffect_->hTechnique("branches_20_bumped");
			TSpeedTreeType::s_billboardsEffect_->hTechnique("billboards_2");

			TSpeedTreeType::s_depthOnlyBranchesFX_->hTechnique("branches");
			TSpeedTreeType::s_depthOnlyLeavesFX_->hTechnique("leaves");
			TSpeedTreeType::s_depthOnlyBillboardsFX_->hTechnique("billboards");
			break;
		case 1:
			//High
#if SPT_ENABLE_NORMAL_MAPS
			TSpeedTreeType::s_leavesEffect_->hTechnique("leaves_20_bumped");
			TSpeedTreeType::s_branchesEffect_->hTechnique("branches_20_bumped");
#else
			//leaves_20
			TSpeedTreeType::s_leavesEffect_->hTechnique("leaves_20");
			TSpeedTreeType::s_branchesEffect_->hTechnique("branches_20");
#endif
			TSpeedTreeType::s_billboardsEffect_->hTechnique("billboards_2");

			TSpeedTreeType::s_depthOnlyBranchesFX_->hTechnique("branches");
			TSpeedTreeType::s_depthOnlyLeavesFX_->hTechnique("leaves");
			TSpeedTreeType::s_depthOnlyBillboardsFX_->hTechnique("billboards");
			break;
		case 2:
			//Medium
			TSpeedTreeType::s_leavesEffect_->hTechnique("leaves_20");
			TSpeedTreeType::s_branchesEffect_->hTechnique("branches_20");
			TSpeedTreeType::s_billboardsEffect_->hTechnique("billboards_2");

			TSpeedTreeType::s_depthOnlyBranchesFX_->hTechnique("branches");
			TSpeedTreeType::s_depthOnlyLeavesFX_->hTechnique("leaves");
			TSpeedTreeType::s_depthOnlyBillboardsFX_->hTechnique("billboards");
			break;
		default:
			//Low
			TSpeedTreeType::s_leavesEffect_->hTechnique("leaves_fixed");
			TSpeedTreeType::s_branchesEffect_->hTechnique("branches_fixed");
			TSpeedTreeType::s_billboardsEffect_->hTechnique("billboards_fixed");

			TSpeedTreeType::s_depthOnlyBranchesFX_->hTechnique("branches_fixed");
			TSpeedTreeType::s_depthOnlyLeavesFX_->hTechnique("leaves_fixed");
			TSpeedTreeType::s_depthOnlyBillboardsFX_->hTechnique("billboards_fixed");
			break;
	}
	//billboards_opt_20_bumped
	//billboards_opt_fixed
}


/**
 *	Callback for shader cap changes.
 */
/*virtual*/ void SpeedTreeSettings::onSelectPSVersionCap( int psVerCap )
{
	BW_GUARD;
	refreshSettings();
}


// ---------------------------------------------------------- SpeedTreeRenderer

// Assigned in SpeedTreeRenderer::init()
SpeedTreeSettings* SpeedTreeRenderer::s_settings_ = NULL;

/**
 *	Default constructor.
 */
SpeedTreeRenderer::SpeedTreeRenderer() :
	treeType_(NULL),
	bbOptimiser_(NULL),
	treeID_(-1),
	drawData_( NULL )
#ifdef EDITOR_ENABLED
	, allowBatching_( true )
#endif // EDITOR_ENABLED
{
	BW_GUARD;
	windOffset_ = float(int(10.0f * (float(rand( )) / RAND_MAX)));
	++TSpeedTreeType::s_totalCount_;
}

/**
 *	Destructor.
 */
SpeedTreeRenderer::~SpeedTreeRenderer()
{
	BW_GUARD;
	--TSpeedTreeType::s_totalCount_;

	if ( drawData_ )
	{
		// Ensure the draw data isn't left lying around..
		treeType_->deleteDeferred( drawData_ );

		delete drawData_;
		drawData_ = NULL;
	}

	#if ENABLE_BB_OPTIMISER
		// try to release bb optimiser
		if (this->bbOptimiser_.exists())
		{
			this->resetTransform( Matrix::identity, false );
			this->bbOptimiser_ = NULL;
		}
	#endif
}

/**
 *	Initialises the speedtree module. Does not initialises FX files.
 *	They will be initialised when the first tree gets loaded.
 */
void SpeedTreeRenderer::init()
{
	BW_GUARD;
	CSpeedTreeRT::Authorize("your license key goes here");

	if ( !CSpeedTreeRT::IsAuthorized() )
	{
		ERROR_MSG("SpeedTree authorisation failed\n");
	}

	DataSectionPtr section = BWResource::instance().openSection(s_speedTreeXML);
	
	if ( !section.exists() )
	{
		CRITICAL_MSG("SpeedTree settings file not found");
		return;
	}

	if ( TSpeedTreeType::s_branchesEffect_.get() != NULL )
	{
		CRITICAL_MSG("SpeedTree already initialised");
		return;
	}

	CSpeedTreeRT::SetNumWindMatrices( 4 );
	CSpeedTreeRT::SetDropToBillboard( true );

	MF_WATCH("SpeedTree/Envirominder Lighting",
		TSpeedTreeType::s_enviroMinderLight_, Watcher::WT_READ_WRITE,
		"Toggles use of envirominder as the source for lighting "
		"information, instead of Moo's light container.");

	MF_WATCH("SpeedTree/Bounding boxes",
		TSpeedTreeType::s_drawBoundingBox_, Watcher::WT_READ_WRITE,
		"Toggles rendering of trees' bounding boxes on/off");

	MF_WATCH("SpeedTree/Batched rendering",
		TSpeedTreeType::s_batchRendering_, Watcher::WT_READ_WRITE,
		"Toggles batched rendering on/off");

	MF_WATCH("SpeedTree/Depth only pass",
		SpeedTreeRenderer::s_depthOnlyPass_, Watcher::WT_READ_WRITE,
		"Toggles speedtree rendering on/off");

	MF_WATCH("SpeedTree/Draw trees",
		TSpeedTreeType::s_drawTrees_, Watcher::WT_READ_WRITE,
		"Toggles speedtree rendering on/off");

	MF_WATCH("SpeedTree/Draw leaves",
		TSpeedTreeType::s_drawLeaves_, Watcher::WT_READ_WRITE,
		"Toggles rendering of leaves on/off");

	MF_WATCH("SpeedTree/Draw branches",
		TSpeedTreeType::s_drawBranches_, Watcher::WT_READ_WRITE,
		"Toggles rendering of branches on/off");

	MF_WATCH("SpeedTree/Draw fronds",
		TSpeedTreeType::s_drawFronds_, Watcher::WT_READ_WRITE,
		"Toggles rendering of fronds on/off");

	MF_WATCH("SpeedTree/Draw billboards",
		TSpeedTreeType::s_drawBillboards_, Watcher::WT_READ_WRITE,
		"Toggles rendering of billboards on/off");

	#if ENABLE_BB_OPTIMISER
		TSpeedTreeType::s_optimiseBillboards_ = section->readBool(
			"billboardOptimiser/enabled", TSpeedTreeType::s_optimiseBillboards_ );
		MF_WATCH("SpeedTree/Optimise billboards",
			TSpeedTreeType::s_optimiseBillboards_, Watcher::WT_READ_WRITE,
			"Toggles billboards optimisations on/off (requires batching to be on)" );
	#endif

	MF_WATCH("SpeedTree/Texturing",
		TSpeedTreeType::s_texturedTrees_, Watcher::WT_READ_WRITE,
		"Toggles texturing on/off");

	MF_WATCH("SpeedTree/Play animation",
		TSpeedTreeType::s_playAnimation_, Watcher::WT_READ_WRITE,
		"Toggles wind animation on/off");

	TSpeedTreeType::s_leafRockFar_ = section->readFloat(
		"leafRockFar", TSpeedTreeType::s_leafRockFar_ );
	MF_WATCH("SpeedTree/Leaf Rock Far Plane",
		TSpeedTreeType::s_leafRockFar_, Watcher::WT_READ_WRITE,
		"Sets the maximun distance to which "
		"leaves will still rock and rustle" );

	TSpeedTreeType::s_lodMode_ = section->readFloat(
		"lodMode", TSpeedTreeType::s_lodMode_ );
	MF_WATCH("SpeedTree/LOD Mode", TSpeedTreeType::s_lodMode_, Watcher::WT_READ_WRITE,
		"LOD Mode: -1: dynamic (near and far LOD defined per tree);  "
		"-2: dynamic (use global near and far LOD values);  "
		"[0.0 - 1.0]: force static." );

	TSpeedTreeType::s_lodNear_ = section->readFloat(
		"lodNear", TSpeedTreeType::s_lodNear_ );
	MF_WATCH("SpeedTree/LOD near",
		TSpeedTreeType::s_lodNear_, Watcher::WT_READ_WRITE,
		"Global LOD near value (distance for maximum level of detail");

	TSpeedTreeType::s_lodFar_ = section->readFloat(
		"lodFar", TSpeedTreeType::s_lodFar_ );
	MF_WATCH("SpeedTree/LOD far",
		TSpeedTreeType::s_lodFar_, Watcher::WT_READ_WRITE,
		"Global LOD far value (distance for minimum level of detail)");

	TSpeedTreeType::s_lod0Yardstick_ = section->readFloat(
		"lod0Yardstick", TSpeedTreeType::s_lod0Yardstick_ );
	MF_WATCH("SpeedTree/LOD-0 Yardstick",
		TSpeedTreeType::s_lod0Yardstick_, Watcher::WT_READ_WRITE,
		"Trees taller than the yardstick will LOD to billboard farther than LodFar. "
		"Trees shorter than the yardstick will LOD to billboard closer than LodFar.");
		
	TSpeedTreeType::s_lod0Variance_ = section->readFloat(
		"lod0Variance", TSpeedTreeType::s_lod0Variance_ );
	MF_WATCH("SpeedTree/LOD-0 Variance",
		TSpeedTreeType::s_lod0Variance_, Watcher::WT_READ_WRITE,
		"How much will the lod level vary depending on the size of the tree" );

	MF_WATCH("SpeedTree/Counters/Unique",
		TSpeedTreeType::s_uniqueCount_, Watcher::WT_READ_ONLY,
		"Counter: number of unique tree models currently loaded" );

	MF_WATCH("SpeedTree/Counters/Species",
		TSpeedTreeType::s_speciesCount_, Watcher::WT_READ_ONLY,
		"Counter: number of speed tree files currently loaded" );

	MF_WATCH("SpeedTree/Counters/Visible",
		TSpeedTreeType::s_visibleCount_, Watcher::WT_READ_ONLY,
		"Counter: number of tree currently visible in the scene" );

	MF_WATCH("SpeedTree/Counters/Instances",
		TSpeedTreeType::s_totalCount_, Watcher::WT_READ_ONLY,
		"Counter: total number of trees instantiated" );

	TSpeedTreeType::s_speedWindFile_ = section->readString(
		"speedWindINI", TSpeedTreeType::s_speedWindFile_ );

	MF_WATCH( "Render/Performance/DrawPrim SpeedTreeRenderer", s_enableDrawPrim,
		Watcher::WT_READ_WRITE,
		"Allow SpeedTreeRenderer to call drawIndexedPrimitive()." );

	MF_WATCH("SpeedTree/BB mipmap bias", 
		BillboardOptimiser::s_mipBias_, Watcher::WT_READ_WRITE, 
		"Amount to offset billboard mip map lod level." );

	// save FX file names
	static const char * fxTag[NUM_EFFECTS] = {
		"fxFiles/branches", 
		"fxFiles/leaves", 
		"fxFiles/billboards", 
		"fxFiles/shadows", 
		"fxFiles/depthOnlyBranches",
		"fxFiles/depthOnlyLeaves",
		"fxFiles/depthOnlyBillboards" };
		
	for (int i=0; i<NUM_EFFECTS; ++i)
	{
		std::string fxFile = section->readString(fxTag[i], "");
		if (!fxFile.empty())
		{
			TSpeedTreeType::s_fxFiles_[i] = fxFile;
		}
		else
		{
			CRITICAL_MSG(
				"Effect file not defined in speedtree config file: %s\n", fxTag[i]);			
		}
	}

	#if ENABLE_BB_OPTIMISER
		// init billboard optimiser
		BillboardOptimiser::init(section);
	#endif

	if (NULL == s_settings_)
	{
		s_settings_ = new SpeedTreeSettings();
	}
}

/**
 *	Finilises the speedtree module.
 */
void SpeedTreeRenderer::fini()
{
	BW_GUARD;
	TSpeedTreeType::s_shadowsFX_ = NULL;
	TSpeedTreeType::s_depthOnlyBranchesFX_ = NULL;
	TSpeedTreeType::s_depthOnlyLeavesFX_ = NULL;
	TSpeedTreeType::s_depthOnlyBillboardsFX_ = NULL;
	TSpeedTreeType::s_branchesEffect_ = NULL;
	TSpeedTreeType::s_leavesEffect_ = NULL;
	TSpeedTreeType::s_billboardsEffect_ = NULL;

	CommonTraits::s_vertexBuffer_ = NULL;
	CommonTraits::s_indexBuffer_ = NULL;
	LeafTraits::s_vertexBuffer_ = NULL;
	LeafTraits::s_indexBuffer_ = NULL;

	TSpeedTreeType::s_branchesLightingSetter_.reset();
	TSpeedTreeType::s_leavesLightingSetter_.reset();

	TSpeedTreeType::fini();

	#if ENABLE_BB_OPTIMISER
		BillboardOptimiser::fini();
	#endif

	if (s_settings_)
	{
		delete s_settings_;
		s_settings_ = NULL;
	}
}

/**
 *	Loads a SPT file.
 *
 *	@param	filename	name of SPT file to load.
 *	@param	seed		seed number to use when generating tree geometry.
 *
 *	@note				will throw std::runtime_error if file can't be loaded.
 */
void SpeedTreeRenderer::load(
	const char   * filename,
	uint           seed,
	const Matrix& world )
{
	BW_GUARD;
	Moo::ScopedResourceLoadContext resLoadCtx( BWResource::getFilename( filename ) );

	this->treeType_ = TSpeedTreeType::getTreeTypeObject( filename, seed );
	this->resetTransform( world );
}

/**
 *	Animates trees.
 *	Should be called only once per frame.
 *
 *	@param	dTime	time elapsed since last tick.
 */
void SpeedTreeRenderer::tick( float dTime )
{
	BW_GUARD_PROFILER( SpeedTree_tick );

	if ( TSpeedTreeType::s_branchesEffect_.get() == NULL )
	{
		if ( !TSpeedTreeType::initFXFiles() )
		{
			throw std::runtime_error( "Could not initialise speedtree FX files" );
		}
		else
		{
			SpeedTreeRenderer::s_settings_->refreshSettings();
		}
	}

	ScopedDogWatch watcher( TSpeedTreeType::s_globalWatch_ );
	TSpeedTreeType::tick( dTime );
	
	#if ENABLE_BB_OPTIMISER
		BillboardOptimiser::tick();
	#endif
}

/**
 *	Marks begining of new frame. No draw call can be done
 *	before this method is called.
 *	@param envMinder	pointer to the current environment minder object
 *						(if NULL, will use constant lighting and wind).
 *
 *	@param	caster		a shadow caster object is this is a shadow pass.
 *						Should be NULL for normal rendering passes.
 */
void SpeedTreeRenderer::beginFrame(
	EnviroMinder * envMinder,
	ShadowCaster * caster )
{
	BW_GUARD_PROFILER( SpeedTree_beginFrame );

	SpeedTreeRenderer::update();

	if ( TSpeedTreeType::s_branchesEffect_.get() == NULL )
	{
		return;
	}
	
	++TSpeedTreeType::s_passNumCount_;
	if ( TSpeedTreeType::s_passNumCount_ == 1 )
	{
		TSpeedTreeType::s_visibleCount_ = 0;
	}
	TSpeedTreeType::s_lastPassCount_ = 0;
	
	
	TSpeedTreeType::s_curShadowCaster_ = caster;

	static DogWatch clearWatch("Clear Deferred");
	ScopedDogWatch watcher1(TSpeedTreeType::s_globalWatch_);
	ScopedDogWatch watcher2(TSpeedTreeType::s_prepareWatch_);

	// store view and projection
	// matrices for future use
	TSpeedTreeType::s_view_ = Moo::rc().view();
	TSpeedTreeType::s_projection_ = Moo::rc().projection();

	Matrix & invView = TSpeedTreeType::s_invView_;
	D3DXMatrixInverse( &invView, NULL, &TSpeedTreeType::s_view_ );

	// update speed tree camera (in speedtree, z is up)
	D3DXVECTOR4 pos;
	D3DXVec3Transform( &pos, &D3DXVECTOR3(0, 0, 0), &invView );

	D3DXVECTOR4 one;
	D3DXVec3Transform( &one, &D3DXVECTOR3(0, 0, -1), &invView );

	D3DXVECTOR3 dir;
	D3DXVec3Subtract( &dir,
		&D3DXVECTOR3(one.x, one.y, one.z),
		&D3DXVECTOR3(pos.x, pos.y, pos.z) );

	const float cam_pos[] = {pos.x, pos.y, -pos.z};
	const float cam_dir[] = {dir.x, dir.y, -dir.z};
	CSpeedTreeRT::SetCamera( cam_pos, cam_dir );

	TSpeedTreeType::saveWindInformation( envMinder );
	TSpeedTreeType::saveLightInformation( envMinder );

	TSpeedTreeType::s_branchesLightingSetter_->begin();
	TSpeedTreeType::s_leavesLightingSetter_->begin();
	TSpeedTreeType::DrawData::s_defaultLightingBatch_ = -1;

	TSpeedTreeType::s_frameStarted_ = true;
	
	TSpeedTreeType::s_deferredCount_ = 0;

	// Reset the wind
	TSpeedTreeType::setWind( NULL, NULL );
}

void SpeedTreeRenderer::flush()
{
	BW_GUARD;
	ScopedDogWatch watcher1(TSpeedTreeType::s_globalWatch_);
	DiaryScribe ds( Diary::instance(), "SF Speedtree Flush" );

	if ( TSpeedTreeType::s_deferredCount_ )
	{
		ScopedDogWatch watcher(TSpeedTreeType::s_drawWatch_);
		TSpeedTreeType::drawDeferredTrees();

		TSpeedTreeType::s_deferredCount_ = 0;
	}
}

/**
 *	Marks end of current frame.  No draw call can be done after
 *	this method (and before beginFrame) is called. It's at this point
 *	that all deferred trees will actually be sent down the rendering
 *	pipeline, if batch rendering is enabled (it is by default).
 */
void SpeedTreeRenderer::endFrame()
{
	BW_GUARD_PROFILER( SpeedTree_endFrame );

	ScopedDogWatch watcher(TSpeedTreeType::s_globalWatch_);
	if ( TSpeedTreeType::s_frameStarted_ )
	{
		TSpeedTreeType::s_frameStarted_ = false;

		if ( TSpeedTreeType::s_lastPassCount_ > 0 )
		{
			ScopedDogWatch watcher(TSpeedTreeType::s_drawWatch_);
			TSpeedTreeType::drawDeferredTrees();
			TSpeedTreeType::drawOptBillboards();
		}
	}
}

/**
 *	Draw an instance of this tree at the given world transform or computes
 *	and stores it's assossiated deffered draw data for later rendering,
 *	if batch rendering is enabled (it is by default).
 *
 *	@param	worldr		transform where to render tree in.
 *	@param	batchCookie	lighting batch for the tree (e.g. chunk address)
 */
void SpeedTreeRenderer::draw( const Matrix & worldt, uint32 batchCookie )
{
	BW_GUARD;
	if ( TSpeedTreeType::s_passNumCount_ == 1 )
	{
		++TSpeedTreeType::s_visibleCount_;
	}
	++TSpeedTreeType::s_lastPassCount_;
	
	// quit if tree rendering is disabled 
	// or if effect hasn't been setup
	if ( !TSpeedTreeType::s_drawTrees_ ||
		TSpeedTreeType::s_branchesEffect_.get() == NULL )
	{
		return;
	}

	// set to 1 if you want
	// to debug trees' rotation
	#if 0
		Matrix world = worldt;
		Matrix rot;
		rot.setRotateY(TSpeedTreeType::s_time*0.3f);
		world.preMultiply(rot);
	#else
		const Matrix & world = worldt;
	#endif

	MF_ASSERT(TSpeedTreeType::s_frameStarted_);
	ScopedDogWatch watcher1(TSpeedTreeType::s_globalWatch_);

	if ( TSpeedTreeType::s_curShadowCaster_ == NULL )
	{
		this->drawRenderPass( world, batchCookie );
	}
	else
	{
		this->drawShadowPass( world, batchCookie );
	}
}

/**
 *	Notifies this tree that it's position has been updated. Calling this every
 *	time the transform of a tree changes is necessary because some optimisations
 *	assume the tree to be static most of the time. Calling this allows the tree
 *	to refresh all cached data that becomes obsolete whenever a change occours.
 *
 *	@param	updateBB	true if the tree should update the bb optimiser.
 *						Default is true.
 */
void SpeedTreeRenderer::resetTransform( const Matrix& world, bool updateBB /*=true*/ )
{
	BW_GUARD;
	#if ENABLE_BB_OPTIMISER
		if ( this->treeID_ != -1 )
		{
			this->bbOptimiser_->removeTree( this->treeID_ );
			this->treeID_ = -1;
		}
		if ( updateBB )
		{
			this->bbOptimiser_ = BillboardOptimiser::retrieve( world.applyToOrigin() );
		}
	#endif
}

/**
 *	Returns tree bounding box.
 */
const BoundingBox & SpeedTreeRenderer::boundingBox() const
{
	BW_GUARD;
	return this->treeType_->treeData_.boundingBox_;
}

/**
 *	Returns tree BSP-Tree.
 */
const BSPTree & SpeedTreeRenderer::bsp() const
{
	BW_GUARD;
	return *this->treeType_->bspTree_;
}

/**
 *	Returns name of SPT file used to generate this tree.
 */
const char * SpeedTreeRenderer::filename() const
{
	BW_GUARD;
	return this->treeType_->filename_.c_str();
}

/**
 *	Returns seed number used to generate this tree.
 */
uint SpeedTreeRenderer::seed() const
{
	BW_GUARD;
	return this->treeType_->seed_;
}

/**
 *	Sets the LOD mode. Returns flag state before call.
 */
float SpeedTreeRenderer::lodMode( float newValue )
{
	BW_GUARD;
	float oldValue = TSpeedTreeType::s_lodMode_;
	TSpeedTreeType::s_lodMode_ = newValue;
	return oldValue;
}

/**
 *	Sets the max LOD. Returns flag state before call.
 */
float SpeedTreeRenderer::maxLod( float newValue )
{
	BW_GUARD;
	float oldValue = TSpeedTreeType::s_maxLod_;
	TSpeedTreeType::s_maxLod_ = newValue;
	return oldValue;
}

/**
 *	Sets the envirominder as the source for lighting
 *	information, instead of the current light containers. 
 *
 *	@param	newValue	true if the environminder should be used as source of
 *						lighing. false is Moo's light containers should be used.
 *
 *	@return				state before call.
 */
bool SpeedTreeRenderer::enviroMinderLighting( bool newValue )
{
	BW_GUARD;
	bool oldValue = TSpeedTreeType::s_enviroMinderLight_;
	TSpeedTreeType::s_enviroMinderLight_ = newValue;
	return oldValue;
}

/**
 *	Enables or disables drawing of trees. 
 *
 *	@param	newValue	true if trees should be drawn. false otherwise
 *
 *	@return				state before call.
 */
bool SpeedTreeRenderer::drawTrees( bool newValue )
{
	BW_GUARD;
	bool oldValue = TSpeedTreeType::s_drawTrees_;
	TSpeedTreeType::s_drawTrees_ = newValue;
	return oldValue;
}


/**
 *	This returns the number of triangles in the tree.
 */
int SpeedTreeRenderer::numTris() const
{
	BW_GUARD;
	int tris = 0;

	if (this->treeType_)
	{
		TSpeedTreeType::TreeData & treeData = this->treeType_->treeData_;
		if (!treeData.branches_.lod_.empty() &&
			treeData.branches_.lod_[ 0 ].index_)
		{
			tris += treeData.branches_.lod_[ 0 ].index_->count() - 2;
		}

		if (!treeData.fronds_.lod_.empty() &&
			treeData.fronds_.lod_[ 0 ].index_)
		{
			tris += treeData.fronds_.lod_[ 0 ].index_->count() - 2;
		}

		if (!treeData.leaves_.lod_.empty() &&
			treeData.leaves_.lod_[ 0 ].index_)
		{
			tris += treeData.leaves_.lod_[ 0 ].index_->count() - 2;
		}
	}

	return tris;
}


/**
 *	This returns the number of primitives in the tree.
 */
int SpeedTreeRenderer::numPrims() const
{
	// One for the branches, one for the trunk and one for the leaves.
	return 3;
}


/**
 *	Do house keeping chores.
 */
void SpeedTreeRenderer::update()
{
	BW_GUARD;
	ScopedDogWatch watcher(TSpeedTreeType::s_globalWatch_);
	TSpeedTreeType::update();
	#if ENABLE_BB_OPTIMISER
		BillboardOptimiser::update();
	#endif
}

/**
 *	Do normal draw render pass.
 *
 *	This function has two modes of operation: immediate and batched.
 *
 *	In immediate mode, it renders the tree immediatly by calling the appropriate
 *	functions to render each of the tree's sub-parts (branches, fronds, leaves and
 *	billboards).
 *
 *	In batched mode, it stores the information required to render the tree and
 *	defers the actual rendering to be performed later, in a more efficient manner,
 *	in the drawDeferredTrees() function.
 *
 *	Both immediate and batched modes rely on the data provided by the 
 *	computeDrawData() function to render the tree.
 *
 *	@see	computeDrawData(), drawDeferredTrees()
 *
 *	@param	worldr		transform where to render tree in.
 *	@param	batchCookie	lighting batch for the tree (e.g. chunk address)
 */
void SpeedTreeRenderer::drawRenderPass( const Matrix & world, uint32 batchCookie )
{
	BW_GUARD_PROFILER( SpeedTreeRenderer_drawRenderPass );

	// Render bounding box
	if ( TSpeedTreeType::s_drawBoundingBox_ )
	{
		Moo::rc().push();
		Moo::rc().world( world );
		Moo::Material::setVertexColour();
		Geometrics::wireBox(
			this->treeType_->treeData_.boundingBox_,
			Moo::Colour(1.0, 0.0, 0.0, 0.0) );
		Moo::rc().pop();
	}

#ifdef OPT_BUFFERS
	if (drawData_ == NULL)
		drawData_ = new TSpeedTreeType::DrawData();
#else
	TSpeedTreeType::DrawData drawData;
	drawData_ = &drawData;
#endif

	{
		ScopedDogWatch watcher2(TSpeedTreeType::s_prepareWatch_);
		this->treeType_->computeDrawData( world, windOffset_, batchCookie, *drawData_ );
	}

	if ( TSpeedTreeType::s_batchRendering_
#ifdef EDITOR_ENABLED
		&& allowBatching_
#endif // EDITOR_ENABLED
		)
	{
		ScopedDogWatch watcher2(TSpeedTreeType::s_prepareWatch_);

#if ENABLE_BB_OPTIMISER
			// optimised billboard
			if ( TSpeedTreeType::s_optimiseBillboards_ &&
				 this->treeType_->bbTreeType_ != -1    &&
				 this->bbOptimiser_.exists() )
			{
				static DogWatch drawBBoardsWatch("Billboards");
				ScopedDogWatch watcher(drawBBoardsWatch);

				if ( drawData_->lod_.billboardDraw_ )
				{
					const float & alphaValue = drawData_->lod_.billboardFadeValue_;

					if ( this->treeID_ != -1 )
					{
						// update tree's alpha test
						// value with the bb optimiser
						this->bbOptimiser_->updateTreeAlpha(
							this->treeID_,
#ifdef EDITOR_ENABLED
							alphaValue, drawData_->fogColour_,
							drawData_->fogNear_, drawData_->fogFar_);
#else
							alphaValue );
#endif // EDITOR_ENABLED
					}
					else
					{
						if ( this->bbOptimiser_->isFull() )
						{
							this->bbOptimiser_ = BillboardOptimiser::retrieve( world.applyToOrigin() );
						}

						// If tree have not yet been registered with
						// the bb optimiser (and the billboard vertices
						// have already been calculated), do it now.
						this->treeID_ = this->bbOptimiser_->addTree(
							this->treeType_->bbTreeType_, world,
#ifdef EDITOR_ENABLED
							alphaValue, drawData_->fogColour_,
							drawData_->fogNear_, drawData_->fogFar_);
#else
							alphaValue );
#endif // EDITOR_ENABLED
					}
				}
				if ( drawData_->lod_.model3dDraw_ )
				{
					this->treeType_->deferTree( drawData_ );
				}
			}
			else
#endif // ENABLE_BB_OPTIMISER
			{
				this->treeType_->deferTree( drawData_ );
			}
	}
	else
	{
#ifdef EDITOR_ENABLED
		// If the editor is temporarily turning off batching for this tree, 
		// don't turn off s_optimiseBillboards_ for good.
		bool prevOptimiseBillboardsState = TSpeedTreeType::s_optimiseBillboards_;
#endif // EDITOR_ENABLED

		#if ENABLE_BB_OPTIMISER
			TSpeedTreeType::s_optimiseBillboards_ = false;
		#endif // ENABLE_BB_OPTIMISER
		{
			ScopedDogWatch watcher2(TSpeedTreeType::s_prepareWatch_);
		}

		ScopedDogWatch watcher2(TSpeedTreeType::s_drawWatch_);
		if ( drawData_->lod_.model3dDraw_ )
		{
			this->treeType_->updateWind();

			bool drawBranches = 
				TSpeedTreeType::s_drawBranches_ && 
				drawData_->lod_.branchDraw_;
				
			bool drawFronds =
				TSpeedTreeType::s_drawFronds_ && 
				drawData_->lod_.frondDraw_;
				
			if ( drawBranches || drawFronds )
			{			
				Moo::EffectMaterialPtr effect = getBranchesEffect().get();

				this->treeType_->uploadCommonRenderConstants( effect );
				TSpeedTreeType::prepareRenderContext( effect );
				this->treeType_->uploadInstanceRenderConstants(	effect, 
					TSpeedTreeType::s_branchesLightingSetter_.get(), *drawData_ );
				this->treeType_->uploadWindMatrixConstants( effect );

				if ( drawBranches )
				{
					this->treeType_->setBranchModel();
					this->treeType_->setBranchRenderStates();
					
					if ( TSpeedTreeType::beginPass( effect, 
							TSpeedTreeType::s_branchesLightingSetter_.get()) )
					{
						this->treeType_->drawBranch( *drawData_ );
						TSpeedTreeType::endPass( effect );
					}
				}


				if ( drawFronds )
				{
					this->treeType_->setCompositeRenderStates( effect );
					
					this->treeType_->setFrondModel();
					this->treeType_->setFrondRenderStates();
					
					if ( TSpeedTreeType::beginPass( effect, 
							TSpeedTreeType::s_branchesLightingSetter_.get() ) )
					{
						this->treeType_->drawFrond( *drawData_ );
						TSpeedTreeType::endPass( effect );
					}
				}
			}

			bool drawLeaves = 
				TSpeedTreeType::s_drawLeaves_ && 
				drawData_->lod_.leafDraw_;
				
			if ( drawLeaves )
			{
				Moo::EffectMaterialPtr effect = getLeavesEffect().get();

				this->treeType_->uploadCommonRenderConstants( effect );				
				TSpeedTreeType::prepareRenderContext( effect );
				this->treeType_->uploadInstanceRenderConstants( effect, 
					TSpeedTreeType::s_leavesLightingSetter_.get(), *drawData_ );
				this->treeType_->setCompositeRenderStates( effect );
				this->treeType_->uploadWindMatrixConstants( effect );

				this->treeType_->setLeafModel();
				this->treeType_->setLeafRenderStates();
				
				if ( TSpeedTreeType::beginPass( effect, 
						TSpeedTreeType::s_leavesLightingSetter_.get()) )
				{
					this->treeType_->drawLeaf( effect, *drawData_ );
					TSpeedTreeType::endPass( effect );
				}
			}
		}

		bool drawBillboards = 
			TSpeedTreeType::s_drawBillboards_ && 
			drawData_->lod_.billboardDraw_;

		if ( drawBillboards )
		{
			Moo::EffectMaterialPtr effect = getBillboardsEffect();

			this->treeType_->uploadCommonRenderConstants( effect );
			TSpeedTreeType::prepareRenderContext( effect );
			this->treeType_->uploadInstanceRenderConstants( effect, NULL, *drawData_ );
			this->treeType_->setCompositeRenderStates( effect );

			this->treeType_->setBillboardRenderStates();
			
			if ( TSpeedTreeType::beginPass( effect, NULL ) )
			{
				this->treeType_->drawBillboard( *drawData_ );
				TSpeedTreeType::endPass( effect );
			}
		}

#ifdef EDITOR_ENABLED
		// If the editor is temporarily turning off batching for this tree, 
		// don't turn off s_optimiseBillboards_ for good.
		if (!allowBatching_)
		{
			TSpeedTreeType::s_optimiseBillboards_ = prevOptimiseBillboardsState;
		}
#endif // EDITOR_ENABLED

	}

#ifndef OPT_BUFFERS
	drawData_=NULL;
#endif
}

/**
 *	Do shadows draw render pass.
 *
 *	@param	worldr		transform where to render tree in.
 *	@param	batchCookie	lighting batch for the tree (e.g. chunk address)
 */
void SpeedTreeRenderer::drawShadowPass( const Matrix & world, uint32 batchCookie )
{
	BW_GUARD;
#ifdef OPT_BUFFERS
	if (drawData_ == NULL)
		drawData_ = new TSpeedTreeType::DrawData();
#else
	TSpeedTreeType::DrawData shadowDrawData;
	drawData_ = &shadowDrawData;
#endif

	{
		ScopedDogWatch watcher2(TSpeedTreeType::s_prepareWatch_);
		this->treeType_->computeDrawData( world, windOffset_, batchCookie, *drawData_ );
	}

	ScopedDogWatch watcher2(TSpeedTreeType::s_drawWatch_);
	if ( drawData_->lod_.model3dDraw_ &&
		 TSpeedTreeType::s_shadowsFX_.get() != NULL )
	{
		#if !DONT_USE_DUPLO
		TSpeedTreeType::s_curShadowCaster_->setupConstants(
			*TSpeedTreeType::s_shadowsFX_);
		#endif // DONT_USE_DUPLO

		this->treeType_->updateWind();
		
		Moo::EffectMaterialPtr effect = TSpeedTreeType::s_shadowsFX_.get();

		this->treeType_->uploadCommonRenderConstants( effect );
		this->treeType_->uploadWindMatrixConstants( effect );
		this->treeType_->uploadInstanceRenderConstants( effect, NULL, *drawData_ );

		if ( TSpeedTreeType::s_drawBranches_ && drawData_->lod_.branchDraw_ )
		{
			this->treeType_->setBranchRenderStates();
			this->treeType_->setBranchModel();

			if ( TSpeedTreeType::beginPass( effect, NULL ) )
			{
				this->treeType_->drawBranch( *drawData_ );
				TSpeedTreeType::endPass( effect );
			}
		}
		if ( TSpeedTreeType::s_drawFronds_ && drawData_->lod_.frondDraw_ )
		{
			this->treeType_->setFrondRenderStates();
			this->treeType_->setFrondModel();

			if ( TSpeedTreeType::beginPass( effect, NULL ) )
			{
				this->treeType_->drawFrond( *drawData_ );
				TSpeedTreeType::endPass( effect );
			}
		}
		if ( TSpeedTreeType::s_drawLeaves_ && drawData_->lod_.leafDraw_ )
		{
			this->treeType_->setLeafRenderStates();
			this->treeType_->setLeafModel();

			if ( TSpeedTreeType::beginPass( effect, NULL ) )
			{
				this->treeType_->drawLeaf( effect, *drawData_ );
				TSpeedTreeType::endPass( effect );
			}
		}
	}
	
#ifndef OPT_BUFFERS
	drawData_ = NULL;
#endif
}

#ifdef EDITOR_ENABLED
void SpeedTreeRenderer::enableLightLines( bool enableLightLines )
{
	s_enableLightLines_ = enableLightLines;
}
#endif

// ------------------------------------------------------ TSpeedTreeType

/**
 *	Default constructor.
 */
TSpeedTreeType::TSpeedTreeType() :
	speedTree_( NULL ),
	pWind_( NULL ),
	treeData_(),
	seed_(1),
	filename_(),
	bspTree_(NULL),
	bbTreeType_(-1),
	refCounter_(0)
{
	BW_GUARD;
	pWind_ = &s_defaultWind_;

#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD( 
		ResourceCounters::DescriptionPool("Speedtree/TSpeedTreeType",
			(uint)ResourceCounters::SYSTEM),
			sizeof(*this) )
#endif
}

/**
 *	Default constructor.
 */
TSpeedTreeType::~TSpeedTreeType()
{
	BW_GUARD;
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_SUB( 
		ResourceCounters::DescriptionPool("Speedtree/TSpeedTreeType",
			(uint)ResourceCounters::SYSTEM),
			sizeof(*this) )
#endif
	#if ENABLE_BB_OPTIMISER
		if ( this->bbTreeType_ != -1 )
		{
			BillboardOptimiser::delTreeType( this->bbTreeType_ );
		}
	#endif

	if ( pWind_ != &TSpeedTreeType::s_defaultWind_ )
	{
		delete pWind_; pWind_ = NULL;
	}
#ifdef OPT_BUFFERS
	TRACE_MSG( "deleting tree type %s\n", filename_.c_str() );
	treeData_.unload();
#endif
}

/**
 *	Initialises FX files (static).
 */
bool TSpeedTreeType::initFXFiles()
{
	BW_GUARD;
	TSpeedTreeType::EffectPtr effects[NUM_EFFECTS];
	for ( int i=0; i<NUM_EFFECTS; ++i )
	{
		effects[i] = new Moo::EffectMaterial;
		
		// i < 2: don't bother if the shadow FX doesn't load.
		if ( !effects[i]->initFromEffect( TSpeedTreeType::s_fxFiles_[i] ) &&
			 i < 3 )
		{
			CRITICAL_MSG(
				"Could not load speedtree effect file: %s\n", 
				TSpeedTreeType::s_fxFiles_[i].c_str() );
			return false;
		}
	}
	
	if ( effects[0]->pEffect() && 
		 effects[1]->pEffect() &&
		 effects[2]->pEffect() )
	{
		TSpeedTreeType::s_branchesEffect_ = effects[0];
		TSpeedTreeType::s_branchesLightingSetter_.reset(
			new Moo::EffectLightingSetter( 
				TSpeedTreeType::s_branchesEffect_->pEffect() ) );

		TSpeedTreeType::s_leavesEffect_ = effects[1];
		TSpeedTreeType::s_leavesLightingSetter_.reset(
			new Moo::EffectLightingSetter( 
				TSpeedTreeType::s_leavesEffect_->pEffect() ) );

		TSpeedTreeType::s_billboardsEffect_ = effects[2];
	}

	if ( effects[3]->pEffect() )
	{
		TSpeedTreeType::s_shadowsFX_ = effects[3];
	}

	if ( effects[4]->pEffect() )
	{
		TSpeedTreeType::s_depthOnlyBranchesFX_ = effects[4];
	}

	if ( effects[5]->pEffect() )
	{
		TSpeedTreeType::s_depthOnlyLeavesFX_ = effects[5];
	}

	if ( effects[6]->pEffect() )
	{
		TSpeedTreeType::s_depthOnlyBillboardsFX_ = effects[6];
	}

	#if ENABLE_BB_OPTIMISER
		// init billboard optimiser
		BillboardOptimiser::initFXFiles();
	#endif
	return true;
}

/**
 *	Finalises the TSpeedTreeType class (static).
 */
void TSpeedTreeType::fini()
{
	BW_GUARD;
	TSpeedTreeType::recycleTreeTypeObjects();
	TSpeedTreeType::clearDXResources();

	if ( !TSpeedTreeType::s_typesMap_.empty() )
	{
		WARNING_MSG(
			"TSpeedTreeType::fini: tree types still loaded: %d\n",
			TSpeedTreeType::s_typesMap_.size());
	}
}

/**
 *	Animates trees (static).
 *
 *	@param	dTime	time elapsed since last tick.
 */
void TSpeedTreeType::tick( float dTime )
{
	BW_GUARD;
	TSpeedTreeType::s_visibleCount_ = 0;
	TSpeedTreeType::s_passNumCount_ = 0;
	TSpeedTreeType::s_time_ += std::min( dTime, c_maxDTime );
}

/**
 *	Prepares trees to be rendered (static).
 *
 *	
 */
void TSpeedTreeType::update()
{
	BW_GUARD;
	if (Moo::rc().reflectionScene())
		return;

	bool initialisedATree = TSpeedTreeType::doSyncInit();
	bool deletedATree = TSpeedTreeType::recycleTreeTypeObjects();
	if ( deletedATree || initialisedATree )
	{
		TSpeedTreeType::s_uniqueCount_ = TSpeedTreeType::s_typesMap_.size();
		TSpeedTreeType::updateRenderGroups();
	}
}

/**
 *	Deletes tree type objects no longer being used and also clear
 *	all deferred trees for those that are still being used (static).
 */
bool TSpeedTreeType::recycleTreeTypeObjects()
{
	BW_GUARD;
	bool deletedAnyTreeType = false;
	typedef TSpeedTreeType::TreeTypesMap::iterator iterator;

	SimpleMutexHolder mutexHolder(TSpeedTreeType::s_syncInitLock_);

	iterator renderIt = TSpeedTreeType::s_typesMap_.begin();
	while ( renderIt != TSpeedTreeType::s_typesMap_.end() )
	{
		if ( renderIt->second->refCount() == 0 )
		{
			delete renderIt->second;
			renderIt = TSpeedTreeType::s_typesMap_.erase( renderIt );
			deletedAnyTreeType = true;
		}
		else
		{
			++renderIt;
		}
	}

	return deletedAnyTreeType;
}

/**
 *	Do the pending synchronous initialisation on newly loaded tree type 
 *	objects (static).
 *
 *	When a new tree type is loaded, as much of the inialisation work as 
 *	possible is done from the loading thread, from the getTreeTypeObject() 
 *	function. There are some tasks, though, that can only be performed from 
 *	the rendering thread (e.g., registering a new tree type into the billboard 
 *	optimiser). 
 *
 *	For this reason, after loading a new tree-type, the getTreeTypeObject() 
 *	function will push it into the sync-init-list. The doSyncInit() function 
 *	iterates through all trees in this list, calling syncInit() on them and 
 *	moving them to the tree-types-map, where they will be ready to be rendered.
 *
 *	@return		true if at list one tree was initialised. false otherwise.
 */
bool TSpeedTreeType::doSyncInit()
{
	BW_GUARD;
	SimpleMutexHolder mutexHolder(TSpeedTreeType::s_syncInitLock_);
	const int listSize = TSpeedTreeType::s_syncInitList_.size();
	typedef TSpeedTreeType::InitVector::iterator iterator;
	iterator initIt  = TSpeedTreeType::s_syncInitList_.begin();
	iterator initEnd = TSpeedTreeType::s_syncInitList_.end();

	// only init one each time.
	if ( initIt != initEnd )
	{
		(*initIt)->syncInit();

		// now that initialisation is
		// finished, register renderer
		const char * filename = (*initIt)->filename_.c_str();
		std::string treeDefName = 
			createTreeDefName( filename, (*initIt)->seed_ );
		TSpeedTreeType::s_typesMap_.insert(
			std::make_pair( treeDefName, *initIt ) );

		TSpeedTreeType::s_syncInitList_.erase( initIt );
	}
	return listSize > 0;
}

/**
 *	Releases all resources used by this tree type.
 */
void TSpeedTreeType::releaseResources()
{
	BW_GUARD;
	this->treeData_.branches_.lod_.clear();
	this->treeData_.branches_.diffuseMap_ = NULL;

	this->treeData_.fronds_.lod_.clear();
	this->treeData_.fronds_.diffuseMap_ = NULL;

	this->treeData_.leaves_.lod_.clear();
	this->treeData_.leaves_.diffuseMap_ = NULL;

	this->treeData_.billboards_.lod_.clear();
	this->treeData_.billboards_.diffuseMap_ = NULL;

	#if SPT_ENABLE_NORMAL_MAPS
		this->treeData_.branches_.normalMap_   = NULL;
		this->treeData_.fronds_.normalMap_     = NULL;
		this->treeData_.leaves_.normalMap_     = NULL;
		this->treeData_.billboards_.normalMap_ = NULL;
	#endif // SPT_ENABLE_NORMAL_MAPS
}

/**
 *	Deletes all tree type objects in the pending initialisation list (static).
 */
void TSpeedTreeType::clearDXResources()
{
	BW_GUARD;
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_syncInitLock_);
		while ( !s_syncInitList_.empty() )
		{
			s_syncInitList_.back()->releaseResources();
			s_syncInitList_.pop_back();
		}
	}

	static DogWatch releaseWatch("release");
	FOREACH_TREETYPE_BEGIN(releaseWatch)
	{
		RENDER->releaseResources();
	}
	FOREACH_TREETYPE_END
}

#ifdef EDITOR_ENABLED
namespace
{
	/*
	 *	Helper functions, draws a line from every light to the
	 *	the point passed in
	 */
	void drawLightLines(Vector3 objectCentre, Moo::LightContainerPtr pContainer)
	{
		for (uint32 i = 0; i < pContainer->nOmnis(); ++i)
		{
			Vector3 oPos = pContainer->omni(i)->worldPosition();
			LineHelper::instance().drawLine( oPos, objectCentre );
		}
		for (uint32 i = 0; i < pContainer->nSpots(); ++i)
		{
			Vector3 sPos = pContainer->spot(i)->worldPosition();
			LineHelper::instance().drawLine( sPos, objectCentre );
		}
	}
}
#endif // EDITOR_ENABLED


/**
 *	Computes the DrawData object required to render a tree of 
 *	this type at the given world transform.
 *
 *	The DrawData stores all the information required to draw a unique
 *	instance of a tree, in either immediate and batched modes.
 *
 *	This method generates a DrawData object based on this tree type
 *	and the world coordinate passed as argument.
 *
 *	LODing information can be generated in one of three ways: (1) based
 *	on the LOD information stored in each SpeedTree, (2) based on global
 *	LOD values set through <speedtree>.xml or the watchers and (3) from
 *	a constant value, also set through <speedtree>.xml or the watchers.
 * 
 *	@see	DrawData, drawRenderPass()
 *
 *	@param	world		transform where tree should be rendered.
 *	@param	batchCookie	cookie for light batching.
 *	@param	o_drawData	(out) will store the defer data struct.
 */
void TSpeedTreeType::computeDrawData(
	const Matrix&	world,
	float			windOffset,
	uint32			batchCookie,
	DrawData&		o_drawData )
{
	BW_GUARD;
	static DogWatch computeLODWatch("LOD");
	static DogWatch computeMatricesWatch("Matrices");
	static DogWatch computeLightsWatch("Lights");
	static DogWatch computeLeavesWatch("Leaves");
	static DogWatch computeBBWatch("Billboards");

	float scaleZ = world.applyToUnitAxisVector(2).length();
	{
		ScopedDogWatch watcher(computeMatricesWatch);

#ifdef OPT_BUFFERS
#ifndef EDITOR_ENABLED
		if (!o_drawData.initialised_)
#endif
#endif
		{
			o_drawData.world_        = world;
			o_drawData.rotation_.setRotate( world.yaw(), world.pitch(), world.roll() );

			o_drawData.rotationInverse_ = o_drawData.rotation_;
			o_drawData.rotationInverse_.invert();

			float scaleX = world.applyToUnitAxisVector(0).length();
			float scaleY = world.applyToUnitAxisVector(1).length();

			o_drawData.treeScale_ = 0.333f * (scaleX + scaleY + scaleZ);
			
			o_drawData.windOffset_ = windOffset;
		}

		D3DXMatrixMultiply( &o_drawData.worldView_, &world, &TSpeedTreeType::s_view_ );
		D3DXMatrixMultiply(
			&o_drawData.worldViewProj_,
			&o_drawData.worldView_,
			&TSpeedTreeType::s_projection_ );
	}

	{
		ScopedDogWatch watcher(computeLODWatch);

		D3DXVECTOR4 eyePos;
		const Matrix & invView = TSpeedTreeType::s_invView_;
		D3DXVec3Transform( &eyePos, &D3DXVECTOR3(0, 0, 0), &invView );

		Vector3 treePos = world.applyToOrigin();
		float treeTop = treePos.y + (scaleZ * this->treeData_.boundingBox_.height());
		float treeBase = treePos.y;
		treePos.y = Math::clamp( treeBase, eyePos.y, treeTop );

		this->speedTree_->SetTreePosition(
			treePos.x, treePos.z, treePos.y );

		if ( TSpeedTreeType::s_lodMode_ == -1.0 )
		{
			float lodNear = 0;
			float lodFar  = 0;
			this->speedTree_->GetLodLimits( lodNear, lodFar );
			LodSettings::instance().applyLodBias( lodNear, lodFar );
			this->speedTree_->SetLodLimits( lodNear, lodFar );
			this->speedTree_->ComputeLodLevel();
		}
		else if ( TSpeedTreeType::s_lodMode_ == -2.0 )
		{
			D3DXVECTOR4 pos;
			D3DXVec3Transform( &pos, &D3DXVECTOR3(0, 0, 0), &invView );

			Vector3 position( pos.x, pos.y, pos.z );
			Vector3 distVector = treePos - position;
			float distance = distVector.length();			
			float lodNear = TSpeedTreeType::s_lodNear_;
			float lodFar  = TSpeedTreeType::s_lodFar_;
			LodSettings::instance().applyLodBias( lodNear, lodFar );
			float lodLevel = 1 - (distance - lodNear) / (lodFar - lodNear);
						
			float lodVari  = TSpeedTreeType::s_lod0Variance_ * 
				( -0.5f + (scaleZ * this->treeData_.boundingBox_.height()) / 
				TSpeedTreeType::s_lod0Yardstick_ );		
		
			lodLevel += lodVari;
			lodLevel = std::min( lodLevel, 1.0f );
			lodLevel = std::max( lodLevel, 0.0f );
			// Manually rounding very small lod values to avoid problems
			// in the speedtree lod calculation.(avoiding problem, not fixing)
			if ( lodLevel < 0.0001f )
				lodLevel = 0.0;
			this->speedTree_->SetLodLevel( lodLevel );
		}
		else
		{
			this->speedTree_->SetLodLevel( s_lodMode_ );
		}
	}

	// set to 1 if you want
	// to debug trees' lod
	#if 0
		float lodLevel = 0.5f * (1+sin(TSpeedTreeType::s_time*0.3f));
		this->speedTree->SetLodLevel(lodLevel);
	#endif

	{
		ScopedDogWatch watcher(computeLeavesWatch);
#ifdef OPT_BUFFERS
		if (o_drawData.lodLevel_ != this->speedTree_->GetLodLevel())
		{
			o_drawData.lodLevel_ = this->speedTree_->GetLodLevel();
			this->computeLodData( o_drawData.lod_ );
		}
#else
		this->computeLodData( o_drawData.lod_ );
#endif
	}

	{
		ScopedDogWatch watcher(computeLightsWatch);

		// don't cache lights for shadows pass and depth pass
		if ( !TSpeedTreeType::s_curShadowCaster_ && 
			depthOnly() == false && 
			o_drawData.lod_.model3dDraw_ )
		{
			Moo::LightContainerPtr pDiffuse = Moo::rc().lightContainer();
			Moo::LightContainerPtr pSpecular = Moo::rc().specularLightContainer();
			//First detect if the lighting is your standard outdoor lighting or not.
			bool newLightingBatch = false;
			bool standardLighting = (pDiffuse->nDirectionals()==1 &&
									!pDiffuse->nOmnis() &&
									!pDiffuse->nSpots() );

			if ( standardLighting )
			{
				if ( DrawData::s_defaultLightingBatch_ == -1 )
				{					
					//then create the default lighting batch and store its index
					DrawData::s_defaultLightingBatch_ = s_deferredLighting_.size();
					newLightingBatch = true;
				}
			}
			else
			{
				if (pDiffuse->nDirectionals() <= 2 &&
					pDiffuse->nOmnis() <= 4 &&
					pDiffuse->nSpots() <= 2)
				{
					if (s_deferredLighting_.size() && s_deferredLighting_.back().batchCookie_ == batchCookie)
					{
						o_drawData.lightingBatch_ = s_deferredLighting_.size() - 1;
					}
					else
					{
						o_drawData.lightingBatch_ = s_deferredLighting_.size();
						newLightingBatch = true;
					}
				}
				else
				{
					o_drawData.lightingBatch_ = Moo::EffectLightingSetter::NO_BATCHING;
				}
			}

			if ( newLightingBatch )
			{
				DeferredLighting dl;

				dl.batchCookie_ = batchCookie;

				// using a full extents bounding box means we don't cull lights.
				// This is alright because the lighting is already culled by the 
				// chunk that has set the light container.
				//
				// If you want to change this to use the bounding box of the 
				// object, then you'll need to store a light container per-tree, 
				// instead of per-chunk.
				static BoundingBox s_fullExtents(
					Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX),
					Vector3(FLT_MAX, FLT_MAX, FLT_MAX) );

				dl.pDiffuse_  = new Moo::LightContainer( pDiffuse, s_fullExtents );
				dl.pSpecular_ = new Moo::LightContainer( pSpecular, s_fullExtents );

				s_deferredLighting_.push_back( dl );
			}

			if ( standardLighting )
			{
				o_drawData.lightingBatch_ = DrawData::s_defaultLightingBatch_;
			}
			else if (o_drawData.lightingBatch_ == Moo::EffectLightingSetter::NO_BATCHING)
			{
				BoundingBox bb = this->treeData_.boundingBox_;
				bb.transformBy( world );

				o_drawData.pDiffuseLights_->init( pDiffuse, bb, true );
				o_drawData.pSpecularLights_->init( pSpecular, bb, true );
			}
#ifdef EDITOR_ENABLED
			if (s_enableLightLines_)
			{
				// Render lines from the centre of the tree bounding box to
				// each of the lights affecting it
				BoundingBox bb = this->treeData_.boundingBox_;
				bb.transformBy( world );
				if (o_drawData.lightingBatch_ == Moo::EffectLightingSetter::NO_BATCHING)
				{
					drawLightLines( bb.centre(), o_drawData.pDiffuseLights_ );
				}
				else
				{
					drawLightLines( bb.centre(), 
						s_deferredLighting_[o_drawData.lightingBatch_].pDiffuse_ );
				}
			}
#endif // EDITOR_ENABLED

		}
	}

#ifdef EDITOR_ENABLED
	int32 fog = Moo::rc().fogColour(); 
	o_drawData.fogColour_.z = ((fog & (0xff << 8*0)) >> 8*0) / 255.0f; 
	o_drawData.fogColour_.y = ((fog & (0xff << 8*1)) >> 8*1) / 255.0f; 
	o_drawData.fogColour_.x = ((fog & (0xff << 8*2)) >> 8*2) / 255.0f; 
	o_drawData.fogColour_.w = ((fog & (0xff << 8*3)) >> 8*3) / 255.0f; 
	o_drawData.fogNear_   = Moo::rc().fogNear(); 
	o_drawData.fogFar_    = Moo::rc().fogFar(); 
#else 
#ifdef OPT_BUFFERS 
	o_drawData.initialised_ = true; 
#endif 
#endif
}


/**
 *	Upload the common render constants for this effect.
 *
 *	@param	effect			the effect material which to upload the constants to.
 *	@param	commit			true if changes should be commited (false can be
 *							used whenever a subsequent processing will need
 *							to commit other constants to the effect file).
 */
void TSpeedTreeType::uploadCommonRenderConstants(
	Moo::EffectMaterialPtr		effect, bool commit )
{
	BW_GUARD;
	static DogWatch computeLODWatch("commonRenderConsts");
	static DogWatch commitLODWatch("commit");
	ScopedDogWatch watcher(computeLODWatch);

	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();

	pEffect->SetMatrix( "g_projection",		&TSpeedTreeType::s_projection_ );

	if ( commit )
	{
		ScopedDogWatch watcher(commitLODWatch);
		pEffect->CommitChanges();
	}
}


/**
 *	Upload transform matrice and other render constants to the rendering device.
 *
 *	@param	effect			the effect material which to upload the constants to.
 *  @param  lightSetter		the light setter to use for uploading light constants
 *	@param	drawData		the deferred rendering data for this tree.
 *	@param	commit			true if changes should be commited (false can be
 *							used whenever a subsequent processing will need
 *							to commit other constants to the effect file).
 */
void TSpeedTreeType::uploadInstanceRenderConstants(
	Moo::EffectMaterialPtr		effect, 
	Moo::EffectLightingSetter * lightSetter,
	const DrawData           & drawData, 
	bool commit )
{
	BW_GUARD;
	static DogWatch computeLODWatch("renderConsts");
	static DogWatch commitLODWatch("commit");
	ScopedDogWatch watcher(computeLODWatch);

	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();

	// for optimisation sake, we're using an array of
	// matrices instead of uploading each matrix at a time.
	#if ENABLE_MATRIX_OPT
		{
			PROFILER_SCOPED( TSpeedTreeType_setEffectParam );
			pEffect->SetMatrixArray( "g_matrices", (D3DXMATRIX*)&drawData.world_, 5 );
		}
	#else
		pEffect->SetMatrix( "g_world",			&drawData.world_ );
		pEffect->SetMatrix( "g_rotation",		&drawData.rotation_ );
		pEffect->SetMatrix( "g_worldViewProj",	&drawData.worldViewProj_ );
		pEffect->SetMatrix( "g_worldView",		&drawData.worldView_ );
		pEffect->SetMatrix( "g_rotationInverse", &drawData.rotationInverse_ );
	#endif

	#ifdef EDITOR_ENABLED
		pEffect->SetVector( "fogColour",    &drawData.fogColour_ );
		pEffect->SetFloat( "fogStart",      drawData.fogNear_ );
		pEffect->SetFloat( "fogEnd",        drawData.fogFar_ );
	#endif

	pEffect->SetFloat( "g_windMatrixOffset", drawData.windOffset_ );

	ShadowCaster * caster = TSpeedTreeType::s_curShadowCaster_;

	if ( caster == NULL &&
		depthOnly() == false &&
		lightSetter != NULL )
	{
		Moo::rc().push();
		Moo::rc().world( drawData.world_ );

		DeferredLighting& dl = s_deferredLighting_[drawData.lightingBatch_];

		if (drawData.lightingBatch_ != Moo::EffectLightingSetter::NO_BATCHING)
		{
			lightSetter->apply(
				dl.pDiffuse_, dl.pSpecular_, 
				false, drawData.lightingBatch_ );
		}
		else
		{
			lightSetter->apply( 
				drawData.pDiffuseLights_, drawData.pSpecularLights_, false );
		}
		
		Moo::rc().pop();
	}	

	if ( commit )
	{
		ScopedDogWatch watcher(commitLODWatch);
		pEffect->CommitChanges();
	}
}

/**
 *	Computes all LOD data needed to render this tree. Assumes the speedtree
 *	object have already been updated in regard to its current LOD level.
 *
 *	@param	o_lod	(out) will store all lod data
 */
void TSpeedTreeType::computeLodData( LodData & o_lod )
{
	BW_GUARD;
	CSpeedTreeRT::SLodValues sLod;

	this->speedTree_->GetLodValues( sLod );

	o_lod.branchLod_   = sLod.m_nBranchActiveLod;
	o_lod.branchDraw_  = sLod.m_nBranchActiveLod >= 0 &&
							this->treeData_.branches_.checkVerts();
	o_lod.branchAlpha_ = sLod.m_fBranchAlphaTestValue;

	o_lod.frondLod_    = sLod.m_nFrondActiveLod;
	o_lod.frondDraw_   = sLod.m_nFrondActiveLod >= 0 &&
						this->treeData_.fronds_.checkVerts();
	o_lod.frondAlpha_  = sLod.m_fFrondAlphaTestValue;

	o_lod.leafLods_[0] = sLod.m_anLeafActiveLods[0];
	o_lod.leafLods_[1] = sLod.m_anLeafActiveLods[1];
	o_lod.leafAlphaValues_[0] = sLod.m_afLeafAlphaTestValues[0];
	o_lod.leafAlphaValues_[1] = sLod.m_afLeafAlphaTestValues[1];
	o_lod.leafLodCount_ =
		(sLod.m_anLeafActiveLods[0] >= 0 ? 1 : 0) +
		(sLod.m_anLeafActiveLods[1] >= 0 ? 1 : 0);
	o_lod.leafDraw_     = o_lod.leafLodCount_ > 0 && 
						this->treeData_.leaves_.checkVerts();

	o_lod.billboardFadeValue_ = sLod.m_fBillboardFadeOut;
	o_lod.billboardDraw_      = (!this->treeData_.billboards_.lod_.empty())
									&& sLod.m_fBillboardFadeOut > 0;

	o_lod.model3dDraw_ =
		o_lod.branchDraw_ ||
		o_lod.frondDraw_  ||
		o_lod.leafDraw_;
}

#define TPART_MASK( part ) 0x1 << part

/**
 *	Draw an individual part of a tree.
 *
 *	@param	renderGroup	List of trees to draw.
 *	@param  part The part of the tree to draw.
 */
void TSpeedTreeType::drawTreePart( RendererVector& renderGroup, ETREE_PARTS part )
{
	BW_GUARD;
	static DogWatch partStateWatch("Part State");
	static DogWatch renderWatch("Render");

	typedef void (TSpeedTreeType::*RenderMethodType)(void);
	static RenderMethodType drawFuncs[TPART_COUNT] =
	{
		&TSpeedTreeType::drawDeferredLeaves,
		&TSpeedTreeType::drawDeferredFronds,
		&TSpeedTreeType::drawDeferredBillboards,
	};

	typedef void (TSpeedTreeType::*StateMethodType)(void);
	static StateMethodType stateFuncs[TPART_COUNT] =
	{
		&TSpeedTreeType::setLeafRenderStates,
		&TSpeedTreeType::setFrondRenderStates,
		&TSpeedTreeType::setBillboardRenderStates,
	};

	Moo::EffectMaterial * effects[2][TPART_COUNT] =
	{
		{
			TSpeedTreeType::s_leavesEffect_.get(),
			TSpeedTreeType::s_branchesEffect_.get(),
			TSpeedTreeType::s_billboardsEffect_.get(),
		},
		{
			TSpeedTreeType::s_depthOnlyLeavesFX_.get(),
			TSpeedTreeType::s_depthOnlyBranchesFX_.get(),
			TSpeedTreeType::s_depthOnlyBillboardsFX_.get(),
		}
	};
	int depthOnlyIndex = depthOnly();

	partStateWatch.start();
	(renderGroup.back()->*stateFuncs[part])();
	renderGroup.back()->setCompositeRenderStates( effects[depthOnlyIndex][part] );
	partStateWatch.stop();

	renderWatch.start();
	RendererVector::iterator renderIt = renderGroup.begin();
	RendererVector::iterator renderEnd = renderGroup.end();
	while ( renderIt != renderEnd )
	{
		if ( !(*renderIt)->deferredTrees_.empty() )
		{
			(*renderIt)->uploadWindMatrixConstants( effects[depthOnlyIndex][part] );
			((*renderIt)->*drawFuncs[part])();
		}
		++renderIt;
	}
	renderWatch.stop();
}

/**
 *	Draw multiple parts of the trees.
 *
 *	@param  parts A bitmask of the parts of the tree to draw.
 */
void TSpeedTreeType::drawTreeParts( uint parts )
{
	BW_GUARD;
	static DogWatch compositeWatch("Composite");

	compositeWatch.start();
	RendererGroupVector & renderGroups = TSpeedTreeType::s_renderGroups_;
	RendererGroupVector::iterator groupIt = renderGroups.begin();
	RendererGroupVector::iterator groupEnd = renderGroups.end();

	#if ENABLE_BB_OPTIMISER
		const bool optimiseBB = TSpeedTreeType::s_optimiseBillboards_;
	#else
		static const bool optimiseBB = false;
	#endif

	// don't do billboards if
	// optimisation is enabled
	while ( groupIt != groupEnd )
	{
		// make sure there is at least one tree
		// to render for this group before proceeding
		RendererVector::iterator renderIt = groupIt->begin();
		RendererVector::iterator renderEnd = groupIt->end();
		while ( renderIt != renderEnd )
		{
			if ( !(*renderIt)->deferredTrees_.empty() )
			{
				break;
			}
			++renderIt;
		}
		if ( renderIt != renderEnd )
		{
			for (int part=0; part<(int)TPART_COUNT; part++)
			{
				if (parts & TPART_MASK(part))
				{
					drawTreePart( *groupIt, (ETREE_PARTS)part );
				}
			}
		}
		++groupIt;
	}
	compositeWatch.stop();
}

/**
 *	Draws all deferred trees in sorted order, according to their required
 *	rendering states. 
 *
 *	Drawing is performed by iterating through all types of trees, setting 
 *	the required rendering states for it and drawing all deferred trees for 
 *	that type. In fact, this nested iteration is performed four times, one 
 *	for each part of a tree model: branches, fronds, leaves and billboards
 *	(billboards will not be draw if the billboard obtimiser is active).
 */
void TSpeedTreeType::drawDeferredTrees()
{
	BW_GUARD;
	static DogWatch windWatch("Wind");
	static DogWatch branchesWatch("Branches");

	Moo::EffectMaterialPtr effect = getBranchesEffect().get();
	if ( TSpeedTreeType::s_typesMap_.empty() || effect == NULL )
	{
		return;
	}

	// Set up the effect visual context.
	Moo::EffectVisualContext::instance().pRenderSet( NULL );
	Moo::EffectVisualContext::instance().staticLighting( false );
	Moo::EffectVisualContext::instance().isOutside( true );

	// first, update wind animation
	bool anyTreeToRender = false;
	FOREACH_TREETYPE_BEGIN( windWatch )
	{
		if ( !RENDER->deferredTrees_.empty() )
		{
			RENDER->updateWind();
			anyTreeToRender = true;
		}
	}
	FOREACH_TREETYPE_END
	
	if ( !anyTreeToRender )
	{
		// no trees to render
		return;
	}

	// If its a depth only pass disable the write mask
	if ( depthOnly() )
	{
		Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE );
		Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE1 );

		Moo::rc().setWriteMask( 0, 0 );
		Moo::rc().setWriteMask( 1, 0 );
	}

#ifndef OPT_BUFFERS
	TSpeedTreeType::prepareRenderContext( effect );
	
	// then, render trees' branches
	std::string lastFilename = "";
	FOREACH_TREETYPE_BEGIN( branchesWatch )
	{
		if ( !RENDER->deferredTrees_.empty() && 
			RENDER->treeData_.branches_.diffuseMap_.exists())
		{
			if ( RENDER->filename_ != lastFilename )
			{
				RENDER->setBranchRenderStates();
				lastFilename = RENDER->filename_;
			}
			RENDER->uploadWindMatrixConstants( effect );
			RENDER->drawDeferredBranches();
		}
	}
	FOREACH_TREETYPE_END
#endif
	Moo::EffectMaterial * effects[TPART_COUNT] =
	{
		TSpeedTreeType::s_leavesEffect_.get(),
		TSpeedTreeType::s_branchesEffect_.get(),
		TSpeedTreeType::s_billboardsEffect_.get(),
	};

	// Prepare the context if its a colour pass (no need in depth only)
	if ( !depthOnly() )
	{
		for ( int i=0; i<TPART_COUNT; ++i )
		{
			TSpeedTreeType::prepareRenderContext( effects[i] );
		}
	}

	uint parts = TPART_MASK( TPART_LEAVES ) | TPART_MASK( TPART_FRONDS );
#ifndef OPT_BUFFERS
	part |= TPART_MASK( TPART_BILLBOARDS );
#endif
	drawTreeParts( parts );

#ifdef OPT_BUFFERS
	TSpeedTreeType::prepareRenderContext( effect );
	
	// then, render trees' branches
	std::string lastFilename = "";
	FOREACH_TREETYPE_BEGIN( branchesWatch )
	{
		if ( !RENDER->deferredTrees_.empty() && 
			RENDER->treeData_.branches_.diffuseMap_.exists())
		{
			if ( RENDER->filename_ != lastFilename )
			{
				RENDER->setBranchRenderStates();
				lastFilename = RENDER->filename_;
			}
			RENDER->uploadWindMatrixConstants( effect );
			RENDER->drawDeferredBranches();
		}
	}
	FOREACH_TREETYPE_END

	if (!depthOnly())
		drawTreeParts( TPART_MASK( TPART_BILLBOARDS ) );
#endif //OPT_BUFFERS

	// Restore colour mask state
	if ( depthOnly() )
	{
		Moo::rc().popRenderState();
		Moo::rc().popRenderState();
	}

	// clear deferred trees, from both lists, the fully-loaded trees, and 
	// trees that still need loading (or are in the process of getting loaded)
	for( TSpeedTreeType::TreeTypesMap::iterator it = TSpeedTreeType::s_typesMap_.begin();
			it != TSpeedTreeType::s_typesMap_.end() ; ++it )
	{
		it->second->clearDeferredTrees();
	}
	for( TSpeedTreeType::InitVector::iterator it = TSpeedTreeType::s_syncInitList_.begin() ;
		it != TSpeedTreeType::s_syncInitList_.end() ; ++it )
	{
		(*it)->clearDeferredTrees();
	}
}

/**
 *	Draw billboards using the billboards optimiser.
 */
void TSpeedTreeType::drawOptBillboards()
{
	BW_GUARD;
	// now, if optimisation is
	// enabled, do the billboards.
	#if ENABLE_BB_OPTIMISER
		if (TSpeedTreeType::s_optimiseBillboards_ && 
			TSpeedTreeType::s_drawBillboards_     && 
			TSpeedTreeType::s_curShadowCaster_ == NULL)
		{
			static DogWatch drawBBoardsWatch("Billboards");
			ScopedDogWatch watcher(drawBBoardsWatch);

			Matrix viewProj;
			D3DXMatrixMultiply( &viewProj,
				&TSpeedTreeType::s_view_,
				&TSpeedTreeType::s_projection_ );

			BillboardOptimiser::FrameData frameData(
				viewProj,
				TSpeedTreeType::s_invView_,
				TSpeedTreeType::s_sunDirection_,
				TSpeedTreeType::s_sunDiffuse_,
				TSpeedTreeType::s_sunAmbient_,
				TSpeedTreeType::s_texturedTrees_ );

			BillboardOptimiser::drawAll( frameData );
		}
	#endif
}

/**
 *	Iterates through all deferred trees for this tree type, drawing
 *	their branches. Assumes all render states have already been set.
 */
void TSpeedTreeType::drawDeferredBranches()
{
	BW_GUARD;
	static DogWatch drawBranchesWatch("Branches");
	ScopedDogWatch watcher(drawBranchesWatch);

	if ( TSpeedTreeType::s_drawBranches_ )
	{
		Moo::EffectMaterialPtr effect = getBranchesEffect();

		this->uploadCommonRenderConstants( effect );
		this->setBranchModel();
		if ( TSpeedTreeType::beginPass( effect, 
				TSpeedTreeType::s_branchesLightingSetter_.get() ) )
		{			
			FOREACH_DEFEREDTREE_BEGIN
			{
				if ( DRAWDATA.lod_.branchDraw_ )
				{
					this->uploadInstanceRenderConstants( effect, 
						TSpeedTreeType::s_branchesLightingSetter_.get(),
						DRAWDATA );
					this->drawBranch( DRAWDATA );
				}
			}
			FOREACH_DEFERREDTREE_END
			TSpeedTreeType::endPass( effect );
		}
	}
}

/**
 *	Iterates through all deferred trees for this tree type, drawing
 *	their fronds. Assumes all render states have already been set.
 */
void TSpeedTreeType::drawDeferredFronds()
{
	BW_GUARD;
	static DogWatch drawFrondsWatch("Fronds");
	ScopedDogWatch watcher(drawFrondsWatch);

	if ( TSpeedTreeType::s_drawFronds_ )
	{
		Moo::EffectMaterialPtr effect = getBranchesEffect();

		this->uploadCommonRenderConstants( effect );		
		this->setFrondModel();
		if ( TSpeedTreeType::beginPass( effect, 
				TSpeedTreeType::s_branchesLightingSetter_.get() ) )
		{
			FOREACH_DEFEREDTREE_BEGIN
			{
				if ( DRAWDATA.lod_.frondDraw_ )
				{
					this->uploadInstanceRenderConstants( effect, 
						TSpeedTreeType::s_branchesLightingSetter_.get(), DRAWDATA );
					this->drawFrond( DRAWDATA );
				}
			}
			FOREACH_DEFERREDTREE_END
			TSpeedTreeType::endPass( effect );
		}
	}
}

/**
 *	Iterates through all deferred trees for this tree type, drawing
 *	their leaves. Assumes all render states have already been set.
 */
void TSpeedTreeType::drawDeferredLeaves()
{
	BW_GUARD;
	static DogWatch drawLeavesWatch("Leaves");
	ScopedDogWatch watcher(drawLeavesWatch);

	if ( TSpeedTreeType::s_drawLeaves_ )
	{
		Moo::EffectMaterialPtr effect = getLeavesEffect();

		this->uploadCommonRenderConstants( effect );	
		this->setLeafModel();

		// This property was sometimes missed when setting it after the call to beginpass.
		// Setting it here works around this issue.
		effect->pEffect()->pEffect()->SetFloat( "g_treeScale", deferredTrees_.front()->treeScale_ );

		if ( TSpeedTreeType::beginPass( effect, 
				TSpeedTreeType::s_leavesLightingSetter_.get() ) )
		{
			FOREACH_DEFEREDTREE_BEGIN
			{
				if ( DRAWDATA.lod_.leafDraw_ )
				{
					this->uploadInstanceRenderConstants( effect, 
						TSpeedTreeType::s_leavesLightingSetter_.get(), 
						DRAWDATA, false );
					this->drawLeaf( effect, DRAWDATA );
				}
			}
			FOREACH_DEFERREDTREE_END
			TSpeedTreeType::endPass( effect );
		}
	}
}

/**
 *	Iterates through all deferred trees for this tree type, drawing
 *	their billboards. Assumes all render states have already been set.
 */
void TSpeedTreeType::drawDeferredBillboards()
{
	BW_GUARD;
	// if billboard optimisation is on and
	// this tree has it setup, skip this call
	#if ENABLE_BB_OPTIMISER
	if ( !TSpeedTreeType::s_optimiseBillboards_ ||
		 this->bbTreeType_ == -1 )
	#endif
	{
		static DogWatch drawBBoardsWatch("Billboards");
		ScopedDogWatch watcher(drawBBoardsWatch);

		if ( TSpeedTreeType::s_drawBillboards_ )
		{
			Moo::EffectMaterialPtr effect = getBillboardsEffect();

			this->uploadCommonRenderConstants( effect );
			if ( TSpeedTreeType::beginPass( effect, NULL ) )
			{
				FOREACH_DEFEREDTREE_BEGIN
				{
					if ( DRAWDATA.lod_.billboardDraw_ )
					{
						this->uploadInstanceRenderConstants( effect, NULL,
													DRAWDATA, false );
						this->drawBillboard( DRAWDATA );
					}
				}
				FOREACH_DEFERREDTREE_END
				TSpeedTreeType::endPass( effect );
			}
		}
	}
}

/**
 *	Draws the branches of a single tree of this type, using the given
 *	DrawData structure. Assumes all render states have already been set.
 *
 *	@param	drawData	the DrawData struct for the tree to be drawn.
 */
void TSpeedTreeType::drawBranch( const DrawData & drawData )
{
	BW_GUARD_PROFILER( TSpeedTreeType_drawBranch );

	if ( this->treeData_.branches_.lod_.empty() )
	{
		return;
	}

	unsigned int lod = drawData.lod_.branchLod_;
	if( lod >= this->treeData_.branches_.lod_.size() )
	{
		ERROR_MSG("Invalid speedtree LOD value, should be [0, %d), is %d\n",
			this->treeData_.branches_.lod_.size(), lod );
		lod = this->treeData_.branches_.lod_.size() - 1;
	}
#ifdef OPT_BUFFERS
	if (this->treeData_.branches_.lod_[lod].index_)
	{
		Moo::rc().setRenderState(
			D3DRS_ALPHAREF,
			(DWORD)drawData.lod_.branchAlpha_);

		int vertexBase = this->treeData_.branches_.verts_->start();
		int primitivesCount = this->treeData_.branches_.lod_[lod].index_->count() - 2;
		int primitivesStart = this->treeData_.branches_.lod_[lod].index_->start();
		if ( s_enableDrawPrim )
		{
			{
				ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
				
				Moo::rc().drawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP, vertexBase,
					0, this->treeData_.branches_.verts_->count(),
					primitivesStart, primitivesCount );
			}
		}
	}
#else
	typedef BranchRenderData::IndexBufferVector::const_iterator citerator;
	citerator stripIt  = this->treeData_.branches_.lod_[lod].strips_.begin();
	citerator stripEnd = this->treeData_.branches_.lod_[lod].strips_.end();
	while ( stripIt != stripEnd )
	{
		if ( (*stripIt)->isValid() )
		{
			Moo::rc().setRenderState(
				D3DRS_ALPHAREF,
				(DWORD)drawData.lod_.branchAlpha_ );

			(*stripIt)->activate();
			int primitivesCount = (*stripIt)->size() - 2;

			if ( s_enableDrawPrim )
			{
				{
					ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
					Moo::rc().drawIndexedPrimitive(
						D3DPT_TRIANGLESTRIP, 0, 0,
						this->treeData_.branches_.lod_[0].vertices_->size(),
						0, primitivesCount );
				}
			}
		}
		++stripIt;
	}
#endif // OPT_BUFFERS
}

/**
 *	Draws the fronds of a single tree of this type, using the given
 *	DrawData structure. Assumes all render states have already been set.
 *
 *	@param	drawData	the DrawData struct for the tree to be drawn.
 */
void TSpeedTreeType::drawFrond( const DrawData & drawData )
{
	BW_GUARD_PROFILER( TSpeedTreeType_drawFrond );

	if (this->treeData_.fronds_.lod_.empty())
	{
		return;
	}

	const int & lod = drawData.lod_.frondLod_;

#ifdef OPT_BUFFERS
	if ( lod < (int)this->treeData_.fronds_.lod_.size() && 
		 this->treeData_.fronds_.lod_[lod].index_ )
	{
		Moo::rc().setRenderState(
			D3DRS_ALPHAREF,
			(DWORD)drawData.lod_.frondAlpha_);

		int vertexBase = this->treeData_.fronds_.verts_->start();
		int primitivesCount = this->treeData_.fronds_.lod_[lod].index_->count() - 2;
		int primitivesStart = this->treeData_.fronds_.lod_[lod].index_->start();
		if ( s_enableDrawPrim )
		{
			{
				ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
				
				Moo::rc().drawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP, vertexBase,
					0, this->treeData_.fronds_.verts_->count(),
					primitivesStart, primitivesCount );
			}
		}
	}

#else

	typedef BranchRenderData::IndexBufferVector::const_iterator citerator;
	citerator stripIt  = this->treeData_.fronds_.lod_[lod].strips_.begin();
	citerator stripEnd = this->treeData_.fronds_.lod_[lod].strips_.end();
	while ( stripIt != stripEnd )
	{
		if ( (*stripIt)->isValid() )
		{
			Moo::rc().setRenderState(
				D3DRS_ALPHAREF,
				(DWORD)drawData.lod_.frondAlpha_ );

			(*stripIt)->activate();
			if ( s_enableDrawPrim )
			{
				int primitivesCount = (*stripIt)->size() - 2;
				{
					ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
					Moo::rc().drawIndexedPrimitive(
						D3DPT_TRIANGLESTRIP, 0, 0,
						this->treeData_.fronds_.lod_[0].vertices_->size(),
						0, primitivesCount );
				}
			}
		}
		++stripIt;
	}
#endif //OPT_BUFFERS
}

/**
 *	Draws the leaves of a single tree of this type, using the given
 *	DrawData structure. Assumes all render states have already been set.
 *
 *  @param	effect		the effect material to use (could be shadows).
 *	@param	drawData	the DrawData struct for the tree to be drawn.
 */
void TSpeedTreeType::drawLeaf(
	Moo::EffectMaterialPtr effect, 
	const DrawData     & drawData )
{
	BW_GUARD_PROFILER( TSpeedTreeType_drawLeaf );

	if ( this->treeData_.leaves_.lod_.empty() )
	{
		return;
	}

	ShadowCaster * caster = TSpeedTreeType::s_curShadowCaster_;
	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();

	{
		PROFILER_SCOPED( TSpeedTreeType_setEffectParam );
		pEffect->SetFloat( "g_treeScale", drawData.treeScale_ );
	}
	pEffect->CommitChanges();

	// overriding the culling for a mirror
	if ( Moo::rc().mirroredTransform() )
		Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	for ( int i=0; i<drawData.lod_.leafLodCount_; ++i )
	{
		const int & lod = drawData.lod_.leafLods_[i];

#ifdef OPT_BUFFERS
		if (lod < (int)this->treeData_.leaves_.lod_.size() &&
			this->treeData_.leaves_.lod_[lod].index_)
		{
			Moo::rc().setRenderState(
				D3DRS_ALPHAREF,
				(DWORD)drawData.lod_.leafAlphaValues_[i]);

			int vertexBase = this->treeData_.leaves_.verts_->start();
			int primitivesCount = this->treeData_.leaves_.lod_[lod].index_->count()  / 3;
			int primitivesStart = this->treeData_.leaves_.lod_[lod].index_->start();

			if ( s_enableDrawPrim )
			{
				{
					ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
					Moo::rc().drawIndexedPrimitive(
						D3DPT_TRIANGLELIST, vertexBase,
						0, this->treeData_.leaves_.verts_->count(),
						primitivesStart, primitivesCount );
				}
			}
		}
#else
		if ( this->treeData_.leaves_.lod_[lod].vertices_->isValid() )
		{
			Moo::rc().setRenderState(
				D3DRS_ALPHAREF,
				(DWORD)drawData.lod_.leafAlphaValues_[i]);

			this->treeData_.leaves_.lod_[lod].vertices_->activate();
			int verticesCount =
				this->treeData_.leaves_.lod_[lod].vertices_->size();
			
			this->treeData_.leaves_.lod_[lod].strips_[0]->activate();
			int primitivesCount =
				this->treeData_.leaves_.lod_[lod].strips_[0]->size() / 3;
			if ( s_enableDrawPrim )
			{
				{
					ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
					Moo::rc().drawIndexedPrimitive(
						D3DPT_TRIANGLELIST, 0, 0,
						verticesCount, 0, primitivesCount );
				}
			}
		}
#endif
	}
}

/**
 *	Draws the billboard of a single tree of this type, using the given
 *	DrawData structure. Assumes all render states have already been set.
 *
 *	@param	drawData	the DrawData struct for the tree to be drawn.
 */
void TSpeedTreeType::drawBillboard( const DrawData & drawData )
{
	BW_GUARD_PROFILER( TSpeedTreeType_drawBillboard );

	if ( !this->treeData_.billboards_.lod_.empty() &&
		this->treeData_.billboards_.lod_[0].vertices_->isValid() )
	{
		ComObjectWrap<ID3DXEffect> pEffect = getBillboardsEffect()->pEffect()->pEffect();

		const float & minAlpha  = BillboardVertex::s_minAlpha_;
		const float & maxAlpha  = BillboardVertex::s_maxAlpha_;
		const float & fadeValue = drawData.lod_.billboardFadeValue_;
		const float   alphaRef  = 1 + fadeValue * (minAlpha/maxAlpha -1);
		{
			PROFILER_SCOPED( TSpeedTreeType_setEffectParam );
			pEffect->SetFloat( "g_bbAlphaRef", alphaRef );
		}
		pEffect->CommitChanges();

		int triCount = this->treeData_.billboards_.lod_[0].vertices_->size() / 3;
		{
			ScopedDogWatch watcher(TSpeedTreeType::s_primitivesWatch_);
			this->treeData_.billboards_.lod_[0].vertices_->activate();
			Moo::rc().drawPrimitive( D3DPT_TRIANGLELIST, 0, triCount );
		}
	}
}


/**
 *	Clears the list of deferred trees for this tree type.
 */
void TSpeedTreeType::clearDeferredTrees()
{
	BW_GUARD;
	// calling erase instead of clear on the vector
	// because it's faster (measured in VC++2005).
	this->deferredTrees_.erase(
		this->deferredTrees_.begin(),
		this->deferredTrees_.end() );

	s_deferredLighting_.erase(
		s_deferredLighting_.begin(),
		s_deferredLighting_.end() );

	// The default lighting batch is now invalid so set to -1
	TSpeedTreeType::DrawData::s_defaultLightingBatch_ = -1;
}

/**
 *	Sets the global render states for drawing the trees.
 *
 *  @param	effect		the effect material to upload the constants to.
 */
void TSpeedTreeType::prepareRenderContext( Moo::EffectMaterialPtr effect )
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	// update lighting setup
	const Moo::Colour & diffuse   = TSpeedTreeType::s_sunDiffuse_;
	const Moo::Colour & ambient   = TSpeedTreeType::s_sunAmbient_;
	const Vector3     & direction = TSpeedTreeType::s_sunDirection_;
	float lighting[] =
	{
		direction.x, direction.y, direction.z, 1.0f,
		diffuse.r, diffuse.g, diffuse.b, 1.0f,
		ambient.r, ambient.g, ambient.b, 1.0f,
	};

	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();
	pEffect->SetVectorArray( "g_sun", (D3DXVECTOR4*)lighting, 3 );

#ifndef EDITOR_ENABLED
	pEffect->SetVector( "fogColour", &FogController::instance().v4colour() );
	pEffect->SetFloat( "fogStart", Moo::rc().fogNear() );
	pEffect->SetFloat( "fogEnd", Moo::rc().fogFar() );
#endif

	Moo::EffectVisualContext::instance().initConstants();
}

/**
 *	Sets the render states for drawing the fronds,
 *	leaves, and billboards of this type of tree.
 *
 *  @param	effect		the effect material to upload the constants to.
 */
void TSpeedTreeType::setCompositeRenderStates( Moo::EffectMaterialPtr effect ) const
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();
	pEffect->SetBool( "alphaTestEnable", true );
	pEffect->SetBool( "g_texturedTrees", TSpeedTreeType::s_texturedTrees_ );
	if ( this->treeData_.leaves_.diffuseMap_.exists() )
		pEffect->SetTexture( "g_diffuseMap",
			this->treeData_.leaves_.diffuseMap_->pTexture() );

	#if SPT_ENABLE_NORMAL_MAPS
		bool bUseNormals = this->treeData_.leaves_.normalMap_.exists();
		if ( bUseNormals )
			pEffect->SetTexture( "g_normalMap", 
				this->treeData_.leaves_.normalMap_->pTexture() );
		pEffect->SetBool( "g_useNormalMap",bUseNormals );
	#endif // SPT_ENABLE_NORMAL_MAPS

	pEffect->CommitChanges();
}

/**
 *	Sets the render states for drawing the branches of this type of tree.
 */
void TSpeedTreeType::setBranchRenderStates() const
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	ComObjectWrap<ID3DXEffect> pEffect;
	if ( TSpeedTreeType::s_curShadowCaster_ == NULL )
	{
		const float* branchMaterial = this->speedTree_->GetBranchMaterial();
		float material[] =
		{
			branchMaterial[0], branchMaterial[1], branchMaterial[2], 1.0f,
			branchMaterial[3], branchMaterial[4], branchMaterial[5], 1.0f,
		};

		pEffect = getBranchesEffect()->pEffect()->pEffect();
		pEffect->SetBool( "alphaTestEnable", false );
		pEffect->SetVectorArray( "g_material", (const D3DXVECTOR4*)&material[0], 2 );
		pEffect->SetBool( "g_texturedTrees", TSpeedTreeType::s_texturedTrees_ );
		if (this->treeData_.branches_.diffuseMap_.exists())
			pEffect->SetTexture( "g_diffuseMap", this->treeData_.branches_.diffuseMap_->pTexture() );

		#if SPT_ENABLE_NORMAL_MAPS
			bool bUseNormals = this->treeData_.branches_.normalMap_.exists();
			if ( bUseNormals )
				pEffect->SetTexture(
					"g_normalMap", 
					this->treeData_.branches_.normalMap_->pTexture() );
			pEffect->SetBool( "g_useNormalMap", bUseNormals );
		#endif // SPT_ENABLE_NORMAL_MAPS
		
		const Matrix & invView = TSpeedTreeType::s_invView_;
		float cameraPos[] = { invView._41, invView._42, invView._43, 1.0f };
		pEffect->SetVector( "g_cameraPos", (D3DXVECTOR4*)cameraPos );
	}
	else
	{
		TSpeedTreeType::s_shadowsFX_->hTechnique( "branches" );
		pEffect = TSpeedTreeType::s_shadowsFX_->pEffect()->pEffect();
	}

	pEffect->SetBool( "g_cullEnabled", true );
	pEffect->CommitChanges();

	Moo::rc().setFVF( BranchVertex::fvf() );
}


#ifdef OPT_BUFFERS

template <class BASE>
void bindModelData( )
{
	// Check for existance
	if ( BASE::s_indexBuffer_ == NULL )
	{
		BASE::s_indexBuffer_ = new IndexBufferWrapper;
	}

	// Copy the data.
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);
		if ( BASE::s_indexList_.size() && BASE::s_indexList_.dirty() )
		{
			if ( BASE::s_indexBuffer_->reset( BASE::s_indexList_.size() ) )
			{
				//TODO: 32-bit indices....
				BASE::s_indexBuffer_->copy( (int*)&BASE::s_indexList_[0] );
				BASE::s_indexList_.dirty( false );
			}
		}
	}

	if ( BASE::s_vertexBuffer_ == NULL )
	{
		typedef VertexBufferWrapper< BASE::VertexType > VBufferType;
		BASE::s_vertexBuffer_ = new VBufferType;
	}

	// Copy the data.
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);
		if ( BASE::s_vertexList_.size() && BASE::s_vertexList_.dirty() )
		{
			if ( BASE::s_vertexBuffer_->reset( BASE::s_vertexList_.size() ) )
			{
				BASE::s_vertexBuffer_->copy( &BASE::s_vertexList_[0] );
				BASE::s_vertexList_.dirty( false );
			}
		}
	}

	// Now bind the buffers.
	BASE::s_vertexBuffer_->activate();
	BASE::s_indexBuffer_->activate();
}
#endif 
/**
 *	Activates the vertex buffer to draw the branches of this type of tree.
 */
void TSpeedTreeType::setBranchModel() const
{
	BW_GUARD;	
#ifdef OPT_BUFFERS
	bindModelData<CommonTraits>();
#else
	if ( this->treeData_.branches_.lod_[0].vertices_->isValid() )
	{
		this->treeData_.branches_.lod_[0].vertices_->activate();
	}
#endif //OPT_BUFFERS
}

/**
 *	Sets the render states for drawing the fronds of this type of tree.
 */
void TSpeedTreeType::setFrondRenderStates()
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	ComObjectWrap<ID3DXEffect> pEffect;
	if ( TSpeedTreeType::s_curShadowCaster_ == NULL )
	{
		const float* frondMaterial = this->speedTree_->GetFrondMaterial();
		float material[] =
		{
			frondMaterial[0], frondMaterial[1], frondMaterial[2], 1.0f,
			frondMaterial[3], frondMaterial[4], frondMaterial[5], 1.0f,
		};

		pEffect = getBranchesEffect()->pEffect()->pEffect();
		pEffect->SetBool( "alphaTestEnable", true );
		pEffect->SetVectorArray( "g_material", (const D3DXVECTOR4*)&material[0], 2 );
		pEffect->SetBool( "g_texturedTrees", TSpeedTreeType::s_texturedTrees_ );
		if ( this->treeData_.leaves_.diffuseMap_.exists() )
			pEffect->SetTexture( "g_diffuseMap",
				this->treeData_.leaves_.diffuseMap_->pTexture());

		#if SPT_ENABLE_NORMAL_MAPS
			bool bUseNormals = this->treeData_.leaves_.normalMap_.exists();
			if ( bUseNormals )
			{
				pEffect->SetTexture(
					"g_normalMap", 
					this->treeData_.leaves_.normalMap_->pTexture() );
			}
			pEffect->SetBool( "g_useNormalMap", bUseNormals );
		#endif // SPT_ENABLE_NORMAL_MAPS

		const Matrix & invView = TSpeedTreeType::s_invView_;
		float cameraPos[] = { invView._41, invView._42, invView._43, 1.0f };
		pEffect->SetVector( "g_cameraPos", (D3DXVECTOR4*)cameraPos );
	}
	else
	{
		TSpeedTreeType::s_shadowsFX_->hTechnique( "branches" );
		pEffect = TSpeedTreeType::s_shadowsFX_->pEffect()->pEffect();
	}

	pEffect->SetBool( "g_cullEnabled", false );
	pEffect->CommitChanges();

	Moo::rc().setFVF( BranchVertex::fvf() );
}

/**
 *	Activates the vertex buffer to draw the fronds of this type of tree.
 */
void TSpeedTreeType::setFrondModel() const
{
	BW_GUARD;	
#ifdef OPT_BUFFERS
	bindModelData<CommonTraits>();
#else
	if ( this->treeData_.fronds_.lod_[0].vertices_->isValid() )
	{
		this->treeData_.fronds_.lod_[0].vertices_->activate();
	}
#endif
}

/**
 *	Sets the render states for drawing the leaves of this type of tree.
 */
void TSpeedTreeType::setLeafRenderStates()
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	ComObjectWrap<ID3DXEffect> pEffect;
	if ( TSpeedTreeType::s_curShadowCaster_ == NULL )
	{
		pEffect = getLeavesEffect()->pEffect()->pEffect();

		// leaf material
		const float* leafMaterial = this->speedTree_->GetLeafMaterial();
		float material[] =
		{
			leafMaterial[0], leafMaterial[1], leafMaterial[2], 1.0f,	
			leafMaterial[3], leafMaterial[4], leafMaterial[5], 1.0f,	
		};
		pEffect->SetVectorArray( "g_material", (D3DXVECTOR4*)material, 2 );

		// light adjustment
		float adjustment = this->speedTree_->GetLeafLightingAdjustment();
		pEffect->SetFloat( "g_leafLightAdj", adjustment );

		// texture, draw textured flag
		pEffect->SetBool( "alphaTestEnable", true );
		pEffect->SetBool( "g_texturedTrees", TSpeedTreeType::s_texturedTrees_ );
		if ( this->treeData_.leaves_.diffuseMap_.exists() )
		{
			pEffect->SetTexture( "g_diffuseMap", 
				this->treeData_.leaves_.diffuseMap_->pTexture() );
		}

		#if SPT_ENABLE_NORMAL_MAPS
			bool bUseNormals = this->treeData_.leaves_.normalMap_.exists();
			if ( bUseNormals )
			{
				pEffect->SetTexture(
					"g_normalMap", 
					this->treeData_.leaves_.normalMap_->pTexture() );
			}
			pEffect->SetBool( "g_useNormalMap", bUseNormals );
		#endif // SPT_ENABLE_NORMAL_MAPS

		const Matrix & invView = TSpeedTreeType::s_invView_;
		float cameraPos[] = { invView._41, invView._42, invView._43, 1.0f };
		pEffect->SetVector( "g_cameraPos", (D3DXVECTOR4*)cameraPos );
	}
	else
	{
		TSpeedTreeType::s_shadowsFX_->hTechnique( "leaves" );
		pEffect = TSpeedTreeType::s_shadowsFX_->pEffect()->pEffect();
		pEffect->SetMatrix( "g_invView", (D3DXMATRIX*)&TSpeedTreeType::s_invView_ );
	}

	// leaf billboard tables
	if ( this->pWind_ )
	{
		//TODO: remove this overriding of the leaf state
		// and replace it with proper sharing of param in the shaders
		bool oldVal = this->pWind_->hasLeaves();
		this->pWind_->hasLeaves( true );

		TSpeedTreeType::setWind( this->pWind_, pEffect );		
		this->pWind_->hasLeaves( oldVal );
	}

	Vector4 leafAngleScalars( this->treeData_.leafRockScalar_,
							this->treeData_.leafRustleScalar_, 0, 0 );
	pEffect->SetVector( "g_leafAngleScalars", (D3DXVECTOR4*)&leafAngleScalars );
	pEffect->SetFloat( "g_LeafRockFar", TSpeedTreeType::s_leafRockFar_ );

	Moo::rc().setFVF( LeafVertex::fvf() );
}


/**
 *	Activates the vertex buffer to draw the leaves of this type of tree.
 */
void TSpeedTreeType::setLeafModel() const
{
	BW_GUARD;	
#ifdef OPT_BUFFERS
	bindModelData<LeafTraits>();
#endif
}

/**
 *	Sets the render states for drawing the billboards of this type of tree.
 */
void TSpeedTreeType::setBillboardRenderStates()
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	// if billboard optimisation is on and
	// this tree has it setup, skip this call
	#if ENABLE_BB_OPTIMISER
	if ( !TSpeedTreeType::s_optimiseBillboards_ ||
		 this->bbTreeType_ == -1 )
	#endif
	{
		ComObjectWrap<ID3DXEffect> pEffect = getBillboardsEffect()->pEffect()->pEffect();

		const Matrix & invView = TSpeedTreeType::s_invView_;
		float cameraDir[] = { invView._31, invView._32, invView._33, 1.0f };
		pEffect->SetVector( "g_cameraDir", (D3DXVECTOR4*)cameraDir );

		// light adjustment
		float adjustment = this->speedTree_->GetLeafLightingAdjustment();
		pEffect->SetFloat( "g_leafLightAdj", adjustment );

		// leaf material
		const float* leafMaterial = this->speedTree_->GetLeafMaterial();
		float material[] =
		{
			leafMaterial[0], leafMaterial[1], leafMaterial[2], 1.0f,	
			leafMaterial[3], leafMaterial[4], leafMaterial[5], 1.0f,	
		};
		pEffect->SetVectorArray( "g_material", (D3DXVECTOR4*)material, 2 );

		pEffect->CommitChanges();
		Moo::rc().setFVF( BillboardVertex::fvf() );
	}
}

/**
 *	Begin the effect render pass.
 *
 *	@param	effect			the effect material that will be used.
 *  @param  lightSetter		the light setter to use for uploading light constants.
 */
bool TSpeedTreeType::beginPass(
	Moo::EffectMaterialPtr		effect,
	Moo::EffectLightingSetter * lightSetter )
{
	BW_GUARD;
	if ( effect->begin() && effect->beginPass(0) )
	{
		if ( lightSetter != NULL )
		{
			lightSetter->resetBatchCookie();
		}
		return true;
	}
	else
		return false;
}

/**
 *	End the effect render pass.
 *
 *	@param	effect			the effect material that's been used.
 */
void TSpeedTreeType::endPass( Moo::EffectMaterialPtr effect )
{
	BW_GUARD;
	effect->endPass();
	effect->end();
}

/**
 *	Stores the current sun light information. It will
 *	later be used to setup the lighting for drawing the trees.
 *
 *	@param	envMinder	the current EnvironMander object.
 */
void TSpeedTreeType::saveLightInformation( EnviroMinder * envMinder )
{
	BW_GUARD;
	// Save for future use
	Vector3   & direction = TSpeedTreeType::s_sunDirection_;
	Moo::Colour & diffuse = TSpeedTreeType::s_sunDiffuse_;
	Moo::Colour & ambient = TSpeedTreeType::s_sunAmbient_;

	if ( !TSpeedTreeType::s_enviroMinderLight_ &&
		 Moo::rc().lightContainer().exists()  &&
		 Moo::rc().lightContainer()->nDirectionals() > 0 )
	{
		direction = Moo::rc().lightContainer()->directional(0)->direction();
		diffuse = Moo::rc().lightContainer()->directional(0)->colour();
		ambient = Moo::rc().lightContainer()->ambientColour();
	}
	else if ( envMinder != NULL )
	{
		const OutsideLighting lighting = envMinder->timeOfDay()->lighting();
		direction = -1.f * lighting.mainLightDir();
		diffuse   = Moo::Colour(
			lighting.sunColour.x, lighting.sunColour.y,
			lighting.sunColour.z, lighting.sunColour.w );

		ambient = Moo::Colour(
			lighting.ambientColour.x, lighting.ambientColour.y,
			lighting.ambientColour.z, lighting.ambientColour.w );
	}
	else
	{
		diffuse   = Moo::Colour(0.58f, 0.51f, 0.38f, 1.0f);
		ambient   = Moo::Colour(0.5f, 0.5f, 0.5f, 1.0f);
		direction = Vector3(0.0f, -1.0f, 0.0f);
	}
}

/**
 *	Retrieves a TSpeedTreeType instance for the given filename and seed. 
 *
 *	If this is the first time this tree is being requested, it  will be 
 *	loaded, generated and cached in the tree-types-map (s_typesMap). 
 *	Otherwise, the cached tree will be retrieved from the map and 
 *	returned, instead. 
 *
 *	Note: this function will also look for cached trees in the 
 *	sync-init-list (s_syncInitList). It may be temporarily stored there if 
 *	the tree have just recently been loaded but not yet sync-initialised 
 *	(that's when it actually get's inserted into the tree-types-map).
 *
 *	After loading the tree file (".spt"), this function will then look 
 *	for a pre-computed-tree file (".ctree"). If one is found, the geometry 
 *	data will be loaded directly from this file. Otherwise,  the tree will 
 *	be computed and the geometry will be extracted from it. The geometry 
 *	will then be saved into a ".ctree" file for future use. 
 *
 *	Note that the ".spt" file is always loaded. That's because some run-time 
 *	operations, most notably LOD level calculations, are responsibility of 
 *	the original CSpeedTreeRT instance.
 *
 *	@see				doSyncInit().
 *
 *	@param	filename	name of source SPT file from where to generate tree.
 *	@param	seed		seed number to use when generation tree.
 *
 *	@return				the requested tree type object.
 *
 *	@note	will throw std::runtime_error if object can't be created.
 */
TSpeedTreeType::TreeTypePtr TSpeedTreeType::getTreeTypeObject(
	const char * filename, uint seed )
{
	BW_GUARD;
	START_TIMER(loadGlobal);

	TreeTypePtr result;

	// first, look at list of
	// existing renderer objects
	START_TIMER(searchExisting);
	std::string treeDefName = createTreeDefName( filename, seed );
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_syncInitLock_);
		TreeTypesMap & typesMap = TSpeedTreeType::s_typesMap_;
		TreeTypesMap::const_iterator rendererIt = typesMap.find( treeDefName );
		if ( rendererIt != typesMap.end() )
		{
			result = rendererIt->second;
		}
		// it may still be in the sync init
		// list. Look for it there, then.
		else
		{
			// list should be small. A linear search
			// here is actually not at all that bad.
			typedef TSpeedTreeType::InitVector::const_iterator citerator;
			citerator initIt  = TSpeedTreeType::s_syncInitList_.begin();
			citerator initEnd = TSpeedTreeType::s_syncInitList_.end();
			while ( initIt != initEnd )
			{
				if ( (*initIt)->seed_ == seed &&
					 (*initIt)->filename_ == filename )
				{
					result = *initIt;
					break;
				}
				++initIt;
			}
		}
	}
	STOP_TIMER(searchExisting, "");

	// if still not found,
	// create a new one
	if ( !result.exists() )
	{
		START_TIMER(actualLoading);
		typedef std::auto_ptr< TSpeedTreeType > RendererAutoPtr;
		RendererAutoPtr renderer(new TSpeedTreeType);

		// not using speedtree's clone function here because
		// it prevents us from saving from memory by calling
		// Delete??Geometry after generating all geometry.

		bool success = false;
		std::string errorMsg = "Unknown error";
		CSpeedTreeRTPtr speedTree;
		try
		{
			DataSectionPtr rootSection = BWResource::instance().rootSection();
			BinaryPtr sptData = rootSection->readBinary( filename );
			if ( sptData.exists() )
			{
				speedTree.reset( new CSpeedTreeRT );
				speedTree->SetTextureFlip( true );
				success = speedTree->LoadTree(
					(uchar*)sptData->data(),
					sptData->len() );
				if (!success)
				{
					errorMsg = CSpeedTreeRT::GetCurrentError();
				}
			}
			else
			{
				errorMsg = "Resource not found";
			}
		}
		catch (...)
		{
			errorMsg = CSpeedTreeRT::GetCurrentError();
		}

		if ( !success )
		{
			errorMsg = std::string(filename) + ": " + errorMsg;
			throw std::runtime_error( errorMsg );
		}
		STOP_TIMER(actualLoading, "");

		renderer->seed_     = seed;
		renderer->filename_ = filename;
		speedTree->SetBranchLightingMethod( CSpeedTreeRT::LIGHT_DYNAMIC );
		speedTree->SetLeafLightingMethod( CSpeedTreeRT::LIGHT_DYNAMIC );
		speedTree->SetFrondLightingMethod( CSpeedTreeRT::LIGHT_DYNAMIC );

		speedTree->SetBranchWindMethod( CSpeedTreeRT::WIND_GPU );
		speedTree->SetLeafWindMethod( CSpeedTreeRT::WIND_GPU );
		speedTree->SetFrondWindMethod( CSpeedTreeRT::WIND_GPU );

		speedTree->SetNumLeafRockingGroups(4);

		std::string noExtension  = BWResource::removeExtension( filename );
		std::string treeDataName = noExtension + ".ctree";

		if ( !speedTree->Compute( NULL, seed ) )
		{
			errorMsg = std::string(filename) + ": Could not compute tree";
			throw std::runtime_error( errorMsg );
		}

		// speed wind
		std::string datapath = BWResource::getFilePath( filename );
		renderer->pWind_ = setupSpeedWind( *speedTree, datapath,
											TSpeedTreeType::s_speedWindFile_ );

		if ( BWResource::isFileOlder( treeDataName, filename ) ||
			!renderer->treeData_.load( treeDataName ) )
		{
			int windAnglesCount = 
				renderer->pWind_->matrixCount();
			
			renderer->treeData_ = TSpeedTreeType::generateTreeData(
									*speedTree, *renderer->pWind_->speedWind(),
									filename, seed, windAnglesCount );

			renderer->treeData_.save( treeDataName );
		}
		
		if ( renderer->pWind_ != &TSpeedTreeType::s_defaultWind_ )
		{
			renderer->pWind_->hasLeaves( renderer->pWind_->hasLeaves() ||
									renderer->treeData_.leaves_.checkVerts() );
		}

		renderer->speedTree_ = speedTree;
		
		// bsp tree
		START_TIMER(bspTree);
		renderer->bspTree_ = createBSPTree(
			renderer->speedTree_.get(),
			renderer->filename_.c_str(),
			renderer->seed_,
			renderer->treeData_.boundingBox_ );
		STOP_TIMER(bspTree, "");

		// release geometry data
		// held internally by speedtree
		renderer->speedTree_->DeleteBranchGeometry();
		renderer->speedTree_->DeleteFrondGeometry();
		renderer->speedTree_->DeleteLeafGeometry();
		renderer->speedTree_->DeleteTransientData();
		
		result = renderer.release();
		
		// Because it involves a lot of D3D work, the last bit
		// of initialisation must be done in the main thread.
		// Put render in deferred initialisation list so that
		// all pending initialisation tasks will be carried
		// next beginFrame call.
		START_TIMER(syncInitList);
		{
			SimpleMutexHolder mutexHolder(TSpeedTreeType::s_syncInitLock_);
			TSpeedTreeType::s_syncInitList_.push_back(result.getObject());
		}
		STOP_TIMER(syncInitList, "");
	}

	STOP_TIMER(loadGlobal, filename);

	return result;
}

/**
 *	Generates all rendering data for the tree (index and vertex buffers
 *	plus auxiliary information).
 *
 *	@param	speedTree		speedtree object from which to extract the data.
 *	@param	filename		name of the spt file from where the tree was loaded.
 *	@param	seed			seed number used to generate tree.
 *	@param	windAnglesCount	number of wind angles used to animate trees.
 *
 *	@return					a filled up TreeData structure.
 */
const TSpeedTreeType::TreeData TSpeedTreeType::generateTreeData(
	CSpeedTreeRT      & speedTree, 
	CSpeedWind	      & speedWind,
	const std::string & filename, 
	int                 seed,
	int                 windAnglesCount )
{
	BW_GUARD;
	START_TIMER(generateTreeData);

	SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);

	TreeData result;

	// get boundingbox
	float bounds[6];
	speedTree.GetBoundingBox( bounds );
	result.boundingBox_.setBounds( 
		Vector3(bounds[0], bounds[1], bounds[2]),
		Vector3(bounds[3], bounds[4], bounds[5] ) );

	// create the geomtry buffers
	//
	std::string datapath = BWResource::getFilePath( filename );

	TRACE_MSG( "generateTreeData for tree type %s\n", filename.c_str() );

	START_TIMER(localBranches);
	result.branches_ = createPartRenderData< BranchTraits >(
		speedTree, speedWind, datapath.c_str() );
	STOP_TIMER(localBranches, "");

	START_TIMER(localFronds);
	result.fronds_ = createPartRenderData< FrondTraits >(
		speedTree, speedWind, datapath.c_str() );
	STOP_TIMER(localFronds, "");

	START_TIMER(localLeaves);
	result.leaves_ = createLeafRenderData(
		speedTree, speedWind, filename, datapath,
		windAnglesCount, result.leafRockScalar_, 
		result.leafRustleScalar_ );
	STOP_TIMER(localLeaves, "");

	START_TIMER(localBillboards);
	result.billboards_ = createBillboardRenderData(
		speedTree, datapath.c_str() );
	STOP_TIMER(localBillboards, "");

	STOP_TIMER(generateTreeData, "");
	
	return result;
}

/**
 *	Performs asynchronous initialisation. Any initialisation
 *	that can be done in the loading thread should be done here.
 */
void TSpeedTreeType::asyncInit()
{
	BW_GUARD;	
}

/**
 *	Performs synchronous initialisation. Any initialisation that
 *	has got to be carried in the rendering thread can be done here.
 */
void TSpeedTreeType::syncInit()
{
	BW_GUARD;
	START_TIMER(addTreeType);
	#if ENABLE_BB_OPTIMISER
		if (TSpeedTreeType::s_optimiseBillboards_ && 
			!this->treeData_.billboards_.lod_.empty() &&
			this->treeData_.leaves_.diffuseMap_)
		{
			this->bbTreeType_ = BillboardOptimiser::addTreeType(
				this->treeData_.leaves_.diffuseMap_,				
#if SPT_ENABLE_NORMAL_MAPS
				this->treeData_.leaves_.normalMap_,
#endif // SPT_ENABLE_NORMAL_MAPS
				*this->treeData_.billboards_.lod_.back().vertices_,
				this->filename_.c_str(),
				this->speedTree_->GetLeafMaterial(),
				this->speedTree_->GetLeafLightingAdjustment());
		}
	#endif
	STOP_TIMER(addTreeType, "");
}

/**
 *	Updates the render groups vector. Trees generated from the same SPT file,
 *	but using different seed numbers have their own TSpeedTreeType object each.
 *	Nonetheless, they still share all rendering states. Render group is a list
 *	of all existing TSpeedTreeType objects sorted by their source SPT filenames.
 *	This list is used when drawing trees to avoid unecessary state changes.
 */
void TSpeedTreeType::updateRenderGroups()
{
	BW_GUARD;
	static DogWatch groupsWatch("Groups");

	RendererGroupVector & renderGroups = TSpeedTreeType::s_renderGroups_;
	renderGroups.clear();

	std::string lastFilename;
	TSpeedTreeType::s_speciesCount_ = 0;
	FOREACH_TREETYPE_BEGIN(groupsWatch)
	{
		if (RENDER->filename_ != lastFilename)
		{
			lastFilename = RENDER->filename_;
			renderGroups.push_back( RendererVector() );
			++TSpeedTreeType::s_speciesCount_;
		}
		renderGroups.back().push_back( RENDER );
	}
	FOREACH_TREETYPE_END
}

/**
 *	Increments the reference counter.
 */
void TSpeedTreeType::incRef() const
{
	++this->refCounter_;
}

/**
 *	Increments the reference count. DOES NOT delete the object if
 *	the reference counter reaches zero. The recycleTreeTypeObjects
 *	method will take care of that (for multithreading sanity).
 */
void TSpeedTreeType::decRef() const
{
	--this->refCounter_;
	MF_ASSERT(this->refCounter_>=0);
}

/**
 *	Returns the current reference counter value.
 */
int  TSpeedTreeType::refCount() const
{
	return this->refCounter_;
}

// ---------------------------------------------------- TSpeedTreeType::TreeData

/**
 *	Contructor.
 */
TSpeedTreeType::TreeData::TreeData() :
	boundingBox_( BoundingBox::s_insideOut_ ),
	branches_(),
	fronds_(),
	leaves_(),
	billboards_(),
	leafRockScalar_( 0.0f ),
	leafRustleScalar_( 0.0f )
{
	BW_GUARD;
}

/**
 *	Saves tree render data to a binary cache file.
 *
 *	@param	filename	path to file to be saved.
 *
 *	@return				true if successfull, false on error.
 */
bool TSpeedTreeType::TreeData::save( const std::string & filename )
{
	BW_GUARD;
	START_TIMER(saveTreeData);
	
	int totalSize = 0;

	// version info
	totalSize += sizeof(int);

	// bounding box + leaf scalars
	totalSize += 2*sizeof(Vector3) + 2*sizeof(float);
	
	// render part data
#ifdef OPT_BUFFERS
	totalSize += this->branches_.size();
	totalSize += this->fronds_.size();
	totalSize += this->leaves_.size();
	totalSize += this->billboards_.oldSize();
#else
	totalSize += this->branches_.oldSize();
	totalSize += this->fronds_.oldSize();
	totalSize += this->leaves_.oldSize();
	totalSize += this->billboards_.oldSize();
#endif
	
	char * pdata = new char[totalSize];
	char * data = pdata;

	int version = SPT_CTREE_VERSION;
	memcpy( data, &version, sizeof(int) );
	data += sizeof(int);

	memcpy( data, &this->boundingBox_.minBounds(), sizeof(Vector3) );
	data += sizeof(Vector3);
	
	memcpy( data, &this->boundingBox_.maxBounds(), sizeof(Vector3) );
	data += sizeof(Vector3);

	*(float*)data = this->leafRockScalar_;
	data += sizeof(float);

	*(float*)data = this->leafRustleScalar_;
	data += sizeof(float);
	
	START_TIMER(saveRenderData);

#if ENABLE_FILE_CASE_CHECKING
	FilenameCaseChecker	caseChecker;
	const std::string & diff1 =  this->branches_.diffuseMap_.exists() ?	
		this->branches_.diffuseMap_->resourceID() : "";
	caseChecker.check( diff1 );

	const std::string & diff2 =  this->fronds_.diffuseMap_.exists() ?	
		this->fronds_.diffuseMap_->resourceID() : "";
	caseChecker.check( diff2 );

	const std::string & diff3 =  this->leaves_.diffuseMap_.exists() ?	
		this->leaves_.diffuseMap_->resourceID() : "";
	caseChecker.check( diff3 );
#endif // ENABLE_FILE_CASE_CHECKING

#ifdef OPT_BUFFERS
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);
		data = this->branches_.write( data, CommonTraits::s_vertexList_, CommonTraits::s_indexList_ );
		data = this->fronds_.write( data, CommonTraits::s_vertexList_, CommonTraits::s_indexList_  );
		data = this->leaves_.write( data, LeafTraits::s_vertexList_, LeafTraits::s_indexList_  );
	}
	data = this->billboards_.oldWrite( data );
#else
	data = this->branches_.oldWrite( data );
	data = this->fronds_.oldWrite( data );
	data = this->leaves_.oldWrite( data );
	data = this->billboards_.oldWrite( data );
#endif
	STOP_TIMER(saveRenderData, "");

	// create a section
	START_TIMER(saveSection);
	size_t lastSep = filename.find_last_of('/');
	std::string parentName = filename.substr( 0, lastSep );
	DataSectionPtr parentSection = BWResource::openSection( parentName );
	MF_ASSERT(parentSection);
	
	std::string nameonly = BWResource::getFilename( filename );

	DataSectionPtr file = new BinSection( 
		nameonly, new BinaryBlock( pdata, totalSize, "BinaryBlock/SpeedTree" ) );

	delete [] pdata;

	DataSectionPtr pExisting = DataSectionCensus::find(filename);
	if (pExisting != NULL)
	{
		// Remove this from the census to ensure we can write a fresh copy
		DataSectionCensus::del(pExisting.getObject());
	}

	file->setParent( parentSection );
	file = DataSectionCensus::add( filename, file );
	bool result = file->save();	
	STOP_TIMER(saveSection, "");
	
	STOP_TIMER(saveTreeData, "");
	return result;	
}

/**
 *	Loads tree render data from a binary cache file.
 *
 *	@param	filename	path to file to be saved.
 *
 *	@return				true if successfull, false on error. Will fail
 *						if it can't other the specified file.
 */
bool TSpeedTreeType::TreeData::load( const std::string & filename )
{
	BW_GUARD;
	START_TIMER(loadTreeData);
	
	START_TIMER(openSection);
	DataSectionPtr file =
		BWResource::openSection( filename, false, BinSection::creator() );
	STOP_TIMER(openSection, "");

	if ( !file )
		return false;

	const char * data = file->asBinary()->cdata();

	// read version data
	int version;
	memcpy( &version, data, sizeof(int) );
	data += sizeof(int);
	if (version != SPT_CTREE_VERSION)
		return false;

	Vector3 minBounds;
	memcpy( &minBounds, data, sizeof(Vector3) );
	data += sizeof(Vector3);
	
	Vector3 maxBounds;
	memcpy( &maxBounds, data, sizeof(Vector3) );
	data += sizeof(Vector3);

	this->boundingBox_.setBounds( minBounds, maxBounds );

	this->leafRockScalar_ = *(float*)data;
	data += sizeof(float);

	this->leafRustleScalar_ = *(float*)data;
	data += sizeof(float);
	
	START_TIMER(loadRenderData);
#ifdef OPT_BUFFERS
	{
		SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);
		data = this->branches_.read( data, CommonTraits::s_vertexList_, CommonTraits::s_indexList_ );
		data = this->fronds_.read( data, CommonTraits::s_vertexList_, CommonTraits::s_indexList_ );
		data = this->leaves_.read( data, LeafTraits::s_vertexList_, LeafTraits::s_indexList_ );
	}
	data = this->billboards_.oldRead( data );
#else
	data = this->branches_.oldRead( data );
	data = this->fronds_.oldRead( data, );
	data = this->leaves_.oldRead( data, );
	data = this->billboards_.oldRead( data );
#endif
	STOP_TIMER(loadRenderData, "");
	
	STOP_TIMER(loadTreeData, "");
	
	return true;
}

#ifdef OPT_BUFFERS
template< typename VertexType >
void TreePartRenderData<VertexType>::unload( 
	STVector< VertexType >& verts,
	STVector< uint32 >& indices )
{
	// remove the vertices from the list.
	if ( verts_ )
	{
		if ( verts_->count() )
		{
			verts.eraseSlot( verts_ );
		}
		verts_ = NULL;
	}

	// remove the indices.
	LodDataVector::iterator it=lod_.begin();
	while ( it!=lod_.end() )
	{
		if ((*it).index_)
		{
			indices.eraseSlot( (*it).index_ );
			(*it).index_ = NULL;
		}
		it++;
	}
}

void TSpeedTreeType::TreeData::unload()
{
	SimpleMutexHolder mutexHolder(TSpeedTreeType::s_vertexListLock_);
	branches_.unload( CommonTraits::s_vertexList_, CommonTraits::s_indexList_ );
	fronds_.unload( CommonTraits::s_vertexList_, CommonTraits::s_indexList_ );
	leaves_.unload( LeafTraits::s_vertexList_, LeafTraits::s_indexList_ );
}
#endif

// ---------------------------------------------------------- Static Definitions

TSpeedTreeType::InitVector TSpeedTreeType::s_syncInitList_;
SimpleMutex TSpeedTreeType::s_syncInitLock_;
SimpleMutex TSpeedTreeType::s_vertexListLock_;

TSpeedTreeType::TreeTypesMap TSpeedTreeType::s_typesMap_;

bool  TSpeedTreeType::s_frameStarted_ = false;
float TSpeedTreeType::s_time_         = 0.f;

TSpeedTreeType::EffectPtr   TSpeedTreeType::s_branchesEffect_;
TSpeedTreeType::EffectPtr   TSpeedTreeType::s_leavesEffect_;
TSpeedTreeType::EffectPtr   TSpeedTreeType::s_billboardsEffect_;

TSpeedTreeType::LightSetterPtr TSpeedTreeType::s_branchesLightingSetter_;
TSpeedTreeType::LightSetterPtr TSpeedTreeType::s_leavesLightingSetter_;
TSpeedTreeType::EffectPtr TSpeedTreeType::s_shadowsFX_;
TSpeedTreeType::EffectPtr TSpeedTreeType::s_depthOnlyBranchesFX_;
TSpeedTreeType::EffectPtr TSpeedTreeType::s_depthOnlyLeavesFX_;
TSpeedTreeType::EffectPtr TSpeedTreeType::s_depthOnlyBillboardsFX_;
TSpeedTreeType::RendererGroupVector TSpeedTreeType::s_renderGroups_;
TSpeedTreeType::DeferredLightingVector TSpeedTreeType::s_deferredLighting_;
int32 TSpeedTreeType::DrawData::s_defaultLightingBatch_ = -1;

ShadowCaster * TSpeedTreeType::s_curShadowCaster_ = NULL;

Matrix      TSpeedTreeType::s_invView_;
Matrix      TSpeedTreeType::s_view_;
Matrix      TSpeedTreeType::s_projection_;
Moo::Colour TSpeedTreeType::s_sunAmbient_;
Moo::Colour TSpeedTreeType::s_sunDiffuse_;
Vector3     TSpeedTreeType::s_sunDirection_;

bool  TSpeedTreeType::s_enviroMinderLight_      = false;
bool  TSpeedTreeType::s_drawBoundingBox_        = false;
bool  TSpeedTreeType::s_batchRendering_         = true;
bool  TSpeedTreeType::s_drawTrees_              = true;
bool  TSpeedTreeType::s_drawLeaves_             = true;
bool  TSpeedTreeType::s_drawBranches_           = true;
bool  TSpeedTreeType::s_drawFronds_             = true;
bool  TSpeedTreeType::s_drawBillboards_         = true;
bool  TSpeedTreeType::s_texturedTrees_          = true;
bool  TSpeedTreeType::s_playAnimation_          = true;
float TSpeedTreeType::s_leafRockFar_            = 400.f;
float TSpeedTreeType::s_maxLod_                = 1.f;
float TSpeedTreeType::s_lodMode_                = -2;
float TSpeedTreeType::s_lodNear_                = 50.f;
float TSpeedTreeType::s_lodFar_                 = 300.f;
float TSpeedTreeType::s_lod0Yardstick_          = 130.0f;
float TSpeedTreeType::s_lod0Variance_           = 1.0f;

#if ENABLE_BB_OPTIMISER
	bool  TSpeedTreeType::s_optimiseBillboards_ = true;
#endif

int TSpeedTreeType::s_speciesCount_  = 0;
int TSpeedTreeType::s_uniqueCount_   = 0;
int TSpeedTreeType::s_visibleCount_  = 0;
int TSpeedTreeType::s_lastPassCount_ = 0;
int TSpeedTreeType::s_deferredCount_ = 0;
int TSpeedTreeType::s_passNumCount_  = 0;
int TSpeedTreeType::s_totalCount_    = 0;

std::string TSpeedTreeType::s_speedWindFile_ = "speedtree/SpeedWind.ini";
std::string TSpeedTreeType::s_fxFiles_[NUM_EFFECTS];

DogWatch TSpeedTreeType::s_globalWatch_("SpeedTree");
DogWatch TSpeedTreeType::s_prepareWatch_("Peparation");
DogWatch TSpeedTreeType::s_drawWatch_("Draw");
DogWatch TSpeedTreeType::s_primitivesWatch_("Primitives");

} // namespace speedtree

#ifdef EDITOR_ENABLED
	// forces linkage of
	// thumbnail provider
	extern int SpeedTreeThumbProv_token;
	int total = SpeedTreeThumbProv_token;
#endif // EDITOR_ENABLED

#endif // SPEEDTREE_SUPPORT ----------------------------------------------------
