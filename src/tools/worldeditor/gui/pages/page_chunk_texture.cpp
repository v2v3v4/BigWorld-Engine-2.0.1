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

#include "page_chunk_texture.hpp"

#include "worldeditor/framework/world_editor_app.hpp"
#include "worldeditor/gui/pages/page_terrain_texture.hpp"
#include "worldeditor/gui/pages/panel_manager.hpp"
#include "worldeditor/misc/editor_renderable.hpp"
#include "worldeditor/scripting/we_python_adapter.hpp"
#include "worldeditor/terrain/editor_chunk_terrain_projector.hpp"
#include "worldeditor/terrain/terrain_paint_brush.hpp"
#include "worldeditor/terrain/terrain_texture_utils.hpp"
#include "worldeditor/undo_redo/terrain_texture_layer_undo.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/world_manager.hpp"

#include "guitabs/guitabs.hpp"

#include "controls/edit_numeric.hpp"
#include "controls/utils.hpp"

#include "chunk/base_chunk_space.hpp"
#include "chunk/chunk_manager.hpp"

#include "gizmo/tool_manager.hpp"

#include "common/popup_menu.hpp"
#include "common/utilities.hpp"
#include "common/user_messages.hpp"

#include "appmgr/options.hpp"


// GUITABS content ID ( declared by the IMPLEMENT_BASIC_CONTENT macro )
const std::wstring PageChunkTexture::contentID = L"PageChunkTexture";


namespace
{
	// offset inwards of separators
    const uint32 SEP_OFFSET                    = 4;
	// offset inwards of controls
	const uint32 CONTROL_OFFSET                = 6;

	// Textures per chunk warning constants
	const char*	OPTION_AUTOTRACK                  = "chunkTexture/autotrack";
	const char*	OPTION_SHOWTRACK                  = "chunkTexture/showtrack";
	const char*	OPTION_LATCH                      = "chunkTexture/latch";
	const char*	OPTION_NUM_LAYER_WARNING          = "chunkTexture/numLayerWarning";
	const char*	OPTION_SHOW_NUM_LAYER_WARNING     = "chunkTexture/numLayerWarningShow";
}


//------------------------------------------------------------------
//	helper class PageChunkTextureView
//------------------------------------------------------------------
class PageChunkTextureView : public EditorRenderable
{
public:
	static void chunk( Chunk* chunk )
	{
		BW_GUARD;

		if ( !view_ )
			view_ = new PageChunkTextureView();

		if ( view_->chunk_ != NULL )
		{
			WorldManager::instance().removeRenderable( view_ );
		}
		view_->chunk_ = chunk;
		if ( view_->chunk_ != NULL )
		{
			WorldManager::instance().addRenderable( view_ );
		}
	}

	static void fini()
	{
		BW_GUARD;

		view_ = NULL;
	}

	void render()
	{
		BW_GUARD;

		if ( !chunk_ )
			return;

		EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
				ChunkTerrainCache::instance( *chunk_ ).pTerrain());

		if (pEct)
		{
			EditorChunkTerrainPtrs spChunks;
			spChunks.push_back( pEct );

			EditorChunkTerrainProjector::instance().projectTexture(
				texture_,
				GRID_RESOLUTION,
				0.f,					
				chunk_->transform().applyToOrigin() +
					Vector3( GRID_RESOLUTION/2.0f, 0.f, GRID_RESOLUTION/2.0f ),
				D3DTADDRESS_BORDER,
				spChunks,
				false );
		}
	}

private:
	typedef SmartPointer<PageChunkTextureView> PageChunkTextureViewPtr;

	PageChunkTextureView() :
		  chunk_( NULL )
	{
		BW_GUARD;

		texture_ = Moo::TextureManager::instance()->get(
			"resources/maps/gizmo/chunktextures.tga" );
	}

	static PageChunkTextureViewPtr view_;
	Chunk* chunk_;
	Moo::BaseTexturePtr texture_;
};
PageChunkTextureView::PageChunkTextureViewPtr PageChunkTextureView::view_ = NULL;



//------------------------------------------------------------------
//	PageChunkTexture
//------------------------------------------------------------------

