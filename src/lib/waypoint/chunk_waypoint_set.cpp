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

#include "chunk_navigator.hpp"
#include "navigator_find_result.hpp"
#include "waypoint_stats.hpp"

#include "chunk/chunk_space.hpp"
#include "chunk/chunk_overlapper.hpp"

DECLARE_DEBUG_COMPONENT2( "Waypoint", 0 )

int ChunkWaypointSet_token = 1;


/**
 *	This is the ChunkWaypointSet constructor.
 */
ChunkWaypointSet::ChunkWaypointSet( ChunkWaypointSetDataPtr pSetData ):
	pSetData_( pSetData ),
	connections_(),
	edgeLabels_(),
	backlinks_()
{
}


/**
 *	This is the ChunkWaypointSet destructor.
 */
ChunkWaypointSet::~ChunkWaypointSet()
{
}


/**
 *	If we have any waypoints with any edges that link to other waypont sets,
 *	remove our labels.  Remove us from the backlinks in the waypointSets we
 *	linked to, then delete our connections list.
 */
void ChunkWaypointSet::removeOurConnections()
{
	// first set all our external edge labels back to 65535.
	edgeLabels_.clear();

	// now remove us from all the backlinks of the chunks we put
	// ourselves in when we set up those connections.
	ChunkWaypointConns::iterator i = connections_.begin();
	for (; i != connections_.end(); ++i)
	{
		i->first->removeBacklink( this );
	}

	connections_.clear();
}


/**
 *	Remove the connection with index conNum from connections and set all edges
 *	corresponding to this connection to 65535.  Also remove us from the
 *	backlinks list in the waypointSet that the connection was to.
 *
 *	@param pSet			The ChunkWaypointSet to remove the connection to.
 */
void ChunkWaypointSet::deleteConnection( ChunkWaypointSetPtr pSet )
{
	ChunkWaypointConns::iterator found;
	if ((found = connections_.find( pSet )) == connections_.end())
	{
		ERROR_MSG( "ChunkWaypointSet::deleteConnection: connection from %u/%s "
				"to %u/%s does not exist\n",
			this->chunk()->space()->id(),
			this->chunk()->identifier().c_str(),
			pSet->chunk()->space()->id(),
			pSet->chunk()->identifier().c_str() );
		return;
	}

	// (1) remove our edge labels for this chunk waypoint set
	std::vector<WaypointEdgeIndex> removingEdges;
	ChunkWaypointEdgeLabels::iterator edgeLabel = edgeLabels_.begin();
	for (;edgeLabel != edgeLabels_.end(); ++edgeLabel)
	{
		if (edgeLabel->second == pSet)
		{
			// add to removingEdges to remove it from the map after loop
			removingEdges.push_back( edgeLabel->first );
		}
	}
	while (removingEdges.size())
	{
		edgeLabels_.erase( removingEdges.back() );
		removingEdges.pop_back();
	}

	// (2) now remove from backlinks list of chunk we connected to.
	pSet->removeBacklink( this );


	// (3) now remove from us, and all trace of connection is gone!
	connections_.erase( pSet );

}


/**
 *	This goes through all waypoint sets that link to us, and tells them not to
 *	link to us anymore.
 */
void ChunkWaypointSet::removeOthersConnections()
{
	// loop around all our backlinks_ [waypoint sets that contain
	// edges that link back to us].

	while (backlinks_.size())
	{
		ChunkWaypointSetPtr pSet = backlinks_.back();

		// loop around the connections in this waypointSet with
		// a backlink to us looking for it..
		if (pSet->connections_.find( this ) == pSet->connectionsEnd())
		{
			ERROR_MSG( "ChunkWaypointSet::removeOthersConnections: "
				"Back connection not found.\n" );
			backlinks_.pop_back();
			continue;
		}

		// will delete the current backlink from our list.
		pSet->deleteConnection( ChunkWaypointSetPtr( this ) );
	}
}


/**
 *	This adds a waypointSet to our backlink list.
 *
 *	@param pWaypointSet		The ChunkWaypointSet to backlink to.
 */
void ChunkWaypointSet::addBacklink( ChunkWaypointSetPtr pWaypointSet )
{
	backlinks_.push_back( pWaypointSet );
}


/**
 *	This removes a waypointSet from our backlink list.
 *
 *	@param pWaypointSet		The ChunkWaypointSet to remove the backlink for.
 */
void ChunkWaypointSet::removeBacklink( ChunkWaypointSetPtr pWaypointSet )
{
	ChunkWaypointSets::iterator blIter =
		std::find( backlinks_.begin(), backlinks_.end(), pWaypointSet );

	if (blIter != backlinks_.end())
	{
		backlinks_.erase( blIter );
		return;
	}

	ERROR_MSG( "ChunkWaypointSet::removeBacklink: "
		"trying to remove backlink that doesn't exist\n" );
}


/**
 *	Print some debugging information for this ChunkWaypointSet.
 */
