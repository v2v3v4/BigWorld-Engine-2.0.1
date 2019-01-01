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
#include "scissors_setter.hpp"

namespace Moo
{

bool ScissorsSetter::isAvailable()
{
	return ((rc().deviceInfo(rc().deviceIndex()).caps_.RasterCaps & D3DPRASTERCAPS_SCISSORTEST) != 0);
}

/**
 *	This constructor takes no arguments.  It simply rememebers the current
 *	scissor state, and sets it on the device when the object goes out of scope.
 */
ScissorsSetter::ScissorsSetter()
{
	rc().device()->GetScissorRect(&oldRect_);
	rc().pushRenderState( D3DRS_SCISSORTESTENABLE );	
}

/**
 *	This constructor takes x and y, width, height and sets a new
 *	temporary scissor region on the device using the passed in values
 *
 *	@param	x			x pixel offset of top left corner of new rect.
 *	@param	y			y pixel offset of top left corner of new rect. 
 *	@param	width		new scissor region width.
 *	@param	height		new scissor region height.
 */
ScissorsSetter::ScissorsSetter( uint32 x, uint32 y, uint32 width, uint32 height )
{
	BW_GUARD;
	rc().pushRenderState( D3DRS_SCISSORTESTENABLE );	
	rc().device()->GetScissorRect(&oldRect_); 	
	newRect_.left = x;
	newRect_.right = x + width;
	newRect_.top = y;
	newRect_.bottom = y + height;		
	rc().device()->SetScissorRect( &newRect_ );
	rc().setRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
}


/**
 *	This destructor automatically resets the saved viewport when
 *	the class goes out of scope.
 */
ScissorsSetter::~ScissorsSetter()
{
	BW_GUARD;
	rc().device()->SetScissorRect( &oldRect_ );
	rc().popRenderState();
}


}	//namespace Moo