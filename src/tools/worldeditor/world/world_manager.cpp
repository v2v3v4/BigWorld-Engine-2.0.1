/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/*~ module WorldEditor
 *	@components{ worldeditor }
 *
 *	The WorldEditor Module is a Python module that provides an interface to
 *	the various information about the world items in WorldEditor.
 *	It also provides functionality to configure the WorldEditor GUI, replicate 
 *	menu item actions, capture user interaction and provides an interface
 *	to the 'bwlockd' (lock server).
 */

#include "pch.hpp"

#include "world_manager.hpp"

#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/editor_chunk_link_manager.hpp"
#include "worldeditor/world/vlo_manager.hpp"
#include "worldeditor/world/editor_chunk_overlapper.hpp"
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/world/items/editor_chunk_entity.hpp"
#include "worldeditor/world/items/editor_chunk_tree.hpp"
#include "worldeditor/world/items/editor_chunk_portal.hpp"
#include "worldeditor/world/items/editor_chunk_user_data_object.hpp"
#include "worldeditor/world/items/editor_chunk_vlo.hpp"
#include "worldeditor/world/items/editor_chunk_water.hpp"
#include "worldeditor/world/items/editor_chunk_flare.hpp"
#include "worldeditor/world/items/editor_chunk_meta_data.hpp"
#include "worldeditor/world/items/editor_chunk_marker_cluster.hpp"
#include "worldeditor/world/items/editor_chunk_particle_system.hpp"
#include "worldeditor/world/items/editor_chunk_sound.hpp"
#include "worldeditor/world/items/editor_chunk_station.hpp"
#include "worldeditor/world/items/editor_chunk_binding.hpp"
#include "worldeditor/world/items/editor_chunk_link.hpp"
#include "worldeditor/framework/world_editor_app.hpp"
#include "worldeditor/framework/world_editor_doc.hpp"
#include "worldeditor/gui/pages/chunk_watcher.hpp"
#include "worldeditor/gui/pages/page_chunk_texture.hpp"
#include "worldeditor/gui/pages/page_properties.hpp"
#include "worldeditor/gui/pages/page_terrain_import.hpp"
#include "worldeditor/gui/pages/page_terrain_texture.hpp"
#include "worldeditor/gui/pages/panel_manager.hpp"
#include "worldeditor/gui/dialogs/low_memory_dlg.hpp"
#include "worldeditor/gui/dialogs/wait_dialog.hpp"
#include "worldeditor/gui/dialogs/new_space_dlg.hpp"
#include "worldeditor/gui/dialogs/splash_dialog.hpp"
#include "worldeditor/gui/dialogs/undo_warn_dlg.hpp"
#include "worldeditor/framework/mainframe.hpp"
#include "worldeditor/collisions/collision_callbacks.hpp"
#include "worldeditor/misc/world_editor_camera.hpp"
#include "worldeditor/misc/cvswrapper.hpp"
#include "worldeditor/misc/sync_mode.hpp"
#include "worldeditor/misc/popup_menu_helper.hpp"
#include "worldeditor/misc/chunk_row_cache.hpp"
#include "worldeditor/misc/selection_filter.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "worldeditor/misc/light_container_debugger.hpp"
#include "worldeditor/height/height_map.hpp"
#include "worldeditor/height/height_module.hpp"
#include "worldeditor/editor/item_frustum_locator.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "worldeditor/editor/chunk_item_placer.hpp"
#include "worldeditor/project/chunk_photographer.hpp"
#include "worldeditor/project/project_module.hpp"
#include "worldeditor/project/space_helpers.hpp"
#include "worldeditor/project/space_map.hpp"

#include "appmgr/app.hpp"
#include "appmgr/application_input.hpp"
#include "appmgr/commentary.hpp"
#include "appmgr/options.hpp"

#include "chunk/chunk_item_amortise_delete.hpp"
#include "chunk/chunk_item_tree_node.hpp"
#include "chunk/chunk_loader.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_vlo.hpp"
#if UMBRA_ENABLE
#include "chunk/chunk_umbra.hpp"
#endif

#include "common/compile_time.hpp"
#include "common/dxenum.hpp"
#include "common/material_properties.hpp"
#include "common/mouse_look_camera.hpp"
#include "common/page_messages.hpp"
#include "common/resource_loader.hpp"
#include "common/utilities.hpp"

#include "duplo/chunk_embodiment.hpp"

#include "gizmo/gizmo_manager.hpp"
#include "gizmo/tool_manager.hpp"

#include "guimanager/gui_functor_option.hpp"
#include "guimanager/gui_input_handler.hpp"
#include "guimanager/gui_manager.hpp"

#include "moo/visual_channels.hpp"

#include "physics2/material_kinds.hpp"

#include "pyscript/py_data_section.hpp"
#include "pyscript/py_output_writer.hpp"
#include "pyscript/py_callback.hpp"
#include "pyscript/personality.hpp"

#include "resmgr/auto_config.hpp"
#include "resmgr/resource_cache.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/data_section_cache.hpp"
#include "resmgr/data_section_census.hpp"

#include "romp/console_manager.hpp"
#include "romp/debug_geometry.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/flora.hpp"
#include "romp/flora.hpp"
#include "romp/fog_controller.hpp"
#include "romp/py_chunk_light.hpp"
#include "romp/py_chunk_spot_light.hpp"
#include "romp/texture_renderer.hpp"
#include "romp/time_of_day.hpp"
#include "romp/water.hpp"
#include "romp/water_scene_renderer.hpp"

#include "speedtree/speedtree_renderer.hpp"

#include "terrain/manager.hpp"
#include "terrain/base_terrain_block.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "terrain/editor_chunk_terrain_projector.hpp"
#include "terrain/horizon_shadow_map.hpp"
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "terrain/terrain2/terrain_lod_controller.hpp"
#include "terrain/terrain_settings.hpp"
#include "terrain/terrain_texture_layer.hpp"

#include "math/sma.hpp"

#include "cstdmf/diary.hpp"
#include "cstdmf/message_box.hpp" 
#include "cstdmf/restart.hpp"
#include "cstdmf/bwversion.hpp"

#include <algorithm>
#include <fstream>
#include <time.h>
#include <sstream>
#include <psapi.h>
#include <hash_set>


#pragma comment( lib, "psapi.lib" )

static DogWatch s_AmortiseChunkItemDelete( "chnk_item_del" );
static DogWatch s_linkManager( "link_manager" );
static DogWatch s_linkerManager( "linker_manager" );
static DogWatch s_chunkTick( "chunk_tick" );
static DogWatch s_chunkDraw( "chunk_draw" );
static DogWatch s_umbraDraw( "umbra_draw" );
static DogWatch s_terrainDraw( "terrain_draw" );
static DogWatch s_rompDraw( "romp_draw" );
static DogWatch s_drawSorted( "draw_sorted" );
static DogWatch s_render( "render" );
static DogWatch s_update( "update" );
static DogWatch s_detailTick( "detail_tick" );
static DogWatch s_detailDraw( "detail_draw" );

//for ChunkManager
std::string g_specialConsoleString;

static AutoConfigString s_terrainSelectionFx( "selectionfx/terrain" );
static AutoConfigString s_terrainSelectionFxLegacy( "selectionfx/terrainLegacy" );
static AutoConfigString s_blankCDataFname( "dummy/cData" );
static AutoConfigString s_dxEnumPath( "system/dxenum" );

// Preload models

static AutoConfigString s_notFoundModel( "system/notFoundModel" );
static AutoConfigString s_entityModel( "editor/entityModel", "resources/models/entity.model" );
static AutoConfigString s_legacyEntityModel( "editor/entityModelLegacy", "helpers/props/standin.model" );

/**
 *	This class is a workaround to the fact that the VisualManager and the
 *	PrimitiveManager are not entirely thread safe. We simply preload the models
 *	that can cause problems (the ones that can potentially get loaded in both
 *	the main thread and the loading thread in the editor).
 */
class WEPreloader : public Singleton< WEPreloader >
{
public:
	WEPreloader()
	{
		notFoundModel_ = Model::get( s_notFoundModel.value() );
		entityModel_ = Model::get( s_entityModel.value() );
		legacyEntityModel_ = Model::get( s_legacyEntityModel.value() );
		udoModel_ = Model::get( "resources/models/user_data_object.model" );
		waterModel_ = Model::get( "resources/models/water.model" );
	}

private:
	ModelPtr notFoundModel_;
	ModelPtr entityModel_;
	ModelPtr legacyEntityModel_;
	ModelPtr udoModel_;
	ModelPtr waterModel_;
};
BW_SINGLETON_STORAGE( WEPreloader );


//--------------------------------
//CHUNKIN EXTERNS
// reference the ChunkInhabitants that we want to be able to load.
extern int ChunkModel_token;
extern int ChunkLight_token;
extern int ChunkTerrain_token;
extern int ChunkFlare_token;
extern int ChunkWater_token;
extern int ChunkAttachment_token;
extern int EditorChunkOverlapper_token;
extern int ChunkParticles_token;
extern int ChunkTree_token;
extern int PyResourceRefs_token;
extern int Servo_token;
extern int InvViewMatrixProvider_token;
static int s_chunkTokenSet = ChunkModel_token | ChunkLight_token |
	ChunkTerrain_token | ChunkFlare_token | ChunkWater_token |
	EditorChunkOverlapper_token | ChunkParticles_token | ChunkAttachment_token |
	ChunkTree_token | PyResourceRefs_token | Servo_token |
	InvViewMatrixProvider_token;

extern int ScriptedModule_token;
static int s_moduleTokenSet = ScriptedModule_token;

extern int genprop_gizmoviews_token;
static int giz = genprop_gizmoviews_token;

extern int Math_token;
extern int PyScript_token;
extern int GUI_token;
extern int ResMgr_token;
static int s_moduleTokens =
	Math_token |
	PyScript_token |
	GUI_token |
	ResMgr_token;

extern int PyShimmerCountProvider_token;
extern int PyGraphicsSetting_token;
static int pyTokenSet = PyShimmerCountProvider_token | PyGraphicsSetting_token;

namespace PostProcessing
{
	extern int tokenSet;
	static int ppTokenSet = tokenSet;
}


static Moo::EffectMaterialPtr s_selectionMaterial = NULL;
static bool s_selectionMaterialOk = false;
static Moo::EffectMaterialPtr s_selectionMaterialLegacy = NULL;
static bool s_selectionMaterialLegacyOk = false;


DECLARE_DEBUG_COMPONENT2( "WorldEditor", 0 )

// static members
SmartPointer<WorldManager> WorldManager::s_instance = NULL;
WorldEditorDebugMessageCallback WorldManager::s_debugMessageCallback_;
WorldEditorCriticalMessageCallback WorldManager::s_criticalMessageCallback_;
bool WorldManager::s_criticalMessageOccurred_ = false;


namespace 
{	
	static const std::string LIGHTING = "lighting";
	static const std::string SHADOW = "shadow";
	static const std::string THUMBNAIL = "thumbnail";
	static const std::string TERRAIN_LOD = "terrainLod";

	class InitFailureCleanup
	{
	public:
		InitFailureCleanup( bool & rInited ) :
			rInited_( rInited )
		{
		}

		~InitFailureCleanup()
		{
			if (!rInited_)
			{
				delete AmortiseChunkItemDelete::pInstance();
				delete ChunkPhotographer::pInstance();
				delete DXEnum::pInstance();
				EditorUserDataObjectType::shutdown();
				EditorEntityType::shutdown();
				MaterialKinds::fini();
				Terrain::Manager::fini();
				HeightMap::fini();
				ChunkManager::instance().fini();
				BgTaskManager::instance().stopAll();
			}
		}
	private:
		bool & rInited_;
	};


	//These are used for Script::tick and the BigWorld.callback fn.
	static double g_totalTime = 0.0;

	void incrementTotalTime( float dTime )
	{
		g_totalTime += (double)dTime;
	}

	double getTotalTime()
	{		
		return g_totalTime;
	}


	typedef stdext::hash_set< Chunk * > VisitedChunksSet;

	/**
	 *	This function finds out if all neightbouring chunks are loaded by
	 *	recursing down to "level" levels of connected chunks.
	 *	IMPORTANT:
	 *	- This function is not meant to be called directly, only from
	 *	  WorldManager::EnsureNeighbourChunkLoaded.
	 *	- This function is potentially slow when using bigger values for
	 *	  "level" (5 or bigger).
	 *	
	 *	@param chunk	Current chunk being analysed.
	 *	@param level	Number of recursions, greater or equal to 0.
	 *	@param visited	Set of visited chunks, must be empty on start.
	 *	@return		true if all neighbours loaded, false otherwise.
	 */
	bool ensureNeighbourChunkLoadedInternal( Chunk* chunk, int level, VisitedChunksSet visited )
	{
		BW_GUARD;

		if( !chunk->isBound() )
			return false;

		if( level == 0 )
			return true;

		VisitedChunksSet::iterator it = visited.find( chunk );
		if (it != visited.end())
			return true; // Already visited, return true so it keeps recursing.

		visited.insert( chunk );

		for( ChunkBoundaries::iterator bit = chunk->joints().begin();
			bit != chunk->joints().end(); ++bit )
		{
			for( ChunkBoundary::Portals::iterator ppit = (*bit)->unboundPortals_.begin();
				ppit != (*bit)->unboundPortals_.end(); ++ppit )
			{
				ChunkBoundary::Portal* pit = *ppit;
				if( !pit->hasChunk() )
					continue;
				return false;
			}
		}

		for( ChunkBoundaries::iterator bit = chunk->joints().begin();
			bit != chunk->joints().end(); ++bit )
		{
			for( ChunkBoundary::Portals::iterator ppit = (*bit)->boundPortals_.begin();
				ppit != (*bit)->boundPortals_.end(); ++ppit )
			{
				ChunkBoundary::Portal* pit = *ppit;
				if( !pit->hasChunk() ) continue;

				if( !ensureNeighbourChunkLoadedInternal( pit->pChunk, level - 1, visited ) )
					return false;
			}
		}
		return true;
	}


	/**
	 *	This function loads all neightbouring chunks by recursing down to
	 *	"level" levels of connected chunks.
	 *	IMPORTANT:
	 *	- This function is not meant to be called directly, only from
	 *	  WorldManager::loadNeighbourChunk.
	 *	- This function is potentially slow when using bigger values for
	 *	  "level" (5 or bigger).
	 *	
	 *	@param chunk	Current chunk being loaded.
	 *	@param level	Number of recursions, greater or equal to 0.
	 *	@param chunkDepths	Map of visited chunks, must be empty on start.
	 */
	void loadNeighbourChunkInternal( Chunk* chunk, int level,
									std::map<Chunk*,int> & chunkDepths )
	{
		BW_GUARD;

		if( level == 0 )
			return;

		std::map<Chunk*,int>::iterator itDepth = chunkDepths.find( chunk );

		if (itDepth != chunkDepths.end())
		{
			if (level > (*itDepth).second)
			{
				// We have already traversed this chunk deeper than now, so
				// skip.
				return;
			}
			(*itDepth).second = level;
		}
		else
		{
			chunkDepths.insert( std::make_pair( chunk, level ) );
		}

		// Load chunks
		std::set<Chunk *> chunksToLoad;

		for( ChunkBoundaries::iterator bit = chunk->joints().begin();
			bit != chunk->joints().end(); ++bit )
		{
			for( ChunkBoundary::Portals::iterator ppit = (*bit)->unboundPortals_.begin();
				ppit != (*bit)->unboundPortals_.end(); ++ppit )
			{
				ChunkBoundary::Portal* pit = *ppit;
				if( pit->hasChunk() )
				{
					chunksToLoad.insert( pit->pChunk );
				}
				else
				{
					std::string chunkName = chunk->mapping()->outsideChunkIdentifier( pit->centre );

					if (!chunkName.empty())
					{
						Chunk* pOutsideChunk = ChunkManager::instance().findChunkByName( chunkName, chunk->mapping() );

						if (pOutsideChunk && !pOutsideChunk->loaded())
						{
							chunksToLoad.insert( pOutsideChunk );
						}
					}
				}
			}
		}

		for (std::set<Chunk *>::iterator it = chunksToLoad.begin();
			it != chunksToLoad.end(); ++it)
		{
			ChunkManager::instance().loadChunkNow( (*it)->identifier(),
								WorldManager::instance().geometryMapping() );
			ChunkManager::instance().checkLoadingChunks();
		}

		// Recurse chunks
		chunksToLoad.clear();

		for( ChunkBoundaries::iterator bit = chunk->joints().begin();
			bit != chunk->joints().end(); ++bit )
		{
			for( ChunkBoundary::Portals::iterator ppit = (*bit)->boundPortals_.begin();
				ppit != (*bit)->boundPortals_.end(); ++ppit )
			{
				ChunkBoundary::Portal* pit = *ppit;
				if( !pit->hasChunk() ) continue;

				chunksToLoad.insert( pit->pChunk );
			}
		}

		for (std::set<Chunk *>::iterator it = chunksToLoad.begin();
			it != chunksToLoad.end(); ++it)
		{
			loadNeighbourChunkInternal( (*it), level - 1, chunkDepths );
		}

	}


	/**
	 *	This helper function returns the list of all chunk items currently
	 *	loaded.
	 */
	void allChunkItems( ChunkItemRevealer::ChunkItems & items )
	{
		BW_GUARD;

		for (ChunkMap::iterator i = ChunkManager::instance().cameraSpace()->chunks().begin();
			i != ChunkManager::instance().cameraSpace()->chunks().end(); i++)
		{
			std::vector<Chunk*> & chunks = i->second;
			for (std::vector<Chunk*>::iterator j = chunks.begin();
				j != chunks.end(); ++j)
			{
				Chunk* pChunk = *j;

				if (!pChunk->isBound() || !EditorChunkCache::instance( *pChunk ).edIsWriteable() )
					continue;

				MatrixMutexHolder lock( pChunk );
				std::vector<ChunkItemPtr> chunkItems =				
					EditorChunkCache::instance( *pChunk ).staticItems();

				for (std::vector<ChunkItemPtr>::const_iterator k = chunkItems.begin();
					k != chunkItems.end(); ++k)
				{
					items.push_back( *k );
				}
			}
		}
	}


	/**
	 *	This helper function filters a list of chunk items and returns only
	 *	the ones that are selectable.
	 */
	void filterSelectableItems( ChunkItemRevealer::ChunkItems & unfilteredItems, bool ignoreVis, bool ignoreFrozen,
							   ChunkItemRevealer::ChunkItems & retItems )
	{
		BW_GUARD;

		std::set< Chunk * > selectableShells;

		for (ChunkItemRevealer::ChunkItems::iterator it = unfilteredItems.begin();
			it != unfilteredItems.end(); ++it)
		{
			ChunkItemPtr item = *it;

			if (item->isShellModel())
			{
				if (SelectionFilter::canSelect( item.get(), true, false, ignoreVis, ignoreFrozen ))
				{
					selectableShells.insert( item->chunk() );
				}
			}
		}

		for (ChunkItemRevealer::ChunkItems::iterator it = unfilteredItems.begin();
			it != unfilteredItems.end(); ++it)
		{
			ChunkItemPtr item = *it;

			if (!SelectionFilter::canSelect( item.get(), true, false, ignoreVis, ignoreFrozen ))
			{
				continue;
			}

			DataSectionPtr ds = item->pOwnSect();

			if (ds && ds->sectionName() == "overlapper")
			{
				continue;
			}

			if (ds && ds->sectionName() == "vlo" &&
				!item->edCheckMark(VeryLargeObject::selectionMark()))
			{
				continue;
			}

			if (!item->isShellModel() && !item->chunk()->isOutsideChunk() &&
				selectableShells.find( item->chunk() ) != selectableShells.end())
			{
				// If the object is inside a shell, and the shell is already selected,
				// then we shouldn't select the shell's contents (this item)
				continue;
			}

			retItems.push_back( item );
		}
	}


	/**
	 *  This class helps when changing the Selection Filters temporarily,
	 *	restoring the previous Selection Filters on destruction.
	 */
	class ScopedSelectionFilter
	{
	public:
		/**
		 *	Constructor.  Default, does nothing.
		 */
		ScopedSelectionFilter() :
			started_( false )
		{
		}


		/**
		 *	Constructor.  Sets the selection filters straight away.
		 */
		ScopedSelectionFilter( SelectionFilter::SelectMode selectMode,
								const std::string & typeFilters,
								const std::string & noSelTypeFilters ) :
			started_( false )
		{
			start( selectMode, typeFilters, noSelTypeFilters );
		}


		/**
		 *	This method starts changing the selection filters on an instance
		 *	created with the default constructor.
		 */
		void start( SelectionFilter::SelectMode selectMode,
								const std::string & typeFilters,
								const std::string & noSelTypeFilters )
		{
			if (!started_)
			{
				oldSelectMode_ = SelectionFilter::getSelectMode();
				oldTypeFilters_ = SelectionFilter::typeFilters();
				oldNoSelTypeFilters_ = SelectionFilter::noSelectTypeFilters();
				SelectionFilter::setSelectMode( selectMode );
				SelectionFilter::typeFilters( typeFilters );
				SelectionFilter::noSelectTypeFilters( noSelTypeFilters );
				started_ = true;
			}
			else
			{
				ERROR_MSG( "Selection filters already started.\n" );
			}
		}


		/**
		 *	Destructor.  Restore the filters, if set.
		 */
		~ScopedSelectionFilter()
		{
			if (started_)
			{
				SelectionFilter::setSelectMode( oldSelectMode_ );
				SelectionFilter::typeFilters( oldTypeFilters_ );
				SelectionFilter::noSelectTypeFilters( oldNoSelTypeFilters_ );
			}
		}

	private:
		bool started_;
		SelectionFilter::SelectMode oldSelectMode_;
		std::string oldTypeFilters_;
		std::string oldNoSelTypeFilters_;
	};
} // anonymous namespace


WorldManager::WorldManager(): 
	inited_( false )
	, workingChunk_( NULL )
	, canUnloadChunk_( false )
	, updating_( false )
	, chunkManagerInited_( false )
    , dTime_( 0.1f )
	, romp_( NULL )
    , globalWeather_( false )
    , totalTime_( 0.f )
    , hwndApp_( NULL )
    , hwndGraphics_( NULL )
    , changedEnvironment_( false )
    , secsPerHour_( 0.f )
	, changedPostProcessing_( false )
	, userEditingPostProcessing_( false )
	, currentMonitoredChunk_( 0 )
	, currentPrimGroupCount_( 0 )
	, lodRegenCount_( 0 )
	, recordLoadedChunks_( false )
	, angleSnaps_( 0.f )
	, movementSnaps_( 0.f, 0.f, 0.f )
	, canSeeTerrain_( false )
	, mapping_( NULL )
	, spaceManager_( NULL )
	, drawSelection_( false )
	, terrainInfoClean_( false )
	, spaceLock_( INVALID_HANDLE_VALUE )
	, renderDisabled_( false )
	, GUI::ActionMaker<WorldManager>(
		"changeSpace|newSpace|recentSpace|clearUndoRedoHistory|doExternalEditor|"
		"doReloadAllTextures|doReloadAllChunks|doExit|setLanguage|recalcCurrentChunk",
		&WorldManager::handleGUIAction )
	, GUI::UpdaterMaker<WorldManager>( "updateUndo|updateRedo|updateExternalEditor|updateLanguage", &WorldManager::handleGUIUpdate )
	, killingUpdatingFiber_( false )
	, updatingFiber_( 0 )
	, lastModifyTime_( 0 )
	, cursor_( NULL )
    , waitCursor_( true )
	, warningOnLowMemory_( true )
	, chunkWatcher_ (new ChunkWatcher() )
	, timeLastUpdateTexLod_( 0.0f )
	, progressBar_( NULL )
	, inEscapableProcess_( false )
	, settingSelection_( false )
	, isSaving_( false )
	, slowTaskCount_( 0 )
	, savedCursor_( NULL )
{
	BW_GUARD;

	SlowTaskHandler::handler( this );
	MaterialProperties::runtimeInitMaterialProperties();
	setPlayerPreviewMode( false );
	resetCursor();
}


WorldManager::~WorldManager()
{
	BW_GUARD;

	if (SlowTaskHandler::handler() == this)
	{
		SlowTaskHandler::handler( NULL );
	}

	if (inited_) this->fini();
	if( spaceLock_ != INVALID_HANDLE_VALUE )
		CloseHandle( spaceLock_ );

	delete spaceManager_;
}


void WorldManager::fini()
{
	BW_GUARD;

	if( inited_ )
	{
		#if UMBRA_ENABLE
		ChunkUmbra::terrainOverride( NULL );
		#endif

		Script::clearTimers();
		BgTaskManager::instance().stopAll();
		GlobalModels::fini();
		stopBackgroundCalculation();

		// Clear objects held on to by the selection and the undo/redo 
		// barriers.
		std::vector<ChunkItemPtr> emptySelection;
		setSelection(emptySelection);
		UndoRedo::instance().clear();

		LightContainerDebugger::fini();
		ChunkItemFrustumLocator::fini();
		CoordModeProvider::fini();
		SnapProvider::fini();
		ObstacleLockCollisionCallback::s_default.clear();

		SpaceMap::deleteInstance();	

		ResourceCache::instance().fini();
		worldEditorCamera_ = NULL;

		if ( romp_ )
			romp_->enviroMinder().deactivate();

		DEBUG_MSG( "Calling WorldEditor Destructor\n" );

		// Fini HeightMap
		HeightMap::fini();

		HeightModule::fini();

		EditorChunkLinkManager::instance().setValid(false);
		mapping_ = NULL;
		ChunkManager::instance().fini();
		progressBar_->fini();
		delete progressBar_; 
		progressBar_ = NULL;

		EditorChunkTerrainProjector::instance().fini();
		MaterialKinds::fini();
		Terrain::Manager::fini();
		ResourceLoader::fini();

		if ( romp_ )
		{
			PyObject * pMod = PyImport_AddModule( "WorldEditor" );
			PyObject_DelAttrString( pMod, "romp" );

			Py_DECREF( romp_ );
			romp_ = NULL;
		}

		while( ToolManager::instance().tool() )
		{
			WARNING_MSG( "WorldManager::fini : There is still a tool on the stack that should have been cleaned up\n" );
			ToolManager::instance().popTool();
		}

		EditorUserDataObjectType::shutdown();
		EditorEntityType::shutdown();

		ChunkItemTreeNode::nodeCache().fini();
		/*EditorChunkSound::fini();*/ // This should be uncommented when sound is added in

		if (s_selectionMaterial)
		{
			s_selectionMaterial = NULL;
		}

		if (s_selectionMaterialLegacy)
		{
			s_selectionMaterialLegacy = NULL;
		}

		delete AmortiseChunkItemDelete::pInstance();
		delete ChunkPhotographer::pInstance();
		delete DXEnum::pInstance();

		GUI::Win32InputDevice::fini();
		PropManager::fini();
		BWResource::instance().purgeAll();

		Diary::fini();
		DebugMsgHelper::fini();

		MetaDataType::fini();

		delete SceneBrowser::pInstance();

		delete WEPreloader::pInstance();

		inited_ = false;
		s_instance = NULL;
	}
}


WorldManager& WorldManager::instance()
{
	if ( !s_instance )
	{
		s_instance = new WorldManager;
	}
	else
	{
		s_instance->registerDelayedChanges();
	}
	return *s_instance;
}

#include "moo/effect_material.hpp"
#include "common/material_editor.hpp"

namespace
{

struct isChunkFileExists
{
	bool operator()( const std::string& filename, GeometryMapping* dirMapping )
	{
		BW_GUARD;

		return BWResource::fileExists( dirMapping->path() + filename + ".chunk" );
	}
}
isChunkFileExists;

};