PageChunkTexture::PageChunkTexture()
	: CFormView(PageChunkTexture::IDD),
	pageReady_( false ),
	enabled_( true ),
	resize_( true ),
	chunkTrackButtonImg_(NULL),
	chunkTrackChkButtonImg_(NULL),
	chunkShowButtonImg_(NULL),
	chunkShowChkButtonImg_(NULL)
{
}


PageChunkTexture::~PageChunkTexture()
{
	BW_GUARD;

	::DestroyIcon(chunkTrackButtonImg_);
	chunkTrackButtonImg_ = NULL;

	::DestroyIcon(chunkTrackChkButtonImg_);
	chunkTrackChkButtonImg_ = NULL;

	::DestroyIcon(chunkShowButtonImg_);
	chunkShowButtonImg_ = NULL;

	::DestroyIcon(chunkShowChkButtonImg_);
	chunkShowChkButtonImg_ = NULL;

	PageChunkTextureView::fini();
}


/**
 *  Static method that returns the current active chunk in the page.
 *
 *  @returns		chunk name, or "" if no chunk set
 */
/*static*/ const std::string PageChunkTexture::chunk()
{
	BW_GUARD;

	PageChunkTexture* panel = static_cast<PageChunkTexture*>(
		PanelManager::instance().panels().getContent( PageChunkTexture::contentID ) );

	if ( !panel )
		return "";

	return panel->chunkName();
}


/**
 *  Static method that sets the current active chunk to manage in the page.
 *
 *  @param chunk		chunk to refresh
 *  @param show			optional that if true, forces showing the page
 */
/*static*/ void PageChunkTexture::chunk( const std::string& chunk, bool show /*=false*/ )
{
	BW_GUARD;

	PageChunkTexture* panel = static_cast<PageChunkTexture*>(
		PanelManager::instance().panels().getContent( PageChunkTexture::contentID ) );

	if ( !panel )
	{
		if ( show )
		{
			panel = static_cast<PageChunkTexture*>(
				PanelManager::instance().panels().insertPanel( PageChunkTexture::contentID, GUITABS::FLOATING ) );
			if ( !panel )
				return;
		}
		else
		{
			return;
		}
	}

	if ( panel )
	{
		if ( show )
		{
			PanelManager::instance().panels().showPanel( panel, true );
		}
		panel->setChunk( chunk );
	}
}


/**
 *  Static method that refreshes the texturesControl if the chunk param the
 *  same asis the active one in the page.
 *
 *  @param chunk		chunk to refresh
 */
/*static*/ void PageChunkTexture::refresh( const std::string& chunk )
{
	BW_GUARD;

	PageChunkTexture* panel = static_cast<PageChunkTexture*>(
		PanelManager::instance().panels().getContent( PageChunkTexture::contentID ) );

	if ( !panel )
		return;

	if ( chunk == panel->chunkName() )
	{
		panel->setChunk( chunk );
	}
}


/**
 *  Static method that returns if the cursor is in tracking mode or not.
 *
 *  @returns		true if the cursor is in tracking mode, false otherwise.
 */
/*static*/ bool PageChunkTexture::trackCursor()
{
	BW_GUARD;

	PageChunkTexture* panel = static_cast<PageChunkTexture*>(
		PanelManager::instance().panels().getContent( PageChunkTexture::contentID ) );

	if ( !panel )
		return false;

	return panel->chunkTrack_.GetCheck() == BST_CHECKED;
}


/**
 *  Static method sets the tracking mode
 *
 *  @returns		true if the cursor is in tracking mode, false otherwise.
 */
/*static*/ void PageChunkTexture::trackCursor( bool track )
{
	BW_GUARD;

	PageChunkTexture* panel = static_cast<PageChunkTexture*>(
		PanelManager::instance().panels().getContent( PageChunkTexture::contentID ) );

	if ( !panel )
		return;

	panel->chunkTrack_.SetCheck( track ? BST_CHECKED : BST_UNCHECKED );
}


