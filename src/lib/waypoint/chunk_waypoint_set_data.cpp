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

#include "chunk_waypoint_set.hpp"
#include "chunk_waypoint_set_data.hpp"
#include "waypoint.hpp"
#include "waypoint_stats.hpp"

#include "cstdmf/concurrency.hpp"

#include "chunk/chunk_space.hpp"

#include "resmgr/bwresource.hpp"

#include <cfloat>
#include <map>

namespace // anonymous
{

// Link it in to the chunk item section map
ChunkItemFactory navmeshFactoryLink( "worldNavmesh", 0, 
	&ChunkWaypointSetData::navmeshFactory );

typedef std::vector< ChunkWaypointSetData * > NavmeshPopulationRecord;
typedef std::map< std::string, NavmeshPopulationRecord > NavmeshPopulation;

NavmeshPopulation s_navmeshPopulation;

SimpleMutex s_navmeshPopulationLock;

const int NAVPOLY_ELEMENT_SIZE = 12;

template <class C>
inline C consume( const char * & ptr )
{
	// gcc shoots itself in the foot with aliasing of this
	// return *((C*&)ptr)++;
	
	C * oldPtr = (C*)ptr;
	ptr += sizeof( C );
	return *oldPtr;
}

/**
 *	Helper class for building a mapping between unique Vector2s in a vector and
 *	vertexindices stored in ChunkWaypoint edge data.
 */
class VertexIndices
{
public:
	typedef std::vector< Vector2 > Vertices;

	VertexIndices( Vertices & vertices ):
			vertices_( vertices ),
			map_()
	{}

	EdgeIndex getIndexForVertex( const Vector2 & vertex )
	{
		EdgeIndex index = 0;

		Map::iterator iVertex = map_.find( vertex );
		if (iVertex == map_.end())
		{
			index = vertices_.size();
			vertices_.push_back( vertex );
			iVertex = map_.insert( Map::value_type( vertex, index ) ).first;

			static const size_t MAX_SIZE = (1 << (sizeof( EdgeIndex ) * 8)) - 1;
			
			if (vertices_.size() > MAX_SIZE)
			{
				// We've overflowed the storage for vertex indices.
				CRITICAL_MSG( "Chunk waypoint vertex indices have overflowed: "
						"%zu > %zu\n",
					vertices_.size(), MAX_SIZE );
			}
		}
		else
		{
			index = iVertex->second;
		}

		return index;
	}
private:
	Vertices & vertices_;

	typedef std::map< Vector2, EdgeIndex > Map;
	Map map_;
};


} // end namespace (anonymous)

extern void NavmeshPopulation_remove( const std::string & source );


/**
 *	This is the ChunkWaypointSetData constructor.
 */
ChunkWaypointSetData::ChunkWaypointSetData() :
	girth_( 0.f ),
	waypoints_(),
	source_(),
	edges_( NULL ),
	numEdges_( 0 ),
	vertices_()
{
}


/**
 *	This is the ChunkWaypointSetData destructor.
 */
ChunkWaypointSetData::~ChunkWaypointSetData()
{
	if (!source_.empty())
	{
		SimpleMutexHolder smh( s_navmeshPopulationLock );

		s_navmeshPopulation.erase( source_ );
	}

	if (edges_)
	{
		delete [] edges_;
	}

	WaypointStats::instance().removeEdgesAndVertices( numEdges_, 
		vertices_.size() );
}


/**
 *	Find the waypoint that contains the given point.
 *
 *	@param point		The point that is used to find the waypoint.
 *	@param ignoreHeight	A flag which indicates vertical range should be
 *						considered in finding the waypoint.  If not, the best
 *						waypoint that is closest to give point is selected (of
 *						course the waypoint should contain the projection of
 *						the given point regardless.)
 *	@return				The index of the waypoint, or -1 if not found.
 */
