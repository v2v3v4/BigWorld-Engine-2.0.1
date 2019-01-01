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

#include "chunk_waypoint_generator.hpp"
#include "physics_handler.hpp"

#include "resmgr/bwresource.hpp"
#include "chunk/chunk_overlapper.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"

#include "waypoint_annotator.hpp"

Matrix g_oldCam;
extern Matrix g_camMatrix;
extern void updateMoo( float dTime, bool allowMovement );
extern void drawMoo(float dTime = 0.0f);
extern HWND	g_hMooWindow;

DECLARE_DEBUG_COMPONENT2( "WPGen", 0 )

const static char *s_navmeshDirtyStr = "navmeshDirty";

/**
 *	Constructor
 */
ChunkWaypointGenerator::ChunkWaypointGenerator( Chunk * pChunk, const std::string& floodResultPath ) :
	pChunk_( pChunk ),
	flooder_( pChunk, floodResultPath ),
	modified_( true )
{
	BW_GUARD;

    DataSectionPtr chunkBinSection = BWResource::openSection( pChunk_->binFileName() );
    if (chunkBinSection)
    {
        DataSectionPtr navmeshDirtySection = 
            chunkBinSection->findChild( s_navmeshDirtyStr );
        if (navmeshDirtySection)
        {
            BinaryPtr bp = navmeshDirtySection->asBinary();
            if (bp->len() == sizeof(bool))
                modified_ = *((bool *)bp->cdata());
        }
		if (!modified_ && !chunkBinSection->findChild( "worldNavmesh" ))
		{
			modified_ = true;
		}
    }

	// move the camera to the centre of pChunk
	g_oldCam = g_camMatrix;
	g_camMatrix.translation( pChunk->centre() );
}

ChunkWaypointGenerator::~ChunkWaypointGenerator()
{
	g_camMatrix = g_oldCam;
}


/**
 *	Ready check method
 */
bool ChunkWaypointGenerator::ready() const
{
	BW_GUARD;

    return canProcess( pChunk_ );
}

/**
 *	Maximum number of points expected in the flood
 */
int ChunkWaypointGenerator::maxFloodPoints() const
{
	BW_GUARD;

	return flooder_.width() * flooder_.height();
}

/**
 *	Do the actual flooding
 */
void ChunkWaypointGenerator::flood( bool (*progressCallback)( int npoints ),
	girth gSpec, bool writeTGAs )
{
	BW_GUARD;

	flooder_.flood( gSpec, progressCallback, 0, writeTGAs );
}

/**
 *	Generate the waypoints
 */
void ChunkWaypointGenerator::generate( bool annotate, girth gSpec )
{
	BW_GUARD;

	int w = flooder_.width(), h = flooder_.height();
	gener_.init( w, h, flooder_.minBounds(), flooder_.resolution() );

	for ( int g = 0; g < 16; ++g )
	{
		memcpy( gener_.adjGrids()[g], flooder_.adjGrids()[g], w*h*4 );
		memcpy( gener_.hgtGrids()[g], flooder_.hgtGrids()[g], w*h*4 );
	}

	INFO_MSG( "Generating..\n" );
	gener_.generate();

	// determine the connections to adjacent chunks
	INFO_MSG( "Determining edges adjacent to other chunks\n" );

	PhysicsHandler phand( pChunk_->space(), gSpec );
	gener_.determineEdgesAdjacentToOtherChunks( pChunk_, &phand );

	// streamline the polygons
	INFO_MSG( "Streamlining polygons\n" );
	gener_.streamline();

	// determine edges that can be traversed one way only (falling
	// off ledge etc).
	//INFO_MSG( "Determining one way adjacencies...\n" );
	//gener_.determineOneWayAdjacencies();

	// annotate edges with visibility and action information
	if( annotate )
	{
		INFO_MSG( "Annotating polygons\n" );
		WaypointAnnotator wanno( &gener_, pChunk_->space() );
		wanno.annotate();
	}

	INFO_MSG( "Extending navPolys through unbound portals.\n" );
	gener_.extendThroughUnboundPortals( pChunk_ );

	// figure out what sets there are
	INFO_MSG( "Calculating set membership\n" );
	gener_.calculateSetMembership( flooder_.entityPts(), pChunk_->identifier() );
}


/**
 * save out write time of navPolys. Separate from output, as don't 
 * want to write write time until nav polys for all girths have been
 * written.
 */
