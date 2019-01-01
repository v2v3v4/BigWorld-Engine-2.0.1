/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <vector>
#include <map>
#include <set>

#include "cstdmf/aligned.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "chunk_boundary.hpp"

#include "moo/moo_math.hpp"

class Chunk;
class ChunkSpace;
class ChunkItem;
typedef uint32 ChunkSpaceID;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;
class GeometryMapping;

#include "umbra_config.hpp"

#if UMBRA_ENABLE
namespace Umbra
{
	class Camera;
	class Cell;
}
#endif


/**
 *	This singleton class manages most aspects of the chunky scene graph.
 *	It contains most of the API that classes outside the chunk library will
 *	need to use. It manages the universe that the game runs.
 *
 *	A universe defines the world for a whole game - both the client and
 *	the server run only one universe. Each universe is split up into
 *	a number of named spaces.
 */
class ChunkManager : public Aligned
{
public:
	ChunkManager();
	~ChunkManager();

	bool init( DataSectionPtr configSection = NULL );
	bool fini();

	// set the camera position
	void camera( const Matrix & cameraTransform, ChunkSpacePtr pSpace, Chunk* pOverride = NULL );
	// get camera transform
	const Matrix & cameraTrans() const { return cameraTrans_; }

	// call everyone's tick method, plus scan for
	// new chunks to load and old chunks to dispose
	void tick( float dTime );

	// draw the scene from the set camera position
	void draw();
	void drawReflection(const std::vector<Chunk*>& pVisibleChunks, bool outsideChunks, float nearPoint);

#if UMBRA_ENABLE
	void umbraDraw();
	void umbraRepeat();
#endif

	// add/del fringe chunks from this draw call
	void addFringe( Chunk * pChunk );
	void delFringe( Chunk * pChunk );

	// add the given chunk pointer into the space, can be called from background thread.
	void addChunkToSpace( Chunk * pChunk, ChunkSpaceID spaceID );

	// append the given chunk to the load list
//	void loadChunkExplicitly( const std::string & identifier, int spaceID );
	void loadChunkExplicitly( const std::string & identifier,
		GeometryMapping * pMapping, bool isOverlapper = false );

	Chunk* findChunkByName( const std::string & identifier, 
        GeometryMapping * pMapping, bool createIfNotFound = true );
	Chunk* findChunkByGrid( int16 x, int16 z, GeometryMapping * pMapping );

	void loadChunkNow( Chunk* chunk );
	void loadChunkNow( const std::string & identifier, GeometryMapping * pMapping );

	// accessors
	ChunkSpacePtr space( ChunkSpaceID spaceID, bool createIfMissing = true );

	ChunkSpacePtr cameraSpace() const;
	Chunk * cameraChunk() const			{ return cameraChunk_; }

	void clearAllSpaces( bool keepClientOnlySpaces = false );

	bool busy() const			{ return !loadingChunks_.empty(); }

	float maxLoadPath() const	{ return maxLoadPath_; }
	float minUnloadPath() const	{ return minUnloadPath_; }
	void maxLoadPath( float v )		{ maxLoadPath_ = v; }
	void minUnloadPath( float v )	{ minUnloadPath_ = v; }
	void maxUnloadChunks( unsigned int maxUnloadChunks)	{ maxUnloadChunks_ = maxUnloadChunks; }
	void autoSetPathConstraints( float farPlane );
	float closestUnloadedChunk( ChunkSpacePtr pSpace ) const;

	void addSpace( ChunkSpace * pSpace );
	void delSpace( ChunkSpace * pSpace );

	static ChunkManager & instance();

	static void drawTreeBranch( Chunk * pChunk, const std::string & why );
	static void drawTreeReturn();
	static const std::string & drawTree();

	static int      s_chunksTraversed;
	static int      s_chunksVisible;
	static int      s_chunksReflected;
	static int      s_visibleCount;
	static int      s_drawPass;

	static bool     s_drawVisibilityBBoxes;

	bool checkLoadingChunks();

	bool canLoadChunks() const { return canLoadChunks_; }
	void canLoadChunks( bool canLoadChunks ) { canLoadChunks_ = canLoadChunks; }

	void switchToSyncMode( bool sync );
	void switchToSyncTerrainLoad( bool sync );

	void chunkDeleted( Chunk* pChunk );
	void mappingCondemned( GeometryMapping* pMapping );

#if UMBRA_ENABLE
	Umbra::Camera*	getUmbraCamera() { return umbraCamera_; }
	void			setUmbraCamera( Umbra::Camera* pCamera ) { umbraCamera_ = pCamera; }
#endif	