void WorldManager::update( float dTime )
{
	BW_GUARD;

	if (!inited_ || updating_ || criticalMessageOccurred())
	{
		return;
	}

	OptionsHelper::tick();

	updating_ = true;

	s_update.start();

	dTime_ = dTime;
    totalTime_ += dTime;

	g_specialConsoleString = "";

	postPendingErrorMessages();

	// set input focus as appropriate
	bool acceptInput = cursorOverGraphicsWnd();
	WorldEditorApp::instance().mfApp()->handleSetFocus( acceptInput );

	//GIZMOS
	if ( InputDevices::isShiftDown() ||
		InputDevices::isCtrlDown() ||
		InputDevices::isAltDown() )
	{
		// if pressing modifier keys, remove the forced gizmo set to enable
		// normal gizmo behaviour with the modifier keys.
		GizmoManager::instance().forceGizmoSet( NULL );
	}

	//TOOLS
	// calculate the current world ray from the mouse position
	// (don't do this if moving the camera around (for more response)
	bool castRay = !InputDevices::isKeyDown(KeyCode::KEY_RIGHTMOUSE);
	if ( acceptInput && castRay )
	{
		worldRay_ = getWorldRay( currentCursorPosition() );

		ToolPtr spTool = ToolManager::instance().tool();
		if ( spTool )
		{
			spTool->calculatePosition( worldRay_ );
			spTool->update( dTime );
		}
	}

	// Tick editor objects that want to be ticked.
	this->tickEditorTickables();

	// Chunks:
	if ( chunkManagerInited_ )
    {
        // Linker manager tick method
		s_linkerManager.start();
		WorldManager::instance().linkerManager().tick(); 
		s_linkerManager.stop();

		// Link manager tick method
		s_linkManager.start();
		EditorChunkLinkManager::instance().update( dTime_ ); 
		s_linkManager.stop();

		s_chunkTick.start();
		markChunks();
		ChunkManager::instance().tick( dTime_ );
		s_chunkTick.stop();

		// Amortise chunk item delete tick method
		s_AmortiseChunkItemDelete.start();
		AmortiseChunkItemDelete::instance().tick();
		s_AmortiseChunkItemDelete.stop();
	}

	// Background tasks:
	BgTaskManager::instance().tick();
	ProviderStore::tick( dTime );

	// Scripts
	incrementTotalTime( dTime );	
	Script::tick( getTotalTime() );
	PyChunkLight::revAll();
	PyChunkSpotLight::revAll();

	// Entity models:
	EditorChunkEntity::calculateDirtyModels();

	// UserDataObject models:
	EditorChunkUserDataObject::calculateDirtyModels();

    if ( romp_ )
    	romp_->update( dTime_, globalWeather_ );

    // update the flora redraw state
	bool drawFlora = !!Options::getOptionInt( "render/environment/drawDetailObjects", 1 );
	drawFlora &= !!Options::getOptionInt( "render/environment", 0 );
	drawFlora &= !Options::getOptionInt( "render/hideOutsideObjects", 0 );
	Flora::enabled(drawFlora);

	static bool firstTime = true;
	static bool canUndo, canRedo, canEE;
	static bool playerPreviewMode;
	static std::string cameraMode;
	static int terrainWireFrame;
	if( firstTime || canUndo != UndoRedo::instance().canUndo() ||
		canRedo != UndoRedo::instance().canRedo() ||
		canEE != !!updateExternalEditor( NULL ) ||
		playerPreviewMode != isInPlayerPreviewMode() ||
		cameraMode != Options::getOptionString( "camera/speed" ) ||
		terrainWireFrame != Options::getOptionInt( "render/terrain/wireFrame", 0 ) )
	{
		firstTime = false;
		canUndo = UndoRedo::instance().canUndo();
		canRedo = UndoRedo::instance().canRedo();
		canEE = !!updateExternalEditor( NULL );
		cameraMode = Options::getOptionString( "camera/speed" );
		playerPreviewMode = isInPlayerPreviewMode();
		terrainWireFrame = Options::getOptionInt( "render/terrain/wireFrame", 0 );
		GUI::Manager::instance().update();
	}

#if FMOD_SUPPORT
	//Tick FMod by setting the camera position
	Matrix view = WorldEditorCamera::instance().currentCamera().view();
	view.invert();
	Vector3 cameraPosition = view.applyToOrigin();
	Vector3 cameraDirection = view.applyToUnitAxisVector( 2 );
	Vector3 cameraUp = view.applyToUnitAxisVector( 1 );
	SoundManager::instance().setListenerPosition(
		cameraPosition, cameraDirection, cameraUp, dTime_ );

    s_update.stop();
#endif // FMOD_SUPPORT

	// Update missing LOD textures at the specified rate.
	timeLastUpdateTexLod_ += dTime_;
	if (timeLastUpdateTexLod_ > Options::getOptionFloat("terrain/texture/lodregentime", 1.0f))
	{
		timeLastUpdateTexLod_ = 0.0f;
		drawMissingTextureLODs(false, false, true); // only update one texture LOD at a time
	}

	ToolPtr spTool = ToolManager::instance().tool();
	if (!spTool || !spTool->applying())
	{
		// Only check for memory load when not applying a tool. Interrupting a
		// tool might result in crashes, etc.
		checkMemoryLoad();
	}


	GlobalModels::tick( dTime );

	updating_ = false;
}


bool WorldManager::isMemoryLow( bool testNow ) const
{
	BW_GUARD;

	static uint64 s_lastMemoryAllocTest = 0;
	static const uint64 s_memoryAllocTestInterval = 1 * stampsPerSecond();
	static bool s_memoryIsFragmented = false;

	if ((int)getMemoryLoad() > Options::getOptionInt( "warningMemoryLoadLevel", 90 ))
	{
		// We are passed the memory usage threshold, return 'true' now.
		return true;
	}

	// Try to alloc some memory, and raise a flag if it fails. We need to test
	// allocation like this to detect memory fragmentation: if we can't alloc a
	// fairly big block, memory is probably fragmented, so we fail.
	uint64 curTime = timestamp();
	if (testNow || curTime > (s_lastMemoryAllocTest + s_memoryAllocTestInterval))
	{
		s_lastMemoryAllocTest = curTime;
		s_memoryIsFragmented = false;
		char * memoryTest = NULL;

		// To detect fragmentation, we test for different size allocations
		// depending on the texture quality. That way, WE can still keep
		// going when using medium or low quality textures, evem if memory
		// is fragmented enough not to fit a high quality textures.

		// If using high texture quality, try to alloc 22M, which is the
		// approximate size of the biggest single alloc that we normally do
		// (a 2k x 2k texture + mips).
		uint32 testSize = 22;

		Moo::GraphicsSetting::GraphicsSettingPtr pTextureQuality =
					Moo::GraphicsSetting::getFromLabel( "TEXTURE_QUALITY" );
		if (pTextureQuality && pTextureQuality->activeOption() != 0)
		{
			// Since we're not using high texture quality, the size of a
			// common big memory block is much smaller, for example, a
			// 1K x 1K medium quality texture with mips is about 5.6MB.
			testSize = 8;
		}

		try
		{
			memoryTest = new char[ testSize * 1024 * 1024 ]; 
		}
		catch(...)
		{
		}

		delete [] memoryTest;
		s_memoryIsFragmented = (memoryTest == NULL);
		if (s_memoryIsFragmented)
		{
			WARNING_MSG( "Memory is fragmented, failed to alloc %d MB (memory load is %d %%).\n",
						testSize, (int)getMemoryLoad() );
		}
	}

	return s_memoryIsFragmented;
}


bool WorldManager::isMemoryLoadWarningNeeded() const
{
	BW_GUARD;

	return warningOnLowMemory_ &&
			(isMemoryLow() ||
			Moo::rc().memoryCritical() ||
			BinaryBlock::memoryCritical());
}


void WorldManager::checkMemoryLoad()
{
	BW_GUARD;

	static uint64 s_lastCanLoadChunksWarning = 0;
	static const uint64 s_canLoadChunksWarningInterval = 5 * stampsPerSecond();

	// Don't allow more chunk loading if memory is low and/or fragmented.
	bool canLoadChunks = !isMemoryLow();
	if (!canLoadChunks)
	{
		Moo::GraphicsSetting::GraphicsSettingPtr pTextureQuality = Moo::GraphicsSetting::getFromLabel( "TEXTURE_QUALITY" );
		if (pTextureQuality && pTextureQuality->activeOption() != 2)
		{
			ShowCursor( TRUE );
			CWaitCursor wait;
			ERROR_MSG( "Memory is Low, setting texture quality to 'Low' to save memory.\n" );
			WorldEditorApp::instance().mainWnd()->RedrawWindow();
			pTextureQuality->selectOption( 2, true );
		}
		if (s_lastCanLoadChunksWarning == 0 || timestamp() > (s_lastCanLoadChunksWarning + s_canLoadChunksWarningInterval))
		{
			s_lastCanLoadChunksWarning = timestamp();
			ERROR_MSG( "Memory is Low, please reduce the far plane, save and/or clear the undo/redo history.\n" );
		}
	}
	else
	{
		s_lastCanLoadChunksWarning = 0;
	}
	ChunkManager::instance().canLoadChunks( canLoadChunks );

	// Warn the user
	if (warningOnLowMemory_)
	{
		// Variable to avoid re-entry
		static bool s_showingDialog = false;

		if (!s_showingDialog && isMemoryLoadWarningNeeded())
		{
			s_showingDialog = true;

			if( LowMemoryDlg().DoModal() == IDC_SAVE )
			{
				CWaitCursor wait;
				UndoRedo::instance().clear();
				quickSave();
				AmortiseChunkItemDelete::instance().purge();
				unloadChunks();

				Moo::rc().memoryCritical( false );
				BinaryBlock::memoryCritical( false );
			}
			else
			{
				warningOnLowMemory_ = false;
			}

			s_showingDialog = false;
		}
	}
}


/**
 *	This method writes out some status panel sections that are done every frame.
 *	i.e. FPS and cursor location.
 */
void WorldManager::writeStatus()
{
	BW_GUARD;

	//Panel 0 - memory load
	this->setStatusMessage( 0, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/MEMORY_LOAD", getMemoryLoad() ) );

	//Panel 1 - num polys
	this->setStatusMessage( 1, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/TRIS", Moo::rc().lastFrameProfilingData().nPrimitives_ ) );

	//Panel 2 - snaps
	if ( this->snapsEnabled() )
	{
		Vector3 snaps = this->movementSnaps();
		if( this->terrainSnapsEnabled() )
		{
			this->setStatusMessage( 2, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SNAP",
				snaps.x, "T", snaps.z ) );
		}
		else
		{
			this->setStatusMessage( 2, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SNAP",
				snaps.x, snaps.y, snaps.z ) );
		}
	}
	else
	{
		if ( this->terrainSnapsEnabled() )
		{
			this->setStatusMessage( 2, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SNAP_TERRAIN") );
		}
		else
		{
			this->setStatusMessage( 2, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SNAP_FREE") );
		}
	}

	//Panel 3 - locator position
	if ( ToolManager::instance().tool() && ToolManager::instance().tool()->locator() )
	{
		Vector3 pos = ToolManager::instance().tool()->locator()->transform().applyToOrigin();
		Chunk* chunk = ChunkManager::instance().cameraSpace()->findChunkFromPoint( pos );

		if (chunk && EditorChunkCache::instance( *chunk ).pChunkSection())
		{
			std::vector<DataSectionPtr>	modelSects;
			EditorChunkCache::instance( *chunk ).pChunkSection()->openSections( "model", modelSects );

			this->setStatusMessage( 3, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHUNK_LOCATOR_POSITION",
				Formatter( pos.x, L"%0.2f" ),
				Formatter( pos.y, L"%0.2f" ),
				Formatter( pos.z, L"%0.2f" ),
				chunk->identifier(),
				(int) modelSects.size(),
				currentPrimGroupCount_ ).c_str() );
		}
		else
		{
			this->setStatusMessage( 3, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHUNK_LOCATOR_POSITION",
				Formatter( pos.x, L"%0.2f" ),
				Formatter( pos.y, L"%0.2f" ),
				Formatter( pos.z, L"%0.2f" ) ).c_str() );
		}
	}
	else
	{
		this->setStatusMessage( 3, "" );
	}



	//Panel5 - fps

	//7 period simple moving average of the frames per second
	static SMA<float>	s_averageFPS(7);
	static float		s_countDown = 1.f;

	float fps = dTime_ == 0.f ? 0.f : 1.f / dTime_;

	float fps2 = WorldEditorApp::instance().mfApp()->fps();

	fps = std::min( fps, fps2 );

	s_averageFPS.append( fps );

	if ( s_countDown < 0.f )
	{
		this->setStatusMessage( 4, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/FPS",
			Formatter( s_averageFPS.average(), L"%0.1f" ) ).c_str() );
		s_countDown = 1.f;
	}
	else
	{
		s_countDown -= dTime_;
	}

	// Panel 6 - number of chunks loaded
	EditorChunkCache::lock();

	unsigned int dirtyTotal = dirtyChunks();
	unsigned int numLodTex  = dirtyLODTextures();
	if ( dirtyTotal != 0 || numLodTex != 0 )
	{
		this->setStatusMessage( 5, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHUNK_LOADED_WITH_DIRTY",
			EditorChunkCache::chunks_.size(), dirtyTotal, numLodTex ) );
	}
	else
	{
		this->setStatusMessage( 5, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHUNK_LOADED",
			EditorChunkCache::chunks_.size() ) );
	}
	EditorChunkCache::unlock();
}


/**
 *	This method renders the scene in a standard way.
 *	Call this method, or call each other method individually,
 *	interspersed with your own custom routines.
 */
void WorldManager::render( float dTime )
{
	BW_GUARD;

	if (renderDisabled_ || !inited_  || criticalMessageOccurred())
	{
	    return;
	}

	if( farPlane() != Options::getOptionInt( "graphics/farclip", (int)farPlane() ) )
		farPlane( (float)Options::getOptionInt( "graphics/farclip" ) );

	EditorChunkItem::hideAllOutside( !!Options::getOptionInt( "render/hideOutsideObjects", 0 ) );

	// Setup the data for counting the amount of primitive groups in the chunk
	// the locator is in, used for the status bar
	currentMonitoredChunk_ = 0;
	currentPrimGroupCount_ = 0;
	if ( ToolManager::instance().tool() && ToolManager::instance().tool()->locator() )
	{
		Vector3 pos = ToolManager::instance().tool()->locator()->transform().applyToOrigin();
		currentMonitoredChunk_ = ChunkManager::instance().cameraSpace()->findChunkFromPoint( pos );
	}

	// update any dynamic textures
	TextureRenderer::updateDynamics( dTime_ );
	//or just the water??

	//TODO: under water effect..
	Waters::instance().checkVolumes();

	// This is used to limit the number of rebuildCombinedLayer calls per frame
	// because they are very expensive.
	Terrain::EditorTerrainBlock2::nextBlendBuildMark();

	//	Make sure lodding occurs		
	Terrain::BasicTerrainLodController::instance().setCameraPosition( 
		Moo::rc().invView().applyToOrigin() );

	this->beginRender();
	this->renderRompPreScene();

	if ( chunkManagerInited_ )
	{
		Moo::EffectVisualContext::instance().initConstants();

		this->renderChunks();

		Moo::LightContainerPtr lc = new Moo::LightContainer;

		lc->addDirectional(
			ChunkManager::instance().cameraSpace()->sunLight() );
		lc->ambientColour(
			ChunkManager::instance().cameraSpace()->ambientLight() );

		Moo::rc().lightContainer( lc );

		EditorChunkLink::flush( dTime );
	}

	#if UMBRA_ENABLE
	if (!UmbraHelper::instance().umbraEnabled())
	{
		// With umbra, terrain gets rendered inside "renderChunks" via the
		// ChunkUmbra::terrainOverride.
	#endif
		this->renderTerrain();
	#if UMBRA_ENABLE
	}
	#endif
	this->renderRompDelayedScene();
	this->renderRompPostScene();
	Moo::rc().setRenderState( D3DRS_CLIPPING, TRUE  );
	this->renderEditorGizmos();
	this->renderEditorRenderables();
	this->renderDebugGizmos();
	GeometryDebugMarker::instance().draw();
	GizmoManager::instance().draw();

	if (Options::getOptionBool( "drawSpecialConsole", false ) &&
		!g_specialConsoleString.empty())
	{
		static XConsole * pSpecCon = new XConsole();
		pSpecCon->clear();
		pSpecCon->setCursor( 0, 0 );
		pSpecCon->print( g_specialConsoleString );
		pSpecCon->draw( 0.1f );
	}

	Chunks_drawCullingHUD();
	this->endRender();

	// write status sections.
	// we write them here, because it is only here
	// that we can retrieve the poly count.
	writeStatus();

    // if no chunks are loaded then show the arrow + 
    showBusyCursor();
}


/**
 *	Note : this method assumes Moo::rc().view() has been set accordingly.
 *		   it is up to the caller to set up this matrix.
 */
void WorldManager::beginRender()
{
	BW_GUARD;

    s_render.start();

	bool useShadows = Moo::rc().stencilAvailable();

	if ( useShadows )
	{
		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER |
			D3DCLEAR_STENCIL, 0x000020, 1, 0 );
	}
	else
	{
		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0x000020, 1, 0 );
	}

	Moo::rc().setWriteMask( 1, D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_ALPHA );

	DX::Viewport viewport;
	viewport.Width = (DWORD)Moo::rc().screenWidth();
	viewport.Height = (DWORD)Moo::rc().screenHeight();
	viewport.MinZ = 0.f;
	viewport.MaxZ = 1.f;
	viewport.X = 0;
	viewport.Y = 0;
	Moo::rc().setViewport( &viewport );

	Moo::rc().reset();
	Moo::rc().updateViewTransforms();
	Moo::rc().updateProjectionMatrix();
}


void WorldManager::renderRompPreScene()
{
	BW_GUARD;

    // draw romp pre scene
    s_rompDraw.start();
    romp_->drawPreSceneStuff();
    s_rompDraw.stop();

	FogController::instance().commitFogToDevice();
}

void WorldManager::renderChunks()
{
	BW_GUARD;

	// draw chunks
	if (chunkManagerInited_)
    {
    	s_chunkDraw.start();
		uint32 scenaryWireFrameStatus_ = Options::getOptionInt( "render/scenery/wireFrame", 0 );		 
		uint32 terrainWireFrameStatus_ = Options::getOptionInt( "render/terrain/wireFrame", 0 );
		ChunkManager::instance().camera( Moo::rc().invView(), ChunkManager::instance().cameraSpace() );

		Chunk::hideIndoorChunks_ = !OptionsScenery::shellsVisible();

		bool forceDrawShells = false;
		Chunk * cc = ChunkManager::instance().cameraChunk();

	#if UMBRA_ENABLE
		if (UmbraHelper::instance().umbraEnabled())
		{
			// Umbra can't handle gameVisibility off or selected shells, so to
			// ensure shells under these circumnstances are drawn, we set the
			// forceDrawShells to true, but this is only needed if the camera
			// is not inside a shell.
			if (cc && cc->isOutsideChunk())
			{
				forceDrawShells = true;
			}

			s_umbraDraw.start();
			Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			ChunkManager::instance().umbraDraw();			

			if (scenaryWireFrameStatus_ || terrainWireFrameStatus_)
			{
				Moo::rc().device()->EndScene();

				Vector3 bgColour = Vector3( 0, 0, 0 );
				Moo::rc().device()->Clear( 0, NULL,
						D3DCLEAR_ZBUFFER,
						Colour::getUint32( bgColour ), 1, 0 );

				Moo::rc().device()->BeginScene();			
				Moo::rc().setRenderState( D3DRS_FILLMODE, scenaryWireFrameStatus_ ?
					D3DFILL_WIREFRAME : D3DFILL_SOLID );

				if ( Terrain::BaseTerrainRenderer::instance()->version() == 200 )
					Terrain::TerrainRenderer2::instance()->zBufferIsClear( true );

				UmbraHelper::instance().wireFrameTerrain( terrainWireFrameStatus_ & 1 );
				ChunkManager::instance().umbraRepeat();
				UmbraHelper::instance().wireFrameTerrain( 0 );

				if ( Terrain::BaseTerrainRenderer::instance()->version() == 200 )
					Terrain::TerrainRenderer2::instance()->zBufferIsClear( false );
			}			
			s_umbraDraw.stop();
		}
		else
		{
			Moo::rc().setRenderState( D3DRS_FILLMODE, scenaryWireFrameStatus_ ?
			D3DFILL_WIREFRAME : D3DFILL_SOLID );
			ChunkManager::instance().draw();
		}		
	#else
			Moo::rc().setRenderState( D3DRS_FILLMODE, scenaryWireFrameStatus_ ?
			D3DFILL_WIREFRAME : D3DFILL_SOLID );		
			ChunkManager::instance().draw();
	#endif
		// render overlapping chunks
		speedtree::SpeedTreeRenderer::beginFrame( &romp_->enviroMinder() );

		// This set makes sure that we draw shells only once when some shells
		// are selected or game visibility is off, no matter if the flag
		// 'forceDrawShells' is on or not.
		std::set< Chunk * > shellsToDraw;

		if (cc)
		{
			// Umbra won't populate the overlapper drawList, this is used by
			// the non-umbra rendering path.
			for (std::vector< Chunk * >::iterator ci = EditorChunkOverlapper::drawList.begin();
				 ci != EditorChunkOverlapper::drawList.end(); ++ci)
			{
				Chunk* c = *ci;
				if ( !c->isBound() )
				{
					//TODO : this shoudln't happen, chunks should get out
					// of the drawList when they are offline.
					DEBUG_MSG( "WorldManager::renderChunks: Trying to draw chunk %s while it's offline!\n", c->resourceID().c_str() );
					continue;
				}
				if (c->drawMark() != cc->drawMark() || forceDrawShells)
				{
					shellsToDraw.insert( c );
				}
			}					
		}
		EditorChunkOverlapper::drawList.clear();

		// Force rendering selected shells, ensuring the user can manipulate
		// them even if the should be culled normally.
		if (cc)
		{
			for (std::vector< ChunkItemPtr >::iterator iter = selectedItems_.begin();
				iter != selectedItems_.end(); ++iter)
			{
				Chunk* c = (*iter)->chunk();
				if (c && !c->isOutsideChunk() &&
					(c->drawMark() != cc->drawMark() || forceDrawShells))
				{
					// Draw the shell if the draw mark requires it or if Umbra
					// is enabled, because Umbra messes up with a shell's draw
					// mark but doesn't draw it.
					shellsToDraw.insert( c );
				}
			}
		}

		// inside chunks will not render if they are not reacheable through portals. 
		// If game visibility is off, the overlappers are used to render not-connected 
		// chunks. But, with the visibility bounding box culling, the overlapper may 
		// not be rendered, causing the stray shell to be invisible, even if it is 
		// itself inside the camera frustum. To fix this situation, when game visibility 
		// is turned off, after rendering the chunks, it goes through all loaded chunks, 
		// trying to render those that are inside and haven't been rendered for this frame. 
		// Visibility bounding box culling still applies.
		if (cc != NULL && Options::getOptionInt("render/scenery/shells/gameVisibility", 1) == 0)
		{			
			ChunkSpacePtr space = ChunkManager::instance().cameraSpace();
			if (space.exists())
			{
				for (ChunkMap::iterator it = space->chunks().begin();
					it != space->chunks().end(); ++it)
				{
					for (uint i = 0; i < it->second.size(); i++)
					{
						if (it->second[i] != NULL                       &&
							!it->second[i]->isOutsideChunk()            &&
							(it->second[i]->drawMark() != cc->drawMark() || forceDrawShells) &&
							it->second[i]->isBound())
						{
							// If Umbra is enabled, simply ignore the draw mark
							// because Umbra messes up with a shell's draw mark
							// but doesn't draw it.
							shellsToDraw.insert( it->second[i] );
						}
					}
				}
			}
		}

		// We set the next mark to be the same as the camera chunks mark
		// so that objects do not get rendered multiple times
		uint32 storedMark = Chunk::s_nextMark_;
		if (cc != NULL)
		{
			Chunk::s_nextMark_ = cc->drawMark();
		}

		// Draw all shells that need to be drawn explicitly.
		for (std::set< Chunk * >::iterator it = shellsToDraw.begin();
			it != shellsToDraw.end(); ++it)
		{
			BoundingBox bb = (*it)->visibilityBox();
			bb.calculateOutcode( Moo::rc().viewProjection() );
			if (!bb.combinedOutcode())
			{
				(*it)->drawSelf();
			}
			(*it)->drawMark( cc->drawMark() );
		}

		// Set the next mark back to the stored value
		Chunk::s_nextMark_ = storedMark;

		speedtree::SpeedTreeRenderer::endFrame();
		Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

        s_chunkDraw.stop();
    }
}


