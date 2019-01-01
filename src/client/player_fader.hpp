/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __BWCLIENT_PLAYER_FADER_HPP__
#define __BWCLIENT_PLAYER_FADER_HPP__

#include "post_processing/phase.hpp"
#include "post_processing/phase_factory.hpp"
#include "romp/py_render_target.hpp"

/**
 *	This class is a post-processing phase that fades out the player.
 *	At tick time, it checks whether or not the near-plane clips
 *	through the player and if so, makes the player's model invisible. 
 *
 *	At draw time, if the player was made invisible, then it simply
 *	renders the player into the currently set render target.
 *	given uses the input textureFullScreenBackBuffer as a buffer
 *	to draw the character and then copies it back onto the real BackBuffer
 *	translucently.
 */

namespace PostProcessing
{

class PlayerFader : public Phase
{
	Py_Header( PlayerFader, Phase )
	DECLARE_PHASE( PlayerFader )
public:
	PlayerFader( PyTypePlus *pType = &s_type_ );
	~PlayerFader();

	void tick( float dTime );
	void draw( class Debug*, RECT* = NULL );
	bool load( DataSectionPtr );
	bool save( DataSectionPtr );

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_RW_ATTRIBUTE_DECLARE( pRenderTarget_, renderTarget )
	PY_RW_ATTRIBUTE_DECLARE( clearRenderTarget_, clearRenderTarget )
	PY_RW_ATTRIBUTE_DECLARE( name_, name )
	PY_RO_ATTRIBUTE_DECLARE( pOpacity_, opacity )

	PY_FACTORY_DECLARE()
	
private:
	void transparency( float amt );
	bool clearRenderTarget_;
	PyRenderTargetPtr pRenderTarget_;
	std::string	name_;

	float transparency_;
	///player transparency power
	float ptp_;
	///max player transparency
	float maxPt_;
	///desired src alpha blend of off-screen buffer
	Vector4ProviderPtr pOpacity_;
	///just ignore everything if the player was invisible to begin with.
	bool wasVisible_;

	SmartPointer<Vector4Basic> opacity_;
	void updateOpacity();
};

}

#endif // __BWCLIENT_PLAYER_FADER_HPP__