int ChunkWaypointSetData::find( const WaypointSpaceVector3 & point, 
		bool ignoreHeight )
{
	int bestWaypoint = -1;
	float bestHeightDiff = FLT_MAX;

	ChunkWaypoints::iterator wit;
	for (wit = waypoints_.begin(); wit != waypoints_.end(); ++wit)
	{
		if (ignoreHeight)
		{
			if (wit->containsProjection( *this, point ))
			{
				if (point.y > wit->minHeight_ - 0.1f &&
						point.y < wit->maxHeight_ + 0.1f)
				{
					return wit - waypoints_.begin();
				}
				else // find best fit
				{
					float wpAvgDiff = fabs( point.y -
						(wit->maxHeight_ + wit->minHeight_) / 2 );
					if (bestHeightDiff > wpAvgDiff)
					{
						bestHeightDiff = wpAvgDiff;
						bestWaypoint = wit - waypoints_.begin();
					}
				}
			}
		}
		else
		{
			if (wit->contains( *this, point ))
			{
				return wit - waypoints_.begin();
			}
		}
	}
	return bestWaypoint;
}


/**
 *	This method finds the waypoint closest to the given point.
 *
 *	@param chunk		The chunk to search in.
 *	@param point		The point that is used to find the closest waypoint.
 *	@param bestDistanceSquared The point must be closer than this to the
 *						waypoint.  It is updated to the new best distance if a
 *						better waypoint is found.
 *	@return				The index of the waypoint, or -1 if not found.
 */
int ChunkWaypointSetData::find( const Chunk * chunk, 
		const WaypointSpaceVector3 & point,
		float & bestDistanceSquared )
{
	int bestWaypoint = -1;
	ChunkWaypoints::iterator wit;
	for (wit = waypoints_.begin(); wit != waypoints_.end(); ++wit)
	{
		float distanceSquared = wit->distanceSquared( *this, chunk,
			MappedVector3( point, chunk ) );
		if (bestDistanceSquared > distanceSquared)
		{
			bestDistanceSquared = distanceSquared;
			bestWaypoint = wit - waypoints_.begin();
		}
	}
	return bestWaypoint;
}


/**
 *	This gets the index of the given edge.
 *
 *	@param edge			The edge to get.
 *	@return				The index of the edge.
 */
int ChunkWaypointSetData::getAbsoluteEdgeIndex(
		const ChunkWaypoint::Edge & edge ) const
{
	return &edge - edges_;
}


/**
 *	Read in waypoint set data from a binary source.
 */
const char * ChunkWaypointSetData::readWaypointSet( const char * pData,
		int numWaypoints, int numEdges )
{
	// Remember what vertices have been mapped to which index, so we can 
	// reuse vertices. 
	VertexIndices vertexIndices( vertices_ ); 

	const char * pEdgeData = pData + numWaypoints * NAVPOLY_ELEMENT_SIZE; 
	waypoints_.resize( numWaypoints ); 
	numEdges_ = numEdges; 
	edges_ = new ChunkWaypoint::Edge[ numEdges ]; 
	WaypointStats::instance().addEdges( numEdges ); 
	ChunkWaypoint::Edge * nedge = edges_; 

	for (int p = 0; p < numWaypoints; ++p) 
	{ 
		ChunkWaypoint & wp = waypoints_[p]; 

		wp.minHeight_ = consume<float>( pData ); 
		wp.maxHeight_ = consume<float>( pData ); 
		int vertexCount = consume<int>( pData ); 

		//dprintf( "poly %d:", p ); 
		wp.edges_ = ChunkWaypoint::Edges( nedge, nedge+vertexCount ); 
		nedge += vertexCount; 

		for (int e = 0; e < vertexCount; e++) 
		{ 
			ChunkWaypoint::Edge & edge = wp.edges_[e]; 
			 
			float x = consume< float >( pEdgeData ); 
			float y = consume< float >( pEdgeData ); 

			Vector2 vertex( x, y ); 
			edge.vertexIndex_ = vertexIndices.getIndexForVertex( vertex ); 

			edge.neighbour_ = consume< int >( pEdgeData ); 
				// 'adj' already encoded as we like it 
			--numEdges; 

			//dprintf( " %08X", edge.neighbour_ ); 
		} 
		//dprintf( "\n" ); 
		wp.calcCentre( *this ); 

		wp.visited_ = 0; 
	} 

	MF_ASSERT( numEdges == 0 ); 

	WaypointStats::instance().addVertices( vertices_.size() ); 
	 
	return pEdgeData; 
}