void WorldManager::renderTerrain()
{	
	BW_GUARD;

	if (Options::getOptionInt( "render/terrain", 1 ))
	{
		// draw terrain
		ScopedDogWatch sdw( s_terrainDraw );

		canSeeTerrain_ = Terrain::BaseTerrainRenderer::instance()->canSeeTerrain();

		Moo::rc().pushRenderState( D3DRS_FILLMODE );

		bool wireFrameTerrain = Options::getOptionInt( "render/terrain/wireFrame", 0 ) != 0;

		#if UMBRA_ENABLE
		if (UmbraHelper::instance().umbraEnabled())
		{
			wireFrameTerrain &= UmbraHelper::instance().wireFrameTerrain();
		}
		#endif

		Moo::rc().setRenderState( D3DRS_FILLMODE, wireFrameTerrain ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

		if (drawSelection())
		{
			Moo::EffectMaterialPtr selectionMaterial = NULL;
			// TODO: We should get the FX name automatically with the version number,
			// instead of having to 'if' the version numbers.

			uint32 curTerrainVer = pTerrainSettings()->version();
			if ( curTerrainVer == 200 )
			{
				if (s_selectionMaterial == NULL)
				{
					s_selectionMaterial = new Moo::EffectMaterial();
					s_selectionMaterialOk =
						s_selectionMaterial->initFromEffect( s_terrainSelectionFx );
				}
				if ( s_selectionMaterialOk )
					selectionMaterial = s_selectionMaterial;
			}
			else if ( curTerrainVer == 100 )
			{
				if (s_selectionMaterialLegacy == NULL)
				{
					s_selectionMaterialLegacy = new Moo::EffectMaterial();
					s_selectionMaterialLegacyOk =
						s_selectionMaterialLegacy->initFromEffect( s_terrainSelectionFxLegacy );
				}
				if ( s_selectionMaterialLegacyOk )
					selectionMaterial = s_selectionMaterialLegacy;
			}

			if ( selectionMaterial != NULL )
			{
				Terrain::EditorBaseTerrainBlock::drawSelection( true );
				Terrain::BaseTerrainRenderer::instance()->drawAll( selectionMaterial );
				Terrain::EditorBaseTerrainBlock::drawSelection( false );
			}
			else
			{
				// TODO: This is printing the error every frame. Should improve.
				ERROR_MSG( "WorldManager::renderTerrain: There is no valid selection shader for the current terrain\n" );
			}
		}
		else
		{
			Terrain::BaseTerrainRenderer::instance()->drawAll();
		}

		if (!readOnlyTerrainBlocks_.empty())
		{
			AVectorNoDestructor< BlockInPlace >::iterator i = readOnlyTerrainBlocks_.begin();            
			for (; i != readOnlyTerrainBlocks_.end(); ++i)
            {
				Terrain::BaseTerrainRenderer::instance()->addBlock( i->second.getObject(),
																i->first );
            }            

			setReadOnlyFog();

			if( !drawSelection() )
				Terrain::BaseTerrainRenderer::instance()->drawAll();

			readOnlyTerrainBlocks_.clear();

			FogController::instance().commitFogToDevice();
		}

		Moo::rc().popRenderState();
	}
	else
	{
		canSeeTerrain_ = false;
		Terrain::BaseTerrainRenderer::instance()->clearBlocks();
	}
}


void WorldManager::renderEditorGizmos()
{
	BW_GUARD;

	// draw tools
	ToolPtr spTool = ToolManager::instance().tool();
	if ( spTool )
	{
		spTool->render();
	}
}


void WorldManager::tickEditorTickables()
{
	BW_GUARD;

	// This allows tickable to add/remove tickables to the list
	// while being ticked, for example, removing itself after tick.
	std::list<EditorTickablePtr> tempCopy = editorTickables_;

	for( std::list<EditorTickablePtr>::iterator i = tempCopy.begin();
		i != tempCopy.end(); ++i )
	{
		(*i)->tick();
	}
}


void WorldManager::renderEditorRenderables()
{
	BW_GUARD;

	// This allows renderables to add/remove renderables to the list
	// while rendering, for example, removing itself after render.
	std::set<EditorRenderablePtr> tempCopy = editorRenderables_;

	for( std::set<EditorRenderablePtr>::iterator i = tempCopy.begin();
		i != tempCopy.end(); ++i )
	{
		(*i)->render();
	}
}


void WorldManager::renderDebugGizmos()
{
}


void WorldManager::renderRompDelayedScene()
{
	BW_GUARD;

	// draw romp delayed scene
    s_rompDraw.start();
    romp_->drawDelayedSceneStuff();
    s_rompDraw.stop();
}


void WorldManager::renderRompPostScene()
{
	BW_GUARD;

    // draw romp post scene
    s_rompDraw.start();
    romp_->drawPostSceneStuff();
    s_rompDraw.stop();
}


void WorldManager::addTickable( EditorTickablePtr tickable )
{
	BW_GUARD;

	editorTickables_.push_back( tickable );
}


void WorldManager::removeTickable( EditorTickablePtr tickable )
{
	BW_GUARD;

	editorTickables_.remove( tickable );
}


void WorldManager::addRenderable( EditorRenderablePtr renderable )
{
	BW_GUARD;

	editorRenderables_.insert( renderable );
}


void WorldManager::removeRenderable( EditorRenderablePtr renderable )
{
	BW_GUARD;

	editorRenderables_.erase( renderable );
}


void WorldManager::endRender()
{
	BW_GUARD;

    s_render.stop();
}

bool WorldManager::init( HINSTANCE hInst, HWND hwndApp, HWND hwndGraphics )
{
	BW_GUARD;

	if ( !inited_ )
    {
		new WEPreloader();

		new SceneBrowser;
		SceneBrowser::instance().callbackSelChange(
			new BWFunctor1<
						WorldManager, const SceneBrowser::ItemList & >(
								this, &WorldManager::sceneBrowserSelChange ) );
		SceneBrowser::instance().callbackCurrentSel(
			new BWFunctor0RC< WorldManager, const std::vector<ChunkItemPtr>& >(
								this, &WorldManager::selectedItems ) );

		if( CVSWrapper::init() == CVSWrapper::FAILURE )
		{
			return false;
		}

		hwndApp_ = hwndApp;
        hwndGraphics_ = hwndGraphics;

	    ::ShowCursor( true );

		//init python data sections
		PyObject * pMod = PyImport_AddModule( "WorldEditor" );	// borrowed
		PyObject * pBW = PyImport_AddModule( "BigWorld" );	// borrowed

		Personality::import( Options::getOptionString( "personality" ) );

		new DXEnum( s_dxEnumPath );

		new ChunkPhotographer();

		new AmortiseChunkItemDelete();

		// Set callback for PyScript so it can know total game time
		Script::setTotalGameTimeFn( getTotalTime );

		// create the editor entities descriptions
		// this cannot be called from within the load thread
		// as python and load load thread hate each other
		EditorEntityType::startup();
		EditorUserDataObjectType::startup();

		// init BWLockD
		if ( Options::getOptionBool( "bwlockd/use", true ))
		{
			std::string host = Options::getOptionString( "bwlockd/host" );
			std::string username = Options::getOptionString( "bwlockd/username" );
			if( username.empty() )
			{
				wchar_t name[1024];
				DWORD size = ARRAY_SIZE( name );
				GetUserName( name, &size );
				bw_wtoutf8( name, username );
			}
			std::string hostname = host.substr( 0, host.find( ':' ) );
			static const int xExtent = Options::getOptionInt( "bwlockd/xExtent",
				(int)( ( MAX_TERRAIN_SHADOW_RANGE + 1.f ) / GRID_RESOLUTION ) );
			static const int yExtent = Options::getOptionInt( "bwlockd/yExtent", 1 );
			conn_.init( host, username, xExtent, yExtent );
		}
		// Init GUI Manager
		// Backwards compatibility for options.xml without this option.
		// Otherwise all buttons light up
		Options::setOptionInt( "render/chunk/vizMode", 
			Options::getOptionInt( "render/chunk/vizMode", 0 ));
		GUI::Manager::instance().optionFunctor().setOption( this );
		updateLanguageList();

		// Init terrain:
		MF_VERIFY( Terrain::Manager::init() );

		// Init Material Types:
		MF_VERIFY( MaterialKinds::init() );

		EditorChunkTerrainProjector::instance().init();

		// Background task manager:
		BgTaskManager::instance().startThreads( 1 );

		// Init chunk manager
		ChunkManager::instance().init();

		// Init HeightMap
		HeightMap::init();

		// Precompile effects?
		if ( Options::getOptionInt( "precompileEffects", 1 ) )
		{
			std::vector<ISplashVisibilityControl*> SVCs;
			if ( CSplashDlg::getSVC() )
				SVCs.push_back(CSplashDlg::getSVC());
			if (WaitDlg::getSVC())
				SVCs.push_back(WaitDlg::getSVC());
			ResourceLoader::instance().precompileEffects( SVCs );
		}

		class WorldEditorMRUProvider : public MRUProvider
		{
			virtual void set( const std::string& name, const std::string& value )
			{
				Options::setOptionString( name, value );
			}
			virtual const std::string get( const std::string& name ) const
			{
				return Options::getOptionString( name );
			}
		};
		static WorldEditorMRUProvider WorldEditorMRUProvider;
		spaceManager_ = new SpaceManager( WorldEditorMRUProvider );


		// This scoped class will clean up in case of an early return.
		InitFailureCleanup failureCleaner( inited_ );


		if( !spaceManager_->num() || !changeSpace( spaceManager_->entry( 0 ), false ) )
		{
			CSplashDlg::HideSplashScreen();
			if (WaitDlg::isValid())
				WaitDlg::getSVC()->setSplashVisible( false );
			for(;;)
			{
				MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();
				MsgBox mb( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_TITLE"),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_TEXT"),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_OPEN"),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_CREATE"),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_EXIT") );
				int result = mb.doModal( mainFrame->m_hWnd );
				if( result == 0 )
				{
					if( changeSpace( GUI::ItemPtr() ) )
						break;
				}
				else if( result == 1 )
				{
					if( newSpace( GUI::ItemPtr() ) )
						break;
				}
				else
				{
					// failureCleaner above takes care of cleaning up
					return false; //sorry
				}
			}
		}

/*		ChunkSpacePtr space = new ChunkSpace(1);
		std::string spacePath = Options::getOptionString( "space/mru0" );
		mapping_ = space->addMapping( SpaceEntryID(), Matrix::identity, spacePath );
		if (!mapping_)
		{
			CRITICAL_MSG( "Couldn't load %s as a space.\n\n"
						"Please run SpaceChooser to select a valid space.\n",
						spacePath.c_str() );
			return false;
		}
		ChunkManager::instance().camera( Matrix::identity, space );
		// Call tick to give it a chance to load the outdoor seed chunk, before
		// we ask it to load the dirty chunks
		ChunkManager::instance().tick( 0.f );

		// We don't want our space going anywhere
		space->incRef();*/

		chunkManagerInited_ = true;

		if ( !initRomp() )
		{
			// failureCleaner above takes care of cleaning up
			return false;
		}

		// start up the camera
		// Inform the camera of the window handle
		worldEditorCamera_ = new WorldEditorCamera();

		// initialise the progressBar
		if ( progressBar_ == NULL )
		{
			progressBar_ = new WorldEditorProgressBar();
		}

		//watchers
		MF_WATCH( "Client Settings/Far Plane",
		*this,
		MF_ACCESSORS( float, WorldManager, farPlane) );

		MF_WATCH( "Render/Draw Portals", ChunkBoundary::Portal::drawPortals_ );

		// set the saved far plane
		float fp = Options::getOptionFloat( "graphics/farclip", 500 );
		farPlane( fp );

		// Use us to provide the snap settings for moving objects etc
		SnapProvider::instance( this );
		CoordModeProvider::ins( this );

		ApplicationInput::disableModeSwitch();

		ResourceCache::instance().init();

		Terrain::EditorTerrainBlock2::blendBuildInterval(
			Options::getOptionInt( "terrain2/blendsBuildInterval",
				Terrain::EditorTerrainBlock2::blendBuildInterval() ) );

		#if UMBRA_ENABLE
		ChunkUmbra::terrainOverride(
			new BWFunctor0< WorldManager >( this, &WorldManager::renderTerrain ) );
		#endif

		inited_ = true;
	}

	return true;
}
BWLock::WorldEditordConnection& WorldManager::connection()
{
	return conn_;
}

bool WorldManager::postLoadThreadInit()
{
	BW_GUARD;

	// create the fibers
	mainFiber_ = ::ConvertThreadToFiber( NULL );
	MF_ASSERT( mainFiber_ );
	updatingFiber_ = ::CreateFiber( 1024*1024, WorldManager::backgroundUpdateLoop, this );
	MF_ASSERT( updatingFiber_ );

#if 0
	::SetThreadPriority(
		ChunkManager::instance().chunkLoader()->thread()->handle(),
		THREAD_PRIORITY_ABOVE_NORMAL );
#endif

    return true;
}

GeometryMapping* WorldManager::geometryMapping()
{
	return mapping_;
}


/*~ attribute WorldEditor.romp
 *	@components{ worldeditor }
 *
 *	The romp attribute gives access to the RompHarness object in the editor. For
 *	more information, see the RompHarness class.
 *
 *	@type	RompHarness
 */

bool WorldManager::initRomp()
{
	BW_GUARD;

	if ( !romp_ )
	{
		romp_ = new RompHarness;

		// set it into the WorldEditor module
		PyObject * pMod = PyImport_AddModule( "WorldEditor" );	// borrowed
		PyObject_SetAttrString( pMod, "romp", romp_ );

		if ( !romp_->init() )
		{
			return false;
		}

		romp_->enviroMinder().activate();
		this->setTimeFromOptions();
	}
	return true;
}

void WorldManager::focus( bool state )
{
	BW_GUARD;

    WorldEditorApp::instance().mfApp()->handleSetFocus( state );
}


void WorldManager::timeOfDay( float t )
{
	BW_GUARD;

	if ( romp_ )
    	romp_->setTime( t );
}

void WorldManager::rainAmount( float a )
{
	BW_GUARD;

	if ( romp_ )
    	romp_->setRainAmount( a );
}

bool WorldManager::escapePressed() 
{
	BW_GUARD;

	if ( !inEscapableProcess_ )
		return false;

	bool escape = false;
	// Check to see if ESC has been pressed
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE) || ::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE) )
	{
		if ( ( msg.message == WM_KEYDOWN ) && ( msg.wParam == VK_ESCAPE ) )
			escape = true;
		else
			DispatchMessage(&msg);
	}

	if ( escape )
	{
		inEscapableProcess_ = false;

		if (progressBar_)
		{
			progressBar_->setLabel( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG_PROGRESS_BAR/QUITTING") );
		}
	}

	return escape;
}

bool WorldManager::cursorOverGraphicsWnd() const
{
	BW_GUARD;

	MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();

	HWND fore = ::GetForegroundWindow();
	if ( fore != mainFrame->GetSafeHwnd() && ::GetParent( fore ) != mainFrame->GetSafeHwnd() )
		return false; // foreground window is not the main window nor a floating panel.

	RECT rt;
	::GetWindowRect( hwndGraphics_, &rt );
	POINT pt;
	::GetCursorPos( &pt );

	if ( pt.x < rt.left ||
		pt.x > rt.right ||
		pt.y < rt.top ||
		pt.y > rt.bottom )
	{
		return false;
	}

	HWND hwnd = ::WindowFromPoint( pt );
	if ( hwnd != hwndGraphics_ )
		return false; // it's a floating panel, return.
	HWND parent = hwnd;
	while( ::GetParent( parent ) != NULL )
		parent = ::GetParent( parent );
	::SendMessage( hwnd, WM_MOUSEACTIVATE, (WPARAM)parent, WM_LBUTTONDOWN * 65536 + HTCLIENT );
	return true;
}

POINT WorldManager::currentCursorPosition() const
{
	BW_GUARD;

	POINT pt;
	::GetCursorPos( &pt );
	::ScreenToClient( hwndGraphics_, &pt );

	return pt;
}

Vector3 WorldManager::getWorldRay(POINT& pt) const
{
	BW_GUARD;

	return getWorldRay( pt.x, pt.y );
}

Vector3 WorldManager::getWorldRay(int x, int y) const
{
	BW_GUARD;

	Vector3 v = Moo::rc().invView().applyVector(
		Moo::rc().camera().nearPlanePoint(
			(float(x) / Moo::rc().screenWidth()) * 2.f - 1.f,
			1.f - (float(y) / Moo::rc().screenHeight()) * 2.f ) );
	v.normalise();

	return v;
}

/*
void WorldManager::pushTool( ToolPtr tool )
{
	tools_.push( tool );
}


void WorldManager::popTool()
{
	if ( !tools_.empty() )
		tools_.pop();
}


ToolPtr WorldManager::tool()
{
	if ( !tools_.empty() )
		return tools_.top();

	return NULL;
}
*/


void WorldManager::addCommentaryMsg( const std::string& msg, int id )
{
	BW_GUARD;

	Commentary::instance().addMsg( msg, id );
}


void WorldManager::addError(Chunk* chunk, ChunkItem* item, const char * format, ...)
{
	BW_GUARD;

	va_list argPtr;
	va_start( argPtr, format );

	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, argPtr);
	buffer[sizeof(buffer)-1] = '\0';

	// add to the gui errors pane
	if( &MsgHandler::instance() )
		MsgHandler::instance().addAssetErrorMessage( buffer, chunk, item );

	// add to the view comments
	addCommentaryMsg( buffer, Commentary::CRITICAL );
}

/**
 *	This is for things that want to mark chunks as changed but don't want to
 *	include big_bang.hpp, ie, EditorChunkItem
 */
void changedChunk( Chunk * pChunk )
{
	BW_GUARD;

	WorldManager::instance().changedChunk( pChunk );
}

bool chunkWritable( const std::string& identifier, bool bCheckSurroundings /*= true*/ )
{
	BW_GUARD;

	BWLock::WorldEditordConnection& conn = WorldManager::instance().connection();
	ChunkPtr chunk = ChunkManager::instance().findChunkByName( identifier, WorldManager::instance().geometryMapping() );
	if( EditorChunkCache::instance( *chunk ).edReadOnly() )
		return false;

	if( *identifier.rbegin() == 'o' )
	{
		int16 gridX, gridZ;
		WorldManager::instance().geometryMapping()->gridFromChunkName( identifier, gridX, gridZ );

		if ( !conn.isLockedByMe( gridX,gridZ ) )
			return false;

		if (bCheckSurroundings)
		{
			for (int x = -conn.xExtent(); x < conn.xExtent() + 1; ++x)
			{
				for (int y = -conn.zExtent(); y < conn.zExtent() + 1; ++y)
				{
					int curX = gridX + x;
					int curY = gridZ + y;

					if (!conn.isLockedByMe(curX,curY))
						return false;
				}
			}
		}

		return true;
	}

	return chunkWritable( chunk, bCheckSurroundings );
}

bool chunkWritable( Chunk * pChunk, bool bCheckSurroundings /*= true*/ )
{
	BW_GUARD;

	if( EditorChunkCache::instance( *pChunk ).edReadOnly() )
		return false;

	BWLock::WorldEditordConnection& conn = WorldManager::instance().connection();

	if( pChunk->isOutsideChunk() )
		return chunkWritable( pChunk->identifier(), bCheckSurroundings );
	if( pChunk->loaded() )
		return EditorChunkCache::instance( *pChunk ).edIsWriteable( bCheckSurroundings );
	return true;// assume any unloaded shells are writable
}

void WorldManager::changedChunk( Chunk * pChunk, bool rebuildNavmesh /*= true*/,
								bool dirtyThumbnail /*= true*/ )
{
	BW_GUARD;

	MF_ASSERT( pChunk );
	MF_ASSERT( pChunk->loading() || pChunk->loaded() );

	if (!chunkWritable( pChunk, false ))
	{
		ERROR_MSG( "Tried to mark non locked chuck %s as dirty\n", pChunk->identifier().c_str() );
		return;
	}

	if (!Moo::g_renderThread)
	{
		SimpleMutexHolder smh( changeMutex );
		pendingChangedChunks_.insert( pChunk );
		return;
	}
	changedChunks_.insert( pChunk );


	if (dirtyThumbnail)
	{
		this->dirtyThumbnail( pChunk );
	}

	if ( rebuildNavmesh )
	{
		// something changed, so mark it's navigation mesh dirty.
		EditorChunkCache::instance( *pChunk ).navmeshDirty( true );
	}

	int16 x, z;
	geometryMapping()->gridFromChunkName(pChunk->identifier(), x, z);
	if (pChunk->isOutsideChunk())
	{
		chunkWatcher_->canUnload(x, z, pChunk->removable());
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onChangedChunkState(x, z);
		}
	}
}


/**
 *	This method tells WorldEditor that a chunk's lighting information is
 *	now out of date.
 */
void WorldManager::dirtyLighting( Chunk* pChunk )
{
	BW_GUARD;

	MF_ASSERT( pChunk );

	if (workingChunk_ == pChunk)
	{
		workingChunk_ = NULL;
		canUnloadChunk_ = false;
	}

	// Don't calc for outside chunks
	if (pChunk->isOutsideChunk() && 
		!pChunk->space()->staticLightingOutside())
		return;

	if (!chunkWritable( pChunk, false ))
		return;

	dirtyChunkLists_[ LIGHTING ].dirty( pChunk );

	if( EditorChunkCache::instance( *pChunk ).lightingUpdated() )
	{
		EditorChunkCache::instance( *pChunk ).lightingUpdated( false );
		changedChunks_.insert( pChunk );
	}

	lastModifyTime_ = GetTickCount();
}


/**
 *	This method marks only a single chunk's terrain shadows as dirty.
 */
void WorldManager::dirtyTerrainShadows( Chunk * pChunk )
{
	BW_GUARD;

	MF_ASSERT( pChunk->isOutsideChunk() );

	if (workingChunk_ == pChunk)
	{
		workingChunk_ = NULL;
		canUnloadChunk_ = false;
	}

	if (!chunkWritable( pChunk, false ))
		return;


	if (ChunkTerrainCache::instance( *pChunk ).pTerrain())
	{
		dirtyChunkLists_[ SHADOW ].dirty( pChunk );
	}
	else
	{
		dirtyChunkLists_[ SHADOW ].clean( pChunk->identifier() );
	}

	if( EditorChunkCache::instance( *pChunk ).shadowUpdated() )
	{
		EditorChunkCache::instance( *pChunk ).shadowUpdated( false );
		changedChunks_.insert( pChunk );
	}

	if (pChunk->isOutsideChunk())
	{
		int16 x, z;
		geometryMapping()->gridFromChunkName(pChunk->identifier(), x, z);
		chunkWatcher_->state(x, z, ChunkWatcher::DIRTY_NEEDS_SHADOW_CALC);
		chunkWatcher_->canUnload(x, z, pChunk->removable());
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onChangedChunkState(x, z);
		}
	}
}


/**
 *	This is the public interface for when a chunk's shadows are dirty.
 *	It flags all appropriate neighbouring chunks as have their shadows dirty as well.
 */
void WorldManager::markTerrainShadowsDirty( Chunk * pChunk )
{
	BW_GUARD;

	MF_ASSERT( pChunk );

	if (!pChunk->isOutsideChunk())
		return;

	if (!EditorChunkCache::instance( *pChunk ).edIsWriteable())
		return;

	//DEBUG_MSG( "marking terrain dirty in chunk %s\n", pChunk->identifier().c_str() );

	// ok, now add every chunk within MAX_TERRAIN_SHADOW_RANGE metres along
	// the x axis of pChunk

	dirtyTerrainShadows( pChunk );

	// shadows were directly changed in this chunk, which means that an item's
	// bounding box is overlapping this chunk, so mark it's navmesh dirty
	EditorChunkCache::instance( *pChunk ).navmeshDirty( true );

	// do 100, -100, 200, -200, etc, so the chunks closest to what just got
	// changed get recalced 1st
	for (float xpos = GRID_RESOLUTION;
		xpos < (MAX_TERRAIN_SHADOW_RANGE + 1.f);
		xpos = (xpos < 0.f ? GRID_RESOLUTION : 0.f) + (xpos * -1.f))
	{
		Vector3 pos = pChunk->centre() + Vector3( xpos, 0.f, 0.f );
		ChunkSpace::Column* col = ChunkManager::instance().cameraSpace()->column( pos, false );
		if (!col)
			continue;

		Chunk* c = col->pOutsideChunk();
		if (!c)
			continue;

		dirtyTerrainShadows( c );
	}

	lastModifyTime_ = GetTickCount();
}

void WorldManager::markTerrainShadowsDirty( const BoundingBox& bb )
{
	BW_GUARD;

	// Find all the chunks bb is in; we know it will be < 100m in the x & z
	// directions, however

	Vector3 a(bb.minBounds().x, 0.f, bb.minBounds().z);
	Vector3 b(bb.minBounds().x, 0.f, bb.maxBounds().z);
	Vector3 c(bb.maxBounds().x, 0.f, bb.maxBounds().z);
	Vector3 d(bb.maxBounds().x, 0.f, bb.minBounds().z);

	std::set<Chunk*> chunks;
	chunks.insert( EditorChunk::findOutsideChunk( a ) );
	chunks.insert( EditorChunk::findOutsideChunk( b ) );
	chunks.insert( EditorChunk::findOutsideChunk( c ) );
	chunks.insert( EditorChunk::findOutsideChunk( d ) );

	// Remove the null chunk, if that got added
	chunks.erase((Chunk*) 0);

	for (std::set<Chunk*>::iterator i = chunks.begin(); i != chunks.end(); ++i)
		markTerrainShadowsDirty( *i );
	lastModifyTime_ = GetTickCount();
}

void WorldManager::lockChunkForEditing( Chunk * pChunk, bool editing )
{
	BW_GUARD;

	// We only care about outside chunks at the moment
	if ( !pChunk || !pChunk->isOutsideChunk() )
		return;

	for (float xpos = -MAX_TERRAIN_SHADOW_RANGE;
		xpos <= MAX_TERRAIN_SHADOW_RANGE;
		xpos += GRID_RESOLUTION )
	{
		Vector3 pos = pChunk->centre() + Vector3( xpos, 0.f, 0.f );
		ChunkSpace::Column* col = ChunkManager::instance().cameraSpace()->column( pos, false );
		if (!col)
			continue;

		Chunk* c = col->pOutsideChunk();
		if (!c)
			continue;

		if ( editing )
		{
			// marking as editing, so insert and interrupt background
			// calculation if it's the current working chunk.
			chunksBeingEdited_.insert( c );
			if (this->isWorkingChunk( c ))
			{
				this->workingChunk( NULL, false );
			}
		}
		else
		{
			// erase it from the set so it can enter the background calculation
			// loop.
			std::set<Chunk*>::iterator it =
				chunksBeingEdited_.find( c );
			if ( it != chunksBeingEdited_.end() )
				chunksBeingEdited_.erase( it );
		}
	}
}

void WorldManager::dirtyThumbnail( Chunk * pChunk, bool justLoaded /*= false*/ )
{
	BW_GUARD;

	if (!chunkWritable( pChunk, false ))
	{
		return;
	}

	if (EditorChunkCache::instance( *pChunk ).thumbnailUpdated())
	{
		EditorChunkCache::instance( *pChunk ).thumbnailUpdated( false );
		//TODO : why does flagging a thumbnail as dirty mean the chunk file
		//has to be saved?  only the cData needs be saved.  In fact the
		//main caller of this method is changedChunk so remove this
		changedChunks_.insert( pChunk );
	}

	dirtyChunkLists_[ THUMBNAIL ].dirty( pChunk );

	SpaceMap::instance().dirtyThumbnail( pChunk );
    HeightMap::instance().dirtyThumbnail( pChunk, justLoaded );
	lastModifyTime_ = GetTickCount();
}

void WorldManager::dirtyTerrainLOD( Chunk * pChunk )
{
	BW_GUARD;

	if (!chunkWritable( pChunk, false ))
	{
		return;
	}

	if (EditorChunkCache::instance( *pChunk ).terrainLODUpdated())
	{
		EditorChunkCache::instance( *pChunk ).terrainLODUpdated( false );
		//TODO : why does flagging a terrain LOD as dirty mean the chunk file
		//has to be saved?  only the cData needs be saved.  In fact the
		//main caller of this method is changedChunk so remove this
		changedChunks_.insert( pChunk );
	}

	dirtyChunkLists_[ TERRAIN_LOD ].dirty( pChunk );

	lastModifyTime_ = GetTickCount();
}

void WorldManager::resetChangedLists()
{
	BW_GUARD;

	dirtyChunkLists_.clear();
	changedChunks_.clear();
	changedTerrainBlocks_.clear();
	changedThumbnailChunks_.clear();
	thumbnailChunksLoading_.clear();
	chunksBeingEdited_.clear();

	VLOManager::instance()->clearLists();
}

bool WorldManager::isDirtyLightChunk( Chunk * pChunk )
{
	BW_GUARD;

	return dirtyChunkLists_[ LIGHTING ].isDirty( pChunk->identifier() );
}

void WorldManager::doBackgroundUpdating()
{
	BW_GUARD;

	// Go to the update fiber, and do some processing
	SwitchToFiber( updatingFiber_ );
}

void WorldManager::startBackgroundProcessing()
{
	BW_GUARD;

	doBackgroundUpdating();
	while( ( (WorldEditorApp*)AfxGetApp() )->mfApp()->presenting() )
		Sleep(0);
}

void WorldManager::endBackgroundProcessing()
{
}
/*
	If we've spent > 30ms in the lighting fiber, switch back to the main one

	I came up with the 30ms value after a lil bit of playing, it's a decent
	comprimise between efficency & interactivity
*/
bool WorldManager::fiberPause()
{
	BW_GUARD;

	App *app = ( (WorldEditorApp*)AfxGetApp() )->mfApp();
	bool presenting = false;
	if (app != NULL)
		presenting = app->presenting();

	if( presenting )
		return false;

	if( killingUpdatingFiber_ )
	{
		this->workingChunk( NULL, false );
		return true;
	}

	SwitchToFiber( mainFiber_ );
	return true;
}

void WorldManager::stopBackgroundCalculation()
{
	BW_GUARD;

	if( updatingFiber_ )
	{
		killingUpdatingFiber_ = true;
		while( killingUpdatingFiber_ )
			doBackgroundUpdating();
	}
}

/*
	Never ending function required to run the background updating fiber.

	Just recalculates lighting, terrain shadows, etc as needed forever
*/
/*static*/ void WorldManager::backgroundUpdateLoop( PVOID param )
{
	BW_GUARD;

	for(;;)
	{
		if(!((WorldManager*)param)->killingUpdatingFiber_)
		{
			WorldManager::instance().workingChunk( NULL, false );

			DirtyChunkList& terrainChunks = WorldManager::instance().dirtyChunkLists_[ SHADOW ];
			DirtyChunkList& lightingChunks = WorldManager::instance().dirtyChunkLists_[ LIGHTING ];
			std::set<Chunk*>& editingChunks = WorldManager::instance().chunksBeingEdited_;
			Vector3 cameraPosition = Moo::rc().invView().applyToOrigin();
			Chunk* nearestLightingChunk = NULL;
			Chunk* nearestShadowChunk = NULL;
			float distance = 99999999.f;

			uint64 startTime = timestamp();
			for( DirtyChunkList::iterator iter = lightingChunks.begin(); iter != lightingChunks.end(); ++iter )
			{
				if( ( iter->second->centre() - cameraPosition ).lengthSquared() < distance &&
					WorldManager::instance().EnsureNeighbourChunkLoaded( iter->second, iter->second->space()->staticLightingPortalDepth() ) &&
					EditorChunkCache::instance( *(iter->second) ).edIsWriteable() &&
					!EditorChunkCache::instance( *(iter->second) ).edIsDeleted() &&
					iter->second->isBound() )
				{
					distance = ( iter->second->centre() - cameraPosition ).lengthSquared();
					nearestLightingChunk = iter->second;
				}

				uint32 ellapsedMillis = (uint32)((timestamp() - startTime) * 1000 / stampsPerSecond());
				if (ellapsedMillis > 5)
				{
					// EnsureNeighbourChunkLoaded is potentially slow, so stop
					// if it's taking too long.
					INFO_MSG( "WorldManager::backgroundUpdateLoop: Finding next static "
								"lighting chunk took to long, stopping search.\n" );
					break;
				}
			}

			for( DirtyChunkList::iterator iter = terrainChunks.begin(); iter != terrainChunks.end(); ++iter )
			{
				if( ( iter->second->centre() - cameraPosition ).lengthSquared() < distance &&
					WorldManager::instance().EnsureNeighbourChunkLoadedForShadow( iter->second ) &&
					EditorChunkCache::instance( *(iter->second) ).edIsWriteable( false ) &&
					!EditorChunkCache::instance( *(iter->second) ).edIsDeleted() &&
					iter->second->isBound() &&
					static_cast<EditorChunkTerrain*>(ChunkTerrainCache::instance( *(iter->second) ).pTerrain() ) &&
					editingChunks.find( iter->second ) == editingChunks.end() )
				{
					distance = ( iter->second->centre() - cameraPosition ).lengthSquared();
					nearestShadowChunk = iter->second;
					nearestLightingChunk = NULL;
				}
			}

			if( nearestLightingChunk )
			{
				WorldManager::instance().workingChunk( nearestLightingChunk, false );
				if( EditorChunkCache::instance( *nearestLightingChunk ).edRecalculateLighting() &&
					WorldManager::instance().isWorkingChunk( nearestLightingChunk ) )
					lightingChunks.clean( nearestLightingChunk->identifier() );
			}
			else if( nearestShadowChunk )
			{
				EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>( ChunkTerrainCache::instance( *nearestShadowChunk ).pTerrain());
				WorldManager::instance().workingChunk( nearestShadowChunk, false );
				pEct->calculateShadows();
				if( WorldManager::instance().isWorkingChunk( nearestShadowChunk ) )
					terrainChunks.clean( nearestShadowChunk->identifier() );
			}
			WorldManager::instance().workingChunk( NULL, false );
			WorldManager::instance().fiberPause();
		}
		WorldManager::instance().killingUpdatingFiber_ = false;
		SwitchToFiber( WorldManager::instance().mainFiber_ );
	}
}