	Vector3 cameraNearPoint() const;
	Vector3 cameraAxis(int axis) const;

	/**
	 * Allow debug_app to write this to "special" console. 
	 */
	static std::string s_specialConsoleString;

	/**
	 *	Access the tickMark it us incremented every time tick is called
	 *	so that objects that tick on draw can work out whether tick has
	 *	been called since last time it was ticked
	 */
	uint32	tickMark() const { return tickMark_; }
	/**
	 *	Access the total tick time in ms, this is the number of milliseconds
	 *	that have elapsed since the chunk manager was started.
	 */
	uint64	totalTickTimeInMS() const { return totalTickTimeInMS_; }
	/**
	 *	Access the last delta time passed into the tick method
	 */
	float	dTime() const { return dTime_; }

private:
	bool		initted_;

	typedef std::map<ChunkSpaceID,ChunkSpace*>	ChunkSpaces;
	ChunkSpaces			spaces_;

	Matrix			cameraTrans_;
	ChunkSpacePtr	pCameraSpace_;
	Chunk			* cameraChunk_;

	typedef std::vector<Chunk*>		ChunkVector;
	ChunkVector		loadingChunks_;
	Chunk			* pFoundSeed_;
	Chunk			* fringeHead_;

	typedef std::pair<std::string, GeometryMapping*> StrMappingPair;
	std::set<StrMappingPair>	pendingChunks_;
	SimpleMutex					pendingChunksMutex_;

	typedef std::pair<Chunk*, ChunkSpaceID> ChunkPtrSpaceIDPair;
	std::set<ChunkPtrSpaceIDPair>	pendingChunkPtrs_;
	std::set<Chunk*>				deletedChunks_;
	SimpleMutex				pendingChunkPtrsMutex_;


	bool scan();
	bool blindpanic();
	bool autoBootstrapSeedChunk();

	void loadChunk( Chunk * pChunk, bool highPriority );

	struct PortalBounds
	{
		bool init( Portal2DRef portal, float minDepth );
		Vector2 min_;
		Vector2 max_;
		float	minDepth_;
		Portal2DRef portal2D_;
	};

	typedef std::vector<PortalBounds> PortalBoundsVector;

	void cullInsideChunks( Chunk* pChunk, ChunkBoundary::Portal *pPortal, Portal2DRef portal2D, 
								  ChunkVector& chunks, PortalBoundsVector& outsidePortals, bool ignoreOutsidePortals );

	void cullOutsideChunks( ChunkVector& chunks, const PortalBoundsVector& outsidePortals );


	void			checkCameraBoundaries();

	float			maxLoadPath_;	// bigger than sqrt(500^2 + 500^2)
	float			minUnloadPath_;

	float			scanSkippedFor_;
	Vector3			cameraAtLastScan_;
	bool			noneLoadedAtLastScan_;

	bool			canLoadChunks_;

	// The maximum number of chunks that can be scheduled for unloading. It
	// should unload chunks aggressively in the tool to free memory.
	unsigned int	maxUnloadChunks_;

	unsigned int	workingInSyncMode_;
	unsigned int	waitingForTerrainLoad_;
#if UMBRA_ENABLE
	// umbra
	Umbra::Camera*	umbraCamera_;
	bool useLatentOcclusionQueries_;
	ChunkVector		umbraInsideChunks_;
#endif


	// These values are used for objects that tick without having their tick
	// method called. 

	// tickMark_ is incremented every time tick is called so that objects
	// that draw multiple times in a frame can identify whether tick has been
	// called since last time it was drawn, this is useful for objects that
	// do their tick when they are drawn (i.e. animating chunk models)
	// See ChunkModel::draw for an example of how this is implemented
	uint32	tickMark_;

	// totalTickTimeInMS_ in the number of milliseconds that have elapsed since
	// the chunk manager was started, this is useful for objects that tick 
	// themselves when they are drawn
	uint64	totalTickTimeInMS_;

	// dTime_ is the last time delta that was passed into the tick method
	float	dTime_;
};


/**
 *  This class is used to turn on/off sync mode of the ChunkManager in a 
 *  scoped fashion.
 */
class ScopedSyncMode
{
public:
    ScopedSyncMode();
    ~ScopedSyncMode();

private:
    ScopedSyncMode(const ScopedSyncMode &);             // not allowed
    ScopedSyncMode &operator=(const ScopedSyncMode &);  // not allowed
};


#endif // CHUNK_MANAGER_HPP
