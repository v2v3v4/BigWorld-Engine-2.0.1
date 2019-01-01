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

#include "ual_drop_manager.hpp"


// Constants implementation.
/*static*/ CRect UalDropManager::HIT_TEST_NONE = CRect( -1, -1, -1, -1 );
/*static*/ CRect UalDropManager::HIT_TEST_MISS = CRect(  0,  0,  0,  0 );
/*static*/ CRect UalDropManager::HIT_TEST_OK_NO_RECT = CRect( -5,  -5,  -5,  -5 );


/**
 *	Constructor.
 */
UalDropManager::UalDropManager():
	ext_(""),
	pen_( PS_SOLID, DRAG_BORDER, DRAG_COLOUR ),
	lastHighlightRect_( 0, 0, 0, 0 ),
	lastHighlightWnd_( NULL )
{
	BW_GUARD;
}


/**
 *	This method adds a drop target callback to the manager.
 *
 *	@param dropping		Drop traget callback object.
 *	@param useHighlighting	True to automatically draw a halftoned rectangle
 *							arround the drop target when the mouse is over it,
 *							false to not have anything drawn by the manager.
 */
void UalDropManager::add( SmartPointer< UalDropCallback > dropping, bool useHighlighting /*= true*/ )
{
	BW_GUARD;

	if (dropping == NULL || dropping->cWnd() == NULL)
	{
		WARNING_MSG( "UalDropManager::add: Tried to add a NULL drop target.\n" );
		return;
	}
	droppings_.insert( DropType( dropping->hWnd(), dropping ) );

	if (useHighlighting)
	{
		dontHighlightHwnds_.erase( dropping->hWnd() );
	}
	else
	{
		dontHighlightHwnds_.insert( dropping->hWnd() );
	}
}


/**
 *	This method must be called when drag and drop operation starts.
 *
 *	@param ext	Filename extension of the dragged asset.
 */
void UalDropManager::start( const std::string& ext )
{
	BW_GUARD;

	ext_ = ext;
	std::transform( ext_.begin(), ext_.end(), ext_.begin(), tolower );
	lastHighlightWnd_ = NULL;
}


/**
 *	This method draws a halftoned highlight rectangle.
 *
 *	@param hwnd		Window handle of the window on which the highlight
 *					rectangle will be drawn.
 *	@param rect		Rectangle coordinates and size.
 */
void UalDropManager::drawHighlightRect( HWND hwnd, const CRect & rect )
{
	BW_GUARD;

	CWnd * pWnd = CWnd::FromHandle( hwnd );
	CRect wndRect;
	pWnd->GetWindowRect( wndRect );
	CRect xformedRect( rect );
	xformedRect.OffsetRect( -wndRect.left, -wndRect.top );
	CDC * dc = pWnd->GetDCEx( NULL, DCX_WINDOW | DCX_CACHE );
	CPen * oldPen = dc->SelectObject( &pen_ );
	int oldROP = dc->SetROP2( R2_NOTXORPEN );
	dc->Rectangle( xformedRect );
	dc->SetROP2( oldROP );
	dc->SelectObject( oldPen );
	pWnd->ReleaseDC( dc );
}


/**
 *	This method highlights a drop target window.
 *
 *	@param hwnd		Window handle of the window on which the highlight
 *					rectangle will be drawn.
 *	@param rect		Rectangle coordinates and size.
 */
void UalDropManager::highlight( HWND hwnd, const CRect & rect )
{
	BW_GUARD;

	if (rect == HIT_TEST_OK_NO_RECT)
	{
		// nothing to do, the user doesn't want our highlighting.
		lastHighlightWnd_ = NULL;
		return;
	}

	if (hwnd == lastHighlightWnd_ && rect == lastHighlightRect_)
	{
		// nothing to do, return to avoid flickering.
		return;
	}

	if (lastHighlightWnd_ && dontHighlightHwnds_.find( lastHighlightWnd_ ) == dontHighlightHwnds_.end())
	{
		drawHighlightRect( lastHighlightWnd_, lastHighlightRect_ );
	}

	if (hwnd && dontHighlightHwnds_.find( hwnd ) == dontHighlightHwnds_.end())
	{
		drawHighlightRect( hwnd, rect );
	}

	lastHighlightRect_ = rect;
	lastHighlightWnd_ = hwnd;
}


/**
 *	This method tests an item being dragged agains the drop targets associated
 *	to a window, if any.
 *
 *	@param hwnd		Window handle of the window under the mouse.
 *	@param ii		Information for the item being dragged.
 *	@return		The drop target callback object under the mouse, or NULL.
 */
SmartPointer< UalDropCallback > UalDropManager::test( HWND hwnd, UalItemInfo* ii )
{
	BW_GUARD;

	std::pair<DropMapIter,DropMapIter> drops = droppings_.equal_range( hwnd );
	for (DropMapIter i=drops.first; i!=drops.second; ++i)
	{
		if (i->second->ext().empty() || i->second->ext() == ext_)
		{
			CRect temp = i->second->test( ii );
			if (temp == HIT_TEST_NONE) // No test
			{
				i->second->cWnd()->GetClientRect( &highlightRect_ );
			}
			else if (temp == HIT_TEST_MISS) // Test failed
			{
				return NULL;
			}
			else // Success
			{
				highlightRect_ = temp;
			}
			
			i->second->cWnd()->ClientToScreen ( &highlightRect_ );
			return i->second;
		}
	}
	return NULL;
}


/**
 *	This method tests an item being dragged agains the drop targets.
 *
 *	@param ii		Information for the item being dragged.
 *	@return		The drop target callback object under the mouse, or NULL.
 */
SmartPointer< UalDropCallback > UalDropManager::test( UalItemInfo* ii )
{
	BW_GUARD;

	HWND hwnd = ::WindowFromPoint( CPoint( ii->x(), ii->y() ) );
	
	SmartPointer< UalDropCallback > drop = test( hwnd, ii );
	if (drop == NULL)
	{
		drop = test( ::GetParent( hwnd ), ii );
	}

	highlight( drop ? hwnd : NULL, highlightRect_ );

	return drop;
}


/**
 *	This method is called when the drag & drop operation has ended.  If
 *	successful (the mouse button was release over a drop target), the drop
 *	target's "execute" method is called.
 *
 *	@param ii	Information for the item being dragged.
 *	@return		True if the drag and drop operation was successful.
 */
bool UalDropManager::end( UalItemInfo* ii )
{
	BW_GUARD;

	SmartPointer< UalDropCallback > res = test( ii );

	highlight( NULL, highlightRect_ );
	
	ext_ = "";
	
	if (res)
		return res->execute( ii );

	return false;
}
