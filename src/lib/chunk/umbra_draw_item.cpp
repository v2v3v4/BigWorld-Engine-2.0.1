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

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_draw_item.hpp"

#include <umbracell.hpp>
#include <umbraobject.hpp>

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 );

/*
 * Destructor
 */
UmbraDrawItem::~UmbraDrawItem()
{
	// Make sure we remove our draw item from the cell
	// As the cell contains a reference to the draw item
	if (pObject_.exists() && pObject_->object())
		updateCell( NULL );
}


/**
 *	This helper method updates the umbra objects cell
 *	@param pNewCell the cell to move the object to
 */
void UmbraDrawItem::updateCell( Umbra::Cell* pNewCell )
{
	MF_ASSERT( pObject_.exists() && pObject_->object() );
	pObject_->object()->setCell( pNewCell );
}

/**
 *	This helper method updates the umbra objects transform
 *	@param newTransform the new transform of the object
 */
void UmbraDrawItem::updateTransform( const Matrix& newTransform )
{
	MF_ASSERT( pObject_.exists() && pObject_->object() );
	pObject_->object()->setObjectToCellMatrix( (Umbra::Matrix4x4&)newTransform );
}

#endif // UMBRA_ENABLE