void WorldManager::loadChunkForThumbnail( const std::string& chunkName )
{
	BW_GUARD;

	if( isChunkFileExists( chunkName, geometryMapping() ) )
	{
		ChunkPtr chunk = ChunkManager::instance().findChunkByName( chunkName, geometryMapping() );
		if( chunk && !chunk->isBound() )
		{
			ChunkManager::instance().loadChunkExplicitly( chunkName, geometryMapping() );
			// We must add this chunk to the list of loading thumbnails,
			// because otherwise "markChunks" will mark it as removable and it
			// will get removed before there's a chance to use it.
			thumbnailChunksLoading_.insert( chunk );
		}
	}
}


void WorldManager::discardChunkForThumbnail( Chunk * pChunk )
{
	BW_GUARD;

	thumbnailChunksLoading_.erase( pChunk );
}


void WorldManager::workingChunk( Chunk* chunk, bool canUnload )
{
	BW_GUARD;

	if (chunk != workingChunk_)
	{
		workingChunk_ = chunk;
		canUnloadChunk_ = canUnload;
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onNewWorkingChunk();
		}
	}
	else if (!canUnload)
	{
		canUnloadChunk_ = false;
	}
}

/** Write a file to disk and (optionally) add it to cvs */
bool WorldManager::saveAndAddChunkBase( const std::string & resourceID,
	const SaveableObjectBase & saver, bool add, bool addAsBinary )
{
	BW_GUARD;

	// add it before saving for if it has been cvs removed but not committed
	if (add)
	{
		CVSWrapper( currentSpace_ ).addFile( resourceID + ".cdata", addAsBinary, false );
		CVSWrapper( currentSpace_ ).addFile( resourceID + ".chunk", addAsBinary, false );
	}

	// save it out
	bool result = saver.save( resourceID );

	// add it again for if it has been ordinarily (re-)created
	if (add)
	{
		CVSWrapper( currentSpace_ ).addFile( resourceID + ".cdata", addAsBinary, false );
		CVSWrapper( currentSpace_ ).addFile( resourceID + ".chunk", addAsBinary, false );
	}

	return result;
}

/** Delete a file from disk and (eventually) remove it from cvs */
void WorldManager::eraseAndRemoveFile( const std::string & resourceID )
{
	BW_GUARD;

	std::string fileName = BWResource::resolveFilename( resourceID );
	std::string backupFileName;
	if( fileName.size() > strlen( "i.chunk" ) &&
		strcmp( fileName.c_str() + fileName.size() - strlen( "i.chunk" ), "i.chunk" ) == 0 )
		backupFileName = fileName.substr( 0, fileName.size() - strlen( "i.chunk" ) ) + "i.~chunk~";
	if( !backupFileName.empty() && !BWResource::fileExists( backupFileName ) )
	{
		std::wstring wfileName, wbackupFilename;
		bw_utf8tow( fileName, wfileName );
		bw_utf8tow( backupFileName, wbackupFilename );
		::MoveFile( wfileName.c_str(), wbackupFilename.c_str() );
	}
	else
	{
		std::wstring wfileName;
		bw_utf8tow( fileName, wfileName );
		::DeleteFile( wfileName.c_str() );
	}

	CVSWrapper( currentSpace_ ).removeFile( fileName );
}


void WorldManager::changedTerrainBlock( Chunk* pChunk, bool rebuildNavmesh /*= true*/ )
{
	BW_GUARD;

	EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
		ChunkTerrainCache::instance( *pChunk ).pTerrain());

	if (pEct)
	{
		changedTerrainBlocks_.insert( &pEct->block() );
		//TODO : since we call changedChunk at the end of this method, we don't
		//need to call dirty thumbnail (changedChunk calls it).  remove
		dirtyThumbnail( pChunk );
	}

	changedChunk( pChunk, rebuildNavmesh );
}


// This method sets a flag so chunks loaded from now on get recorded.
void WorldManager::recordLoadedChunksStart()
{
	BW_GUARD;

	recordLoadedChunks_ = true;
	loadedChunks_.clear();
}


// This method turns off chunk loading recording.
std::set<Chunk*> WorldManager::recordLoadedChunksStop()
{
	BW_GUARD;

	std::set<Chunk*> result = loadedChunks_;
	loadedChunks_.clear();
	recordLoadedChunks_ = false;
	return result;
}


/*
 *	This method resaves all the terrain blocks in the space, handy for when the
 *	file format changes and the client does not support the same format.
 */
void WorldManager::resaveAllTerrainBlocks()
{
	BW_GUARD;

	// stop background chunk loading
	SyncMode chunkStopper;

	float steps = float((mapping_->maxLGridY() - mapping_->minLGridY() + 1) *
		(mapping_->maxLGridX() - mapping_->minLGridX() + 1));

	ProgressTask terrainTask( progressBar_, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RESAVE_TERRAIN"), 
		float(steps) );

	// Iterate over all chunks in the current space
	for (int32 z = mapping_->minLGridY(); z <= mapping_->maxLGridY(); z++)
	{
		for (int32 x = mapping_->minLGridX(); x <= mapping_->maxLGridX(); x++)
		{
			terrainTask.step();

			// Get the centre of the chunk so that we can get the chunk 
			// identifier
			float posX = float(x) * GRID_RESOLUTION + GRID_RESOLUTION * 0.5f;
			float posZ = float(z) * GRID_RESOLUTION + GRID_RESOLUTION * 0.5f;

			std::string chunkIdentifier = 
				mapping_->outsideChunkIdentifier( Vector3(posX, 0.f, posZ ) );

			// See if the current chunk is in the chunk cache
			Chunk* pChunk = NULL;

			std::set<Chunk*>::iterator it = EditorChunkCache::chunks_.begin();

			while (it != EditorChunkCache::chunks_.end())
			{
				if ((*it)->identifier() == chunkIdentifier)
				{
					pChunk = *it;
					break;
				}
				++it;
			}

			// If the current chunk is in the chunk cache, set it as changed
			if (pChunk)
			{
				EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
					ChunkTerrainCache::instance( *pChunk ).pTerrain());

				pEct->block().rebuildNormalMap(Terrain::NMQ_NICE);

				this->workingChunk( pChunk, false );
				this->changedTerrainBlock( pChunk );
			}
			else
			{
				Matrix xform = Matrix::identity;
				xform.setTranslate(x*Terrain::BLOCK_SIZE_METRES, 0.0f, z*Terrain::BLOCK_SIZE_METRES);

				// The current chunk is not in the chunk cache, load up the
				// terrain block and save it out again
				std::string resourceName = mapping_->path() + 
					chunkIdentifier + ".cdata/terrain";
				Terrain::EditorBaseTerrainBlockPtr pEtb = 
					EditorChunkTerrain::loadBlock( resourceName, NULL,
					xform.applyToOrigin() );

				if( pEtb )
				{
					resourceName = mapping_->path() + chunkIdentifier + 
						".cdata";

					DataSectionPtr pCDataSection = 
						BWResource::openSection( resourceName );
					if ( pCDataSection )
					{
						// We must save to terrain1 or terrain2 section
						DataSectionPtr dataSection = 
							pCDataSection->openSection( pEtb->dataSectionName(), 
														true );
						if ( dataSection )
						{
							EditorChunkCache::UpdateFlags uf( pCDataSection );

							uf.terrainLOD_ = pEtb->rebuildLodTexture(xform);
							uf.save();

							pEtb->rebuildNormalMap(Terrain::NMQ_NICE);
							pEtb->save( dataSection );

							pCDataSection->save();
						}
					}
				}
			}
		}
	}
	// Do a quick save to save out all the terrain blocks that are in memory
	quickSave();
}


void WorldManager::restitchAllTerrainBlocks()
{
	BW_GUARD;

	ChunkRowCache rowCache(1); // we do processing in 3x3 blocks

	float steps = float((mapping_->maxLGridY() - mapping_->minLGridY() + 1) *
		(mapping_->maxLGridX() - mapping_->minLGridX() + 1));

	ProgressTask terrainTask( progressBar_, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RE_STITCHING_ALL_TERRAIN_BLOCKS"), 
		float(steps) );

	for (int32 z = mapping_->minLGridY(); z <= mapping_->maxLGridY(); z++)
	{
		rowCache.row(z);

		for (int32 x = mapping_->minLGridX(); x <= mapping_->maxLGridX(); x++)
		{
			// find chunk and neighbors
			std::string chunkIdentifier[9];
			Chunk*		pChunk[9];
			uint32		ci = 0;

			for ( int32 i = -1; i <= 1; i++ )
			{
				for ( int32 j = -1; j <= 1; j++ )
				{
					int16 wx = (int16)(x + j);
					int16 wz = (int16)(z + i);
					::chunkID(chunkIdentifier[ci], wx, wz);
					pChunk[ci] =
						ChunkManager::instance().findChunkByName
						(
							chunkIdentifier[ci],
							mapping_
						);
					++ci;
				}
			}

			// we were trying to do a chunk that isn't in space
			if ( pChunk[4] == NULL )
			{
				continue;
			}

			// tell the chunk watcher window what we're up to
			this->workingChunk( pChunk[4], true );

			EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
				ChunkTerrainCache::instance( *pChunk[4] ).pTerrain());
			MF_ASSERT( pEct );

			pEct->onEditHeights();

			// Save:
			Terrain::EditorBaseTerrainBlock & pTerrain = pEct->block();
			std::string resourceName = mapping_->path() 
				+ chunkIdentifier[4] + ".cdata";
			pTerrain.save(resourceName);

			// update progress bar
			terrainTask.step();
		}
	}
}


/**
 *	This function goes through all chunks, both loaded and unloaded, and
 *	recalculates the thumbnails and saves them directly to disk.  Chunks
 *	which were unloaded are ejected when it finishes with them, so large
 *	spaces can be regenerated.  The downside is that there is no undo/redo and
 *	the .cdata files are modified directly.  It also assumes that the shadow
 *	data is up to date.
 *
 *	This function also deletes the time stamps and dds files.
 */
void WorldManager::regenerateThumbnailsOffline()
{
	BW_GUARD;

	// Stop background chunk loading:
	SyncMode chunkStopper;

	// Remove the time stamps and space*thumbnail dds files:
	std::string spacePath    = mapping_->path();
	std::string timeStamps   = spacePath + "space.thumbnail.timestamps";
	std::string spaceDDS     = spacePath + "space.thumbnail.dds";
	std::string spaceTempDDS = spacePath + "space.temp_thumbnail.dds";
	timeStamps		= BWResource::resolveFilename(timeStamps  );
	spaceDDS		= BWResource::resolveFilename(spaceDDS    );
	spaceTempDDS	= BWResource::resolveFilename(spaceTempDDS);
	::unlink(timeStamps  .c_str());
	::unlink(spaceDDS    .c_str());
	::unlink(spaceTempDDS.c_str());


	float steps = 
		(mapping_->maxLGridY() - mapping_->minLGridY() + 1.0f)
		*
		(mapping_->maxLGridX() - mapping_->minLGridX() + 1.0f);
	ProgressTask thumbProgress( progressBar_, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/REGENERATING_THUMBNAILS"), steps);

	for (int32 z = mapping_->minLGridY(); z <= mapping_->maxLGridY(); ++z)
	{
		if (Moo::rc().device()->TestCooperativeLevel() != D3D_OK)
		{
			ERROR_MSG("Device is lost, regenerating thumbnails has been stopped\n");
			this->workingChunk( NULL, false );
			return;
		}

		for (int32 x = mapping_->minLGridX(); x <= mapping_->maxLGridX(); ++x)
		{
			// Get the chunk's name:
			std::string chunkName;
			chunkID(chunkName, (int16)x, (int16)z);

			// Get the chunk:
			Chunk *chunk = ChunkManager::instance().findChunkByName(
													chunkName,
													this->geometryMapping() );

			if (chunk != NULL)
			{
				// Set the working chunk:
				this->workingChunk( chunk, true );

				// Is the chunk in memory yet?  We use this below to unload
				// chunks which weren't in memory:
				bool inMemory = chunk->loaded();

				// Force to memory:
				if (!inMemory)
				{
					ChunkManager::instance().loadChunkNow
					(
						chunkName,
						geometryMapping()
					);
					ChunkManager::instance().checkLoadingChunks();
				}

				// Photograph the chunk:
				ChunkPhotographer::photograph(*chunk);

				// If we forced the chunk to memory then remove it from memory:
				if (!inMemory)
				{					
					if (chunk->removable())
					{
						chunk->unbind( false );
						chunk->unload();
						onUnloadChunk(chunk);
					}
				}
			}

			// Update the progress indicator:
			thumbProgress.step();
		}
	}

	// Set the working chunk back to NULL.
	this->workingChunk( NULL, false );

	// Regenerate the space map:
	ProjectModule::regenerateAllDirty();

	// Save:
	quickSave();
}


/**
 *	This function converts the current space to use zip sections.
 */
void WorldManager::convertSpaceToZip()
{
	BW_GUARD;

	// Prompt if the world has been modified:
	if( !canClose( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CONVERT") ) )
		return;

	// Stop background chunk loading, clear the DataSection cache:
	SyncMode chunkStopper;
	BWResource::instance().purgeAll(); 

	// Find all .cdata files:
	std::vector< std::wstring > cdataFiles;
	std::wstring spacePath;
	bw_utf8tow( BWResource::resolveFilename( mapping_->path() ), spacePath );	
	std::wstring cdataFilesRE = spacePath + L"*.cdata";	
	WIN32_FIND_DATA fileInfo;
	HANDLE findResult = ::FindFirstFile( cdataFilesRE.c_str(), &fileInfo );
	if (findResult != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::wstring cdataFullFile = spacePath + fileInfo.cFileName;
			cdataFiles.push_back( cdataFullFile );
		}
		while (::FindNextFile( findResult, &fileInfo ) != FALSE);
		::FindClose( findResult );
	}

	// Convert the .cdata files to ZipSections:
	ProgressTask 
		zipProgress
		( 
			progressBar_, 
			LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CONVERTING_TO_ZIP"), 
			(float)cdataFiles.size()
		);
	for
	(
		std::vector< std::wstring >::const_iterator it = cdataFiles.begin();
		it != cdataFiles.end();
		++it
	)
	{
		std::string nfilename;
		bw_wtoutf8( (*it), nfilename );
		std::string    cdataFile   = BWResource::dissolveFilename( nfilename );
		DataSectionPtr dataSection = BWResource::openSection( cdataFile );
		if (dataSection)
		{
			dataSection = dataSection->convertToZip();
			dataSection->save();
		}
		zipProgress.step();
	}

	// Force the space to reload:
	reloadAllChunks( false );
}


/**
 *	This function goes through all chunks, both loaded and unloaded, and
 *	recalculates the terrain LODs and saves them directly to disk.  Chunks
 *	which were unloaded are ejected when it finishes with them, so large
 *	spaces can be regenerated.  The downside is that there is no undo/redo and
 *	the .cdata files are modified directly.
 */
void WorldManager::regenerateLODsOffline()
{
	BW_GUARD;

	inEscapableProcess_ = true;
	// Stop background chunk loading:
	SyncMode chunkStopper;

	float steps = 
		(mapping_->maxLGridY() - mapping_->minLGridY() + 1.0f)
		*
		(mapping_->maxLGridX() - mapping_->minLGridX() + 1.0f);
	ProgressTask thumbProgress( progressBar_, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/REGENERATING_LODS"), steps);
	progressBar_->escapable(true);

	std::string space = this->getCurrentSpace();

	bool escape = false;

	for (int32 z = mapping_->minLGridY(); z <= mapping_->maxLGridY() && !escape; ++z)
	{
		if (Moo::rc().device()->TestCooperativeLevel() != D3D_OK)
		{
			ERROR_MSG("Device is lost, regenerating LODs has been stopped\n");
			this->workingChunk( NULL, false );
			return;
		}

		for (int32 x = mapping_->minLGridX(); x <= mapping_->maxLGridX(); ++x)
		{
			if (this->escapePressed())
			{
				escape = true;
				break;
			}

			// Get the chunk's name:
			std::string chunkName;
			chunkID(chunkName, (int16)x, (int16)z);

			// Get the chunk:
			Chunk *chunk = ChunkManager::instance().findChunkByName(
													chunkName,
													this->geometryMapping() );

			if (chunk != NULL)
			{
				// Set the working chunk:
				this->workingChunk( chunk, true );

				// Is the chunk in memory yet?  We use this below to unload
				// chunks which weren't in memory:
				bool inMemory = chunk->loaded();

				// Force to memory:
				if (!inMemory)
				{
					ChunkManager::instance().loadChunkNow
					(
						chunkName,
						geometryMapping()
					);
					ChunkManager::instance().checkLoadingChunks();
				}

				// Re-LOD the chunk and generate the dominant texture layers:
				bool ok = false;
				EditorChunkTerrain* ect = 
					static_cast<EditorChunkTerrain*>
					(
						ChunkTerrainCache::instance( *chunk ).pTerrain() 
					);
				if (ect != NULL)
				{
					Terrain::EditorBaseTerrainBlock &block = ect->block();
					block.rebuildCombinedLayers();
					ok = block.rebuildLodTexture( chunk->transform() );				
					EditorChunkCache::instance( *chunk ).terrainLODUpdated( ok );
				}

				// If we forced the chunk to memory then remove it from memory:
				if (!inMemory)
				{
					if (ok)
					{
						ect->block().save(space + '/' + chunk->identifier() + ".cdata");
						dirtyChunkLists_[ TERRAIN_LOD ].clean( chunk->identifier() );
					}
					if (chunk->removable())
					{
						chunk->unbind( false );
						chunk->unload();
						onUnloadChunk(chunk);
					}
				}
				else
				{
					changedTerrainBlock(chunk);
				}
			}

			// Update the progress indicator:
			thumbProgress.step();
		}
	}

	progressBar_->escapable(false);

	// Reset the working chunk:
	this->workingChunk( NULL, false );

	// Save:
	if (!escape)
	{
		this->quickSave();
		this->writeCleanList();
	}

	inEscapableProcess_ = false;
}


void WorldManager::chunkShadowUpdated( Chunk * pChunk )
{
	BW_GUARD;

	EditorChunkCache::instance( *pChunk ).shadowUpdated( true );
	//TODO : so here a chunk's shadowing has been calcualted.  This means
	//just its cData should be saved.  Of course if it was making a chunk dirty
	//that originally caused this shadow recalc, then the chunk would already
	//be on the changedChunks list making the following call unnecessary.
	changedChunks_.insert( pChunk );
	//TODO : a shadow was updated so the thumbnail is dirty.  this is true, and
	//this kind of call should be checked across the codebase.
	dirtyThumbnail( pChunk );

	if (pChunk->isOutsideChunk())
	{
		int16 x, z;
		geometryMapping()->gridFromChunkName( pChunk->identifier(), x, z);
		chunkWatcher_->state(x, z, ChunkWatcher::DIRTYSHADOW_CALCED);
		chunkWatcher_->canUnload(x, z, pChunk->removable());
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onChangedChunkState( x, z );
		}
	}

	changedChunk( pChunk, false, false );
}


/**
 *	Call this method when a thumbnail has now been generated for a chunk.
 */
void WorldManager::chunkThumbnailUpdated( Chunk* pChunk )
{
	BW_GUARD;

	MF_ASSERT( pChunk );

	if (!chunkWritable( pChunk, false ))
	{
		ERROR_MSG( "Tried to mark read-only chunk %s as dirty\n", pChunk->identifier().c_str() );
		return;
	}

	EditorChunkCache::instance( *pChunk ).thumbnailUpdated( true );	
	changedThumbnailChunks_.insert( pChunk );

	//Now the thumbnail is calculated, we can removed it from the dirty list.
	dirtyChunkLists_[ THUMBNAIL ].clean( pChunk->identifier() );

	SpaceMap::instance().chunkThumbnailUpdated( pChunk );

	changedChunk( pChunk, false, false );
}


/**
 * Save changed terrain and chunk files, without recalculating anything
 */
bool WorldManager::saveChangedFiles( SuperModelProgressDisplay* progress )
{
	BW_GUARD;

	DataSectionCache::instance()->clear();
	DataSectionCensus::clear();
	bool errors = false;

	SyncMode chunkStopper;

    PanelManager::instance().onBeginSave();

	// Rebuild missing terrain texture LODs.  Complain if any cannot be
	// rebuilt, do all of them, add to the list of changed chunks and show the
	// progress bar when doing this.
	drawMissingTextureLODs(true, true, true, true);

	// Save terrain chunks
	ProgressTask terrainTask( progress, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVE_TERRAIN"), float(changedTerrainBlocks_.size()) );

    std::set<Terrain::EditorBaseTerrainBlockPtr>::iterator it = 
		changedTerrainBlocks_.begin();
	std::set<Terrain::EditorBaseTerrainBlockPtr>::iterator end = 
		changedTerrainBlocks_.end();

	while ( it != end )
	{
		terrainTask.step();

		Terrain::EditorBaseTerrainBlockPtr spBlock = *it++;

		Commentary::instance().addMsg( spBlock->resourceName(), 0 );

		const std::string& resID = spBlock->resourceName();

		// Before we save, what if the resource exists in a binary section?
		//   e.g. blahblah.cdata/terrain
		// Strip off anything after the last dot.			
		int posDot = resID.find_last_of( '.' );
		int posSls = resID.substr( posDot, resID.size() ).find_first_of( "/" );
		if ( (posDot>=0) && (posSls>0) )
		{
			std::string filename = spBlock->resourceName().substr(0, posDot + 6);
			bool add = !BWResource::fileExists( filename );
			if (!this->saveAndAddChunk( filename, spBlock, add, true ))
			{
				errors = true;
			}
		}
		else
		{
			bool add = !BWResource::fileExists( spBlock->resourceName() );
			//legacy .terrain file suport
			if (!this->saveAndAddChunk(
					spBlock->resourceName(), spBlock, add, true ))
				errors = true;
		}
	}

	// Find chunks touched by VLO editing that haven't been loaded
	std::set<std::string> touchedColumns;
	VLOManager::instance()->getDirtyColumns( touchedColumns );

	// Remove touched chunks that are already loaded.
	for( std::set<std::string>::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); )
	{
		// if the chunk is already loaded, then it will take care of itself,
		// marking itself as changed if it needs to save some VLO info.
		Chunk* chunk = ChunkManager::instance().findChunkByName( *it, geometryMapping(), false );
		if ( chunk && chunk->isBound() )
			it = touchedColumns.erase( it );
		else
			++it;
	}

	ProgressTask chunkTask(
					progress,
					LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVE_SCENE_DATA"),
					float( changedChunks_.size() + touchedColumns.size() ) );

	// Load unloaded chunks found to have VLOs. Loading should update the VLO
	// references in the chunks, so loaded chunks are saved.
	for( std::set<std::string>::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); ++it )
	{
		chunkTask.step();

		// Load it, and record the chunk(s) loaded (could be loading shells as
		// well).
		recordLoadedChunksStart();

		ChunkManager::instance().loadChunkNow( *it, geometryMapping() );
		// Loop until it fully loads, to make sure shells load too
		while ( ChunkManager::instance().checkLoadingChunks() )
		{
			ChunkManager::instance().tick(0);
			Sleep(0);
		}

		std::set<Chunk*> loadedChunks = recordLoadedChunksStop();

		// Now, save the recently loaded chunks that were marked as dirty.
		for( std::set<Chunk*>::iterator it2 = loadedChunks.begin();
			it2 != loadedChunks.end(); ++it2 )
		{
			Chunk* chunk = *it2;
			this->workingChunk( chunk, true );

			// save it
			if ( changedChunks_.find( chunk ) != changedChunks_.end() )
			{
				if ( !EditorChunkCache::instance( *chunk ).edSave() )
				{
					errors = true;
				}
			}

			// remove it from the changed chunks list
			std::set<Chunk*>::iterator itDelete = changedChunks_.find( chunk );
			if ( itDelete != changedChunks_.end() )
				changedChunks_.erase( itDelete );

			chunk->unbind( false );
			chunk->unload();
		}
	}

	// Update the "original VLO bounds" list with the new transform, and mark
	// VLOs that were deleted for later cleanup.
	VLOManager::instance()->postSave();

	// Save object chunks.
	std::set<Chunk*>::iterator cit = changedChunks_.begin();
	std::set<Chunk*>::iterator cend = changedChunks_.end();

	while ( cit != cend )
	{
		chunkTask.step();

		Chunk * pChunk = *cit++;
		this->workingChunk( pChunk, true );

		Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVING_CDATA",
			pChunk->identifier() ), 0 );

		if ( !EditorChunkCache::instance( *pChunk ).edSave() )
			errors = true;
	}

	VeryLargeObject::saveAll();

	std::string space = this->getCurrentSpace();
    std::string spaceSettingsFile = space + "/" + SPACE_SETTING_FILE_NAME;
	DataSectionPtr	pDS = BWResource::openSection( spaceSettingsFile  );	
	if (pDS)
	{		
        if (romp_ != NULL)
            romp_->enviroMinder().save(pDS);            
		DataSectionPtr terrainSection = pDS->openSection("terrain");
		if (terrainSection != NULL)
			pTerrainSettings()->save(terrainSection);
		// Personality script.  Allow scripts to save any files they've modified,
		// and allow them to save the space.settings file too.
		PyObject* tuple = PyTuple_New(2);		
		PyTuple_SetItem( tuple, 0, PyString_FromString(space.c_str()) );
		PyTuple_SetItem( tuple, 1, new PyDataSection(pDS) );
		Script::call(
			PyObject_GetAttrString( Personality::instance(), "onSave" ),
			tuple,
			"Personality onSave: ",
			true );
		pDS->save( spaceSettingsFile );
        changedEnvironment_ = false;
	}

	// Thumbnail data
	if ( !changedThumbnailChunks_.empty() )
	{	
		ProgressTask thumbnailTask( progress, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVE_THUMBNAIL_DATA"), float(changedThumbnailChunks_.size()) );
		//Before clearing the changed object chunks, we go through the dirty thumbnaillist.
		//if the thumbnail is changed but the chunk was not, then we just save the .cdata.
		//
		//if the thumbnail and chunk is dirty, then the chunk would have already saved
		//the .cdata and thus the thumbnail.
		std::set<Chunk*>::iterator tit = changedThumbnailChunks_.begin();
		std::set<Chunk*>::iterator tend = changedThumbnailChunks_.end();

		while ( tit != tend )
		{
			thumbnailTask.step();

			Chunk * pChunk = *tit++;

			this->workingChunk( pChunk, true );

			if ( std::find( changedChunks_.begin(), changedChunks_.end(), pChunk ) == changedChunks_.end() )
			{
				//only need to save the .cdata, since the chunk itself has not changed
				//(according to the std::find we just did)
				if ( !EditorChunkCache::instance( *pChunk ).edSaveCData() )
				{
					errors = true;
					Commentary::instance().addMsg(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/ERROR_SAVING_CDATA",
							pChunk->identifier() ), Commentary::CRITICAL );
				}
				else
					Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVING_CDATA",
						pChunk->identifier() ), 0 );
			}			
		}
	}

	StationGraph::saveAll();
	SpaceMap::instance().save();

	if ( !errors )
	{
		// clear dirty lists only if no errors were generated
		changedTerrainBlocks_.clear();
		changedThumbnailChunks_.clear();
		changedChunks_.clear();

		VLOManager::instance()->clearDirtyList();
	}

    PanelManager::instance().onEndSave();

	return !errors;
}