void ChunkWaypointGenerator::outputDirtyFlag(bool dirty /*= false*/)
{
	BW_GUARD;

    DataSectionPtr chunkBinSection = BWResource::openSection( pChunk_->binFileName() );
    if (chunkBinSection)
    {
        DataSectionPtr navmeshDirtySection = chunkBinSection->openSection(s_navmeshDirtyStr, true);
        if (navmeshDirtySection)
        {
            navmeshDirtySection->setBinary
			(
				new BinaryBlock
				(
					&dirty, 
					sizeof(dirty), 
					"BinaryBlock/ChunkWaypointGenerator"
				)
			);
            navmeshDirtySection->setParent(chunkBinSection);
            navmeshDirtySection->save();
            navmeshDirtySection->setParent(NULL);
        }
        else
        {
            ERROR_MSG
            ( 
                "Couldn't create NavMesh dirty section for chunk %s", 
                pChunk_->identifier() 
            );
        }
    }
    chunkBinSection->save();
}


/**
 *	Save them out
 */
void ChunkWaypointGenerator::output( float girth, bool firstGirth )
{
	BW_GUARD;

#if 0
	// find the chunk's section
	DataSectionPtr pChunkSect = BWResource::openSection(
		pChunk_->resourceID() );

	// remove any existing waypoint sets (legacy) and navPolySets of 
	// the same girth ourselves (writeToSection can only remove all) 
	// also remove girthless wps / nps.
	for ( int i = 0; i < pChunkSect->countChildren(); ++i )
	{
		DataSectionPtr ds = pChunkSect->openChild( i );
		if ( !((ds->sectionName() == "waypointSet") || 
			(ds->sectionName() == "navPolySet")) ) continue;
		float ag = ds->readFloat( "girth" );
		if (!(ag == 0.f || ag == girth)) continue;
		pChunkSect->delChild( ds );
		i--;
	}

	// add the newly generated waypoints
	gener_.writeToSection( pChunkSect, pChunk_, girth, firstGirth );

	// and save it out!
	pChunkSect->save();
#else
	gener_.saveOut( pChunk_, girth, /*removeAllOld:*/firstGirth );
#endif
}


/*static*/ bool ChunkWaypointGenerator::canProcess(Chunk *chunk)
{
	BW_GUARD;

	ScopedSyncMode scopedSyncMode;

	updateMoo( 0.05f, false );

	if (IsWindowVisible( g_hMooWindow ))
		drawMoo();

	if (!chunk->isOutsideChunk())
	{
		std::string outsideChunkName = chunk->mapping()->outsideChunkIdentifier(
			chunk->transform().applyToOrigin() );

		if (outsideChunkName.empty())
		{
			return chunk->isBound();
		}

		Chunk* outside = ChunkManager::instance().findChunkByName( outsideChunkName,
			chunk->mapping() );

		return canProcess( outside );
	}

	Vector3 centre = chunk->boundingBox().centre();

	for (int x = -1; x < 2; ++x)
	{
		for (int z = -1; z < 2; ++z)
		{
			Vector3 pos( centre.x + x * GRID_RESOLUTION,
				centre.y, centre.z + z * GRID_RESOLUTION );
			std::string chunkName = chunk->mapping()->outsideChunkIdentifier( pos );

			if (!chunkName.empty())
			{
				Chunk* outside = ChunkManager::instance().findChunkByName( chunkName,
					chunk->mapping() );

				if (!outside->isBound())
				{
					if (!outside->loading())
					{
						ChunkManager::instance().loadChunkExplicitly(
							chunkName, chunk->mapping() );
						ChunkManager::instance().checkLoadingChunks();
					}
					return false;
				}

				ChunkOverlappers& co = ChunkOverlappers::instance( *outside );

				ChunkOverlappers::Overlappers coo = co.overlappers();

				for (ChunkOverlappers::Overlappers::iterator it = coo.begin();
					it != coo.end(); ++it)
				{
					if (!(*it)->pOverlapper()->isBound())
					{
						if (!(*it)->pOverlapper()->loading())
						{
							ChunkManager::instance().loadChunkExplicitly(
								(*it)->pOverlapper()->identifier(), (*it)->pOverlapper()->mapping() );
							ChunkManager::instance().checkLoadingChunks();
						}
						return false;
					}
				}

			}
		}
	}

	return true;
}
