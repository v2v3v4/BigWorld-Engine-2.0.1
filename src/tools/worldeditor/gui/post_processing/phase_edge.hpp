/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHASE_EDGE_HPP
#define PHASE_EDGE_HPP


#include "graph/edge.hpp"


/**
 *	This class implements a Graph Edge for going to a Phase node.
 */
class PhaseEdge : public Graph::Edge
{
public:
	PhaseEdge( const Graph::NodePtr & start, const Graph::NodePtr & end );
};
typedef SmartPointer< PhaseEdge > PhaseEdgePtr;


#endif // PHASE_EDGE_HPP
