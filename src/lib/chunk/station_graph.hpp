/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATION_GRAPH_HPP
#define STATION_GRAPH_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/unique_id.hpp"
#include "math/vector3.hpp"
#include "resmgr/datasection.hpp"
#include <vector>
#include <map>

class Chunk;
class ChunkStationNode;
class GeometryMapping;
class SimpleMutex;

/**
 *  A graph of nodes
 */
class StationGraph
{
public:
	static StationGraph* getGraph( const UniqueID& graphName );
	static std::vector<StationGraph*> getAllGraphs();

	const UniqueID &    name() const;
	bool                isReady() const;

	bool   canTraverseFrom( const UniqueID& src, const UniqueID& dst );
	uint32 traversableNodes( const UniqueID& src, std::vector<UniqueID>& retNodeIDs );
	bool   worldPosition( const UniqueID& node, Vector3& retWorldPos );
	const  UniqueID& nearestNode( const Vector3& worldPos );	

	void   registerNode( ChunkStationNode* node, Chunk* pChunk );
	void   deregisterNode( ChunkStationNode* node );
#ifdef EDITOR_ENABLED
	bool   isRegistered( const UniqueID& id );
	bool   isRegistered( ChunkStationNode* node );
#endif

	/**
	 *	EDITOR interface.  The editor can create/delete nodes interactively.
	 *	There is one ChunkStationNode for every StationGraph::Node, whereby
	 *	the ChunkStationNodes are ChunkItems that can be moved around etc.
	 *	The StationGraph::Nodes contain information that will be used at
	 *	runtime by the server, and possibly the client.
	 */
#ifdef EDITOR_ENABLED
	ChunkStationNode* getNode( const UniqueID& id );

	std::vector<ChunkStationNode*> getAllNodes();

	static ChunkStationNode* getNode( const UniqueID& graphName, 
        const UniqueID& id );

    static void mergeGraphs( UniqueID const &graph1Id, 
        UniqueID const &graph2Id, GeometryMapping * pMapping );

    void loadAllChunks( GeometryMapping * pMapping );

	bool save();

	static void saveAll();

    virtual bool isValid(std::string &failureMsg) const;

	bool updateNodeIds(const UniqueID &nodeId, const std::vector<UniqueID> &links);
#endif	//EDITOR_ENABLED

private:
	class Node
	{
	public:
		Node();
		bool						load( DataSectionPtr pSect, const Matrix& mapping );
#ifdef EDITOR_ENABLED
		///only the editor needs the code to save StationGraph::Nodes
		bool						save( DataSectionPtr pSect );
#endif
		bool						hasTraversableLinkTo( const UniqueID& nodeId ) const;

		void						addLink( const UniqueID& id );
		void						delLink( const UniqueID& id );
		std::vector<UniqueID>&		links();

		Vector3						worldPosition_;
		UniqueID					id_;

        std::string                 userString_;

	private:
		std::vector<UniqueID>		links_;
	};

	StationGraph( const UniqueID& name );

	void                        addNode ( const StationGraph::Node& n );
	StationGraph::Node*         node( const UniqueID& id );
	void                        constructFilename( const std::string& spacePath );
    bool                        load( const Matrix& mapping );

	std::string			                        filename_;
	UniqueID			                        name_;
	DataSectionPtr		                        pSect_;
	bool                                        isReady_;	    /** false until loaded */
	std::map<UniqueID, StationGraph::Node>      nodes_;         /** client / server nodes list */
#ifdef EDITOR_ENABLED	
	std::map<UniqueID, ChunkStationNode*>       edNodes_;       /** Maps loaded node ids to editor nodes */
#endif
	static std::map<UniqueID, StationGraph*>    graphs_;        /** Cache of StationGraphs */
};

#endif	//STATION_GRAPH_HPP