/**
 *	This method is called whenever a chunk is loaded (called from Chunk::bind)
 */
void WorldManager::checkUpToDate( Chunk * pChunk )
{
	BW_GUARD;

	MF_ASSERT( pChunk );

	VLOManager::instance()->updateChunkReferences( pChunk );

	if ( recordLoadedChunks_ )
		loadedChunks_.insert( pChunk );

	// Inform the link manager that the chunk has been loaded so that it can
	// update any relevant links
	EditorChunkLinkManager::instance().chunkLoaded(pChunk);

	// broadcast that the chunk is loaded:
	if (pChunk->isOutsideChunk())
	{
		int16 x, z;
		geometryMapping()->gridFromChunkName( pChunk->identifier(), x, z);
		chunkWatcher_->state(x, z, ChunkWatcher::LOADED);
		chunkWatcher_->canUnload(x, z, pChunk->removable());
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onChangedChunkState( x, z );
		}
	}

	if (chunkWritable( pChunk, false ))
	{
		EditorChunkCache& cache = EditorChunkCache::instance( *pChunk );
		std::string name = pChunk->identifier();

		if (!cache.lightingUpdated())
		{
			dirtyLighting( pChunk );
		}

		if (!cache.shadowUpdated())
		{
			dirtyTerrainShadows( pChunk );
		}

		if (!cache.terrainLODUpdated())
		{
			dirtyTerrainLOD( pChunk );
		}

		if (!cache.thumbnailUpdated())
		{
			dirtyThumbnail( pChunk, true );
		}

		cleanChunkList_->update( pChunk );
	}

	if( !terrainInfoClean_ && pChunk->isOutsideChunk() )
	{
		EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
			ChunkTerrainCache::instance( *pChunk ).pTerrain() );
		if( pEct )
		{
			Terrain::TerrainHeightMap &thm	= pEct->block().heightMap();
			terrainInfo_.poleSpacingX		= thm.spacingX();
			terrainInfo_.poleSpacingY		= thm.spacingZ();
			terrainInfo_.widthM				= GRID_RESOLUTION;
	        terrainInfo_.heightM			= GRID_RESOLUTION;
			terrainInfo_.polesWidth			= thm.polesWidth();
			terrainInfo_.polesHeight		= thm.polesHeight();
			terrainInfo_.visOffsX			= thm.xVisibleOffset();
			terrainInfo_.visOffsY			= thm.zVisibleOffset();
			terrainInfo_.blockWidth			= thm.blocksWidth();
			terrainInfo_.blockHeight		= thm.blocksHeight();
			terrainInfoClean_ = true;
		}
	}
}


/**
 *	This gets called when a chunk is tossed.
 */
void WorldManager::onUnloadChunk(Chunk *pChunk)
{
	BW_GUARD;

	// We only care about outside chunks:
	if (!pChunk->isOutsideChunk())
		return;

	// Inform the link manager that the chunk is being tossed so that it can
	// update any relevant links
	EditorChunkLinkManager::instance().chunkTossed(pChunk);

	if (!pChunk->removable())
	{
		int16 x, z;
		geometryMapping()->gridFromChunkName( pChunk->identifier(), x, z);
		chunkWatcher_->canUnload(x, z, pChunk->removable());
		if (PanelManager::pInstance())
		{
			PanelManager::instance().onChangedChunkState( x, z );
		}
		return;
	}

	// The outdoor chunk is really being unloaded:
	int16 x, z;
	geometryMapping()->gridFromChunkName( pChunk->identifier(), x, z);
	chunkWatcher_->state(x, z, ChunkWatcher::UNLOADED);
	if (PanelManager::pInstance())
	{
		PanelManager::instance().onChangedChunkState( x, z );
	}

	dirtyChunkLists_.clean( pChunk->identifier() );
}

namespace
{
	std::string getPythonStackTrace()
	{
		BW_GUARD;

		std::string stack = "";

		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch( &ptype, &pvalue, &ptraceback);

		if (ptraceback != NULL)
		{
			// use traceback.format_exception 
			// to get stacktrace as a string
			PyObject* pModule = PyImport_ImportModule( "traceback" );
			if (pModule != NULL)
			{
				PyObject * formatFunction = PyObject_GetAttrString( pModule, "format_exception" );

				if (formatFunction != NULL)
				{
					PyObject * list = Script::ask( 
							formatFunction, Py_BuildValue( "(OOO)", ptype, pvalue, ptraceback ), 
							"WorldEditor", false, false );

					if (list != NULL)
					{
						for (int i=0; i < PyList_Size( list ); ++i)
						{
							stack += PyString_AS_STRING( PyList_GetItem( list, i ) );
						}
						Py_DECREF( list );
					}
				}
				Py_DECREF( pModule );
			}
		}

		// restore error so that PyErr_Print still sends
		// traceback to console (PyErr_Fetch clears it)
		PyErr_Restore( ptype, pvalue, ptraceback);

		return stack;
	}



}

/**
 * Write the current set (loaded and non loaded) of clean chunks out.
 */
bool WorldManager::writeCleanList()
{
	BW_GUARD;

	cleanChunkList_->save();
	return true;
}

bool WorldManager::checkForReadOnly() const
{
	BW_GUARD;

	bool readOnly = Options::getOptionInt("objects/readOnlyMode", 0) != 0;
	if( readOnly )
	{
		MessageBox( *WorldEditorApp::instance().mainWnd(),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/READ_ONLY_WARN_TEXT"),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/READ_ONLY_WARN_TITLE"),
			MB_OK );
	}
	return readOnly;
}
/**
 * Only save changed chunk and terrain data, don't recalculate anything.
 *
 * Dirty lists are persisted to disk.
 */
void WorldManager::quickSave()
{
	BW_GUARD;

	saveFailed_ = false;

	if( checkForReadOnly() )
		return;

	bool errors = false;

	Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/QUICK_SAVING"), 1 );

	if ( !saveChangedFiles( progressBar_ ) )
		errors = true;

	if ( !writeCleanList() )
		errors = true;

	if ( errors )
	{
		MessageBox( *WorldEditorApp::instance().mainWnd(),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/QUICK_SAVE_ERROR_TEXT"),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/QUICK_SAVE_ERROR_TITLE"),
			MB_ICONERROR );
		addError( NULL, NULL, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/QUICK_SAVE_ERROR_TEXT").c_str() );
	}
	else
		Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/QUICK_SAVE_ERROR_COMPLETE"), 1 );

	MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();
	mainFrame->InvalidateRect( NULL );
	mainFrame->UpdateWindow();

	saveFailed_ = errors;
}


bool WorldManager::EnsureNeighbourChunkLoaded( Chunk* chunk, int level )
{
	BW_GUARD;

	// Map used to prevent recursing into already visited chunks.
	VisitedChunksSet visited;

	return ensureNeighbourChunkLoadedInternal( chunk, level, visited );
}


bool WorldManager::EnsureNeighbourChunkLoadedForShadow( Chunk* chunk )
{
	BW_GUARD;

	int16 gridX, gridZ;
	if( !geometryMapping()->gridFromChunkName( chunk->identifier(), gridX, gridZ ) )
		return true;// assume
	int16 dist = (int16)( ( MAX_TERRAIN_SHADOW_RANGE + 1.f ) / GRID_RESOLUTION );
	for( int16 z = gridZ - 1; z <= gridZ + 1; ++z )
		for( int16 x = gridX - dist; x <= gridX + dist; ++x )
		{
			std::string chunkName;
			chunkID( chunkName, x, z );

			if( chunkName.empty() )
				continue;

			Chunk* c = ChunkManager::instance().findChunkByName( chunkName, geometryMapping() );

			if( !c )
				continue;
			if( !c->loaded() )
				return false;
		}
	return true;
}


void WorldManager::loadNeighbourChunk( Chunk* chunk, int level )
{
	BW_GUARD;

	std::map<Chunk*,int> chunkDepths;
	loadNeighbourChunkInternal( chunk, level, chunkDepths );
}


void WorldManager::loadChunkForShadow( Chunk* chunk )
{
	BW_GUARD;

	int16 gridX, gridZ;
	if( !geometryMapping()->gridFromChunkName( chunk->identifier(), gridX, gridZ ) )
		return;
	int16 dist = (int16)( ( MAX_TERRAIN_SHADOW_RANGE + 1.f ) / GRID_RESOLUTION );
	for( int16 z = gridZ - 1; z <= gridZ + 1; ++z )
		for( int16 x = gridX - dist; x <= gridX + dist; ++x )
		{
			std::string chunkName;
			chunkID( chunkName, x, z );

			if( chunkName.empty() )
				continue;

			Chunk* c = ChunkManager::instance().findChunkByName( chunkName, geometryMapping() );

			if( c && c->loaded() )
				continue;

			ChunkManager::instance().loadChunkNow( chunkName, geometryMapping() );
		}
	ChunkManager::instance().checkLoadingChunks();
}

bool WorldManager::saveChunk( const std::string& chunkName, ProgressTask* task )
{
	BW_GUARD;

	return this->saveChunk( ChunkManager::instance().findChunkByName( chunkName, geometryMapping() ), task );
}

