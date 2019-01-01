/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ZIGZAG_PATROL_GRAPH_HPP
#define ZIGZAG_PATROL_GRAPH_HPP

#include "patrol_graph.hpp"

using namespace Patrol;

namespace ZigzagPatrol
{
	// This could probably derive from GraphTraverser itself, but I want to do
	// it as a standalone class as a learning experience
	class ZigzagGraphTraverser : public GraphTraverser
	{
	public:
		static const int DEFAULT_CORRIDOR_WIDTH;
		
		ZigzagGraphTraverser (const Graph &graph, float & speed,
							  Vector3 &startPosition,
							  float corridorWidth = DEFAULT_CORRIDOR_WIDTH);
		virtual bool nextStep (float & speed,
							   float dTime,
							   Vector3 &pos,
							   Direction3D &dir);
		
	protected:
		void setZigzagPos (const Vector3 &currPos);

		float corridorWidth_;
		Vector3 sourcePos_;
		Vector3 zigzagPos_;
	};
};

#endif // ZIGZAG_PATROL_GRAPH_HPP
