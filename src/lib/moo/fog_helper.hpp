/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FOG_HELPER_HPP
#define FOG_HELPER_HPP

namespace Moo
{
	/**
	 *	This collection of methods helps with fog setting.
	 *	The methods automatically masks out settings that are not
	 *	supported by the underlying hardware, substituting vertex
	 *	fog for table fog when table fog is not supported.
	 */
	namespace FogHelper
	{
		void setFog( float start, float end, 
			D3DRENDERSTATETYPE fogState, D3DFOGMODE mode );
		void setFogStart( float start );
		void setFogEnd( float end );
		void setFogVertexMode(D3DFOGMODE mode);
		void setFogTableMode(D3DFOGMODE mode);
		bool tableModeSupported();
		void setFogColour( uint32 colour );
		void setFogEnable( bool state );
	};
};

#endif // FOG_HELPER_HPP