bool WorldManager::saveChunk( Chunk* chunk, ProgressTask* task )
{
	BW_GUARD;

	isSaving_ = true;
	processMessages();

	if (!chunkWritable( chunk, false ))
	{
		isSaving_ = false;
		return false;
	}
	if (!chunk->loaded())
	{
		// Make sure all cached chunks items are removed to free memory.
		AmortiseChunkItemDelete::instance().purge();

		ChunkManager::instance().loadChunkNow( chunk );
		ChunkManager::instance().checkLoadingChunks();
	}
	if (!chunkWritable( chunk, false ))
	{
		isSaving_ = false;
		return false;
	}

	ChunkManager::instance().cameraSpace()->focus( chunk->centre() );
	this->loadNeighbourChunk( chunk, chunk->space()->staticLightingPortalDepth() );
	ChunkManager::instance().cameraSpace()->focus( chunk->centre() );

	// load neighbouring chunks
	if (chunk->isOutsideChunk())
	{
		// Make sure all cached chunks items are removed to free memory.
		AmortiseChunkItemDelete::instance().purge();

		loadChunkForShadow( chunk );
	}

	ChunkManager::instance().cameraSpace()->focus( chunk->centre() );
	EditorChunkCache& chunkCache = EditorChunkCache::instance( *chunk );

	if (chunk->isBound())
	{
		if (!chunkCache.edIsDeleted())
		{
			this->workingChunk( chunk, true );

			EditorChunkTerrain * pEct = static_cast< EditorChunkTerrain * >(
				ChunkTerrainCache::instance( *chunk ).pTerrain());

			if (!chunkCache.lightingUpdated())
			{
				Commentary::instance().addMsg( 
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CALC_LIGHTING", chunk->identifier() ), 0 );
				chunkCache.edRecalculateLighting( task );
			}

			if (!chunkCache.shadowUpdated())
			{
				if (pEct)
				{
					Commentary::instance().addMsg( 
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CALC_SHADOW", chunk->identifier() ), 0 );
					pEct->calculateShadows( false, task );
				}
			}

			if (!chunkCache.terrainLODUpdated())
			{
				this->drawMissingTextureLOD( chunk, false );
			}

			if (!chunkCache.thumbnailUpdated())
			{
				Commentary::instance().addMsg( 
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CALC_THUMBNAIL", chunk->identifier() ), 0 );
				EditorChunkCache::instance( *chunk ).calculateThumbnail();
			}

			if (chunkCache.terrainLODUpdated())
			{
				dirtyChunkLists_[ TERRAIN_LOD ].clean( chunk->identifier() );
			}

			dirtyChunkLists_[ LIGHTING ].clean( chunk->identifier() );
			dirtyChunkLists_[ SHADOW ].clean( chunk->identifier() );
			dirtyChunkLists_[ THUMBNAIL ].clean( chunk->identifier() );

			this->workingChunk( NULL, false );
			isSaving_ = false;
			return true;
		}
		else
		{
			isSaving_ = false;
			return false;
		}
	}
	else
	{
		ERROR_MSG( "chunk %s is marked as dirty, but isn't bound!\n", 
			chunk->identifier().c_str() );
		isSaving_ = false;
		return false;
	}
}

struct ChunkComparor
{
public:
	bool operator()( const std::string& name1, const std::string& name2 ) const
	{
		BW_GUARD;

		int16 x1, x2, z1, z2;
		bool isOutside1 = WorldManager::instance().geometryMapping()->gridFromChunkName( name1, x1, z1 );
		bool isOutside2 = WorldManager::instance().geometryMapping()->gridFromChunkName( name2, x2, z2 );
		if( isOutside1 && isOutside2 )
		{
			static const int GRID = Options::getOptionInt( "fullSave/stripeSize", 40 );
			int xGrid1 = ( x1 - WorldManager::instance().geometryMapping()->minGridX() ) / GRID;
			int xGrid2 = ( x2 - WorldManager::instance().geometryMapping()->minGridX() ) / GRID;
			if( xGrid1 < xGrid2 )
				return true;
			if( xGrid1 > xGrid2 )
				return false;
			if( z1 < z2 )
				return true;
			if( z1 == z2 )
				return x1 < x2;
			return false;
		}
		else if( isOutside1 && !isOutside2 )
			return true;
		else if( !isOutside1 && isOutside2 )
			return false;
		return name1 < name2;
	}
};


typedef std::set<std::string, ChunkComparor> DirtyChunkSet;

void updateDirtyChunks( DirtyChunkSet& loaded, DirtyChunkSet& nonLoaded, const DirtyChunkList& dcl )
{
	for (DirtyChunkList::const_iterator iter = dcl.begin();\
		iter != dcl.end(); ++iter)
	{
		loaded.insert( iter->second->identifier() );

		if (nonLoaded.find( iter->second->identifier() ) != nonLoaded.end())
		{
			nonLoaded.erase( iter->second->identifier() );
		}
	}
}


void updateDirtyChunks( DirtyChunkSet& loaded, DirtyChunkSet& nonLoaded, const DirtyChunkLists& dcl )
{
	updateDirtyChunks( loaded, nonLoaded, dcl[ LIGHTING ] );
	updateDirtyChunks( loaded, nonLoaded, dcl[ SHADOW ] );
	updateDirtyChunks( loaded, nonLoaded, dcl[ THUMBNAIL ] );
	updateDirtyChunks( loaded, nonLoaded, dcl[ TERRAIN_LOD ] );
}


/** Save everything, and make sure all dirty data (static lighting, terrain shadows )is up to date */
void WorldManager::save( const std::set<std::string>* chunkToSave/*= NULL*/, bool recalcOnly/*= false*/ )
{
	BW_GUARD;

	class ProgressBarGuard
	{
		WorldEditorProgressBar* savedProgressBar_;
		WorldEditorProgressBar*& guardedProgressBar_;
	public:
		ProgressBarGuard( WorldEditorProgressBar*& progressBar )
			: guardedProgressBar_( progressBar )
		{
			savedProgressBar_ = guardedProgressBar_;
		}
		~ProgressBarGuard()
		{
			guardedProgressBar_ = savedProgressBar_;
		}
	}
	progressBarGuard( progressBar_ );

	WorldEditorProgressBar* progressBar = progressBar_;
	progressBar_ = NULL;

	saveFailed_ = false;
	inEscapableProcess_ = true;

	if( checkForReadOnly() )
		return;

	if (Options::getOptionBool( "fullSave/warnUndoOnProcessData", true ))
	{
		// Warn the user that Process Data might clear the undo redo history.
		UndoWarnDlg undoWarn;
		INT_PTR result = undoWarn.DoModal();
		if (result == IDOK)
		{
			// Continue even if the Undo/Redo history is lost.
			Options::setOptionBool( "fullSave/warnUndoOnProcessData", !undoWarn.dontRepeat() );
		}
		else if (result == IDCANCEL)
		{
			// User doesn't want to lose the undo/redo stack.
			return;
		}
		else
		{
			// Something else happened, this is probably a coding error.
			ERROR_MSG( "Failed to display the UndoWarnDlg on save.\n" );
		}
	}

	bool errors = false;

	stopBackgroundCalculation();

	EditorChunkOverlapper::drawList.clear();

	ChunkManager::instance().switchToSyncMode( true );

	if( !recalcOnly )
		Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVING"), 1 );

	DirtyChunkSet dirtyChunks;
	DirtyChunkSet unloadedChunks;

	if( chunkToSave )
	{
		dirtyChunks.insert( chunkToSave->begin(), chunkToSave->end() );
	}
	else
	{
		GeometryMapping* dirMap = geometryMapping();

		ChunkSet cs = Utilities::gatherChunks( dirMap );

		for (ChunkSet::iterator iter = cs.begin(); iter != cs.end(); ++iter)
		{
			Chunk* chunk = ChunkManager::instance().findChunkByName(
				*iter, dirMap, false );

			if (!chunk || !chunk->isBound())
			{
				unloadedChunks.insert( *iter );
			}
		}

		updateDirtyChunks( dirtyChunks, unloadedChunks, dirtyChunkLists_ );
	}

	{
		std::string syncCleanList = LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SYNC_CLEAN_LIST");
		ProgressTask syncTask( progressBar, syncCleanList, 1.f );

		cleanChunkList_->sync( &syncTask );

		while (!unloadedChunks.empty() &&
			cleanChunkList_->isClean( *unloadedChunks.begin() ))
		{
			unloadedChunks.erase( unloadedChunks.begin() );
		}
	}

	std::string recalcShadow = LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RECALC_SHADOW");
	float taskTotal = float( dirtyChunks.size() + unloadedChunks.size() );
	float taskProgress = 0.f;
	ProgressTask savingTask( progressBar, recalcShadow, taskTotal );
	progressBar->escapable( true );

	int savedChunk = 0;

	// Use lowest texture quality to save memory during process data
	Moo::GraphicsSetting::GraphicsSettingPtr pTextureQuality = Moo::GraphicsSetting::getFromLabel( "TEXTURE_QUALITY" );
	int oldTextureQuality = 0;
	if (pTextureQuality)
	{
		oldTextureQuality = pTextureQuality->activeOption();
		pTextureQuality->selectOption( 2, true );
	}

	bool escape = false;
	while( !dirtyChunks.empty() || !unloadedChunks.empty() )
	{
		if (taskTotal - dirtyChunks.size() - unloadedChunks.size() > taskProgress)
		{
			taskProgress = taskTotal - dirtyChunks.size() - unloadedChunks.size();
			savingTask.set( taskProgress );
		}

		if (dirtyChunks.empty())
		{
			int loadedChunks = 0;
			while (!unloadedChunks.empty() && dirtyChunks.empty())
			{
				if (taskTotal - dirtyChunks.size() - unloadedChunks.size() > taskProgress)
				{
					taskProgress = taskTotal - dirtyChunks.size() - unloadedChunks.size();
					savingTask.set( taskProgress );
					WorldManager::processMessages();
				}

				++loadedChunks;
				bool memoryIsLow = isMemoryLow( true /* test now */ );

				if (memoryIsLow || loadedChunks >= 100)
				{
					loadedChunks = 0;

					if (memoryIsLow)
					{
						UndoRedo::instance().clear();
						INFO_MSG( "Process Data: Memory is running low, clearing the Undo/Redo stack.\n" );
					}

					unloadChunks();
				}

				if ( !inEscapableProcess_  || escapePressed() ) 
				{
					escape = true;
					break;
				}

				std::string chunkID = *unloadedChunks.begin();
				std::string chunkFileName =
					geometryMapping()->path() + chunkID + ".chunk";

				unloadedChunks.erase( unloadedChunks.begin() );

				if (!chunkWritable( chunkID, false ))
				{
					continue;
				}

				DataSectionPtr ds = BWResource::openSection( chunkFileName );

				if (ds)
				{
					ChunkManager::instance().loadChunkNow( chunkID, geometryMapping() );
					ChunkManager::instance().checkLoadingChunks();

					updateDirtyChunks( dirtyChunks, unloadedChunks, dirtyChunkLists_ );

					Chunk* chunk = ChunkManager::instance().findChunkByName(
						chunkID, geometryMapping() );

					EditorChunkCache& cache = EditorChunkCache::instance( *chunk );

					if (!cache.shadowUpdated() || !cache.lightingUpdated() ||
						!cache.thumbnailUpdated() || !cache.terrainLODUpdated())
					{
						dirtyChunks.insert( chunkID );
						changedChunk( chunk, false, false );
					}
				}
			}
		}

		if (dirtyChunks.empty())
		{
			break;
		}

		if ( !inEscapableProcess_  || escapePressed() ) 
		{
			escape = true;
			break;
		}

		if (!chunkWritable( *dirtyChunks.begin(), false ) ||
			!BWResource::fileExists( getCurrentSpace() + '/' + (*dirtyChunks.begin()) + ".chunk" ))
		{
			dirtyChunks.erase( dirtyChunks.begin() );
			continue;
		}

		// Calculate lighting, shadows, and thumbnail for the chunk
		bool chunkSaved = saveChunk( *dirtyChunks.begin(), &savingTask );

		if (unloadedChunks.find( *dirtyChunks.begin() ) != unloadedChunks.end())
		{
			unloadedChunks.erase( *dirtyChunks.begin() );
		}

		dirtyChunks.erase( dirtyChunks.begin() );

		if( chunkSaved )
		{
			++savedChunk;

			if( !chunkToSave )
			{
				updateDirtyChunks( dirtyChunks, unloadedChunks, dirtyChunkLists_ );
			}

			if( recalcOnly )
				continue;

			bool memoryIsLow = isMemoryLow( true /* test now */ );

			if (memoryIsLow || savedChunk >= 100)
			{
				savedChunk = 0;
				if ( !saveChangedFiles( NULL ) )
					errors = true;
				if ( !writeCleanList() )
					errors = true;

				// Clear working chunk, otherwise it gets marked as unremovable
				this->workingChunk( NULL, false ); 

				if (memoryIsLow)
				{
					// Make sure we release all saved chunks by clearing the
					// undo-redo history.
					UndoRedo::instance().clear();
					INFO_MSG( "Process Data: Memory is running low, clearing the Undo/Redo stack.\n" );
				}
				unloadChunks();
			}

			writeStatus();
			((MainFrame *)WorldEditorApp::instance().mainWnd())->updateStatusBar( true );

			// Primitive count needs to be reset as App::updateFrame is not being called 
		}
	}

	progressBar->escapable( false ); 

	if ( !escape )
	{
		// Write out the current state of the non loaded dirty list
		if( !recalcOnly )
		{
			if ( !saveChangedFiles( NULL ) )
				errors = true;

			if ( !writeCleanList() )
				errors = true;

			// Get the project module to update the dirty chunks.
			ProjectModule::regenerateAllDirty();

			// Get the terrain height import/export module to save its height map.
			HeightModule::ensureHeightMapCalculated();

			if ( errors )
			{
				MessageBox( *WorldEditorApp::instance().mainWnd(),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/FULL_SAVE_ERROR_TEXT"),
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/FULL_SAVE_ERROR_TITLE"),
					MB_ICONERROR );
				addError( NULL, NULL, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/FULL_SAVE_ERROR_TEXT").c_str() );
			}
			else
			{
				Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVE_COMPLETE"), 1 );
			}
		}
		else if( errors )
		{
			Commentary::instance().addMsg( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RECALC_ERROR"), 1 );
		}
	}

	// restore texture quality
	if (pTextureQuality)
	{
		pTextureQuality->selectOption( oldTextureQuality, true );
	}

	ChunkManager::instance().switchToSyncMode( false );

	EditorChunkOverlapper::drawList.clear();


	MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();
	mainFrame->InvalidateRect( NULL );
	mainFrame->UpdateWindow();

	saveFailed_ = errors | escape;
	inEscapableProcess_ = false;
}


float WorldManager::farPlane() const
{
	BW_GUARD;

	Moo::Camera camera = Moo::rc().camera();
	return camera.farPlane();
}


void WorldManager::farPlane( float f )
{
	BW_GUARD;

	float farPlaneDistance = Math::clamp( 0.0f, f, getMaxFarPlane() );

	Moo::Camera camera = Moo::rc().camera();
	camera.farPlane( farPlaneDistance );
	Moo::rc().camera( camera );

	// mark only things within the far plane as candidates for loading
	ChunkManager::instance().autoSetPathConstraints( farPlaneDistance );
}

bool WorldManager::isItemSelected( ChunkItemPtr item ) const
{
	BW_GUARD;

	for (std::vector<ChunkItemPtr>::const_iterator it = selectedItems_.begin();
		it != selectedItems_.end();
		it++)
	{
		if ((*it) == item)
			return true;
	}

	return false;
}

bool WorldManager::isChunkSelected( Chunk * pChunk ) const
{
	BW_GUARD;

	for (std::vector<ChunkItemPtr>::const_iterator it = selectedItems_.begin();
		it != selectedItems_.end();
		it++)
	{
		if ((*it)->chunk() == pChunk && (*it)->isShellModel())
			return true;
	}

	return false;
}

bool WorldManager::isChunkSelected( bool forceCalculateNow /*= false*/ ) const
{
	BW_GUARD;

	static uint32 s_frameMark = 0;
	static bool s_ret = false;
	if (forceCalculateNow || s_frameMark != Moo::rc().frameTimestamp())
	{
		s_ret = false;
		s_frameMark = Moo::rc().frameTimestamp();
		for (std::vector<ChunkItemPtr>::const_iterator it = selectedItems_.begin();
			it != selectedItems_.end();	++it)
		{
			if ((*it)->isShellModel())
			{
				s_ret = true;
				break;
			}
		}
	}

	return s_ret;
}

bool WorldManager::isChunkSelectable( Chunk * pChunk ) const
{
	BW_GUARD;

	for (std::vector<ChunkItemPtr>::const_iterator it = selectedItems_.begin();
		it != selectedItems_.end();
		it++)
	{
		if (strcmp( (*it)->edClassName(), "ChunkVLO" ) == 0)
		{
			const EditorChunkVLO* vlo = (EditorChunkVLO*)it->getObject();

			if (vlo->object()->chunkItems().size() != 1)
			{
				continue;
			}
		}
		if ((*it)->chunk() == pChunk && !(*it)->isShellModel())
			return true;
	}

	return false;
}

bool WorldManager::isInPlayerPreviewMode() const
{
	return isInPlayerPreviewMode_;
}

void WorldManager::setPlayerPreviewMode( bool enable )
{
	BW_GUARD;

	if( enable )
	{
		GUI::ItemPtr hideOrtho = GUI::Manager::instance()( "/MainToolbar/Edit/ViewOrtho/HideOrthoMode" );
		if( hideOrtho && hideOrtho->update() )
			hideOrtho->act();
	}
	isInPlayerPreviewMode_ = enable;
}


bool WorldManager::touchAllChunks()
{
	BW_GUARD;

	std::string spacePath = WorldManager::instance().geometryMapping()->path();
	std::set<std::string> chunks = Utilities::gatherChunks( WorldManager::instance().geometryMapping() );

	for( std::set<std::string>::iterator iter = chunks.begin(); iter != chunks.end(); ++iter )
	{
		DataSectionPtr cdata = BWResource::openSection( spacePath + (*iter) + ".cdata" );
		bool flag = true;

		if( cdata )
		{
			EditorChunkCache::UpdateFlags uf( cdata );

			uf.lighting_ = uf.shadow_ = uf.terrainLOD_ = uf.thumbnail_ = 0;
			uf.save();

			DataSectionPtr navmeshSec = cdata->openSection( "navmeshDirty", true );
			navmeshSec->setBinary( new BinaryBlock( &flag, sizeof( flag ), "BinaryBlock/WorldEditor" ) );
			navmeshSec->setParent( cdata );

			cdata->save();

			navmeshSec->setParent( NULL );
		}
	}

	// let dirty chunk list rebuild next time when the space gets loaded
	WorldManager::instance().dirtyChunkLists_.clear();
	WorldManager::instance().writeCleanList();
	WorldManager::instance().chunksBeingEdited_.clear();
	WorldManager::instance().reloadAllChunks( false );

	return true;
}


/**
 *	This method sets or resets the "removable" flag for all
 *	chunks in memory, on the basis of whether or not they
 *	possess modified data.
 */
void WorldManager::markChunks()
{
	BW_GUARD;

	if( !EditorChunkCache::chunks_.empty() )
		getSelection();
	else
		selectedItems_.clear();

	for( std::set<Chunk*>::iterator iter = EditorChunkCache::chunks_.begin();
		iter != EditorChunkCache::chunks_.end(); ++iter )
		(*iter)->removable( true );

	if (workingChunk_ && !canUnloadChunk_)
	{
		workingChunk_->removable( false );
	}

	for( std::set<Chunk*>::iterator iter = changedChunks_.begin();
		iter != changedChunks_.end(); ++iter )
		(*iter)->removable( false );

	for( std::set<Chunk*>::iterator iter = changedThumbnailChunks_.begin();
		iter != changedThumbnailChunks_.end(); ++iter )
		(*iter)->removable( false );

	for( std::set<Chunk*>::iterator iter = thumbnailChunksLoading_.begin();
		iter != thumbnailChunksLoading_.end(); ++iter )
		(*iter)->removable( false );

	UndoRedo::instance().markChunk();

	for( std::vector<ChunkItemPtr>::iterator iter = selectedItems_.begin();
		iter != selectedItems_.end(); ++iter )
		if ( (*iter)->chunk() )
			(*iter)->chunk()->removable( false );
}

void WorldManager::unloadChunks()
{
	BW_GUARD;

	stopBackgroundCalculation();

	EditorChunkOverlapper::drawList.clear();

	ChunkManager::instance().switchToSyncMode( true );

	markChunks();

	std::set<Chunk*> chunks = EditorChunkCache::chunks_;
	for( std::set<Chunk*>::iterator iter = chunks.begin();
		iter != chunks.end(); ++iter )
	{
		Chunk* chunk = *iter;
		if (chunk->removable() && chunk->isBound())
		{
			chunk->unbind( false );
			chunk->unload();
			this->onUnloadChunk( chunk );
			// Make sure all the chunk's items are removed.
			AmortiseChunkItemDelete::instance().purge();
		}
	}

	ChunkManager::instance().switchToSyncMode( false );
}

void WorldManager::setSelection( const std::vector<ChunkItemPtr>& items, bool updateSelection )
{
	BW_GUARD;

	PyObject* pModule = PyImport_ImportModule( "WorldEditorDirector" );
	if (pModule != NULL)
	{
		PyObject* pScriptObject = PyObject_GetAttrString( pModule, "bd" );

		if (pScriptObject != NULL)
		{
			ChunkItemGroupPtr group = 
				ChunkItemGroupPtr(new ChunkItemGroup( items ), true);

			settingSelection_ = true;

			Script::call(
				PyObject_GetAttrString( pScriptObject, "setSelection" ),
				Py_BuildValue( "(Oi)",
					static_cast<PyObject *>(group.getObject()),
					(int) updateSelection ),
				"WorldEditor");

			settingSelection_ = false;

			if (!updateSelection)
			{
				// Note that this doesn't update snaps etc etc - it is assumed
				// that revealSelection will be called some time in the near
				// future, and thus this will get updated properly. This only
				// happens here so that a call to selectedItems() following
				// this will return what's expected.
				this->updateSelection( items );
			}
			Py_DECREF( pScriptObject );
		}
		Py_DECREF( pModule );
	}
}

void WorldManager::getSelection()
{
	BW_GUARD;

	PyObject* pModule = PyImport_ImportModule( "WorldEditorDirector" );
	if( pModule != NULL )
	{
		PyObject* pScriptObject = PyObject_GetAttrString( pModule, "bd" );

		if( pScriptObject != NULL )
		{
			ChunkItemGroup* cig = new ChunkItemGroup();
			Script::call(
				PyObject_GetAttrString( pScriptObject, "getSelection" ),
				Py_BuildValue( "(O)", static_cast<PyObject*>( cig ) ),
				"WorldEditor");

			updateSelection( cig->get() );

			Py_DecRef( cig );
			Py_DECREF( pScriptObject );
		}
		Py_DECREF( pModule );
	}
}

bool WorldManager::drawSelection() const
{
	return drawSelection_;
}


/**
 *	This method sets the current rendering state of WorldEditor. It also resets
 *	the list of registered selectable items.
 *
 *	@param drawingSelection Set to 'true' means that everything should render
 *							in marquee selection mode, 'false' means normal
 *							3D rendering.
 */
void WorldManager::drawSelection( bool drawingSelection )
{
	BW_GUARD;

	if (drawSelection_ != drawingSelection)
	{
		drawSelection_ = drawingSelection;
		if (drawSelection_)
		{
			// About to start draw selection, so clear the selection items
			drawSelectionItems_.clear();
		}
	}

	EditorChunkItem::drawSelection( drawingSelection );
}


/**
 *	This method is called by chunk items that wish to be selectable using the
 *	marquee selection, allowing WorldEditor to prepare the render states for
 *	the item and registering the item as selectable.
 *
 *	@param item		Chunk item to add to the list of selectable items.
 */
void WorldManager::registerDrawSelectionItem( EditorChunkItem* item )
{
	BW_GUARD;

	drawSelectionItems_.insert( item );

	// This render state change works for most chunk items, but in some cases,
	// like terrain, the actual rendering is delayed, so these objects might 
	// need to set this render state again before issuing the draw calls.
	Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, (DWORD)item );
}


/**
 *	This method verifies if a chunk item pointer is actually a valid selectable
 *	chunk item.
 *
 *	@param item		Chunk item pointer to verify its validity.
 *	@return			'true' if the item is an actual selectable chunk item.
 */
bool WorldManager::isDrawSelectionItemRegistered( EditorChunkItem* item ) const
{
	BW_GUARD;

	return drawSelectionItems_.find( item ) != drawSelectionItems_.end();
}


bool WorldManager::snapsEnabled() const
{
	BW_GUARD;

	return !!OptionsSnaps::snapsEnabled();
}

bool WorldManager::freeSnapsEnabled() const
{
	BW_GUARD;

	if (isChunkSelected())
		return false;

	return OptionsSnaps::placementMode() == 0;
}

bool WorldManager::terrainSnapsEnabled() const
{
	BW_GUARD;

	if (isChunkSelected())
		return false;

	return OptionsSnaps::placementMode() == 1;
}

bool WorldManager::obstacleSnapsEnabled() const
{
	BW_GUARD;

	if (isChunkSelected())
		return false;

	return OptionsSnaps::placementMode() == 2;
}

WorldManager::CoordMode WorldManager::getCoordMode() const
{
	BW_GUARD;

	std::string mode( OptionsSnaps::coordMode() );
	if(mode == "Local")
		return COORDMODE_OBJECT;
	if(mode == "View")
		return COORDMODE_VIEW;
	return COORDMODE_WORLD;
}

Vector3 WorldManager::movementSnaps() const
{
	BW_GUARD;

	Vector3 movementSnap( OptionsSnaps::movementSnaps() );

	// Don't snap in the y-direction if snaps and terrain locking are both 
    // enabled.
	if (snapsEnabled() && terrainSnapsEnabled())
    {
       movementSnap.y = 0.f;
    }

	return movementSnap;
}

float WorldManager::angleSnaps() const
{
	BW_GUARD;

	if (snapsEnabled())
		return Snap::satisfy( angleSnaps_, OptionsSnaps::angleSnaps() );
	else
		return angleSnaps_;
}

void WorldManager::calculateSnaps()
{
	BW_GUARD;

	angleSnaps_ = 0.f;
	movementDeltaSnaps_ = Vector3( 0.f, 0.f, 0.f );

	for (std::vector<ChunkItemPtr>::iterator it = selectedItems_.begin();
		it != selectedItems_.end();
		it++)
	{
		angleSnaps_ = Snap::satisfy( angleSnaps_, (*it)->edAngleSnaps() );
		Vector3 m = (*it)->edMovementDeltaSnaps();

		movementDeltaSnaps_.x = Snap::satisfy( movementDeltaSnaps_.x, m.x );
		movementDeltaSnaps_.y = Snap::satisfy( movementDeltaSnaps_.y, m.y );
		movementDeltaSnaps_.z = Snap::satisfy( movementDeltaSnaps_.z, m.z );
	}
}


int WorldManager::drawBSP() const
{
	BW_GUARD;

	static uint32 s_settingsMark_ = -16;
	static int drawBSP = 0;
	if (Moo::rc().frameTimestamp() != s_settingsMark_)
	{
		drawBSP = Options::getOptionInt( "drawBSP", 0 );
		s_settingsMark_ = Moo::rc().frameTimestamp();
	}
	return drawBSP;
}

void WorldManager::snapAngles( Matrix& v )
{
	BW_GUARD;

	if ( snapsEnabled() )
		Snap::angles( v, angleSnaps() );
}

SnapProvider::SnapMode WorldManager::snapMode( ) const
{
	BW_GUARD;

	return	terrainSnapsEnabled()	?	SNAPMODE_TERRAIN :
			obstacleSnapsEnabled()	?	SNAPMODE_OBSTACLE :
										SNAPMODE_XYZ ;
}

bool WorldManager::snapPosition( Vector3& v )
{
	BW_GUARD;

    Vector3 origPosition = v;
	if ( snapsEnabled() )
    	v = Snap::vector3( v, movementSnaps() );
	if ( terrainSnapsEnabled() )
		v = Snap::toGround( v );
	else if ( obstacleSnapsEnabled() )
	{
		Vector3 startPosition = Moo::rc().invView().applyToOrigin();
		if (selectedItems_.size() > 1)
			startPosition = v - (Moo::rc().invView().applyToOrigin().length() * worldRay());
        bool hitObstacle = false;
        Vector3 newV = Snap::toObstacle( startPosition, worldRay(), false,
			 getMaxFarPlane(), &hitObstacle );
        if (!hitObstacle)
        {
            v = origPosition;
            return false;
        }
        else
        {
            v = newV;
        }
	}
    return true;
}

Vector3 WorldManager::snapNormal( const Vector3& v )
{
	BW_GUARD;

	Vector3 result( 0, 1, 0 );// default for y axis, should never used
	if ( obstacleSnapsEnabled() )
	{
		Vector3 startPosition = Moo::rc().invView().applyToOrigin();
		if (selectedItems_.size() > 1)
			startPosition = v - (Moo::rc().invView().applyToOrigin().length() * worldRay());
		result = Snap::toObstacleNormal( startPosition, worldRay() );
	}
	return result;
}

void WorldManager::snapPositionDelta( Vector3& v )
{
	BW_GUARD;

	v = Snap::vector3( v, movementDeltaSnaps_ );
}

float WorldManager::angleSnapAmount()
{
	BW_GUARD;

	return angleSnaps();
}

void WorldManager::startSlowTask()
{
	BW_GUARD;

	SimpleMutexHolder smh( savedCursorMutex_ );
	++slowTaskCount_;

	if (slowTaskCount_ == 1)
	{
		savedCursor_ = cursor_;
	}

	if (MainThreadTracker::isCurrentThreadMain())
	{
		cursor_ = ::LoadCursor( NULL, IDC_WAIT );
	}
	else
	{
		cursor_ = ::LoadCursor( NULL, IDC_APPSTARTING );
	}

	setCursor();
}

void WorldManager::stopSlowTask()
{
	BW_GUARD;

	SimpleMutexHolder smh( savedCursorMutex_ );

	--slowTaskCount_;
	if (slowTaskCount_ == 0)
	{
		cursor_ = savedCursor_;
		savedCursor_ = NULL;
		setCursor();
	}
}

void WorldManager::addReadOnlyBlock( const Matrix& transform, Terrain::BaseTerrainBlockPtr pBlock )
{
	BW_GUARD;

	readOnlyTerrainBlocks_.push_back( BlockInPlace( transform, pBlock ) );
}


void WorldManager::setColourFog( DWORD colour )
{
	BW_GUARD;

	Moo::rc().fogColour( colour );
	Moo::rc().fogEnabled( true );

	Moo::rc().fogNear( -10000.f );
	Moo::rc().fogFar( 10000.f );
}


void WorldManager::setReadOnlyFog()
{
	BW_GUARD;

	setColourFog( 0x00AA0000 );
}


void WorldManager::setFrozenFog()
{
	BW_GUARD;

	setColourFog( 0x00AAAAAA );
}


bool WorldManager::isPointInWriteableChunk( const Vector3& pt ) const
{
	BW_GUARD;

	return EditorChunk::outsideChunkWriteable( pt );
}

bool WorldManager::isBoundingBoxInWriteableChunk( const BoundingBox& box, const Vector3& offset ) const
{
	BW_GUARD;

	return EditorChunk::outsideChunksWriteableInSpace( BoundingBox( box.minBounds() + offset, box.maxBounds() + offset ) );
}

/**
 *	Checks to see if the space is fully locked and editable, and if it's not
 *	it'll popup a warning and return false.
 *
 *	@return		true if the space is fully editable (locked), false, otherwise.
 */
bool WorldManager::warnSpaceNotLocked()
{
	BW_GUARD;

	if( connection().isAllLocked() )
		return true;

	// Some parts of the space are not locked, so show the warning.
	::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
		Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/WARN_NOT_LOCKED"),
		Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/WARN_NOT_LOCKED_CAPTION"),
		MB_OK | MB_ICONWARNING );

	return false;
}


void WorldManager::updateChunk( Chunk* pChunk )
{
	BW_GUARD;

	cleanChunkList_->update( pChunk );
}


void WorldManager::reloadAllChunks( bool askBeforeProceed )
{
	BW_GUARD;

	if( askBeforeProceed && !canClose( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RELOAD") ) )
		return;

	std::string space = currentSpace_;
	currentSpace_ = LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/EMPTY");
	MsgHandler::instance().removeAssetErrorMessages();
	changeSpace( space, true );
}

// -----------------------------------------------------------------------------
// Section: Error message handling
// -----------------------------------------------------------------------------

class LogFileIniter
{
public:
	LogFileIniter() :
		logFile_( NULL )
	{
		BW_GUARD;

		char dateStr [9];
	    char timeStr [9];
	    _strdate( dateStr );
	    _strtime( timeStr );

		static const int MAX_LOG_NAME = 8192;
		static wchar_t logName[ MAX_LOG_NAME + 1 ] = L"";
		if( logName[0] == 0 )
		{
			DWORD len = GetModuleFileName( NULL, logName, MAX_LOG_NAME );
			while( len && logName[ len - 1 ] != '.' )
				--len;
			wcscpy( logName + len, L"log" );
		}

		if( logName[0] != 0 )
		{
			logFile_ = new std::ofstream( logName, std::ios::app );

			*logFile_ << "\n/-----------------------------------------------"
				"-------------------------------------------\\\n";

			*logFile_ << "BigWorld World Editor " << BWVersion::versionString() <<
				" (compiled at " << aboutCompileTimeString << ")"
				"starting on " << dateStr << " " << timeStr << "\n\n";

			logFile_->flush();
		}

		//Catch any commentary messages
		Commentary::instance().logFile( logFile_ );

		//Instanciate the Message handler to catch BigWorld messages
		MsgHandler::instance().logFile( logFile_ );
	}

	~LogFileIniter()
	{
		BW_GUARD;

		Commentary::instance().logFile( NULL );
		MsgHandler::instance().logFile( NULL );
		MsgHandler::fini();
		delete logFile_;
	}
private:
	std::ostream* logFile_;
};
// creating statically to allow output from very early in the app's lifetime.
LogFileIniter logFileIniter;

SimpleMutex WorldManager::pendingMessagesMutex_;
WorldManager::StringVector WorldManager::pendingMessages_;

/**
 * Post all messages we've recorded since the last time this was called
 */
void WorldManager::postPendingErrorMessages()
{
	BW_GUARD;

	pendingMessagesMutex_.grab();
	StringVector::iterator i = pendingMessages_.begin();
	for (; i !=	pendingMessages_.end(); ++i)
	{
		Commentary::instance().addMsg( *i, Commentary::WARNING );
	}
	pendingMessages_.clear();
	pendingMessagesMutex_.give();
}

/**
 *	This static function implements the callback that will be called for each
 *	*_MSG.
 *
 *	This is thread safe. We only want to add the error as a commentary message
 *	from the main thread, thus the adding them to a vector. The actual posting
 *	is done in postPendingErrorMessages.
 */
bool WorldEditorDebugMessageCallback::handleMessage( int componentPriority,
		int messagePriority,
		const char * format, va_list argPtr )
{
	return WorldManager::messageHandler( componentPriority, messagePriority, format, argPtr );
}


/**
 *	This static function implements the callback that will be called when a
 *	critical msg occurs.
 */
void WorldEditorCriticalMessageCallback::handleCritical( const char * msg )
{
	WorldManager::criticalMessageOccurred( true );
}


bool WorldManager::messageHandler( int componentPriority,
		int messagePriority,
		const char * format, va_list argPtr )
{
	BW_GUARD;

	char buf[8192];
	_vsnprintf( buf, ARRAY_SIZE(buf), format, argPtr );
	buf[sizeof(buf)-1]=0;
	if (buf[ strlen(buf) - 1 ] == '\n')
	{
		buf[ strlen(buf) - 1 ] = '\0';
	}

	if ( DebugFilter::shouldAccept( componentPriority, messagePriority ) &&     
		messagePriority == MESSAGE_PRIORITY_ERROR)                          
	{
		bool isNewError	   = false;
		bool isPythonError = false;

		// make sure Python is initialised before a check
		if( Py_IsInitialized() )
			isPythonError = PyErr_Occurred() != NULL;

		if (isPythonError)
		{
			std::string stacktrace = getPythonStackTrace();
			if( &MsgHandler::instance() )
				isNewError = MsgHandler::instance().addAssetErrorMessage( 
					buf, NULL, NULL, stacktrace.c_str());
			else
				isNewError = true;

			strncat( buf, LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SEE_MSG_PANEL").c_str(), ARRAY_SIZE(buf) - strlen( buf ) );
		}

		if (isNewError) 
		{
			pendingMessagesMutex_.grab();
			pendingMessages_.push_back( buf );
			pendingMessagesMutex_.give();
		}
	}

	return false;
}

void WorldManager::addPrimGroupCount( Chunk* chunk, uint n )
{
	if (chunk == currentMonitoredChunk_)
		currentPrimGroupCount_ += n;
}

void WorldManager::refreshWeather()
{
	BW_GUARD;

	if( romp_ )
		romp_->update( 1.0, globalWeather_ );
}

void WorldManager::setTimeFromOptions()
{
	this->timeOfDay()->gameTime(
		float(
			Options::getOptionInt(
				"graphics/timeofday",
				12 * TIME_OF_DAY_MULTIPLIER /*noon*/ )
		) / float( TIME_OF_DAY_MULTIPLIER )
	);
}

void WorldManager::setStatusMessage( unsigned int index, const std::string& value )
{
	BW_GUARD;

	if( index >= statusMessages_.size() )
		statusMessages_.resize( index + 1 );
	statusMessages_[ index ] = value;
}

const std::string& WorldManager::getStatusMessage( unsigned int index ) const
{
	BW_GUARD;

	static std::string empty;
	if( index >= statusMessages_.size() )
		return empty;
	return statusMessages_[ index ];
}

void WorldManager::setCursor( HCURSOR cursor )
{
	BW_GUARD;

	SimpleMutexHolder smh( savedCursorMutex_ );

	if (savedCursor_ != NULL)
	{
		savedCursor_ = cursor;
	}
	else if( cursor_ != cursor )
	{
		cursor_ = cursor;
		setCursor();
	}
}

void WorldManager::resetCursor()
{
	BW_GUARD;

	static HCURSOR cursor = ::LoadCursor( NULL, IDC_ARROW );
	setCursor( cursor );
}

void WorldManager::setCursor()
{
	BW_GUARD;

	POINT mouse;
	GetCursorPos( &mouse );
	SetCursorPos( mouse.x, mouse.y + 1 );
	SetCursorPos( mouse.x, mouse.y );
}


void WorldManager::updateSelection( const std::vector< ChunkItemPtr > & items )
{
	BW_GUARD;

    selectedItems_.clear();
	for (std::vector< ChunkItemPtr >::const_iterator it = items.begin();
		it != items.end(); ++it)
	{
        if ((*it)->edCanAddSelection())
		{
            selectedItems_.push_back( *it );
			(*it)->edSelected();
		}
	}
}


unsigned int WorldManager::dirtyChunks() const
{
	BW_GUARD;

	return dirtyChunkLists_[ LIGHTING ].num() + dirtyChunkLists_[ SHADOW ].num();
}


unsigned int WorldManager::dirtyLODTextures() const
{
	BW_GUARD;

	return (unsigned int)dirtyChunkLists_[ TERRAIN_LOD ].num();
}


/**
 *	This decreases the LOD regeneration count by one.  If the count drops to
 *	zero then LODs can be regenerated.
 */
void WorldManager::startLODTextureRegen()
{
	--lodRegenCount_;
}


/**
 *	This increases the LOD regeneration count by one, and stops LOD 
 *	regeneration.
 */
void WorldManager::stopLODTextureRegen()
{
	++lodRegenCount_;
}


/**
 *  Tell the texture layers page to refresh (if visible) 
 *
 *	@param chunk		chunk to refresh
 */
void WorldManager::chunkTexturesPainted( Chunk *chunk, bool rebuiltLodTexture )
{
	BW_GUARD;

	if (chunk == NULL)
	{
		WARNING_MSG( "WorldManager::chunkTexturesPainted: chunk is NULL\n" );
	}

	std::string chunkID;
	if ( chunk != NULL )
		chunkID = chunk->identifier();

	PageChunkTexture::refresh( chunkID );

	if (!rebuiltLodTexture && chunk != NULL)
	{
		EditorChunkCache::instance( *chunk ).terrainLODUpdated( false );
		dirtyChunkLists_[ TERRAIN_LOD ].dirty( chunk );
	}
}


/**
 *  Show the context menu about textures in a chunk
 *
 *	@param chunk		chunk where the click was done
 */
void WorldManager::chunkTexturesContextMenu( Chunk* chunk )
{
	BW_GUARD;

	this->resetCursor();
	// force setting the cursor now
	SetCursor( this->cursor() );
	ShowCursor( TRUE );

	// If the button was released near where it was pressed, assume
	// its a right-click instead of a camera movement.
	PopupMenu menu;

	// Get the textures under the cursor and sort by strength:
	Vector3 cursorPos;
	TerrainTextureUtils::LayerVec layers;
	bool canEditProjections = false;
	if (PageTerrainTexture::instance() != NULL)
	{
		cursorPos = PageTerrainTexture::instance()->toolPos();
		PageTerrainTexture::instance()->layersAtPoint(cursorPos, layers);
		canEditProjections = PageTerrainTexture::instance()->canEditProjections();
	}
	std::sort(layers.begin(), layers.end());

	// build menu items
	const int toggleTrackCursorCmd = 1000;
	std::string trackCursor;
	if ( PageChunkTexture::trackCursor() )
	{	
		trackCursor = "##";
	}
	menu.addItem( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/TRACK_CURSOR", trackCursor ), toggleTrackCursorCmd );
	menu.addSeparator();

	const int chunkTexturesCmd = 2000;
	if ( chunk != NULL )
	{
		menu.addItem( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/MANAGE_TEXTURE"), chunkTexturesCmd );
	}

	const int clearSelectedCmd = 3000;
	if ( !PageChunkTexture::trackCursor() && !PageChunkTexture::chunk().empty() )
	{
		menu.addItem( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/DESELECT_CHUNK"), clearSelectedCmd );
	}
	menu.addSeparator();

	const int selectTextureCmd = 4000;
	if (chunk != NULL)
	{
		menu.startSubmenu
		( 
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SELECT_TEXTURE")
		);
		size_t maxLayers = std::min((size_t)1000, layers.size()); // clip to 1000 texture layers
		for (size_t i = 0; i < maxLayers; ++i)
		{
			// Do not include zero-strength layers.  Note we break instead of
			// continue because the layers are sorted by strength and so there
			// are no layers with any strength after the first one with a 
			// strength of zero.
			if (layers[i].strength_ == 0)
				break;
			std::wstring texName;
			bw_utf8tow( BWResource::getFilename(layers[i].textureName_), texName );
			menu.addItem(texName, selectTextureCmd + i);
		}
		menu.endSubmenu();
	}

	const int opacityCmd = 5000;
	if (chunk != NULL)
	{
		menu.startSubmenu
		( 
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/MATCH_OPACITY")
		);
		size_t maxLayers = std::min((size_t)1000, layers.size()); // clip to 1000 texture layers
		for (size_t i = 0; i < maxLayers; ++i)
		{
			// Do not include zero-strength layers.  Note we break instead of
			// continue because the layers are sorted by strength and so there
			// are no layers with any strength after the first one with a 
			// strength of zero.
			if (layers[i].strength_ == 0)
				break;
			std::wstring texName = 
				Localise
				(
					L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/MATCH_OPACITY_ENTRY", 
					BWResource::getFilename(layers[i].textureName_), 
					(int)(100.0f*layers[i].strength_/255.0f + 0.5f)
				);
			menu.addItem(texName, opacityCmd + i);
		}
		menu.endSubmenu();
	}

	const int selTexForMaskCmd = 6000;
	if (chunk != NULL)
	{
		menu.startSubmenu
		( 
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SELECT_TEXTURE_FOR_MASK")
		);
		size_t maxLayers = std::min((size_t)1000, layers.size()); // clip to 1000 texture layers
		std::set<std::string> addedTextures;
		for (size_t i = 0; i < maxLayers; ++i)
		{
			// Do not include zero-strength layers.  Note we break instead of
			// continue because the layers are sorted by strength and so there
			// are no layers with any strength after the first one with a 
			// strength of zero.
			if (layers[i].strength_ == 0)
				break;
			std::string texName = BWResource::getFilename(layers[i].textureName_);
			if (addedTextures.find(texName) == addedTextures.end())
			{
				addedTextures.insert(texName);
				std::wstring wtexName;
				bw_utf8tow( texName, wtexName );
				menu.addItem(wtexName, selTexForMaskCmd + i);
			}
		}
		menu.endSubmenu();
	}

	const int selTexAndProjForMaskCmd = 7000;
	if (chunk != NULL && canEditProjections)
	{
		menu.startSubmenu
		( 
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SELECT_TEXTURE_AND_PROJ_FOR_MASK")
		);
		size_t maxLayers = std::min((size_t)1000, layers.size()); // clip to 1000 texture layers
		for (size_t i = 0; i < maxLayers; ++i)
		{
			// Do not include zero-strength layers.  Note we break instead of
			// continue because the layers are sorted by strength and so there
			// are no layers with any strength after the first one with a 
			// strength of zero.
			if (layers[i].strength_ == 0)
				break;
			std::wstring texName;
			bw_utf8tow( BWResource::getFilename(layers[i].textureName_), texName );
			menu.addItem(texName, selTexAndProjForMaskCmd + i);
		}
		menu.endSubmenu();
	}

	const int editProjectionCmd = 8000;
	if (chunk != NULL && canEditProjections)
	{
		menu.startSubmenu
		( 
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/EDIT_PROJECTION_AND_SCALE")
		);
		size_t maxLayers = std::min((size_t)1000, layers.size()); // clip to 1000 texture layers
		for (size_t i = 0; i < maxLayers; ++i)
		{
			// Do not include zero-strength layers.  Note we break instead of
			// continue because the layers are sorted by strength and so there
			// are no layers with any strength after the first one with a 
			// strength of zero.
			if (layers[i].strength_ == 0)
				break;
			std::wstring texName;
			bw_utf8tow( BWResource::getFilename(layers[i].textureName_), texName );
			menu.addItem(texName, editProjectionCmd + i);
		}
		menu.endSubmenu();
	}


	// run the menu
	int result = menu.doModal( this->hwndGraphics() );

	// handle the results
	if (result == toggleTrackCursorCmd)
	{
		PageChunkTexture::trackCursor( !PageChunkTexture::trackCursor() );
	}
	else if (result == chunkTexturesCmd)
	{
		if ( chunk != NULL )
		{
			PageChunkTexture::trackCursor( false );
			PageChunkTexture::chunk( chunk->identifier(), true );
		}
	}
	else if (selectTextureCmd <= result && result < selectTextureCmd + 1000)
	{
		if ( chunk != NULL && PageTerrainTexture::instance() != NULL)
		{
			size_t idx      = result - selectTextureCmd;
			size_t layerIdx = layers[idx].layerIdx_;
			PageTerrainTexture::instance()->selectTextureAtPoint(cursorPos, layerIdx);
		}
	}
	else if (opacityCmd <= result && result < opacityCmd + 1000)
	{
		if ( chunk != NULL && PageTerrainTexture::instance() != NULL)
		{
			size_t idx      = result - opacityCmd;
			size_t layerIdx = layers[idx].layerIdx_;
			float opacity   = layers[idx].strength_/2.55f; // convert to %
			PageTerrainTexture::instance()->selectTextureAtPoint(cursorPos, layerIdx);
			PageTerrainTexture::instance()->setOpacity(opacity);
		}
	}
	else if (selTexForMaskCmd <= result && result < selTexForMaskCmd + 1000)
	{
			size_t idx      = result - selTexForMaskCmd;
			size_t layerIdx = layers[idx].layerIdx_;
			PageTerrainTexture::instance()->selectTextureMaskAtPoint(cursorPos, layerIdx, false);
	}
	else if (selTexAndProjForMaskCmd <= result && result < selTexAndProjForMaskCmd + 1000)
	{
			size_t idx      = result - selTexAndProjForMaskCmd;
			size_t layerIdx = layers[idx].layerIdx_;
			PageTerrainTexture::instance()->selectTextureMaskAtPoint(cursorPos, layerIdx, true);
	}
	else if (editProjectionCmd <= result && result < editProjectionCmd + 1000)
	{
		if ( chunk != NULL && PageTerrainTexture::instance() != NULL)
		{
			size_t idx      = result - editProjectionCmd;
			size_t layerIdx = layers[idx].layerIdx_;
			PageTerrainTexture::instance()->editProjectionAtPoint(cursorPos, layerIdx);
		}
	}
	else if (result == clearSelectedCmd)
	{
		PageChunkTexture::chunk( "" );
	}

	// restore previous cursor visibility state to whatever it was
	ShowCursor( FALSE );
}


/**
 *	Returns the terrain version used in the space by looking into the actual
 *  terrain blocks.
 */
uint32 WorldManager::getTerrainVersion()
{
	BW_GUARD;

	uint32 version = 0;
	GeometryMapping* dirMap = geometryMapping();
	for( int i = dirMap->minGridX(); i <= dirMap->maxGridX(); ++i )
	{
		for( int j = dirMap->minGridY(); j <= dirMap->maxGridY(); ++j )
		{
			std::string resName = dirMap->path() +
				dirMap->outsideChunkIdentifier( i, j ) +
				".cdata/terrain";
			version = Terrain::BaseTerrainBlock::terrainVersion( resName );
			if( version > 0 )
				break;
		}
		if( version > 0 )
			break;
	}
	return version;
}


/**
 *  Returns the a block that has terrain in the space.
 */
Terrain::BaseTerrainBlockPtr WorldManager::getTerrainBlock()
{
	BW_GUARD;

	Terrain::BaseTerrainBlockPtr block;
	GeometryMapping* dirMap = geometryMapping();
	for( int i = dirMap->minGridX(); i <= dirMap->maxGridX(); ++i )
	{
		for( int j = dirMap->minGridY(); j <= dirMap->maxGridY(); ++j )
		{
			std::string resName = dirMap->path() +
				dirMap->outsideChunkIdentifier( i, j ) +
				".cdata/terrain";
			block = Terrain::BaseTerrainBlock::loadBlock( resName,
				Matrix::identity, Vector3::zero(), pTerrainSettings() );
			if( block )
				break;
		}
		if( block )
			break;
	}
	return block;
}


/**
 *  Returns terrain parameters from a block with terrain in the space.
 */
const TerrainUtils::TerrainFormat& WorldManager::getTerrainInfo()
{
	BW_GUARD;

	if( !terrainInfoClean_ )
	{
		Terrain::BaseTerrainBlockPtr block = getTerrainBlock();
		MF_ASSERT( block );
		if( block )
		{
			Terrain::TerrainHeightMap &thm = block->heightMap();
			terrainInfo_.poleSpacingX  = thm.spacingX();
			terrainInfo_.poleSpacingY  = thm.spacingZ();
	        terrainInfo_.widthM        = GRID_RESOLUTION;
	        terrainInfo_.heightM       = GRID_RESOLUTION;
			terrainInfo_.polesWidth    = thm.polesWidth();
			terrainInfo_.polesHeight   = thm.polesHeight();
			terrainInfo_.visOffsX      = thm.xVisibleOffset();
			terrainInfo_.visOffsY      = thm.zVisibleOffset();
			terrainInfo_.blockWidth    = thm.blocksWidth();
			terrainInfo_.blockHeight   = thm.blocksHeight();
			terrainInfoClean_ = true;
		}
	}
	return terrainInfo_;
}


/**
 *	This resets any cached terrain information.  It is automatically reset when
 *	the space is changed, there is a conversion to newer terrain versions etc.
 */
void WorldManager::resetTerrainInfo()
{
	terrainInfoClean_ = false;
}

Terrain::TerrainSettingsPtr WorldManager::pTerrainSettings()
{
	return geometryMapping()->pSpace()->terrainSettings();
}


/**
 *	This method is called from the Scene Browser whenever the selection in it
 *	changes. The items in the "selection" list are guaranteed to be loaded only
 *	during the scope of this call.
 *
 *	@param selection	New selection from the SceneBrowser.
 */
void WorldManager::sceneBrowserSelChange(
									const SceneBrowser::ItemList & selection )
{
	BW_GUARD;

	std::vector< ChunkItemPtr > items;
	for (SceneBrowser::ItemList::const_iterator it = selection.begin();
		it != selection.end(); ++it)
	{
		items.push_back( (*it)->chunkItem() );
	}

	UndoRedo::instance().add(
							new SelectionOperation( selectedItems_, items ) );

	this->setSelection( items, true );

	UndoRedo::instance().barrier(
			LocaliseUTF8(
				L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SELECTION_CHANGE"),
			false );
}


/**
 * This method can clear any records of changes
 */
void WorldManager::forceClean()
{
	BW_GUARD;

	changedTerrainBlocks_.clear();
	changedChunks_.clear();
	changedThumbnailChunks_.clear();
	thumbnailChunksLoading_.clear();
	Script::call(
		PyObject_GetAttrString( Personality::instance(), "forceClean" ),
		PyTuple_New(0),
		"WorldManager::forceClean: ",
		true );
	changedEnvironment_ = false;
}


/**
 * This method returns whether or not the scripts have anything
 * they need saving.
 *
 * @return true if there are any changes, false otherwise.
 */
bool WorldManager::isScriptDirty() const
{
	BW_GUARD;

	PyObject * ret = Script::ask(
			PyObject_GetAttrString( Personality::instance(), "isDirty" ),
			PyTuple_New(0),
			"WorldManager::isDirty: ",
			true );

	bool isDirty = false;
	Script::setAnswer( ret, isDirty, "WorldManager isScriptDirty retval" );
	return isDirty;
};


/**
 * This method can return if there are any changes need to be saved
 *
 * @return true if there are any changes, false otherwise.
 */
bool WorldManager::isDirty() const
{
	BW_GUARD;

	bool changedTerrain = (changedTerrainBlocks_.size() != 0);
	bool changedScenery = (changedChunks_.size() != 0);
	bool changedThumbnail = (changedThumbnailChunks_.size() != 0);	

	return (changedTerrain || 
			changedScenery || 
			changedThumbnail || 
			changedEnvironment_ || 
			isScriptDirty());
}


/**
 * This method can return if the chunk has finished processing data
 *
 * @return true if processing data is finished, false otherwise.
 */
bool WorldManager::isChunkProcessed( Chunk* chunk ) const
{
	BW_GUARD;

	MF_ASSERT( chunk->loaded() );

	return !dirtyChunkLists_.isDirty( chunk->identifier() );
}


bool WorldManager::canClose( const std::string& action )
{
	BW_GUARD;

	if (isSaving_)
	{
		return false;
	}

	PropertyList::deselectAllItems();

	if (isDirty())
	{
		std::string actionNoShortcut( action );
		std::string::iterator lastChar = std::remove( actionNoShortcut.begin(), actionNoShortcut.end(), '&' );
		actionNoShortcut.erase( lastChar, actionNoShortcut.end() );

		MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();
		MsgBox mb( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHANGED_FILES_TITLE", actionNoShortcut ),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHANGED_FILES_TEXT"),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SAVE"),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/PROCESS_AND_SAVE"),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/WITHOUT_SAVE", action ),
			Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CANCEL") );

		// Remove input focus so that keyup events get forced through now.
		WorldEditorApp::instance().mfApp()->handleSetFocus( false );

		int result = mb.doModal( mainFrame->m_hWnd );
		if( result == 3 )
			return false;
		else if( result == 0 )
		{
			GUI::ItemPtr quickSave = GUI::Manager::instance()( "/MainToolbar/File/QuickSave" );
			quickSave->act();
			if ( saveFailed_ )
				return false;
		}
		else if( result == 1 )
		{
			GUI::ItemPtr save = GUI::Manager::instance()( "/MainToolbar/File/Save" );
			save->act();
			if ( saveFailed_ )
				return false;
		}
        else if (result == 2 )
        {
			HeightModule::doNotSaveHeightMap();
        }
		WorldEditorApp::instance().mfApp()->consumeInput();
	}

	// Delete VLOs no longer used in the space. Must do here to ensure it
	// happens both when changing space and quiting. Also, when a VLO is
	// deleted, it stays alive because there's a reference in the UndoRedo
	// history hanging to it, so must do this before clearing UndoRedo.
	VeryLargeObject::deleteUnused();

	UndoRedo::instance().clear();
	CSplashDlg::HideSplashScreen();
	return true;
}

void WorldManager::updateUIToolMode( const std::wstring& pyID )
{
	BW_GUARD;

	PanelManager::instance().updateUIToolMode( pyID );
}


//---------------------------------------------------------------------------

PY_MODULE_STATIC_METHOD( WorldManager, worldRay, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, repairTerrain, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, markAllChunksClean, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, farPlane, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, save, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, quickSave, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, update, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, render, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, revealSelection, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, isChunkSelected, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, selectAll, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, isSceneBrowserFocused, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, cursorOverGraphicsWnd, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, importDataGUI, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, exportDataGUI, WorldEditor )
PY_MODULE_STATIC_METHOD( WorldManager, rightClick, WorldEditor )

/*~ function WorldEditor.worldRay
 *	@components{ worldeditor }
 *
 *	This function returns the world ray.
 *
 *	@return Returns the world ray from the view frustrum to the mouse position.
 */
PyObject * WorldManager::py_worldRay( PyObject * args )
{
	BW_GUARD;

	return Script::getData( s_instance->worldRay_ );
}


/*~ function WorldEditor.repairTerrain
 *	@components{ worldeditor }
 *
 *	This function puts back "terrain" resources inside a .chunk file if its
 *	corresponding .cdata file contains terrain.
 */
PyObject * WorldManager::py_repairTerrain( PyObject * args )
{
	BW_GUARD;

	GeometryMapping* dirMap = WorldManager::instance().geometryMapping();
	for( int i = dirMap->minGridX(); i <= dirMap->maxGridX(); ++i )
	{
		for( int j = dirMap->minGridY(); j <= dirMap->maxGridY(); ++j )
		{
			std::string prefix = dirMap->path() + dirMap->outsideChunkIdentifier( i, j );
			DataSectionPtr cs = BWResource::openSection( prefix + ".chunk" );
			DataSectionPtr ds = BWResource::openSection( prefix + ".cdata" );
			if( cs && ds && !cs->openSection( "terrain" ) && ds->openSection( "terrain" ) )
			{
				cs->newSection( "terrain" )->writeString( "resource",
					dirMap->outsideChunkIdentifier( i, j ) + ".cdata/terrain" );
				cs->save();
			}
		}
	}

	Py_Return;
}


/*~ function WorldEditor.markAllChunksClean
 *	@components{ worldeditor }
 *
 *	This function resets the dirty flags of chunks that have it set. This might
 *	be useful in the event that the dirty flags get out of sync.
 */
PyObject * WorldManager::py_markAllChunksClean( PyObject * args )
{
	BW_GUARD;

	std::string spacePath = BWResource::resolveFilename( WorldManager::instance().geometryMapping()->path() );
	std::set<std::string> chunks = Utilities::gatherChunks( WorldManager::instance().geometryMapping() );
	std::string space = WorldManager::instance().getCurrentSpace() + '/';

	for( std::set<std::string>::iterator iter = chunks.begin(); iter != chunks.end(); ++iter )
	{
		DataSectionPtr cdata = BWResource::openSection( space + (*iter) + ".cdata" );
		bool flag = false;

		if( cdata )
		{
			EditorChunkCache::UpdateFlags uf( cdata );

			uf.lighting_ = uf.shadow_ = uf.terrainLOD_ = uf.thumbnail_ = 1;
			uf.save();

			DataSectionPtr navmeshSec = cdata->openSection( "navmeshDirty", true );
			navmeshSec->setBinary( new BinaryBlock( &flag, sizeof( flag ), "BinaryBlock/WorldEditor" ) );
			navmeshSec->setParent( cdata );

			cdata->save();

			navmeshSec->setParent( NULL );
		}
	}

	WorldManager::instance().dirtyChunkLists_.clear();
	WorldManager::instance().chunksBeingEdited_.clear();
	WorldManager::instance().writeCleanList();
	WorldManager::instance().reloadAllChunks( false );

	Py_Return;
}

/*~ function WorldEditor.farPlane
 *	@components{ worldeditor }
 *
 *	This function queries and sets the far plane distance.
 *
 *	@param farPlane Optional float value. The distance to set the far plane to.
 *
 *	@return If the farPlane parameter was not supplied, then this function returns the 
 *			currently set far plane distance, otherwise returns the new far plane distance.
 */
PyObject * WorldManager::py_farPlane( PyObject * args )
{
	BW_GUARD;

	float nfp = -1.f;
	if (!PyArg_ParseTuple( args, "|f", &nfp ))
	{
		//There was not a single float argument,
		//therefore return the far plane
		return PyFloat_FromDouble( s_instance->farPlane() );
	}

	if (nfp != -1) s_instance->farPlane( nfp );

	return Script::getData( s_instance->farPlane() );
}

/*~ function WorldEditor.update
 *	@components{ worldeditor }
 *
 *	This function forces an update to be called in WorldEditor. 
 *	Usually called every frame, but it still receives a dTime 
 *  value which informs the update function how much time has passed 
 *	since the last update call.
 *
 *	@param dTime	The amount of time to be passed since the last update. 
 *					Default value is one frame (1/30s).
 */
PyObject * WorldManager::py_update( PyObject * args )
{
	BW_GUARD;

	float dTime = 0.033f;

	if (!PyArg_ParseTuple( args, "|f", &dTime ))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.update() "
			"takes only an optional float argument for dtime" );
		return NULL;
	}

	s_instance->update( dTime );

	Py_Return;
}


/*~ function WorldEditor.render
 *	@components{ worldeditor }
 *	
 *	This function forces WorldEditor to render everything on the scene. 
 *	Usually called every frame, but it still receives a dTime value 
 *	which informs the renderer how much time has passed since the last 
 *	render call.
 *
 *	@param dTime	The amount of time that has passed since the last 
 *					render call. Default value is one frame	(1/30s).
 */
PyObject * WorldManager::py_render( PyObject * args )
{
	BW_GUARD;

	float dTime = 0.033f;

	if (!PyArg_ParseTuple( args, "|f", &dTime ))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.render() "
			"takes only an optional float argument for dtime" );
		return NULL;
	}

	s_instance->render( dTime );

	Py_Return;
}


/*~ function WorldEditor.save
 *	@components{ worldeditor }
 *
 *	This function forces a full save and process all operation.
 */
PyObject * WorldManager::py_save( PyObject * args )
{
	BW_GUARD;

	s_instance->save();

	Py_Return;
}

/*~ function WorldEditor.quickSave
 *	@components{ worldeditor }
 *
 *	This function forces a quick save operation.
 */
PyObject * WorldManager::py_quickSave( PyObject * args )
{
	BW_GUARD;

	s_instance->quickSave();

	Py_Return;
}

SelectionOperation::SelectionOperation( const std::vector<ChunkItemPtr>& before, const std::vector<ChunkItemPtr>& after ) :
	UndoRedo::Operation( 0 ), before_( before ), after_( after )
{
	BW_GUARD;

	for( std::vector<ChunkItemPtr>::iterator iter = before_.begin(); iter != before_.end(); ++iter )
		addChunk( (*iter)->chunk() );
	for( std::vector<ChunkItemPtr>::iterator iter = after_.begin(); iter != after_.end(); ++iter )
		addChunk( (*iter)->chunk() );
}

void SelectionOperation::undo()
{
	BW_GUARD;

	WorldManager::instance().setSelection( before_, false );
	UndoRedo::instance().add( new SelectionOperation( after_, before_ ) );
}

bool SelectionOperation::iseq( const UndoRedo::Operation & oth ) const
{
	BW_GUARD;

	// these operations never replace each other
	return false;
}

/*~ function WorldEditor.revealSelection
 *	@components{ worldeditor }
 *	
 *	This function informs WorldEditor what is currently selected,
 *	i.e., what object/s such as shells, models, particle systems, etc. 
 *	are currently selected.
 *
 *	@param selection	A ChunkItemRevealer object to the selected object(s).
 */
PyObject * WorldManager::py_revealSelection( PyObject * args )
{
	BW_GUARD;

	PyObject * p;
	if (PyArg_ParseTuple( args, "O", &p ))
	{
		if (ChunkItemRevealer::Check(p))
		{
			ChunkItemRevealer * revealer = static_cast<ChunkItemRevealer *>( p );

			std::vector<ChunkItemPtr> selectedItems = s_instance->selectedItems_;

            std::vector<ChunkItemPtr> newSelection;
			revealer->reveal( newSelection );

			s_instance->updateSelection( newSelection );

			s_instance->calculateSnaps();

			bool different = selectedItems.size() != s_instance->selectedItems_.size();
			if( !different )
			{
				std::sort( selectedItems.begin(), selectedItems.end() );
				std::sort( s_instance->selectedItems_.begin(), s_instance->selectedItems_.end() );
				different = ( selectedItems != s_instance->selectedItems_ );
			}
			if( different )
			{
				UndoRedo::instance().add( new SelectionOperation( selectedItems, s_instance->selectedItems_ ) );

				if( !s_instance->settingSelection_ )
					UndoRedo::instance().barrier( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SELECTION_CHANGE"), false );
			}


			//TODO : put back in when page scene is working correctly
			//PageScene::instance().updateSelection( s_instance->selectedItems_ );
		}
	}

	Py_Return;
}

/*~	function WorldEditor.isChunkSelected
 *	@components{ worldeditor }
 *
 *	This function queries whether a shell is currently selected.
 *
 *	@return Returns True if a shell is selected, False otherwise.
 */
PyObject * WorldManager::py_isChunkSelected( PyObject * args )
{
	BW_GUARD;

	return Script::getData( WorldManager::instance().isChunkSelected( true /* force now */) );
}

/*~	function WorldEditor.selectAll
 *	@components{ worldeditor }
 *
 *	This function selects all editable items in all loaded chunks.
 *
 *	@param ignoreVis	(Optional) If True, it allows selecting hidden items.
 *	@param ignoreFrozen	(Optional) If True, it allows selecting frozen items.
 *	@param ignoreFilter	(Optional) If True, it ignores the Selection Filters.
 *	@return Returns a group of the selected chunk items in the form of a ChunkItemGroup object.
 */
PyObject * WorldManager::py_selectAll( PyObject * args )
{
	BW_GUARD;

	ChunkItemRevealer::ChunkItems allItems;
	VeryLargeObject::updateSelectionMark();

	bool ignoreVis = false;
	bool ignoreFrozen = false;
	bool ignoreFilters = false;

	// grab a optional arguments for including hidden/frozen and/or ignore the
	// Selection Filter
	PyArg_ParseTuple( args, "|bbb", &ignoreVis, &ignoreFrozen, &ignoreFilters );

	ScopedSelectionFilter selectionFilter;
	if (ignoreFilters)
	{
		selectionFilter.start( SelectionFilter::SELECT_ANY, "", "" );
	}

	ChunkItemRevealer::ChunkItems intermediateItems;
	if (!SceneBrowser::instance().currentItems( intermediateItems ))
	{
		// Not set from the scene browser, so get all the items from the chunks.
		allChunkItems( intermediateItems );
	}

	// Filter items.
	filterSelectableItems( intermediateItems, ignoreVis, ignoreFrozen, allItems );

	return new ChunkItemGroup( allItems );
}


/*~ function WorldEditor.isSceneBrowserFocused
 *	@components{ worldeditor }
 *
 *	This function returns whether or not the Scene Browser is in focus.
 *
 *	@return Returns True (1) if the SceneBrowser is in focus.
 */
PyObject * WorldManager::py_isSceneBrowserFocused( PyObject * args )
{
	BW_GUARD;

	// Select hidden objects if we are in the scene 
	bool ret = SceneBrowser::instance().isFocused();

	return PyInt_FromLong( (long)ret );
}


/*~ function WorldEditor.cursorOverGraphicsWnd
 *	@components{ worldeditor }
 *
 *	This function queries whether the mouse cursor is currently over 
 *	graphics window. This is usually used to see whether WorldEditor
 *	should be expecting input from the user.
 *
 *	@return Returns True (1) if the cursor is over the graphics window, False (0) otherwise.
 *
 *	Code Example:
 *	@{
 *	def onKeyEvent( ... ):
 *		if not WorldEditor.cursorOverGraphicsWnd():
 *			return 0
 *		...
 *	@}
 */
PyObject * WorldManager::py_cursorOverGraphicsWnd( PyObject * args )
{
	BW_GUARD;

	return PyInt_FromLong((long)s_instance->cursorOverGraphicsWnd());
}


/*~ function WorldEditor.importDataGUI
 *	@components{ worldeditor }
 *
 *	This function enables the TerrainImport Tool Mode.
 */
PyObject * WorldManager::py_importDataGUI( PyObject * args )
{
	BW_GUARD;

    PanelManager::instance().setToolMode( L"TerrainImport" );

	Py_Return;
}

/*~ function WorldEditor.rightClick
 *	@components{ worldeditor }
 *
 *	This function opens an item's context menu. It is usually called
 *	when right clicking an item in the world.
 *
 *	@param selection	A ChunkItemRevealer object to the selected object. 
 */
PyObject * WorldManager::py_rightClick( PyObject * args )
{
	BW_GUARD;

	PyObject * p;
	if (PyArg_ParseTuple( args, "O", &p ))
	{
		if (ChunkItemRevealer::Check(p))
		{
			ChunkItemRevealer * revealer = static_cast<ChunkItemRevealer *>( p );
			std::vector<ChunkItemPtr> items;
			revealer->reveal( items );

			PopupMenu menu;

			static const int commandBaseId = 1;
			static const int idProperties =			0xFF00;
			static const int idSeeInSceneBrowser =	0xFF01;
			static const int idHide =				0xFF02;
			static const int idUnhideAllFilter =	0xFF03;
			static const int idUnhideAll =			0xFF04;
			static const int idFreeze =				0xFF05;
			static const int idUnfreezeAllFilter =	0xFF06;
			static const int idUnfreezeAll =		0xFF07;
			static const int idVisLights =			0xFF08;
			static const int idClearVisLights =		0xFF09;

			ChunkItemPtr item;
			std::vector< ChunkItemPtr > curSel = instance().selectedItems_;
			std::vector< ChunkItemPtr > * multiSelectItems = NULL;
			if (items.size() == 1)
			{
				item = items[0];
				// must copy, selection changes underneath
				if (std::find( curSel.begin(), curSel.end(), item ) != curSel.end())
				{
					multiSelectItems = &curSel;
				}
				else
				{
					multiSelectItems = &items;
				}

				int numCmds = PopupMenuHelper::buildCommandMenu(
												item, commandBaseId, menu );

				if (item->edCanAddSelection())
				{
					if (numCmds > 0)
					{
						menu.addSeparator();
					}

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_HIDE"),
						idHide );

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNHIDE_ALL_FILTER"),
						idUnhideAllFilter );

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNHIDE_ALL"),
						idUnhideAll );

					menu.addSeparator();

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_FREEZE"),
						idFreeze );

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNFREEZE_ALL_FILTER"),
						idUnfreezeAllFilter );

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNFREEZE_ALL"),
						idUnfreezeAll );

					menu.addSeparator();

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/SEE_IN_SCENE_BROWSER"),
						idSeeInSceneBrowser );

					menu.addItem(
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/PROPERTIES"),
						idProperties );
				}
			}
			else
			{
				multiSelectItems = &curSel;

				menu.addItem(
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNHIDE_ALL_FILTER"),
					idUnhideAllFilter );

				menu.addItem(
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNHIDE_ALL"),
					idUnhideAll );

				menu.addSeparator();

				menu.addItem(
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNFREEZE_ALL_FILTER"),
					idUnfreezeAllFilter );

				menu.addItem(
					Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/RCLK_UNFREEZE_ALL"),
					idUnfreezeAll );
			}

			size_t numLightVisItems = LightContainerDebugger::instance()->numItems();
			if ( !items.empty() || numLightVisItems > 0)
			{
				menu.addSeparator();
				if (!items.empty())
				{
					menu.addItem( 
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/VIS_LIGHT_CONTRIBUTION"), 
						idVisLights );
				}

				if (numLightVisItems > 0)
				{
					menu.addItem( 
						Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CLEAR_LIGHT_CONTRIBUTION_VIS"), 
						idClearVisLights );
				}
			}


			if (!menu.empty())
			{
				ShowCursor( TRUE );

				// Remove input focus so that keyup events get forced through now.
				WorldEditorApp::instance().mfApp()->handleSetFocus( false );

				int result = menu.doModal( WorldManager::instance().hwndGraphics() );

				ShowCursor( FALSE );

				// Consume input, otherwise input given while the popup menu was visible
				// would be sent to the main window
				WorldEditorApp::instance().mfApp()->consumeInput();

				if (result)
				{
					if (result == idProperties)
					{
						WorldManager::instance().setSelection( *multiSelectItems );

						PanelManager::instance().showPanel( PageProperties::contentID, true );
					}
					else if (result == idSeeInSceneBrowser)
					{
						WorldManager::instance().setSelection( items );

						SceneBrowser::instance().scrollTo( items[0] );
					}
					else if (result == idHide)
					{
						ChunkItemPlacer::hideChunkItems( *multiSelectItems, true, false );
					}
					else if (result == idUnhideAllFilter || result == idUnhideAll)
					{
						VeryLargeObject::updateSelectionMark();
						ChunkItemRevealer::ChunkItems allItems;
						ChunkItemRevealer::ChunkItems intermediateItems;
						ScopedSelectionFilter filterSetter;
						if (result == idUnhideAll)
						{
							// select all, regardless of the selection filter
							filterSetter.start( SelectionFilter::SELECT_ANY, "", "" );
						}
						allChunkItems( intermediateItems );
						filterSelectableItems( intermediateItems, true, true, allItems );
						ChunkItemPlacer::hideChunkItems( allItems, false, true );
					}
					else if (result == idFreeze)
					{
						ChunkItemPlacer::freezeChunkItems( *multiSelectItems, true, false );
					}
					else if (result == idUnfreezeAllFilter || result == idUnfreezeAll)
					{
						VeryLargeObject::updateSelectionMark();
						ChunkItemRevealer::ChunkItems allItems;
						ChunkItemRevealer::ChunkItems intermediateItems;
						ScopedSelectionFilter filterSetter;
						if (result == idUnfreezeAll)
						{
							// select all, regardless of the selection filter
							filterSetter.start( SelectionFilter::SELECT_ANY, "", "" );
						}
						allChunkItems( intermediateItems );
						filterSelectableItems( intermediateItems, true, true, allItems );
						ChunkItemPlacer::freezeChunkItems( allItems, false, true );
					}
					else if (result == idVisLights)
					{
						WorldManager::instance().setSelection( *multiSelectItems );

						LightContainerDebugger::instance()->setItems( 
								WorldManager::instance().selectedItems() 
							);
					}
					else if (result == idClearVisLights)
					{
						LightContainerDebugger::instance()->clearItems();
					}
					else if (item)
					{
						item->edExecuteCommand( "", result - commandBaseId );
					}
				}
			}
		}
	}

	Py_Return;
}


