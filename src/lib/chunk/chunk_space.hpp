/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_SPACE_HPP
#define CHUNK_SPACE_HPP

#ifdef MF_SERVER
#include "server_chunk_space.hpp"
typedef ServerChunkSpace ConfigChunkSpace;
#else
#include "client_chunk_space.hpp"
class ClientChunkSpace;
typedef ClientChunkSpace ConfigChunkSpace;
#endif

#include "chunk_item.hpp"

#include "cstdmf/aligned.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/guard.hpp"

#include "math/matrix.hpp"
#include "physics2/worldtri.hpp" // For WorldTriangle::Flags
#include "terrain/base_terrain_block.hpp"

#include <set>

//	Forward declarations relating to chunk obstacles
class CollisionCallback;
extern CollisionCallback & CollisionCallback_s_default;
class ChunkObstacle;

typedef uint32 ChunkSpaceID;

class ChunkSpace;
class GeometryMapping;
class GeometryMappingFactory;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;




/**
 *	This class defines a space and maintains the chunks that live in it.
 *
 *	A space is a continuous three dimensional Cartesian medium. Each space
 *	is divided piecewise into chunks, which occupy the entire space but
 *	do not overlap. i.e. every point in the space is in exactly one chunk.
 *	Examples include: planets, parallel spaces, spacestations, 'detached'
 *	apartments / dungeon levels, etc.
 */
class ChunkSpace : public ConfigChunkSpace
{
public:
	ChunkSpace( ChunkSpaceID id );
	~ChunkSpace();

	bool hasChunksInMapping( GeometryMapping * pMapping ) const;

	typedef std::map< SpaceEntryID, GeometryMapping * > GeometryMappings;

	GeometryMapping * addMapping( SpaceEntryID mappingID, float * matrix,
		const std::string & path, DataSectionPtr pSettings = NULL );
	void addMappingAsync( SpaceEntryID mappingID, float * matrix,
		const std::string & path );

	GeometryMapping * getMapping( SpaceEntryID mappingID );
	void delMapping( SpaceEntryID mappingID );
	const GeometryMappings & getMappings()
		{ return mappings_; }

	bool isMapped() const { return mappings_.size() > 0; }

	void clear();
	bool cleared() const { return cleared_; }

	Chunk * findChunkFromPoint( const Vector3 & point );
	Chunk * findChunkFromPointExact( const Vector3 & point );

	Column * column( const Vector3 & point, bool canCreate = true );

	// Collision related methods
	float collide( const Vector3 & start, const Vector3 & end,
		CollisionCallback & cc = CollisionCallback_s_default ) const;

	float collide( const WorldTriangle & start, const Vector3 & end,
		CollisionCallback & cc = CollisionCallback_s_default ) const;

	bool setClosestPortalState( const Vector3 & point,
			bool isPermissive, WorldTriangle::Flags collisionFlags = 0 );

	void dumpDebug() const;

	BoundingBox gridBounds() const;

	Chunk * guessChunk( const Vector3 & point, bool lookInside = false );

	void unloadChunkBeforeBinding( Chunk * pChunk );

	void ignoreChunk( Chunk * pChunk );
	void noticeChunk( Chunk * pChunk );

	void closestUnloadedChunk( float closest ) { closestUnloadedChunk_ = closest; }
	float closestUnloadedChunk()			   { return closestUnloadedChunk_;	}

	bool loadNavmesh() const					{ return loadNavmesh_; }

	bool validatePendingTask( BackgroundTask * pTask );

	Terrain::TerrainSettingsPtr terrainSettings() const;

	void pMappingFactory( GeometryMappingFactory * pFactory )
	{
		pMappingFactory_ = pFactory;
	}

#ifdef EDITOR_ENABLED
	bool	staticLightingOutside() const { return staticLightingOutside_; }
	uint32	staticLightingPortalDepth() const { return staticLightingPortalDepth_; }
#endif

private:
	void recalcGridBounds();

	/**
	 *	This class is used by addMappingAsync to perform the required background
	 *	loading.
	 */
	class LoadMappingTask : public BackgroundTask
	{
	public:
		LoadMappingTask( ChunkSpacePtr pChunkSpace, SpaceEntryID mappingID,
				float * matrix, const std::string & path );

		virtual void doBackgroundTask( BgTaskManager & mgr );
		virtual void doMainThreadTask( BgTaskManager & mgr );

	private:
		ChunkSpacePtr pChunkSpace_;

		SpaceEntryID mappingID_;
	   	Matrix matrix_;
		std::string path_;

		DataSectionPtr pSettings_;
	};

	bool cleared_;

	Terrain::TerrainSettingsPtr	terrainSettings_;

	GeometryMappings		mappings_;
	GeometryMappingFactory * pMappingFactory_;

	float					closestUnloadedChunk_;
	std::set< BackgroundTask * > backgroundTasks_;
#ifdef EDITOR_ENABLED
	bool					staticLightingOutside_;
	uint32					staticLightingPortalDepth_;
#endif
	bool					loadNavmesh_;

	SimpleMutex				mappingsLock_;
};


// TODO: Remove this. Here temporarily while splitting GeometryMapping out of
// this file.
#ifndef MF_SERVER
#include "geometry_mapping.hpp"
#endif

#endif // CHUNK_SPACE_HPP