/**
 *	Factory method for ChunkWaypointSetData's.
 */
ChunkItemFactory::Result ChunkWaypointSetData::navmeshFactory( Chunk * pChunk, 
	    DataSectionPtr pSection ) 
{
	if (!pChunk->space()->loadNavmesh())
	{
		return ChunkItemFactory::SucceededWithoutItem();
	}

    std::string resName = pSection->readString( "resource" ); 
    std::string fullName = pChunk->mapping()->path() + resName; 
 
    // store the sets into a vector and add them into chunk 
    // after release s_navmeshPopulationLock to avoid deadlock 
    std::vector<ChunkWaypointSetPtr> sets; 
 
    { // s_navmeshPopulationLock 
        SimpleMutexHolder smh( s_navmeshPopulationLock ); 
        // see if we've already loaded this navmesh 
        NavmeshPopulation::iterator found = 
            s_navmeshPopulation.find( fullName ); 
 
        if (found != s_navmeshPopulation.end()) 
        { 
            sets.reserve( found->second.size() ); 
 
            NavmeshPopulationRecord::const_iterator iter; 
            for (iter = found->second.begin(); 
              	   iter != found->second.end(); 
				   ++iter) 
            { 
                // First check if we can claim a reference this element 
                if ((*iter)->incRefTry()) 
                { 
                    ChunkWaypointSet * pSet = new ChunkWaypointSet( *iter ); 
                    (*iter)->decRef(); 
                    sets.push_back( pSet ); 
                } 
                else 
                { //if not, we can't use this record so get rid of it 
                    sets.clear(); 
                    break; 
                } 
            } 
        } 
    } // !s_navmeshPopulationLock 
 
    if (!sets.empty()) 
    { 
        for (std::vector<ChunkWaypointSetPtr>::iterator iter = sets.begin(); 
            	iter != sets.end(); 
				++iter) 
        { 
            pChunk->addStaticItem( *iter ); 
        } 
 
        return ChunkItemFactory::SucceededWithoutItem(); 
    } 
 
    // ok, no, time to load it then 
    BinaryPtr pNavmesh = BWResource::instance().rootSection()->readBinary( 
        pChunk->mapping()->path() + resName ); 
 
    if (!pNavmesh) 
    { 
        ERROR_MSG( "Could not read navmesh '%s'\n", resName.c_str() ); 
        return ChunkItemFactory::Result( NULL ); 
    } 
 
    if (pNavmesh->len() == 0) 
    { 
        // empty navmesh (not put in popln) 
        return ChunkItemFactory::SucceededWithoutItem(); 
    } 
 
    NavmeshPopulationRecord newRecord; 
 
    const char * dataBeg = pNavmesh->cdata(); 
    const char * dataEnd = dataBeg + pNavmesh->len(); 
    const char * dataPtr = dataBeg; 
 
    while (dataPtr < dataEnd) 
    { 
        int aVersion = consume<int>( dataPtr ); 
        float aGirth = consume<float>( dataPtr ); 
        int numWaypoints = consume<int>( dataPtr ); 
        int numEdges = consume<int>( dataPtr ); 
 
        MF_ASSERT( aVersion == 0 ); 
 
        ChunkWaypointSetDataPtr pSetData = new ChunkWaypointSetData(); 
        pSetData->girth( aGirth ); 
         
        dataPtr = pSetData->readWaypointSet( dataPtr, numWaypoints, numEdges ); 
		pSetData->source_ = fullName;
		newRecord.push_back( pSetData.get() );
 
        ChunkWaypointSet * pSet = new ChunkWaypointSet( pSetData ); 
        pChunk->addStaticItem( pSet ); 
    } 
 
    { // s_navmeshPopulationLock 
        SimpleMutexHolder smh( s_navmeshPopulationLock ); 
        NavmeshPopulation::iterator found = 
            s_navmeshPopulation.insert( std::make_pair(fullName, 
                NavmeshPopulationRecord() ) ).first; 
        found->second.swap( newRecord ); 
    } // !s_navmeshPopulationLock 
 
    return ChunkItemFactory::SucceededWithoutItem(); 
} 
// chunk_waypoint_set_data.cpp