/*~ function WorldEditor.py_exportDataGUI
 *	@components{ worldeditor }
 *
 *	This function enables the TerrainImport Tool Mode.
 */
PyObject * WorldManager::py_exportDataGUI( PyObject *args )
{
	BW_GUARD;

    PanelManager::instance().setToolMode( L"TerrainImport" );

	Py_Return;
}


//---------------------------------------------------------------------------
/*
static PyObject * py_recalcTerrainShadows( PyObject * args )
{
	ChunkMap& chunks = ChunkManager::instance().cameraSpace()->chunks();

	std::vector<Chunk*> outsideChunks;
	for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); it++)
	{
		Chunk* pChunk = it->second;

		if (!pChunk->isBound())
			continue;

		if (!pChunk->isOutsideChunk())
			continue;

		if (!EditorChunkCache::instance( *pChunk ).edIsLocked())
			continue;

		outsideChunks.push_back( pChunk );
	}

	ProgressTask terrainShadowsTask( progressBar_, "Calculating terrain shadows", float(outsideChunks.size()) );

	std::vector<Chunk*>::iterator it = outsideChunks.begin();
	for (; it != outsideChunks.end(); ++it)
	{
		terrainShadowsTask.step();

		EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
			ChunkTerrainCache::instance( **it ).pTerrain());

		pEct->calculateShadows();
	}

	Py_Return;
}
PY_MODULE_FUNCTION( recalcTerrainShadows, WorldEditor )
*/

