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
#include "worldeditor/editor/item_frustum_locator.hpp"
#include "worldeditor/editor/item_view.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/misc/selection_filter.hpp"
#include "appmgr/options.hpp"
#include "chunk/chunk_umbra.hpp"
#include "chunk/chunk_item.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_vlo.hpp"
#include "physics2/worldtri.hpp"
#include "romp/geometrics.hpp"
#include "input/input.hpp"
#include "resmgr/auto_config.hpp"
#include "cstdmf/debug.hpp"
#include <hash_set>


DECLARE_DEBUG_COMPONENT2( "Tool", 0 )


static AutoConfigString s_selectionFxPrefix( "selectionfx/prefix" );
// Used for showing the triangles swept through the collision scene
//#define DRAW_DEBUG_GEOMETRY


#ifdef DRAW_DEBUG_GEOMETRY
#include "romp/debug_geometry.hpp"
#endif


// Define this macro to save the marquee render target texture to disk for
// debugging.
//#define DEBUG_RENDER_TARGET 1


static Moo::RenderTargetPtr s_renderTarget;

static Moo::RenderTargetPtr renderTarget()
{
	BW_GUARD;

	if( s_renderTarget == NULL )
	{
		s_renderTarget = new Moo::RenderTarget( "ChunkFrustumLocator" );
		s_renderTarget->create(
				Options::getOptionInt( "marqueeSelect/bufferWidth", 2048 ),
				Options::getOptionInt( "marqueeSelect/bufferHeight", 1024 ),
				false );
	}
	return s_renderTarget;
}

// -----------------------------------------------------------------------------
// Section: ChunkItemFrustumLocatorRevealer
// -----------------------------------------------------------------------------

class ChunkItemFrustumLocatorRevealer : public ChunkItemRevealer
{
	Py_Header( ChunkItemFrustumLocatorRevealer, ChunkItemRevealer )
public:
	ChunkItemFrustumLocatorRevealer( SmartPointer<ChunkItemFrustumLocator> pLoc,
			PyTypePlus * pType = &s_type_ ) :
		ChunkItemRevealer( pType ),
		pLoc_( pLoc )
	{
	}

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	// should really be a C++ virtual method...
	//  with attribute in base class
	PY_RO_ATTRIBUTE_DECLARE( pLoc_->items.size(), size );

private:
	virtual void reveal( std::vector< ChunkItemPtr > & items )
	{
		BW_GUARD;

		items.clear();

		std::vector<ChunkItemPtr>::iterator i = pLoc_->items.begin();
		for (; i != pLoc_->items.end(); ++i)
			items.push_back( *i );
	}

	SmartPointer<ChunkItemFrustumLocator>	pLoc_;
};

PY_TYPEOBJECT( ChunkItemFrustumLocatorRevealer )

PY_BEGIN_METHODS( ChunkItemFrustumLocatorRevealer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ChunkItemFrustumLocatorRevealer )
	PY_ATTRIBUTE( size )
PY_END_ATTRIBUTES()



/**
 *	Get an attribute for python
 */
PyObject * ChunkItemFrustumLocatorRevealer::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();
	
	return ChunkItemRevealer::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int ChunkItemFrustumLocatorRevealer::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return ChunkItemRevealer::pySetAttribute( attr, value );
}


// -----------------------------------------------------------------------------
// Section: ChunkItemFrustumLocator
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( ChunkItemFrustumLocator )

PY_BEGIN_METHODS( ChunkItemFrustumLocator )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ChunkItemFrustumLocator )
	PY_ATTRIBUTE( revealer )
	PY_ATTRIBUTE( subLocator )
	PY_ATTRIBUTE( enabled )
PY_END_ATTRIBUTES()

PY_FACTORY( ChunkItemFrustumLocator, Locator )

PY_SCRIPT_CONVERTERS( ChunkItemFrustumLocator );


ChunkItemFrustumLocator::ChunkItemFrustumLocator( ToolLocatorPtr pSub, PyTypePlus * pType ) :
	ToolLocator( pType ),
	subLocator_( pSub ),
	enabled_( true ),
	oldDrawReflection_( false )
{
	BW_GUARD;

	startPosition_.x = -1;
	startPosition_.y = -1;
	currentPosition_.x = -1;
	currentPosition_.y = -1;
	
	// push_back is fairly expensive if it doesn't have
	// capacity for the first few element, so reserve some
	// space in advance.
	items.reserve( 1024 );
}

ChunkItemFrustumLocator::~ChunkItemFrustumLocator()
{
}

Moo::Visual::DrawOverride* ChunkItemFrustumLocator::s_override_ = NULL;