void ChunkWaypointSet::print() const
{
	DEBUG_MSG( "ChunkWayPointSet: 0x%p - %s\tWayPointCount: %d\n", 
		this, pChunk_->identifier().c_str(), this->waypointCount() );

	for (int i = 0; i < waypointCount(); ++i)
	{
		waypoint( i ).print( *this );
	}

	for (ChunkWaypointConns::const_iterator iter = this->connectionsBegin();
			iter != this->connectionsEnd(); 
			++iter)
	{
		DEBUG_MSG( "**** connecting to 0x%p %s", 
			iter->first.get(), 
			iter->first->pChunk_->identifier().c_str() );
	}
}


/**
 *	 Connect edge in the current waypointSet to pWaypointSet.
 *
 *	(1)	If the connection doesn't exist in connections, this is created together
 *		with a backlink to us in pWaypointSet.
 *
 *	(2)	Update the edge Label.
 *
 *	@param pWaypointSet		The ChunkWaypointSet to connect to.
 *	@param pPortal			The connection portal.
 *	@param edge				The edge along which the ChunkWaypointSets are
 *							connected.
 */
void ChunkWaypointSet::connect(
						ChunkWaypointSetPtr pWaypointSet,
						ChunkBoundary::Portal * pPortal,
						const ChunkWaypoint::Edge & edge )
{
	WaypointEdgeIndex edgeIndex = pSetData_->getAbsoluteEdgeIndex( edge );

	if (edge.neighbour_ != 65535)
	{
		WARNING_MSG( "ChunkWaypointSet::connect called on "
			"non chunk-adjacent edge\n" );
		return;
	}

	ChunkWaypointConns::iterator found = connections_.find( pWaypointSet );

	if (found == connections_.end())
	{
		connections_[pWaypointSet] = pPortal;

		// now add backlink to chunk connected to.
		pWaypointSet->addBacklink( this );

	}
	else if (found->second != pPortal)
	{
		NOTICE_MSG( "ChunkWaypointSet::connect: "
				"Chunk %s is connected to chunk %s via two different portals. "
				"This may cause computed paths to be sub-optimal.\n"
				"Fix by changing the shell to make sure there is no more than "
				"one portal between adjacent shells.\n",
			chunk()->identifier().c_str(), 
			pWaypointSet->chunk()->identifier().c_str() );
	}

	edgeLabels_[edgeIndex] = pWaypointSet;
}


/**
 *	This adds or removes us from the given chunk.
 *
 *	@param pChunk			The chunk that the ChunkWaypointSet is being added.
 *							If this is NULL then the ChunkWaypointSet is being
 *							removed.
 */
void ChunkWaypointSet::toss( Chunk * pChunk )
{
	if (pChunk == pChunk_) return;

	// out with the old
	if (pChunk_ != NULL)
	{
		// Note: program flow arrives here when pChunk_ is
		// being ejected (with pChunk = NULL)

		//this->toWorldCoords();
		this->removeOthersConnections();
		this->removeOurConnections();

		ChunkNavigator::instance( *pChunk_ ).del( this );
	}

	this->ChunkItem::toss( pChunk );

	// and in with the new
	if (pChunk_ != NULL)
	{
		if (pChunk_->isBound())
		{
			CRITICAL_MSG( "ChunkWaypointSet::toss: "
				"Tossing after loading is not supported\n" );
		}

		//this->toLocalCoords() // ... now already there

		// now that we are in local co-ords we can add ourselves to the
		// cache maintained by ChunkNavigator
		ChunkNavigator::instance( *pChunk_ ).add( this );
	}
}


/*
 *	Check if ready to bind
 */
bool ChunkWaypointSet::readyToBind() const
{
	int gridX = int( floorf( chunk()->centre().x / (float)GRID_RESOLUTION ) );
	int gridZ = int( floorf( chunk()->centre().z / (float)GRID_RESOLUTION ) );

	if (gridX == chunk()->mapping()->minGridX() ||
		gridX == chunk()->mapping()->maxGridX() ||
		gridZ == chunk()->mapping()->minGridY() ||
		gridZ == chunk()->mapping()->maxGridY())
	{
		return true;
	}

	ChunkOverlappers::Overlappers& overlappers = 
		ChunkOverlappers::instance( *pChunk_ ).overlappers();

	for (ChunkOverlappers::Overlappers::iterator iter = overlappers.begin();
		iter != overlappers.end(); ++iter)
	{
		if (!(*iter)->pOverlapper()->loaded())
		{
			return false;
		}
	}

	for (ChunkBoundaries::iterator iter = pChunk_->joints().begin();
		iter != pChunk_->joints().end(); ++iter)
	{
		if (!(*iter)->unboundPortals_.empty())
		{
			return false;
		}
	}

	for (Chunk::piterator iter = pChunk_->pbegin();
		iter != pChunk_->pend(); iter++)
	{
		if (iter->hasChunk() && !iter->pChunk->loaded())
		{
			return false;
		}
	}

	return true;
}