//---------------------------------------------------------------------------

void findRelevantChunks( ToolPtr tool, float buffer = 0.f )
{
	BW_GUARD;

	if ( tool->locator() )
	{
		float halfSize = buffer + tool->size() / 2.f;
		Vector3 start( tool->locator()->transform().applyToOrigin() -
						Vector3( halfSize, 0.f, halfSize ) );
		Vector3 end( tool->locator()->transform().applyToOrigin() +
						Vector3( halfSize, 0.f, halfSize ) );

		EditorChunk::findOutsideChunks(
			BoundingBox( start, end ), tool->relevantChunks() );

		tool->currentChunk() =
			EditorChunk::findOutsideChunk( tool->locator()->transform().applyToOrigin() );
	}
}
//---------------------------------------------------------------------------
bool WorldManager::changeSpace( const std::string& space, bool reload )
{
	BW_GUARD;

	static int id = 1;

	if( currentSpace_ == space )
		return true;

	if( !BWResource::fileExists( space + '/' + SPACE_SETTING_FILE_NAME ) )
		return false;

	DataSectionPtr spaceSettings = BWResource::openSection( space + "/" + SPACE_SETTING_FILE_NAME );
	if (!spaceSettings)
		return false;

	// It's possible that the space settings file exists but was totally
	// corrupted due to a version control conflict.  It may in fact be read as
	// a BinSection with the file's contents as the data as a result.  To
	// prevent this case we check for the existance of the "bounds" section.
	DataSectionPtr boundsSection = spaceSettings->openSection("bounds");
	if (!boundsSection)
		return false;

	EditorChunkCache::forwardReadOnlyMark();

	if( !reload )
	{
		if( spaceLock_ != INVALID_HANDLE_VALUE )
			CloseHandle( spaceLock_ );
		std::wstring wspace;
		bw_utf8tow( BWResolver::resolveFilename( space + "/space.lck" ), wspace );
		spaceLock_ = CreateFile( wspace.c_str(), GENERIC_READ | GENERIC_WRITE,
			0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL );
		if( spaceLock_ == INVALID_HANDLE_VALUE )
		{
			MainFrame* mainFrame = (MainFrame *)WorldEditorApp::instance().mainWnd();
			MsgBox mb( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OPEN_SPACE_TITLE"),
				Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/UNABLE_TO_OPEN_SPACE", space ),
				Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/OK") );
			mb.doModal( mainFrame->m_hWnd );
			return false;
		}
	}

	renderDisabled_ = true;

	stopBackgroundCalculation();

	if ( romp_ )
		romp_->enviroMinder().deactivate();

	if( !currentSpace_.empty() )
	{
		setSelection( std::vector<ChunkItemPtr>(), false );
		setSelection( std::vector<ChunkItemPtr>(), true );
	}

	ChunkManager::instance().switchToSyncMode( true );

	if( !reload )
	{
		if (inited_)
		{
			// Clear the message list before changing space, but not if it's
			// the first time.
			MsgHandler::instance().clear();
		}

		if( conn_.enabled() )
		{
			CWaitCursor wait;
			if( conn_.changeSpace( space ) )
				WaitDlg::overwriteTemp( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CONNECT_TO_BWLOCKD_DONE", conn_.host() ), 500 );
			else
				WaitDlg::overwriteTemp( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CONNECT_TO_BWLOCKD_FAILED", conn_.host() ), 500 );
		}
		else
			conn_.changeSpace( space );

		resetTerrainInfo();
	}

	LightContainerDebugger::instance()->clearItems();
	EditorChunkOverlapper::drawList.clear();
	AmortiseChunkItemDelete::instance().purge();
	EditorChunkLink::flush( 0, true );

	BWResource::instance().purgeAll();

	this->workingChunk( NULL, false );

	ChunkManager::instance().clearAllSpaces();
	ChunkManager::instance().camera( Matrix::identity, NULL );

	// Clear the linker manager lists
	WorldManager::instance().linkerManager().reset();

	ChunkSpacePtr chunkSpace = ChunkManager::instance().space( id );
	++id;
	// TODO: find out why addMapping is passing a matrix as a float and fix it.
	Matrix* nonConstIdentity = const_cast<Matrix*>(&Matrix::identity);
	mapping_ = chunkSpace->addMapping( SpaceEntryID(), (float*)nonConstIdentity, space );
	if( !mapping_ )
	{
		ChunkManager::instance().switchToSyncMode( false );
		renderDisabled_ = false;
		return false;
	}

	currentSpace_ = space;

	if( !reload && PanelManager::pInstance() != NULL )
		PanelManager::instance().setToolMode( L"Objects" );

	resetChangedLists();

	ChunkManager::instance().switchToSyncMode( false );
	ChunkManager::instance().camera( Matrix::identity, chunkSpace );

	cleanChunkList_ = new CleanChunkList( *geometryMapping() );

	ChunkManager::instance().tick( 0.f );

	ToolManager::instance().changeSpace( getWorldRay( currentCursorPosition() ) );

	DataSectionPtr localSpaceSettings = BWResource::openSection( space + "/" + SPACE_LOCAL_SETTING_FILE_NAME );
	if( !reload )
	{
		Vector3 dir( 0.f, 0.f, 0.f ), pos( 0.f, 2.f, 0.f );
		if( localSpaceSettings )
		{
			pos = localSpaceSettings->readVector3( "startPosition", Vector3( 0.f, 2.f, 0.f ) );
			dir = localSpaceSettings->readVector3( "startDirection" );
		}
		Matrix m;
		m.setIdentity();
		m.setRotate( dir[2], dir[1], dir[0] );
		m.translation( pos );
		m.invert();
		Moo::rc().view( m );
	}

	// set the window title to the current space name
	std::wstring wspace;
	bw_utf8tow( space + " - ", wspace );
	AfxGetMainWnd()->SetWindowText(	( wspace + Localise(L"WORLDEDITOR/APPLICATION_NAME" ) ).c_str() );

	spaceManager_->addSpaceIntoRecent( space );

	if (WorldEditorCamera::pInstance())
	{
		WorldEditorCamera::instance().respace( Moo::rc().view() );
	}

	DataSectionPtr terrainSettings = spaceSettings->openSection( "terrain" );
	if ( terrainSettings == NULL ||
		( terrainSettings != NULL && terrainSettings->readInt( "version", 0 ) == 2 ) )
	{
		// If it doesn't have a terrain section in the space.settings, or
		// if the terrain version in the terrain section is '2', then
		// generate a new terrain section using the appropriate values
		// because the old space.settings value is wrong.
		if ( terrainSettings != NULL ) 
		{
			// discard the old space.settings section
			spaceSettings->deleteSection( "terrain" );
		}
		terrainSettings = spaceSettings->openSection( "terrain", true );

		uint32 terrainVersion = getTerrainVersion();
		Terrain::TerrainSettingsPtr pTempSettings = new Terrain::TerrainSettings;
		pTempSettings->initDefaults();
		pTempSettings->version(terrainVersion);
		if ( terrainVersion == 200 )
		{
			// set to old defaults for terrain 2
			pTempSettings->heightMapSize( 128 );
			pTempSettings->normalMapSize( 128 );
			pTempSettings->holeMapSize( 25 );
			pTempSettings->shadowMapSize( 32 );
			pTempSettings->blendMapSize( 256 );
			pTempSettings->save( terrainSettings );
			spaceSettings->save();
		}
		else if ( terrainVersion == 100 )
		{
			pTempSettings->save( terrainSettings );
			spaceSettings->save();
		}
		else
		{
			ERROR_MSG( "Couldn't create space.settings/terrain section: unknown terrain version.\n" );
		}
	}

	if ( romp_ )
		romp_->enviroMinder().activate();
	this->setTimeFromOptions();
	Flora::floraReset();
	UndoRedo::instance().clear();

	updateRecentFile();

	if( !reload && PanelManager::pInstance() != NULL )
		PanelManager::instance().setDefaultToolMode();

    secsPerHour_ = romp_->timeOfDay()->secondsPerGameHour();

	romp_->changeSpace();

	update( 0.f );

	unsigned int spaceWidth = 0;
	unsigned int spaceHeight = 0;

	if (spaceSettings)
	{
		int minX = spaceSettings->readInt( "bounds/minX", 1 );
		int minY = spaceSettings->readInt( "bounds/minY", 1 );
		int maxX = spaceSettings->readInt( "bounds/maxX", -1 );
		int maxY = spaceSettings->readInt( "bounds/maxY", -1 );

		SpaceMap::instance().spaceInformation(
			SpaceInformation( space, GridCoord( minX, minY ), maxX - minX + 1, maxY - minY + 1 ) );

		chunkWatcher_->onNewSpace(minX, minY, maxX, maxY);

		spaceWidth  = maxX - minX + 1;
		spaceHeight = maxY - minY + 1;
	}

	Script::call(
		PyObject_GetAttrString( Personality::instance(),
			"onCameraSpaceChange" ),
		Py_BuildValue( "iN", chunkSpace->id(), new PyDataSection(spaceSettings) ),
		"Personality onCameraSpaceChange: ",
		true );

	if (PanelManager::pInstance())
	{
		PanelManager::instance().onNewSpace( spaceWidth, spaceHeight );	
	}

	localSpaceSettings = BWResource::openSection( Options::getOptionString( "space/mru0" ) + '/' +
		SPACE_LOCAL_SETTING_FILE_NAME );

	renderDisabled_ = false;
	return true;
}

bool WorldManager::handleGUIAction( GUI::ItemPtr item )
{
	BW_GUARD;

	const std::string& action = item->action();

	if (action == "changeSpace" )				return changeSpace( item );
	else if(action == "newSpace")				return newSpace( item );
	else if(action == "recentSpace")			return recentSpace( item );
	else if(action == "clearUndoRedoHistory")	return clearUndoRedoHistory( item );
	else if(action == "doExternalEditor")		return doExternalEditor( item );
	else if(action == "doReloadAllTextures")	return doReloadAllTextures( item );
	else if(action == "doReloadAllChunks")		return doReloadAllChunks( item );
	else if(action == "doExit")					return doExit( item );
	else if(action == "setLanguage")			return setLanguage( item );
	else if(action == "recalcCurrentChunk")		return recalcCurrentChunk( item );

	return false;
}

bool WorldManager::changeSpace( GUI::ItemPtr item )
{
	BW_GUARD;

	if( !canClose( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHANGE_SPACE") ) )
		return false;
	std::string space = spaceManager_->browseForSpaces( hwndApp_ );
	space = BWResource::dissolveFilename( space );
	if( !space.empty() )
		return changeSpace( space, false );
	return false;
}

bool WorldManager::newSpace( GUI::ItemPtr item )
{
	BW_GUARD;

	if( !canClose( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHANGE_SPACE") ) )
		return false;
	NewSpaceDlg dlg;
	bool result = ( dlg.DoModal() == IDOK );
	if( result )
	{
		std::string nspace;
		bw_wtoutf8( (LPCTSTR)dlg.createdSpace(), nspace );
		result = changeSpace( nspace, false );
	}
	return result;
}

bool WorldManager::recentSpace( GUI::ItemPtr item )
{
	BW_GUARD;

	if( !canClose( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CHANGE_SPACE") ) )
		return false;
	std::string spaceName = (*item)[ "spaceName" ];
	bool ok = changeSpace( spaceName, false );
	if (!ok)
	{
		ERROR_MSG
		(
			LocaliseUTF8(
				L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/CANNOT_CHANGE_SPACE", 
				spaceName
			).c_str()
		);
		spaceManager_->removeSpaceFromRecent( spaceName );
		updateRecentFile();
	}
	return ok;
}

bool WorldManager::setLanguage( GUI::ItemPtr item )
{
	BW_GUARD;

	std::string languageName = (*item)[ "LanguageName" ];
	std::string countryName = (*item)[ "CountryName" ];

	// Do nothing if we are not changing language
	if (currentLanguageName_ == languageName && currentCountryName_ == countryName)
	{
		return true;
	}

	unsigned int result;
	if (isDirty())
	{
		result = MsgBox( Localise(L"RESMGR/CHANGING_LANGUAGE_TITLE"), Localise(L"RESMGR/CHANGING_LANGUAGE"),
			Localise(L"RESMGR/SAVE_AND_RESTART"), Localise(L"RESMGR/DISCARD_AND_RESTART"),
			Localise(L"RESMGR/RESTART_LATER"), Localise(L"RESMGR/CANCEL") ).doModal();
	}
	else
	{
		result = MsgBox( Localise(L"RESMGR/CHANGING_LANGUAGE_TITLE"), Localise(L"RESMGR/CHANGING_LANGUAGE"),
			Localise(L"RESMGR/RESTART_NOW"), Localise(L"RESMGR/RESTART_LATER"), Localise(L"RESMGR/CANCEL") ).doModal() + 1;
	}
	switch (result)
	{
	case 0:
		Options::setOptionString( "currentLanguage", languageName );
		Options::setOptionString( "currentCountry", countryName );
		quickSave();
		startNewInstance();
		AfxGetApp()->GetMainWnd()->PostMessage( WM_COMMAND, ID_APP_EXIT );
		break;
	case 1:
		Options::setOptionString( "currentLanguage", languageName );
		Options::setOptionString( "currentCountry", countryName );
		forceClean();
		startNewInstance();
		AfxGetApp()->GetMainWnd()->PostMessage( WM_COMMAND, ID_APP_EXIT );
		break;
	case 2:
		Options::setOptionString( "currentLanguage", languageName );
		Options::setOptionString( "currentCountry", countryName );
		currentLanguageName_ = languageName;
		currentCountryName_ = countryName;
		break;
	case 3:
		break;
	}
	return true;
}

bool WorldManager::doReloadAllTextures( GUI::ItemPtr item )
{
	BW_GUARD;

	AfxGetApp()->DoWaitCursor( 1 );
	Moo::TextureManager::instance()->reloadAllTextures();
	AfxGetApp()->DoWaitCursor( 0 );
	return true;
}

bool WorldManager::recalcCurrentChunk( GUI::ItemPtr item )
{
	BW_GUARD;

	Chunk* chunk = ChunkManager::instance().cameraSpace()->
		findChunkFromPoint( Moo::rc().invView().applyToOrigin() );
	if( chunk && EditorChunkCache::instance( *chunk ).edIsWriteable() )
	{
		AfxGetApp()->DoWaitCursor( 1 );

		if( chunk->isOutsideChunk() )
		{
			dirtyTerrainShadows( chunk );
			if (chunk->space()->staticLightingOutside())
				dirtyLighting( chunk );
		}
		else
			dirtyLighting( chunk );
		dirtyThumbnail( chunk );

		std::set<std::string> chunks;
		chunks.insert( chunk->identifier() );
		save( &chunks, true );

		AfxGetApp()->DoWaitCursor( 0 );
	}
	return true;
}

bool WorldManager::doReloadAllChunks( GUI::ItemPtr item )
{
	BW_GUARD;

	AfxGetApp()->DoWaitCursor( 1 );
	reloadAllChunks( true );
	resetTerrainInfo();
	AfxGetApp()->DoWaitCursor( 0 );
	return true;
}

bool WorldManager::doExit( GUI::ItemPtr item )
{
	BW_GUARD;

	AfxGetApp()->GetMainWnd()->PostMessage( WM_COMMAND, ID_APP_EXIT );
	return true;
}

void WorldManager::updateRecentFile()
{
	BW_GUARD;

	GUI::ItemPtr recentFiles = GUI::Manager::instance()( "/MainMenu/File/RecentFiles" );
	if( recentFiles )
	{
		while( recentFiles->num() )
			recentFiles->remove( 0 );
		for( unsigned int i = 0; i < spaceManager_->num(); ++i )
		{
			std::stringstream name, displayName;
			name << "mru" << i;
			displayName << '&' << i << "  " << spaceManager_->entry( i );
			GUI::ItemPtr item = new GUI::Item( "ACTION", name.str(), displayName.str(),
				"",	"", "", "recentSpace", "", "" );
			item->set( "spaceName", spaceManager_->entry( i ) );
			recentFiles->add( item );
		}
	}
}

void WorldManager::updateLanguageList()
{
	BW_GUARD;

	GUI::ItemPtr languageList = GUI::Manager::instance()( "/MainMenu/Languages/LanguageList" );
	if( languageList )
	{
		while( languageList->num() )
			languageList->remove( 0 );
		for( unsigned int i = 0; i < StringProvider::instance().languageNum(); ++i )
		{
			LanguagePtr l = StringProvider::instance().getLanguage( i );
			std::wstringstream wname, wdisplayName;
			wname << L"language" << i;
			wdisplayName << L'&' << l->getLanguageName();
			std::string name, displayName;
			bw_wtoutf8( wname.str(), name );
			bw_wtoutf8( wdisplayName.str(), displayName );
			GUI::ItemPtr item = new GUI::Item( "CHILD", name, displayName,
				"",	"", "", "setLanguage", "updateLanguage", "" );
			item->set( "LanguageName", l->getIsoLangNameUTF8() );
			item->set( "CountryName", l->getIsoCountryNameUTF8() );
			languageList->add( item );
		}
	}
}


bool WorldManager::clearUndoRedoHistory( GUI::ItemPtr item )
{
	BW_GUARD;

	UndoRedo::instance().clear();
	return true;
}


unsigned int WorldManager::handleGUIUpdate( GUI::ItemPtr item )
{
	BW_GUARD;

	const std::string& updater = item->updater();;

	if (updater == "updateUndo")				return updateUndo( item );
	else if (updater == "updateRedo")			return updateRedo( item );
	else if (updater == "updateExternalEditor")	return updateExternalEditor( item );
	else if (updater == "updateLanguage")		return updateLanguage( item );

	return 0;
}


unsigned int WorldManager::updateUndo( GUI::ItemPtr item )
{
	BW_GUARD;

	return UndoRedo::instance().canUndo();
}


unsigned int WorldManager::updateRedo( GUI::ItemPtr item )
{
	BW_GUARD;

	return UndoRedo::instance().canRedo();
}


bool WorldManager::doExternalEditor( GUI::ItemPtr item )
{
	BW_GUARD;

	if( selectedItems_.size() == 1 )
		selectedItems_[ 0 ]->edExecuteCommand( "", 0 );
	return true;
}


unsigned int WorldManager::updateExternalEditor( GUI::ItemPtr item )
{
	return selectedItems_.size() == 1 && !selectedItems_[ 0 ]->edCommand( "" ).empty();
}


unsigned int WorldManager::updateLanguage( GUI::ItemPtr item )
{
	BW_GUARD;

	if (currentLanguageName_.empty())
	{
		currentLanguageName_ = StringProvider::instance().currentLanguage()->getIsoLangNameUTF8();
		currentCountryName_ = StringProvider::instance().currentLanguage()->getIsoCountryNameUTF8();
	}
	return currentLanguageName_ == (*item)[ "LanguageName" ] && currentCountryName_ == (*item)[ "CountryName" ];
}


std::string WorldManager::get( const std::string& key ) const
{
	BW_GUARD;

	return Options::getOptionString( key );
}


bool WorldManager::exist( const std::string& key ) const
{
	BW_GUARD;

	return Options::optionExists( key );
}


void WorldManager::set( const std::string& key, const std::string& value )
{
	BW_GUARD;

	Options::setOptionString( key, value );
}


/**
 *	This function draws the terrain texture LOD for the given chunk.
 *
 *	@param chunk		The chunk to update.
 *	@param markDirty	If true then the chunk is added to the changed chunk
 *						list, if false then it isn't.
 *	@returns			True if the terrain texture LOD could be updated.
 */
bool WorldManager::drawMissingTextureLOD(Chunk *chunk, bool markDirty)
{
	BW_GUARD;

	// Is LOD regeneration disabled?
	if (lodRegenCount_ != 0)
		return false;

	// Handle the case where chunk is NULL or not loaded:
	if (chunk == NULL)
		return false;
	if (!chunk->loaded())
		return false;

	bool updateOk = true;

	this->workingChunk( chunk, !markDirty ); // Let chunk watcher know about the chunk

	// Regenerate the texture LOD:
	ChunkTerrain *chunkTerrain = 
        ChunkTerrainCache::instance(*chunk).pTerrain();
    if (chunkTerrain != NULL)
    {
        Terrain::EditorBaseTerrainBlock *terrainBlock = 
            static_cast<Terrain::EditorBaseTerrainBlock *>(chunkTerrain->block().getObject());
		updateOk = terrainBlock->rebuildLodTexture( chunk->transform() );
		EditorChunkCache::instance( *chunk ).terrainLODUpdated( updateOk );
	}
	else
	{
		updateOk = false;
	}

	if (updateOk && markDirty)
	{
		changedTerrainBlock(chunk, false);
		changedChunk( chunk, false, false );
	}

	return updateOk;
}


/**
 *	This function draws missing texture LODs.  These can be missing if the
 *	device is lost.
 *
 *	@param complainIfNotDone	If true and redrawing failed then print an
 *						error message.  If false then no messages are printed.
 *	@param doAll		If true then do all of the missing texture LODs.
 *						If false then only do one.
 *	@param markDirty	If true then the chunk is added to the dirty list,
 *						if false then it isn't.
 *	@param progress		If true then a progress bar is displayed, if false then
 *						no progress bar is displayed.  This can only be set to
 *						true if doAll is also true.
 */
void WorldManager::drawMissingTextureLODs
(
	bool		complainIfNotDone, 
	bool		doAll,
	bool		markDirty,
	bool		progress
)
{
	BW_GUARD;

	// Show the progress bar if it has been requested:
	ProgressTask *paintTask = NULL;
	if (progress && doAll)
	{
		paintTask = 
			new ProgressTask
			( 
				progressBar_, 
				LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG/UPDATE_TEXTURE_LODS"), 
				(float)dirtyChunkLists_[ TERRAIN_LOD ].num() 
			);
	}

	size_t numberToDo = doAll ? std::numeric_limits<size_t>::max() : 1;

	// Update texture LODS
	bool updateOk = true;
	for (DirtyChunkList::iterator it = dirtyChunkLists_[ TERRAIN_LOD ].begin();
		it != dirtyChunkLists_[ TERRAIN_LOD ].end() && numberToDo != 0; )
	{
		Chunk *chunk = it->second;
		if (chunk && chunk->loaded())
		{
			updateOk = drawMissingTextureLOD(chunk, markDirty);
			if (updateOk)
			{
				it = dirtyChunkLists_[ TERRAIN_LOD ].erase(it);
			}
			else
			{
				++it;
			}
			if (paintTask != NULL)
				paintTask->step();
			--numberToDo;
		}
		else
		{
			++it;
		}
	}
	if (!updateOk && complainIfNotDone)
		ERROR_MSG("Unable to regenerate some terrain texture LODS\n");

	// Cleanup:
	if (paintTask != NULL)
	{
		delete paintTask; 
		paintTask = NULL;
	}
}


std::string WorldManager::getCurrentSpace() const
{
	return currentSpace_;
}	


void WorldManager::showBusyCursor()
{
	BW_GUARD;

    // Set the cursor to the arrow + hourglass if there are not yet any loaded
    // chunks, or reset it to the arrow cursor if we were displaying the wait
    // cursor.
    EditorChunkCache::lock();
	bool loadedChunk = EditorChunkCache::chunks_.size() > 0;
	EditorChunkCache::unlock();
    if (waitCursor_ || !loadedChunk)
    {
        WorldManager::instance().setCursor
        (
            loadedChunk 
                ? ::LoadCursor(NULL, IDC_ARROW)
                : ::LoadCursor(NULL, IDC_APPSTARTING)
        );
        waitCursor_ = !loadedChunk;
    }
}


unsigned int WorldManager::getMemoryLoad()
{
	BW_GUARD;

	MEMORYSTATUSEX memoryStatus = { sizeof( memoryStatus ) };
	GlobalMemoryStatusEx( &memoryStatus );
	DWORDLONG cap = memoryStatus.ullTotalVirtual - 300 * 1024 * 1024; //  300M room gives some sense of safety
	if( cap > memoryStatus.ullTotalPhys * 2 )
		cap = memoryStatus.ullTotalPhys * 2;

	PROCESS_MEMORY_COUNTERS pmc = { sizeof( pmc ) };
	GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) );

	DWORDLONG used = pmc.PagefileUsage;
	if( used > cap )
		used = cap;
	return unsigned int( used * 100 / cap );
}


float WorldManager::getMaxFarPlane() const
{
	BW_GUARD;

	return Options::getOptionFloat( "render/maxFarPlane", 5000 );
}

void WorldManager::registerDelayedChanges()
{
	BW_GUARD;

	if (Moo::g_renderThread && pendingChangedChunks_.size())
	{
		changeMutex.grab();
		if (pendingChangedChunks_.size())
		{
			std::set< Chunk * > tmp = pendingChangedChunks_;
			pendingChangedChunks_.clear();
			changeMutex.give();

			try
			{
				for (std::set< Chunk * >::iterator it = tmp.begin();
					it != tmp.end(); it ++)
				{
					changedChunk( *it );
				}
			}
			catch( ... )
			{
				;
			}
		}
		else
		{
			changeMutex.give();
		}
	}
}


bool WorldManager::changedPostProcessing() const
{
	BW_GUARD;

	return changedPostProcessing_;
}


void WorldManager::changedPostProcessing( bool changed )
{
	BW_GUARD;

	changedPostProcessing_ = changed;
}


bool WorldManager::userEditingPostProcessing() const
{
	BW_GUARD;

	return userEditingPostProcessing_;
}


void WorldManager::userEditingPostProcessing( bool editing )
{
	BW_GUARD;

	userEditingPostProcessing_ = editing;
}


/**
 * This function process messages in current message queue. But all mouse events,
 * keyboard events and menu events will be discarded.
 * This is used for preventing window from losing responding during some long time
 * calculation
 */
/*static */void WorldManager::processMessages()
{
	BW_GUARD;

	instance().escapePressed();
	MSG msg;
	while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
	{
		if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
		{
			continue;
		}
		if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
		{
			continue;
		}
		if (msg.message == WM_COMMAND)
		{
			continue;
		}
		TranslateMessage( &msg );
		DispatchMessage(  &msg );
	}
}

class WEMetaDataEnvironment : public MetaData::Environment
{
	virtual void changed( void* param )
	{
		BW_GUARD;

		EditorChunkCommonLoadSave* eccl = (EditorChunkCommonLoadSave*)param;

		eccl->edCommonChanged();
	}
};

static WEMetaDataEnvironment s_WEMetaDataEnvironment;
