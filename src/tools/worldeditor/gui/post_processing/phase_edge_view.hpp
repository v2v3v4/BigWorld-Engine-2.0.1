/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHASE_EDGE_VIEW_HPP
#define PHASE_EDGE_VIEW_HPP


#include "graph/graph_view.hpp"


// Forward declarartions
class PhaseEdge;
typedef SmartPointer< PhaseEdge > PhaseEdgePtr;


/**
 *	This class implements a Graph Edge View for displaying edges that go to
 *	a Phase node.
 */
class PhaseEdgeView : public Graph::EdgeView
{
public:
	PhaseEdgeView( Graph::GraphView & graphView, const PhaseEdgePtr edge );

	// Graph::NodeView interface
	const CRect & rect() const { return rect_; }

	void draw( CDC & dc, uint32 frame, const CRect & rectStartNode, const CRect & rectEndNode );

private:
	PhaseEdgePtr edge_;
	CRect rect_;
};


#endif // PHASE_EDGE_VIEW_HPP