Moo::Visual::DrawOverride* ChunkItemFrustumLocator::visualDrawOverride()
{
	BW_GUARD;

	if( s_override_ == NULL )
	{
		s_override_ = new MaterialDrawOverride( s_selectionFxPrefix, true );
	}
	return s_override_;
}

/*static*/ void ChunkItemFrustumLocator::fini()
{
	BW_GUARD;

	delete s_override_;
	s_override_ = NULL;
	s_renderTarget = NULL;
}

void ChunkItemFrustumLocator::enterSelectionMode()
{
	BW_GUARD;

	oldOverride_ = Moo::Visual::s_pDrawOverride;
	Moo::Visual::s_pDrawOverride = visualDrawOverride();

	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );
	renderTarget()->push();
	Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1, 0 );

	// set the default texture factor to 0, so if no objects are being drawn,
	// the pixels are set to 0 (NULL)
	Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, (DWORD)0x00000000 );

	WorldManager::instance().drawSelection( true );

	// Additional states that need changing
	oldDrawReflection_ = Waters::drawReflection();
	Waters::drawReflection( false );

	// Tick to make sure everything updates (i.e. water reflection mirrors)
	ChunkManager::instance().tick( 0.f );
}

void ChunkItemFrustumLocator::leaveSelectionMode()
{
	BW_GUARD;

	WorldManager::instance().drawSelection( false );

	// Restoring additional states changed
	Waters::drawReflection( oldDrawReflection_ );

	renderTarget()->pop();
	
#ifdef DEBUG_RENDER_TARGET
	static int count = 0;
	static char buf[256];
	bw_snprintf( buf, sizeof(buf), "c:\\marquee%04d.bmp", count++ );
	D3DXSaveTextureToFile( buf, D3DXIFF_BMP, renderTarget()->pTexture(), NULL );
#endif // DEBUG_RENDER_TARGET

	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );

	Moo::Visual::s_pDrawOverride = oldOverride_;
}

void ChunkItemFrustumLocator::calculatePosition( const Vector3& worldRay, Tool& tool )
{
	BW_GUARD;

	// first call our sublocator to set the matrix
	if (subLocator_)
	{
		subLocator_->calculatePosition( worldRay, tool );
		transform_ = subLocator_->transform();
	}
	else
	{
		transform_ = Matrix::identity;
	}

	// This is for debugging, so we can hold the right mouse button down and
	// inspect the generated triangles that get swept through the collision
	// scene
	if (InputDevices::isKeyDown( KeyCode::KEY_RIGHTMOUSE ) )
		return;

	// now find the chunk item
	if (enabled_)
	{
		if (startPosition_.x == -1)
		{
			startPosition_ = WorldManager::instance().currentCursorPosition();
		}

		POINT pt = WorldManager::instance().currentCursorPosition();

#ifndef DRAW_DEBUG_GEOMETRY
		// Don't do anything if the cursor hasn't moved
		if (pt.x == currentPosition_.x && pt.y == currentPosition_.y)
			return;
#endif

		currentPosition_ = pt;

		if (currentPosition_.x == startPosition_.x || currentPosition_.y == startPosition_.y)
			return;

		items.clear();

		enterSelectionMode();

		Moo::rc().beginScene();

		WorldManager::instance().renderChunks();

		Moo::LightContainerPtr lc = new Moo::LightContainer;

		lc->addDirectional(
			ChunkManager::instance().cameraSpace()->sunLight() );
		lc->ambientColour(
			ChunkManager::instance().cameraSpace()->ambientLight() );

		Moo::rc().lightContainer( lc );

		#if UMBRA_ENABLE
		if (!UmbraHelper::instance().umbraEnabled())
		{
			// With umbra, terrain gets rendered inside "renderChunks" via the
			// ChunkUmbra::terrainOverride.
		#endif
			WorldManager::instance().renderTerrain();
		#if UMBRA_ENABLE
		}
		#endif

		Moo::rc().endScene();

		leaveSelectionMode();
		
		DX::Texture* texture = (DX::Texture*)( renderTarget()->pTexture() );
		ComObjectWrap<DX::Surface> inmemSurface;
		ComObjectWrap<DX::Texture> inmemTexture;
		ComObjectWrap<DX::Surface> rtSurface;

		D3DSURFACE_DESC desc;
		D3DLOCKED_RECT lockRect;

		if( SUCCEEDED( texture->GetSurfaceLevel( 0, &rtSurface ) ) &&
			SUCCEEDED( rtSurface->GetDesc( &desc ) ) &&
			!!( inmemTexture = Moo::rc().createTexture( desc.Width, desc.Height, 0, 0, desc.Format, D3DPOOL_SYSTEMMEM,
				"texture/chunk item frustum" ) ) &&
			SUCCEEDED( inmemTexture->GetSurfaceLevel( 0, &inmemSurface ) ) &&
			SUCCEEDED( Moo::rc().device()->GetRenderTargetData( rtSurface.pComObject(), inmemSurface.pComObject() ) ) &&
			SUCCEEDED( inmemSurface->LockRect( &lockRect, NULL, 0 ) ) )
		{
			stdext::hash_set< ChunkItem * > fastItems;
			
			// This variable reduces processing time by a massive amount
			DWORD lastItemPixel = 0;
			// Look for objects
			VeryLargeObject::updateSelectionMark();
		
			uint32 minx = min( startPosition_.x, currentPosition_.x ) * renderTarget()->width() / uint32(Moo::rc().screenWidth());
			uint32 maxx = max( startPosition_.x, currentPosition_.x ) * renderTarget()->width() / uint32(Moo::rc().screenWidth());
			uint32 miny = min( startPosition_.y, currentPosition_.y ) * renderTarget()->height() / uint32(Moo::rc().screenHeight());
			uint32 maxy = max( startPosition_.y, currentPosition_.y ) * renderTarget()->height() / uint32(Moo::rc().screenHeight());

			for( uint32 y = miny; y <= maxy; ++y )
			{
				DWORD* pixel = (DWORD*)( (unsigned char*)lockRect.pBits + lockRect.Pitch * y ) + minx;
				DWORD* end = pixel + (maxx - minx + 1);
				for( ; pixel < end; ++pixel )
				{
					if( *pixel != 0 && *pixel != lastItemPixel )
					{
						lastItemPixel = *pixel;
						ChunkItem* item = ( ChunkItem* )(*pixel);
						if (WorldManager::instance().isDrawSelectionItemRegistered( item ))
						{
							// The pixel corresponds to an actual selectable chunk item.
							if (fastItems.find( item ) == fastItems.end() &&
								SelectionFilter::canSelect( item ))
							{
								if( strcmp( item->edClassName(), "ChunkVLO" ) == 0 && !item->edCheckMark( VeryLargeObject::selectionMark() ) )
									continue;
								fastItems.insert( item );
								items.push_back( item );
							}
						}
					}
				}
			}
			inmemSurface->UnlockRect();
		}
	}
	else
	{
		items.clear();
	}
}