void PageChunkTexture::DoDataExchange(CDataExchange* pDX)
{
	BW_GUARD;

	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAGE_TEXTURE_LATCH, latch_);
    DDX_Control(pDX, IDC_CHUNK_TEXTURE_CHUNK, chunkID_ );
	DDX_Control(pDX, IDC_CHUNK_TEXTURES_AUTOTRACK, chunkTrack_ );
	DDX_Control(pDX, IDC_CHUNK_TEXTURES_SHOW, chunkShow_ );
    DDX_Control(pDX, IDC_PAGE_TERRAIN_TEXS_PER_CHUNK_EDIT, warningNum_ );
	DDX_Control(pDX, IDC_PAGE_TERRAIN_TEXS_PER_CHUNK_BUTTON, warningShow_ );
	DDX_Control
    (
        pDX, 
        IDC_PAGE_TERRAIN_TEX_MANAGE_SEPARATOR, 
        textureManagementSeparator_
    );
    DDX_Control
    (
        pDX, 
        IDC_PAGE_TERRAIN_TEXTURES_CHUNK_SEPARATOR, 
        texturesChunkSeparator_
    );
}


/**
 *  Init the page
 */
void PageChunkTexture::InitPage()
{
	BW_GUARD;

	if ( pageReady_ )
		return;

	latch_.SetCheck( Options::getOptionInt( OPTION_LATCH ) ? BST_CHECKED : BST_UNCHECKED );

	chunkID_.EnableWindow( FALSE );

	chunkTrackButtonImg_ = (HICON)LoadImage( AfxGetInstanceHandle(),
			MAKEINTRESOURCE( IDI_CHUNK_TEXTURE_NOTRACK ),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	chunkTrackChkButtonImg_ = (HICON)LoadImage( AfxGetInstanceHandle(),
			MAKEINTRESOURCE( IDI_CHUNK_TEXTURE_TRACK ),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	chunkTrack_.SetIcon( chunkTrackButtonImg_ );
	chunkTrack_.SetCheckedIcon( chunkTrackChkButtonImg_ );
	chunkTrack_.SetCheck(
		Options::getOptionInt( OPTION_AUTOTRACK ) ? BST_CHECKED : BST_UNCHECKED );

	chunkShowButtonImg_ = (HICON)LoadImage( AfxGetInstanceHandle(),
			MAKEINTRESOURCE( IDI_CHUNK_TEXTURE_HIDE ),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	chunkShowChkButtonImg_ = (HICON)LoadImage( AfxGetInstanceHandle(),
			MAKEINTRESOURCE( IDI_CHUNK_TEXTURE_SHOW ),
			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	chunkShow_.SetIcon( chunkShowButtonImg_ );
	chunkShow_.SetCheckedIcon( chunkShowChkButtonImg_ );
	chunkShow_.SetCheck(
		Options::getOptionInt( OPTION_SHOWTRACK, 1 ) ? BST_CHECKED : BST_UNCHECKED );

	warningNum_.SetNumericType( controls::EditNumeric::ENT_INTEGER );
	warningNum_.SetValue(
		(float)Options::getOptionInt( OPTION_NUM_LAYER_WARNING, 0 ) );
	warningShow_.SetCheck(
		Options::getOptionInt( OPTION_SHOW_NUM_LAYER_WARNING, 1 ) ? BST_CHECKED : BST_UNCHECKED );
	warningShow_.SetIcon( chunkShowButtonImg_ );
	warningShow_.SetCheckedIcon( chunkShowChkButtonImg_ );

	texturesControl_.Create(WS_CHILD | WS_VISIBLE, textureControlExtents(), this);

	for
	(
		const char **ext = Moo::TextureManager::validTextureExtensions();
		*ext != NULL;
		++ext
	)
	{
		UalManager::instance().dropManager().add
		(
			new UalDropFunctor<PageChunkTexture>(
				&texturesControl_,
				*ext, 
				this,
				&PageChunkTexture::onDropTexture,
				false,
				&PageChunkTexture::onDropTextureTest )
		);
	}

	INIT_AUTO_TOOLTIP();

	pageReady_ = true;
}


/**
 *  Converts the content of the edit box and returns it as a std::string
 *
 *	@returns		content of the edit box as a std::string
 */
const std::string PageChunkTexture::chunkName() const
{
	BW_GUARD;

	CString chunk;
	chunkID_.GetWindowText( chunk );
	return bw_wtoutf8( chunk.GetString() );
}


/**
 *  Sets the chunk to be managing.
 *
 *  @param chunk    chunk name.
 */
void PageChunkTexture::setChunk( const std::string& chunk )
{
	BW_GUARD;

	chunkID_.SetWindowText( bw_utf8tow( chunk ).c_str() );
	if ( chunk.empty() )
	{
		PageChunkTextureView::chunk( NULL );
		setTextureControl( NULL );
		return;
	}

	Chunk* pChunk =
		ChunkManager::instance().findChunkByName( chunk, WorldManager::instance().geometryMapping() );
	if ( !pChunk )
	{
		ERROR_MSG( "PageChunkTexture::chunk: Invalid chunk name %s.\n", chunk.c_str() );
		chunkID_.SetWindowText( L"" );
		PageChunkTextureView::chunk( NULL );
		setTextureControl( NULL );
		return;
	}
	EditorChunkTerrain* pEct = static_cast<EditorChunkTerrain*>(
		ChunkTerrainCache::instance( *pChunk ).pTerrain() );
	if ( !pEct )
	{
		ERROR_MSG( "PageChunkTexture::chunk: No EditorChunkTerrain for chunk %s.\n", chunk.c_str() );
		chunkID_.SetWindowText( L"" );
		PageChunkTextureView::chunk( NULL );
		setTextureControl( NULL );
		return;
	}

	if ( chunkShow_.GetCheck() == BST_CHECKED )
	{
		PageChunkTextureView::chunk( pChunk );
	}
	else
	{
		PageChunkTextureView::chunk( NULL );
	}
	setTextureControl( pEct );
}


/**
 *  Sets the TerrainTexturesControl's chunk block and size.
 *
 *  @param block    chunk block to display.
 */
void PageChunkTexture::setTextureControl( EditorChunkTerrain *terrain )
{
	BW_GUARD;

	resize_ = false;
	texturesControl_.terrain( terrain );
    CRect sepExtents = 
        controls::childExtents(*this, IDC_PAGE_TERRAIN_TEXTURES_CHUNK_SEPARATOR);

	CSize viewSize = GetTotalSize();
	SetScrollSizes(
		MM_TEXT,
		CSize( viewSize.cx, sepExtents.bottom + texturesControl_.height() ) );

	texturesControl_.resizeFullHeight( viewSize.cx - CONTROL_OFFSET * 2 );
	texturesControl_.Invalidate();
	texturesControl_.UpdateWindow();
	resize_ = true;
}


/**
 *  This function calculates the extents of the TerrainTexturesControl.
 *
 *  @returns    The position of the TerrainTexturesControl.
 */
CRect PageChunkTexture::textureControlExtents(int cx /*=0*/, int cy /*=0*/) const
{
	BW_GUARD;

    CRect clientRect;
    GetClientRect(&clientRect);
    if (cx != 0)
        clientRect.right = clientRect.left + cx;
    if (cy != 0)
        clientRect.bottom = clientRect.top + cy;
    CRect  sepExtents = 
        controls::childExtents(*this, IDC_PAGE_TERRAIN_TEXTURES_CHUNK_SEPARATOR);
    return 
        CRect
        (
            clientRect.left   + CONTROL_OFFSET,
            sepExtents.bottom + CONTROL_OFFSET,
            clientRect.right  - CONTROL_OFFSET,
            clientRect.bottom - CONTROL_OFFSET
        );
}


/**
 *  This function sets editing mode in the TerrainTexturePage and starts editing
 *	the appropriate layer.
 */
void PageChunkTexture::editLayerInfo( int idx )
{
	BW_GUARD;

	EditorChunkTerrainPtr terrain = texturesControl_.terrain();
	TerrainTextureUtils::LayerInfo layerInfo;
	layerInfo.terrain_		= terrain;
	layerInfo.layerIdx_		= idx;
	layerInfo.textureName_	= terrain->block().textureLayer(idx).textureName();
	if (PageTerrainTexture::instance() != NULL)
		PageTerrainTexture::instance()->editTextureLayer(layerInfo);
}


/**
 *	This function sets the texture mask in the TerrainTexturePage.
 */
void PageChunkTexture::setTextureMask( int idx )
{
	BW_GUARD;

	EditorChunkTerrainPtr terrain = texturesControl_.terrain();
	Terrain::TerrainTextureLayer const &layer =
		terrain->block().textureLayer(idx);
	if (PageTerrainTexture::instance() != NULL)
	{
		PageTerrainTexture::instance()->selectTextureMask
		(
			layer.textureName(),
			layer.hasUVProjections(),
			layer.uProjection(),
			layer.vProjection(),
			false
		);
	}
}


/**
 *  This deletes the selected layers.
 *
 *	@param selection		The layers to delete.
 */
void PageChunkTexture::deleteLayers( std::vector<size_t> const &selection )
{
	BW_GUARD;

	if (selection.size() == 0)
		return;

	Terrain::EditorBaseTerrainBlock* block = texturesControl_.terrainBlock();
	Chunk* pChunk =
		ChunkManager::instance().findChunkByName(
			chunkName(), WorldManager::instance().geometryMapping() );
	EditorChunkTerrainPtr ect = texturesControl_.terrain();
	if ( !block || !pChunk || !ect)
		return;

	UndoRedo::instance().add( new TerrainTextureLayerUndo( block, pChunk ) );
	UndoRedo::instance().barrier( LocaliseUTF8(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/LAYER_DELETE"), true );

	int dstIdx = TerrainTextureUtils::mostPopulousLayer(*block, selection);
	if ( dstIdx >= 0 )
	{
		std::vector<size_t> mergedLayers = selection;
		mergedLayers.push_back(dstIdx);
		TerrainTextureUtils::mergeLayers(ect, mergedLayers);
	}

	// refresh the textures control
	texturesControl_.refresh();
	WorldManager::instance().changedTerrainBlock( pChunk , false );
}


/**
 *  This merges the selected layers.
 *
 *	@param selection		The layers to delete.
 */
void PageChunkTexture::mergeLayers( std::vector<size_t> const &selection )
{
	BW_GUARD;

	if (selection.empty())
		return;

	Terrain::EditorBaseTerrainBlock* block = texturesControl_.terrainBlock();
	Chunk* pChunk =
		ChunkManager::instance().findChunkByName(
			chunkName(), WorldManager::instance().geometryMapping() );
	EditorChunkTerrainPtr ect = texturesControl_.terrain();
	if ( !block || !pChunk || !ect)
		return;

	UndoRedo::instance().add( new TerrainTextureLayerUndo( block, pChunk ) );
	UndoRedo::instance().barrier( LocaliseUTF8(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/LAYERS_MERGED"), true );

	TerrainTextureUtils::mergeLayers(ect, selection);

	// refresh the textures control
	texturesControl_.refresh();
	WorldManager::instance().changedTerrainBlock( pChunk , false );
}


/**
 *  Enables/Disables all controls on the dialog.
 */
void PageChunkTexture::enableControls( bool enable )
{
	BW_GUARD;

	BOOL winEnable = enable ? TRUE : FALSE;
	int  winShow   = enable ? SW_SHOW : SW_HIDE;

	enabled_ = enable;

	if ( enable )
		OnBnClickedShow();
	else
		PageChunkTextureView::chunk( NULL );

	chunkShow_.EnableWindow( winEnable );
	chunkTrack_.EnableWindow( winEnable );
	warningNum_.EnableWindow( winEnable );
	warningShow_.EnableWindow( winEnable );
	texturesControl_.ShowWindow( winShow );
}


BEGIN_MESSAGE_MAP(PageChunkTexture, CFormView)
	ON_MESSAGE(WM_ACTIVATE_TOOL  , OnActivateTool  )
	ON_MESSAGE(WM_UPDATE_CONTROLS, OnUpdateControls)
	ON_MESSAGE(WM_NEW_SPACE, OnNewSpace)
	ON_MESSAGE(WM_TERRAIN_TEXTURE_SELECTED, OnTerrainTextureSelected)
	ON_MESSAGE(WM_TERRAIN_TEXTURE_RIGHT_CLICK, OnTerrainTextureRightClick)
	ON_MESSAGE(WM_CLOSING_PANELS, OnClosing)
	ON_BN_CLICKED(IDC_PAGE_TEXTURE_LATCH, OnBnClickedLatch)
	ON_BN_CLICKED(IDC_CHUNK_TEXTURES_AUTOTRACK, OnBnClickedTrack)
	ON_BN_CLICKED(IDC_CHUNK_TEXTURES_SHOW, OnBnClickedShow)
	ON_EN_CHANGE(IDC_PAGE_TERRAIN_TEXS_PER_CHUNK_EDIT, OnEnChangeWarning)
	ON_BN_CLICKED(IDC_PAGE_TERRAIN_TEXS_PER_CHUNK_BUTTON, OnBnClickedWarning)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// PageChunkTexture message handlers

/**
 *  Message sent by the BigWorld app whenever the current tool changes
 */
LRESULT PageChunkTexture::OnActivateTool( WPARAM wParam, LPARAM lParam )
{
	BW_GUARD;

	if ( !pageReady_ )
		InitPage();

	const wchar_t *toolId = (const wchar_t *)wParam;
	if ( toolId == PageTerrainTexture::contentID )
	{
		if ( Options::getOptionInt( OPTION_LATCH ) && !IsWindowVisible() )
		{
			PanelManager::instance().showPanel( contentID, true );
		}
		enableControls( true );
	}
	else
	{
		enableControls( false );
	}

	return 0;
}


/**
 *  Message sent by the BigWorld app each frame
 */
LRESULT PageChunkTexture::OnUpdateControls(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	if ( !pageReady_ )
		InitPage();

	if ( enabled_ &&
		chunkTrack_.GetCheck() == BST_CHECKED &&
		ToolManager::instance().tool() &&
		ToolManager::instance().tool()->currentChunk() )
	{
		std::string chunk = ToolManager::instance().tool()->currentChunk()->identifier();
		if ( chunk != chunkName() )
		{
			setChunk( chunk );
		}
	}

	return 0;
}


/**
 *  Message sent by the BigWorld app each frame
 */
LRESULT PageChunkTexture::OnNewSpace(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	setChunk( "" );
	return 0;
}


/**
 *  Message sent by the TerrainTextureControl when a texture is selected
 */
LRESULT PageChunkTexture::OnTerrainTextureSelected(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	Terrain::TerrainTextureLayer & ttl =
		texturesControl_.terrainBlock()->textureLayer( (int)lParam );

	Vector4 u, v;
	Terrain::TerrainTextureLayer::defaultUVProjections(u, v);
	if (ttl.hasUVProjections())
	{
		u = ttl.uProjection();
		v = ttl.vProjection();
	}

	TerrainPaintBrushPtr brush = TerrainPaintBrushPtr( new TerrainPaintBrush(), true );
	brush->paintTexture_	= ttl.textureName();
	brush->paintUProj_		= u;
	brush->paintVProj_		= v;

	if (PageTerrainTexture::instance() != NULL)
	{
		PageTerrainTexture::instance()->setTerrainBrush( brush );
	}

	return 0;
}


/**
 *  Message sent by the TerrainTextureControl when a texture is right-clicked
 */
LRESULT PageChunkTexture::OnTerrainTextureRightClick(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	std::vector<size_t> const &selection = texturesControl_.selection();

	PopupMenu menu;

	const int selectCmd = 1;
	const int editCmd   = 2;
	const int maskCmd	= 3;
	const int deleteCmd = 4;
	const int mergeCmd  = 5;
	menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_USE_TEXTURE"), selectCmd );

	Terrain::EditorBaseTerrainBlock* block = texturesControl_.terrainBlock();
	if ( block && block->textureLayer( (int)lParam ).hasUVProjections() )
	{
		menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_EDIT_PROJECTION_AND_SCALE"), editCmd );
	}

	if (block && block->numberTextureLayers() != 0)
	{
		menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_SET_TEXTURE_MASK"), maskCmd );
	}

	bool addedSeparator = false;

	if ( block && selection.size() > 0 && selection.size() != block->numberTextureLayers())
	{
		if (!addedSeparator)
		{
			addedSeparator = true;
			menu.addSeparator();
		}

		if (selection.size() == 1)
			menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_REMOVE_TEXTURE"), deleteCmd );
		else
			menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_REMOVE_TEXTURES"), deleteCmd );
	}

	if (block && selection.size() > 1)
	{
		if (!addedSeparator)
		{
			addedSeparator = true;
			menu.addSeparator();
		}

		menu.addItem( Localise(L"WORLDEDITOR/GUI/PAGE_CHUNK_TEXTURE/MENU_MERGE_TEXTURES"), mergeCmd );
	}

	int result = menu.doModal( *this );

	std::vector<size_t> newSelection;
	newSelection.push_back(lParam);

	switch ( result )
	{
	case selectCmd:
		OnTerrainTextureSelected( wParam, lParam );
		texturesControl_.selection(newSelection);
		break;

	case editCmd:
		editLayerInfo( (int)lParam );
		texturesControl_.selection(newSelection);
		break;

	case maskCmd:
		setTextureMask( (int)lParam );
		texturesControl_.selection(newSelection);
		break;

	case deleteCmd:
		deleteLayers( selection );
		break;

	case mergeCmd:
		mergeLayers( selection );
		break;
	}

	return 0;
}


/**
 *	This is called when the program is closing down.
 *
 *	@param wParam	ignored.
 *	@param lParam	ignored.
 *	@returns		0.
 */
LRESULT PageChunkTexture::OnClosing(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	BW_GUARD;

	texturesControl_.terrain(NULL); // Remove any references to terrain blocks
	return 0;
}


/**
 *  This is called when the page is resized.
 *
 *  @param type     The type of resize event.
 *  @param cx       The new width.
 *  @param cy       The new height.
 */
void PageChunkTexture::OnSize(UINT type, int cx, int cy)
{
	BW_GUARD;

    CFormView::OnSize(type, cx, cy);

	if ( cx <= 1 && cy <= 1 )
	{
		// Somehow, this method gets called twice on resize, and the first
		// time is with size 1,1. In order to avoid redrawing problems, we
		// just ignore these ultra-small resize events.
		return;
	}

	controls::childResize
	(
	    textureManagementSeparator_, 
	    SEP_OFFSET     , controls::NO_RESIZE, 
	    cx - SEP_OFFSET, controls::NO_RESIZE
	);

	controls::childResize
	(
	    texturesChunkSeparator_, 
	    SEP_OFFSET     , controls::NO_RESIZE, 
	    cx - SEP_OFFSET, controls::NO_RESIZE
	);

	if ( resize_ )
		texturesControl_.resizeFullHeight( cx - CONTROL_OFFSET * 2 );

	RedrawWindow( NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN );
}


/**
 *	Handler for enabling/disabling latching to the PageTerrainTexture tool
 */
void PageChunkTexture::OnBnClickedLatch()
{
	BW_GUARD;

	Options::setOptionInt(
		OPTION_LATCH,
		latch_.GetCheck() == BST_CHECKED ? 1 : 0 );
}


/**
 *	Handler for enabling/disabling auto-track of the current chunk
 */
void PageChunkTexture::OnBnClickedTrack()
{
	BW_GUARD;

	if ( chunkTrack_.GetCheck() == BST_CHECKED )
	{
		Options::setOptionInt( OPTION_AUTOTRACK, 1 );
	}
	else
	{
		Options::setOptionInt( OPTION_AUTOTRACK, 0 );
		// clear the current chunk
		setChunk( "" );
	}
}


/**
 *	Handler for showing/hiding the chunk highlight
 */
void PageChunkTexture::OnBnClickedShow()
{
	BW_GUARD;

	std::string chunk = chunkName();
	Chunk* pChunk = NULL;
	if ( !chunk.empty() )
	{
		pChunk = ChunkManager::instance().findChunkByName( chunk, WorldManager::instance().geometryMapping() );
	}

	if ( chunkShow_.GetCheck() == BST_CHECKED )
	{
		Options::setOptionInt( OPTION_SHOWTRACK, 1 );
		PageChunkTextureView::chunk( pChunk );
	}
	else
	{
		Options::setOptionInt( OPTION_SHOWTRACK, 0 );
		PageChunkTextureView::chunk( NULL );
	}
}


/**
 *	Handler for when the chunk warning for too-much-textures changes
 */
void PageChunkTexture::OnEnChangeWarning()
{
	BW_GUARD;

	int numTex = max( 0, (int)warningNum_.GetValue() );
	int lastNumTex = Options::getOptionInt(OPTION_NUM_LAYER_WARNING);
	if (numTex != lastNumTex)
	{
		Options::setOptionInt( OPTION_NUM_LAYER_WARNING, numTex );
	}
}


/**
 *	Handler for showing/hiding the chunk warning for too-much-textures
 */
void PageChunkTexture::OnBnClickedWarning()
{
	BW_GUARD;

	Options::setOptionInt(
		OPTION_SHOW_NUM_LAYER_WARNING,
		(warningShow_.GetCheck() == BST_CHECKED) ? 1 : 0 );
}


/**
 *	This gets called on every message passed to the window before it is
 *	processed.  We override it to look for key-strokes which are normally
 *	filtered out in dialogs.
 *
 *	@param msg		The message passed to the window.
 *	@return			TRUE if the message was processed, FALSE otherwise.
 */
/*virtual*/ BOOL PageChunkTexture::PreTranslateMessage(MSG *msg)
{
	BW_GUARD;

	CALL_TOOLTIPS(msg);
	CWnd *focusWnd = GetFocus();
	if 
	(
		msg->message == WM_KEYDOWN 
		&& 
		focusWnd != NULL 
		&& 
		focusWnd->GetSafeHwnd() == texturesControl_.GetSafeHwnd()
	)
	{
		UINT repCnt = msg->lParam & 0x0f; // From http://msdn2.microsoft.com/en-us/library/ms646280.aspx
		UINT flags	= msg->lParam >> 4;
		OnKeyDown(msg->wParam, repCnt, flags);
		return TRUE;
	}
	else
	{
		return CFormView::PreTranslateMessage(msg);
	}
}


/**
 *	This is called when the user drops a texture onto the texturesControl_.
 *
 *	@param ii		The drop information.
 *	@returns		True if the drop was successful.
 */
bool PageChunkTexture::onDropTexture(UalItemInfo *ii)
{
	BW_GUARD;

	// Do some sanity checks:
	if (ii == NULL)
		return false;
	EditorChunkTerrainPtr terrain = texturesControl_.terrain();
	if (!terrain)
		return false;

	// Find the layer that the user dragged onto:
	CPoint pt(ii->x(), ii->y());
	texturesControl_.ScreenToClient(&pt);
	int layer = texturesControl_.layerAtPoint(pt);
	if (layer < 0)
		return false;

	// Get the default projections:
	Vector4 uproj, vproj;
	Terrain::TerrainTextureLayer::defaultUVProjections(uproj, vproj);

	bool isCtrlDown = ::GetAsyncKeyState(VK_CONTROL) < 0;

	// Replace one texture with another:
	return
		TerrainTextureUtils::replaceTexture
		(
			(size_t)layer, 
			terrain,
			BWResource::dissolveFilename(bw_wtoutf8( ii->longText()) ),
			uproj,
			vproj,
			true, 		// undoable
			true, 		// add undo barrier
			isCtrlDown 	// do a flood-fill only if the ctrl key is down
		);
}


/**
 *	This is called when testing a drag and drop operation.
 *
 *	@param ii		The drop information.
 *	@return 		True if the drop was successful, false otherwise.
 */
CRect PageChunkTexture::onDropTextureTest( UalItemInfo *ii )
{
	BW_GUARD;

	CRect result( UalDropManager::HIT_TEST_MISS ); 

	if (ii != NULL)
	{
		CPoint pt(ii->x(), ii->y());
		texturesControl_.ScreenToClient( &pt );
		CRect layerRect;
		if (texturesControl_.layerRectAtPoint( pt, layerRect ))
		{
			result = layerRect;
		}
	}

	return result;
}


/**
 *	This is called when a key press is made.
 *
 *	@param key		The key pressed down.
 *	@param repCnt	The repeat count.
 *	@param flags	Key flags such as the control key state.
 */
void PageChunkTexture::OnKeyDown(UINT key, UINT /*repCnt*/, UINT /*flags*/)
{
	BW_GUARD;

	if (key == VK_DELETE)
	{
		std::vector<size_t> const &selection = texturesControl_.selection();
		deleteLayers(selection);
	}
	else if (key == VK_ESCAPE)
	{
		std::vector<size_t> emptySelection;
		texturesControl_.selection(emptySelection);
	}
	else if (key == VK_SPACE)
	{
		std::vector<size_t> const &selection = texturesControl_.selection();
		mergeLayers(selection);
	}
}
