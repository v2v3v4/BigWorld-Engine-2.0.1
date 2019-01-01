/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_HPP
#define SPACE_HPP

#include "cellapp_interface.hpp"

#include "cell_range_list.hpp"
#include "server_geometry_mappings.hpp"

#include "chunk/chunk_space.hpp"

#include "math/math_extra.hpp"
#include "network/basictypes.hpp"

#include <vector>

class BinaryIStream;
class Cell;
class CellInfo;
class CellInfoVisitor;
class Chunk;
class SpaceNode;

class ChunkSpace;
typedef SmartPointer< ChunkSpace > ChunkSpacePtr;

class Entity;
typedef SmartPointer< Entity > EntityPtr;

// These do not need to be reference counted since they are added in the
// construction and removed in destruction (or destroy) of entity.
typedef std::vector< EntityPtr >                 SpaceEntities;
typedef SpaceEntities::size_type                 SpaceRemovalHandle;

class Watcher;
typedef SmartPointer< Watcher > WatcherPtr;

/**
 *	This class is used to represent a space.
 */
class Space : public TimerHandler
{
public:
	Space( SpaceID id );
	virtual ~Space();

	void reuse();

	const CellInfo * pCellAt( float x, float z ) const;
	void visitRect( const BW::Rect & rect, CellInfoVisitor & visitRect );

	// ---- Accessors ----
	SpaceID id() const						{ return id_; }

	Cell * pCell() const					{ return pCell_; }
	void pCell( Cell * pCell );

	ChunkSpacePtr pChunkSpace() const;

	Vector3 cellCentre() const;

	// ---- Entity ----
	void createGhost( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

		// ( EntityID id, ...
	void createGhost( const EntityID entityID,
					  BinaryIStream & data );
		// ( TypeID type, Position3D & pos,
		//  string & ghostState, Mercury::Address & owner );

	void addEntity( Entity * pEntity );
	void removeEntity( Entity * pEntity );

	EntityPtr newEntity( EntityID id, EntityTypeID entityTypeID );

	Entity * findNearestEntity( const Vector3 & position );

	// ---- Static methods ----
	enum UpdateCellAppMgr
	{
		UPDATE_CELL_APP_MGR,
		DONT_UPDATE_CELL_APP_MGR
	};

	enum DataEffected
	{
		ALREADY_EFFECTED,
		NEED_TO_EFFECT
	};

	static WatcherPtr pWatcher();

	// ---- Space data ----
	void spaceData( BinaryIStream & data );
	void allSpaceData( BinaryIStream & data );

	void updateGeometry( BinaryIStream & data );

	void spaceGeometryLoaded( BinaryIStream & data );

	void setLastMappedGeometry( const std::string& lastMappedGeometry )
	{ lastMappedGeometry_ = lastMappedGeometry; }

	void shutDownSpace( BinaryIStream & data );
	void requestShutDown();

	CellInfo * findCell( const Mercury::Address & addr ) const;

	SpaceNode * readTree( BinaryIStream & stream, const BW::Rect & rect );

	bool spaceDataEntry( const SpaceEntryID & entryID, uint16 key,
		const std::string & value,
		UpdateCellAppMgr cellAppMgrAction = UPDATE_CELL_APP_MGR,
		DataEffected effected = NEED_TO_EFFECT );

	int32 begDataSeq() const	{ return begDataSeq_; }
	int32 endDataSeq() const	{ return endDataSeq_; }

	const std::string * dataBySeq( int32 seq,
		SpaceEntryID & entryID, uint16 & key ) const;
	int dataRecencyLevel( int32 seq ) const;

	const RangeList & rangeList() const	{ return rangeList_; }
	bool getRealEntitiesBoundary( BW::Rect & boundary,
		   int numToSkip = 0 ) const;

	void debugRangeList();

	SpaceEntities & spaceEntities() { return entities_; }
	const SpaceEntities & spaceEntities() const { return entities_; }

	void writeDataToStream( BinaryOStream & steam ) const;
	void readDataFromStream( BinaryIStream & stream );

	void chunkTick();
	void calcLoadedRect( BW::Rect & loadedRect ) const;
	void prepareNewlyLoadedChunksForDelete();

	bool isFullyUnloaded() const;

	float timeOfDay() const;

	bool isShuttingDown() const		{ return shuttingDownTimerHandle_.isSet(); }

	void writeRecoveryData( BinaryOStream & stream ) const;

	void setPendingCellDelete( const Mercury::Address & addr );

	size_t numEntities() const	{ return entities_.size(); }

private:
	typedef std::map< Mercury::Address, SmartPointer< CellInfo > > CellInfos;

	virtual void handleTimeout( TimerHandle handle, void * arg );

	void calcBound( bool isMin, bool isY, int numToSkip,
			BW::Rect & boundary ) const;

	void checkForShutDown();

	bool hasSingleCell() const;

	SpaceID	id_;

	Cell *	pCell_;
	ChunkSpacePtr pChunkSpace_;

	SpaceEntities	entities_;
	CellInfos		cellInfos_;

	RangeList	rangeList_;

	int32	begDataSeq_;
	int32	endDataSeq_;

	/**
	 *	This is the information we record about recent data entries
	 */
	struct RecentDataEntry
	{
		SpaceEntryID	entryID;
		GameTime		time;
		uint16			key;
	};
	typedef std::vector<RecentDataEntry> RecentDataEntries;
	RecentDataEntries	recentData_;
	// TODO: Need to clean out recent data every so often (say once a minute)
	// Also don't let beg/endDataSeq go negative.
	// Yes that will require extensive fixups.

	ServerGeometryMappings	geometryMappings_;
	std::list< Chunk * > loadingChunks_;

	float	initialTimeOfDay_;
	float	gameSecondsPerSecond_;

	std::string lastMappedGeometry_;

	SpaceNode *	pCellInfoTree_;

	TimerHandle shuttingDownTimerHandle_;

public:
	static uint32 s_allSpacesDataChangeSeq_;
};

#ifdef CODE_INLINE
#include "space.ipp"
#endif

#endif // SPACE_HPP

// space.hpp
