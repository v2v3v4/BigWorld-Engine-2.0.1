/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	cooperative_moo.hpp
 */

#ifndef _COOPERATIVE_MOO_HPP_
#define _COOPERATIVE_MOO_HPP_


/**
 *	This class helps make Moo windowed apps cooperative with other DX apps.
 */
class CooperativeMoo
{
public:
	/// Operation mode enum type.
	enum MODE
	{
		OFF,
		ON,
		AUTO
	};

	static bool init( DataSectionPtr configSection = NULL );

	static void mode( MODE newMode );
	static MODE mode();

	static bool beginOnPaint();
	static void endOnPaint();

	static bool canUseMoo( bool isWindowActive, uint32 minTextureMemMB = 32 );

private:
	static bool s_inited_;
	static bool s_wasPaused_;
	static MODE s_mode_;
	static bool s_otherAppsRunning_;
	static uint64 s_lastCheckTime_;
	static std::string s_thisAppName_;
	static std::vector< std::wstring > s_otherApps_;

	static void tick();

	static void deactivate();

	static bool needsToCooperate();
	static bool isCooperativeApp( const std::wstring & procName );
};


#endif _COOPERATIVE_MOO_HPP_