/**
 *	Get an attribute for python
 */
PyObject * ChunkItemFrustumLocator::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();
	
	return ToolLocator::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int ChunkItemFrustumLocator::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return ToolLocator::pySetAttribute( attr, value );
}

/**
 *	Get a revealer object for the chunkitem in this locator
 */
PyObject * ChunkItemFrustumLocator::pyGet_revealer()
{
	BW_GUARD;

	return new ChunkItemFrustumLocatorRevealer( this );
}

/**
 *	Python factory method
 */
PyObject * ChunkItemFrustumLocator::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject * pSubLoc = NULL;
	if (!PyArg_ParseTuple( args, "|O", &pSubLoc ) ||
		pSubLoc && !ToolLocator::Check( pSubLoc ))
	{
		PyErr_SetString( PyExc_TypeError, "ChunkItemFrustumLocator() "
			"expects an optional ToolLocator argument" );
		return NULL;
	}

	ToolLocatorPtr spSubLoc = static_cast<ToolLocator*>( pSubLoc );

	return new ChunkItemFrustumLocator( spSubLoc );
}





















// -----------------------------------------------------------------------------
// Section: DragBoxView
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( DragBoxView )

PY_BEGIN_METHODS( DragBoxView )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( DragBoxView )
PY_END_ATTRIBUTES()

PY_FACTORY( DragBoxView, View )


DragBoxView::DragBoxView( ChunkItemFrustumLocatorPtr locator, Moo::Colour colour,
						 PyTypePlus * pType) :
	ToolView( pType ),
	locator_( locator ),
	colour_( colour )
{
}

DragBoxView::~DragBoxView()
{
}

/**
 *	This method draws the appropriate item selected
 */
void DragBoxView::render( const class Tool& tool )
{
	int minx = min( locator_->startPosition_.x, locator_->currentPosition_.x );
	int maxx = max( locator_->startPosition_.x, locator_->currentPosition_.x );
	int miny = min( locator_->startPosition_.y, locator_->currentPosition_.y );
	int maxy = max( locator_->startPosition_.y, locator_->currentPosition_.y );


	Geometrics::drawRect(
		Vector2( (float) minx, (float) miny ),
		Vector2( (float) maxx, (float) maxy ),
		colour_ );
}


/**
 *	Get an attribute for python
 */
PyObject * DragBoxView::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	
	return ToolView::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int DragBoxView::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();

	return ToolView::pySetAttribute( attr, value );
}
