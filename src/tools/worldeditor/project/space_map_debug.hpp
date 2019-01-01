/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_MAP_DEBUG_HPP
#define SPACE_MAP_DEBUG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/project/space_information.hpp"
#include <deque>


class SpaceMapDebug
{
public:
	static SpaceMapDebug& instance();
	static void deleteInstance();

	void spaceInformation( const SpaceInformation& info );	
	void onDraw( int16 gridX, int16 gridZ, uint32 colour );
	void onConsidered( int16 gridX, int16 gridZ, uint32 colour );	
	void draw();
private:
	SpaceMapDebug();
	~SpaceMapDebug();

	struct DebugInfo
	{		
		DebugInfo( int16 gridX, int16 gridZ, uint32 colour, float age = 1.f ):
			age_( age ),
			gridX_( gridX ),
			gridZ_( gridZ ),
			colour_( colour )
		{
		}
		float	age_;
		int16	gridX_;
		int16	gridZ_;
		uint32	colour_;
	};
	std::deque<DebugInfo>		debugInfo_;
	Moo::EffectMaterialPtr		material_;
	SpaceInformation			info_;
	bool						visible_;

	static SpaceMapDebug*		s_instance_;
};


#endif // SPACE_MAP_DEBUG_HPP