/**
 *	Bind any unbound waypoints
 */
void ChunkWaypointSet::bind()
{
	if (!readyToBind())
	{
		return;
	}

	// We would like to first make sure that all our existing connections
	// still exist.  Unfortunately we can't tell what is being unbound,
	// so we have to delay this until the set we are connected to is tossed
	// out of its chunk.

	// now make new connections
	ChunkWaypoints::const_iterator wit;
	ChunkWaypoint::Edges::const_iterator eit;

	for (wit = pSetData_->waypoints().begin(); 
			wit != pSetData_->waypoints().end(); 
			++wit)
	{
		float wymin = wit->minHeight_;
		float wymax = wit->maxHeight_;
		float wyavg = (wymin + wymax) * 0.5f + 0.1f;

		for (eit = wit->edges_.begin(); eit != wit->edges_.end(); ++eit)
		{
			if (eit->neighbour_ != 65535) 
			{
				continue;
			}

			ChunkWaypoint::Edges::const_iterator nextEdgeIter = eit + 1;
			if (nextEdgeIter == wit->edges_.end())
			{
				nextEdgeIter = wit->edges_.begin();
			}

			const Vector2 & start = pSetData_->vertexByIndex( 
				eit->vertexIndex_ );
			const Vector2 & end = pSetData_->vertexByIndex( 
				nextEdgeIter->vertexIndex_ );

			WaypointSpaceVector3 v( (start.x + end.x) / 2.f,
				0.f, (start.y + end.y) / 2.f );

			WaypointSpaceVector3 wv = v;
			wv.y = wyavg;
			Vector3 lwv = MappedVector3( wv, pChunk_ ).asChunkSpace();

			Chunk::piterator pit = pChunk_->pbegin();
			Chunk::piterator pnd = pChunk_->pend();
			ChunkBoundary::Portal * cp = NULL;
			while (pit != pnd)
			{
				WorldSpaceVector3 wsv = MappedVector3( wv, pChunk_ );

				if (pit->hasChunk() && 
							pit->pChunk->boundingBox().intersects( wsv ))
				{
					// only use test for minimum distance from portal plane
					// for indoor chunks
					float minDist = pit->pChunk->isOutsideChunk() ?
						0.f : 1.f;
					if (Chunk::findBetterPortal( cp, minDist, &*pit, lwv ))
					{
						cp = &*pit;
					}
				}
				pit++;
			}
			if (cp == NULL)
			{
				// try a second time with height set to maxHeight_ since there
				// is a chance that one chunk contains steep slope avg waypoint
				// height lies just outside of the portal.
				wv.y = wymax + 0.1f;
				lwv = MappedVector3( wv, pChunk_ ).asChunkSpace();
				const WorldSpaceVector3 wsv = MappedVector3( wv, pChunk_ );

				Chunk::piterator pit = pChunk_->pbegin();
				Chunk::piterator pnd = pChunk_->pend();
				while (pit != pnd)
				{
					if (pit->hasChunk() && 
							pit->pChunk->boundingBox().intersects( wsv ) )
					{
						// only use test for minimum distance from portal plane
						// for indoor chunks
						float minDist = pit->pChunk->isOutsideChunk() ?
							0.f : 1.f;
						if (Chunk::findBetterPortal( cp, minDist, &*pit, lwv ))
						{
							cp = &*pit;
						}
					}
					pit++;
				}
				if (cp == NULL)
				{
					continue;
				}
			}
			Chunk * pConn = cp->pChunk;

			// make sure we have a corresponding waypoint on the other side
			// of the portal.

			NavigatorFindResult navigatorFindResult;
			Vector3 ltpv = cp->origin + cp->uAxis*cp->uAxis.dotProduct( lwv ) +
								cp->vAxis*cp->vAxis.dotProduct( lwv );
			MappedVector3 tpwv( ltpv, pChunk_, MappedVector3::CHUNK_SPACE );

			if (!ChunkNavigator::instance( *pConn ).find(
				tpwv, girth(), navigatorFindResult, false ))
			{
				// check to see if pConn has _any_ navPoly sets of this girth
				// in it before complaining.  If not, then don't complain
				// (assume that we didn't mean to create them - ie. further
				// then navPolyGenerateRange from marker).  If pConn contains
				// no navPolys at all, then we have complained about this
				// before.
				if (ChunkNavigator::instance( *pConn ).hasNavPolySet( girth() ))
				{
					ERROR_MSG( "ChunkWaypointSet::bind: No adjacent navPoly set"
						" through bound portal from %s to %s with girth %f\n",
						pChunk_->identifier().c_str(),
						pConn->identifier().c_str(), girth() );
				}
				continue;
			}

			this->connect( navigatorFindResult.pSet().get(), cp, *eit );
		}
	}
}


// chunk_waypoint_set.cpp
