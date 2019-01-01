/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WAYPOINT_ANNOTATOR_HPP
#define WAYPOINT_ANNOTATOR_HPP

#include "moo/render_context.hpp"
#include "moo/render_target.hpp"
#include "math/vector3.hpp"

class IWaypointView;
class ChunkSpace;

/**
 *	This class annotates a waypoint graph (through the IWaypointView
 *	interface) with extra visibility and action information.
 */
class WaypointAnnotator
{
public:
	WaypointAnnotator( IWaypointView * pView, ChunkSpace * pSpace );
	~WaypointAnnotator();

	void annotate( bool slowly = false, float girth = 0.f );

private:
	WaypointAnnotator( const WaypointAnnotator& );
	WaypointAnnotator& operator=( const WaypointAnnotator& );

	int annotate( int p, int v, bool slowly );

	IWaypointView * pView_;
	ChunkSpace *	pSpace_;

public:
	std::vector<Vector3>	colTriDebug_;

	ComObjectWrap<DX::Surface> pSurface_;
	ComObjectWrap<DX::Surface> pRTSurface_;
	Moo::RenderTargetPtr pTarget_;
};


#endif // WAYPOINT_ANNOTATOR_HPP
