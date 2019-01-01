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
#include "init.hpp"

#include "render_context.hpp"

namespace Moo
{

// Is this library initialised?
static bool	    s_initialised	= false;

// Library globals
RenderContext*  g_RC			= NULL;

bool init()
{
	BW_GUARD;
	if ( !s_initialised )
	{
		// Create render context
		MF_ASSERT_DEV( g_RC == NULL );
		if( g_RC == NULL )
			g_RC = new RenderContext();
		s_initialised = g_RC->init();

		return s_initialised;
	}

	return true;
}

void fini()
{
	BW_GUARD;
	if ( s_initialised )
	{
		// Destroy render context
		g_RC->fini();
		delete g_RC;
		g_RC = NULL;
		
		s_initialised = false;
	}
}

} // namespace Moo
