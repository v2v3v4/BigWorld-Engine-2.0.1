/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GRAPH_HPP
#define GRAPH_HPP


#include "cstdmf/smartpointer.hpp"


namespace Graph
{


// Forward declarations.
class Node;
typedef SmartPointer< Node > NodePtr;


class Edge;
typedef SmartPointer< Edge > EdgePtr;


typedef std::set< NodePtr > NodesSet;

typedef std::set< EdgePtr > EdgesSet;

typedef std::multimap< NodePtr, EdgePtr > EdgesMap;


/**
 *	This class stores info about and manages a set of connected nodes.
 */
class Graph : public ReferenceCount
{
public:
	Graph();
	virtual ~Graph();

	virtual bool addNode( const NodePtr & node );
	virtual bool removeNode( const NodePtr & node );

	virtual bool addEdge( const EdgePtr & edge );
	virtual bool removeEdge( const EdgePtr & edge );

	virtual void updateEdge( EdgePtr & edge, const NodePtr & start, const NodePtr & end );

	virtual void clear();

	virtual void adjacentNodes( const NodePtr & startNode, NodesSet & retNodes ) const;

	virtual void backAdjacentNodes( const NodePtr & endNode, NodesSet & retNodes ) const;

	virtual void edges( const NodePtr & startNode, EdgesSet & retEdges ) const;

	virtual void backEdges( const NodePtr & endNode, EdgesSet & retEdges ) const;

	virtual const NodesSet & allNodes() const { return nodes_; }

	virtual const EdgesSet & allEdges() const { return edges_; }

private:
	Graph( const Graph & other );

	NodesSet nodes_;

	EdgesSet edges_;

	EdgesMap edgesMap_;
	EdgesMap backEdgesMap_;

	void removeEdgeFromMap( const EdgePtr & edge, EdgesMap & edgesMap );
};


typedef SmartPointer< Graph > GraphPtr;


} // namespace Graph


#endif // GRAPH_VIEW_HPP
