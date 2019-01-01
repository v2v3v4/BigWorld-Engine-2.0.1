/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_EDGE_VIEW_HPP
#define EFFECT_EDGE_VIEW_HPP


#include "graph/graph_view.hpp"


// Forward declarartions
class EffectEdge;
typedef SmartPointer< EffectEdge > EffectEdgePtr;


/**
 *	This class implements a Graph Edge View for displaying edges that go from
 *	Effect to Effect.
 */
class EffectEdgeView : public Graph::EdgeView
{
public:
	EffectEdgeView( Graph::GraphView & graphView, const EffectEdgePtr edge );

	// Graph::NodeView interface
	const CRect & rect() const { return rect_; }

	void draw( CDC & dc, uint32 frame, const CRect & rectStartNode, const CRect & rectEndNode );

private:
	EffectEdgePtr edge_;
	CRect rect_;
};


#endif // EFFECT_EDGE_VIEW_HPP
