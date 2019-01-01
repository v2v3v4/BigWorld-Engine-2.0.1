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
#include "render_context.hpp"
#include "fog_helper.hpp"

/**
 *	This helper function sets up the fog distance and fog mode
 *
 *	@param start the start distance of the fog
 *	@param end the end distance of the fog
 *	@param fogState either D3DRS_FOGTABLEMODE or D3DRS_FOGVERTEXMODE
 *	@param mode the fogging mode to use i.e. D3DFOG_LINEAR, D3DFOG_EXP
 */
void Moo::FogHelper::setFog( float start, float end, 
							D3DRENDERSTATETYPE fogState, D3DFOGMODE mode )
{
	BW_GUARD;
	setFogStart( start );
	setFogEnd( end );
	if (fogState == D3DRS_FOGTABLEMODE)
		setFogTableMode( mode );
	else
		setFogVertexMode(mode );
}

/**
 *	This helper function sets the Fogs enable state
 *
 *	@param state true to enable fog false to disable
 */
void Moo::FogHelper::setFogEnable( bool state )
{
	BW_GUARD;
	Moo::rc().setRenderState( D3DRS_FOGENABLE, state );
}

/**
 *	This helper function sets the Fogs colour.
 *
 *	@param colour The colour to set the Fog to.
 */
void Moo::FogHelper::setFogColour( uint32 colour )
{
	BW_GUARD;
	Moo::rc().setRenderState( D3DRS_FOGCOLOR, colour );
}

/**
 *	This helper function sets the Fog start state.
 *
 *	@param start The start distance for fog.
 */
void Moo::FogHelper::setFogStart( float start )
{
	BW_GUARD;
	Moo::rc().setRenderState( D3DRS_FOGSTART, *(DWORD*)&start );
}

/**
 *	This helper function sets the Fog end state.
 *
 *	@param end The end distance for fog.
 */
void Moo::FogHelper::setFogEnd( float end )
{
	BW_GUARD;
	Moo::rc().setRenderState( D3DRS_FOGEND, *(DWORD*)&end );
}

/**
 *	This helper function sets the fog vertex mode, it also 
 *	disables the fog table mode.
 *
 *	@param mode The fog vertex mode
 */
void Moo::FogHelper::setFogVertexMode( D3DFOGMODE mode )
{
	BW_GUARD;
	if (tableModeSupported())
		Moo::rc().setRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
	Moo::rc().setRenderState( D3DRS_FOGVERTEXMODE, mode );
}

/**
 *	This helper function sets the fog table mode, it also 
 *	disables the fog vertex mode.
 *
 *	@param mode The fog table mode
 */
void Moo::FogHelper::setFogTableMode( D3DFOGMODE mode )
{
	BW_GUARD;
	if (tableModeSupported())
	{
		setFogVertexMode( D3DFOG_NONE );
		Moo::rc().setRenderState( D3DRS_FOGTABLEMODE, mode );
	}
	else
	{
		setFogVertexMode( mode );
	}
}

/**
 *	This helper function checks whether fog table mode is supported.
 *
 *	@return true if table mode is supported.
 */
bool Moo::FogHelper::tableModeSupported()
{
	BW_GUARD;
	uint32 caps = rc().deviceInfo(rc().deviceIndex()).caps_.RasterCaps;
	return (caps & D3DPRASTERCAPS_FOGTABLE) != 0;
}
