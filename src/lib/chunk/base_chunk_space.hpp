/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_CHUNK_SPACE_HPP
#define BASE_CHUNK_SPACE_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stringmap.hpp"

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "network/basictypes.hpp"
#include "physics2/quad_tree.hpp"

#include <map>

class Chunk;
class ChunkItem;
class BoundingBox;
class HullTree;
class HullBorder;
class HullContents;
class BSP;
class WorldTriangle;
class OutsideLighting;
class DataSection;

class ChunkObstacle;

typedef SmartPointer<DataSection> DataSectionPtr;

typedef QuadTree< ChunkObstacle> ObstacleTree;
typedef QTTraversal< ChunkObstacle > ObstacleTreeTraversal;


/**
 *	Outside chunk size in metres. Chunks may not be bigger than this in the x or
 *	z dimension.
 */
const float GRID_RESOLUTION = 100.f;
const float MAX_CHUNK_HEIGHT = 10000.f;
const float MAX_SPACE_WIDTH = 50000.f;
const float MIN_CHUNK_HEIGHT = -10000.f;
extern float g_gridResolution;	// use to avoid recompilation when it changes

typedef uint32 ChunkSpaceID;
/**
 *	The null value for an ChunkSpaceID meaning that no space is referenced. 
 */
const ChunkSpaceID NULL_CHUNK_SPACE = ((ChunkSpaceID)0);

/*
 *	Forward declarations relating to chunk obstacles
 */
class CollisionCallback;
extern CollisionCallback & CollisionCallback_s_default;


/**
 *	This map is used to map a chunk's name to its pointer.
 */
typedef StringHashMap< std::vector<Chunk*> > ChunkMap;

/**
 *	This map is used to map an outside chunk's coordinate its pointer.
 */
typedef std::map< std::pair<int, int>, std::vector<Chunk*> > GridChunkMap;


static const std::string& SPACE_SETTING_FILE_NAME = "space.settings";
static const std::wstring& SPACE_SETTING_FILE_NAME_W = L"space.settings";
static const std::string& SPACE_LOCAL_SETTING_FILE_NAME = "space.localsettings";
static const std::wstring& SPACE_LOCAL_SETTING_FILE_NAME_W = L"space.localsettings";

/**
 *	This class is the base class that is used by the client and the server to
 *	implement their ChunkSpace.
 */
class BaseChunkSpace : public SafeReferenceCount
{
public:
	BaseChunkSpace( ChunkSpaceID id );
	virtual ~BaseChunkSpace();

	ChunkSpaceID id() const					{ return id_; }

	void addChunk( Chunk * pChunk );
	Chunk * findOrAddChunk( Chunk * pChunk );
	Chunk * findChunk( const std::string & identifier,
		const std::string & mappingName );
	void delChunk( Chunk * pChunk );
	void clear();
	void clearSpaceData();

	bool	inBounds( int gridX, int gridY ) const;

	int minGridX() const 			{ return minGridX_; }
	int maxGridX() const 			{ return maxGridX_; }
	int minGridY() const 			{ return minGridY_; }
	int maxGridY() const 			{ return maxGridY_; }

	float minCoordX() const 			{ return GRID_RESOLUTION * minGridX_; }
	float maxCoordX() const 			{ return GRID_RESOLUTION * (maxGridX_ + 1); }
	float minCoordZ() const 			{ return GRID_RESOLUTION * minGridY_; }
	float maxCoordZ() const 			{ return GRID_RESOLUTION * (maxGridY_ + 1); }

	static int obstacleTreeDepth()		{ return s_obstacleTreeDepth; }
	static void obstacleTreeDepth( int v )	{ s_obstacleTreeDepth = v; }

	/**
	 *	This class is used to represent a grid square of the space. It stores
	 *	all of the chunks and obstacles that overlap this grid.
	 */
	class Column
	{
	public:
		Column( int x, int z );
		~Column();

		// Chunk related methods
		void addChunk( HullBorder & border, Chunk * pChunk );
		bool hasInsideChunks() const			{ return pChunkTree_ != NULL; }

		Chunk * findChunk( const Vector3 & point );
		Chunk * findChunkExcluding( const Vector3 & point, Chunk * pNot );

		// Obstacle related methods
		void addObstacle( const ChunkObstacle & obstacle );
		const ObstacleTree &	obstacles() const	{ return *pObstacleTree_; }
		ObstacleTree &			obstacles()			{ return *pObstacleTree_; }

		void addDynamicObstacle( const ChunkObstacle & obstacle );
		void delDynamicObstacle( const ChunkObstacle & obstacle );

		Chunk *		pOutsideChunk() const	{ return pOutsideChunk_; }

		// The stale flag gets set to indicate that the column needs to be
		// recreated. For example, when obstacles want to get removed from the
		// space, the will mark the column as stale, and the column will get
		// recreated in the next frame.
		void		stale()				{ stale_ = true; }
		bool		isStale() const		{ return stale_; }

		long size() const;

		void		openAndSee( Chunk * pChunk );
		void		shutIfSeen( Chunk * pChunk );

	protected:

		typedef std::vector< const ChunkObstacle * > HeldObstacles;
		typedef std::vector< HullContents * >	Holdings;

		HullTree *		pChunkTree_;
		ObstacleTree *	pObstacleTree_;
		HeldObstacles	heldObstacles_;
		Holdings		holdings_;

		Chunk *			pOutsideChunk_;

		bool 			stale_;
		Chunk *			shutTo_;

		std::vector< Chunk * >			seen_;

	};

	static inline float gridToPoint( int grid )
	{
		return grid * GRID_RESOLUTION;
	}

	static inline int pointToGrid( float point )
	{
		return static_cast<int>( floorf( point / GRID_RESOLUTION ) );
	}

	static inline float alignToGrid( float point )
	{
		return gridToPoint( pointToGrid( point ) );
	}

	/**
	 *	This is the extended key for a data entry in our map of data.
	 *	It is sorted first by key then by entry id.
	 */
	struct DataEntryMapKey
	{
		SpaceEntryID	entryID;
		uint16			key;
		bool operator<( const DataEntryMapKey & other ) const
		{ return key < other.key ||
			(key == other.key && entryID < other.entryID); }
	};
	typedef std::map<DataEntryMapKey,std::string> DataEntryMap;
	typedef std::pair<uint16,const std::string*> DataValueReturn;

	uint16 dataEntry( const SpaceEntryID & entryID, uint16 key,
		const std::string & data );
	DataValueReturn dataRetrieveSpecific(
		const SpaceEntryID & entryID, uint16 key = uint16(-1) );

	const std::string * dataRetrieveFirst( uint16 key );
	DataEntryMap::const_iterator dataRetrieve( uint16 key );

	const DataEntryMap & dataEntries()	{ return dataEntries_; }
	// more similar functions can easily be defined as they become necessary

	void mappingSettings( DataSectionPtr pSS );
	virtual bool isMapped() const = 0;

	void blurredChunk( Chunk * pChunk );
	bool removeFromBlurred( Chunk * pChunk );

private:
	ChunkSpaceID	id_;

	DataEntryMap	dataEntries_;

protected:
	void recalcGridBounds()		{ }	// implemented in derived classes

	void fini();

	int				minGridX_;
	int				maxGridX_;
	int				minGridY_;
	int				maxGridY_;

	ChunkMap		currentChunks_;
	GridChunkMap	gridChunks_;

	std::vector<Chunk*>			blurred_;	// bound but not focussed

	static int s_obstacleTreeDepth;
};

#endif // BASE_CHUNK_SPACE_HPP
