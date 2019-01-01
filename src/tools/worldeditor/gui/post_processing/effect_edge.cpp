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
#include "effect_edge.hpp"
#include "graph/node.hpp"


/**
 *	Constructor.
 *
 *	@param	start	Start node, a Post-processing Effect.
 *	@param	end		End node, a Post-processing Effect.
 */
EffectEdge::EffectEdge( const Graph::NodePtr & start, const Graph::NodePtr & end ) :
	Graph::Edge( start, end )
{
	BW_GUARD;
}
