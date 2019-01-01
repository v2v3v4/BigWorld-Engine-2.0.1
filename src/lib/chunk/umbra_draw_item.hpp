/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UMBRA_DRAW_ITEM_HPP
#define UMBRA_DRAW_ITEM_HPP

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_proxies.hpp"

class Chunk;
namespace Umbra
{
	class Cell;
};

/**
 *	This class defines the interface the 
 *	Umbra implementation uses to render objects.
 *	Any object that wants to be rendered through Umbra will need to create a 
 *	derived class and set up the umbra objects.
 *	The class contains three pure virtual methods that need to be implemented
 *	- draw()
 *	- drawDepth()
 *	- drawReflection()
 */
class UmbraDrawItem 
{
public:
	virtual Chunk*	draw(Chunk* pChunkContext) = 0;
	virtual Chunk*	drawDepth(Chunk* pChunkContext) = 0;
	virtual ~UmbraDrawItem();

	UmbraObjectProxyPtr pUmbraObject() { return pObject_; }

	void updateCell( Umbra::Cell* pNewCell );
	void updateTransform( const Matrix& newTransform );

protected:
	UmbraObjectProxyPtr	pObject_;
};

#endif // UMBRA_ENABLE
#endif // UMBRA_DRAW_ITEM_HPP
