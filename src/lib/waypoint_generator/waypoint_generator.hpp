/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WAYPOINT_GENERATOR_HEADER
#define WAYPOINT_GENERATOR_HEADER

#include "waypoint_flood.hpp"
#include "waypoint_view.hpp"

#include "cstdmf/stdmf.hpp"

#include "math/planeeq.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include "resmgr/datasection.hpp"

#include <set>
#include <string>
#include <vector>


#undef DEBUG_UNRECIPROCATED_ADJACENCY

class WaypointGenerator : public IWaypointView
{
public:
	WaypointGenerator();
	virtual ~WaypointGenerator();

	void	clear();
	bool	readTGA(const char* filename);
	bool	init( int width, int height,
		const Vector3 & gridMin, float gridResolution );

	uint32	gridX() const 					{ return gridX_; }
	uint32 	gridZ() const 					{ return gridZ_; }
	uint8 	gridMask(int x, int z) const;// { return grid_[x + gridX_ * z]; }
	Vector3	gridMin() const;
	Vector3 gridMax() const;
	float	gridResolution() const			{ return gridResolution_; }

	int		gridPollCount() const			{ return gridX_*gridZ_*WaypointFlood::MAX_HEIGHTS; }
	bool	gridPoll( int cursor, Vector3 & pt, Vector4 & ahgts ) const;

	AdjGridElt *const*	adjGrids() const	{ return adjGrids_; }
	float *const*		hgtGrids() const	{ return hgtGrids_; }

	void	generate( );
	//void	determineOneWayAdjacencies( );
	void	determineEdgesAdjacentToOtherChunks( Chunk * pChunk, IPhysics* physics ); 
	void	extendThroughUnboundPortals( Chunk * pChunk );
	void	streamline();
	void	calculateSetMembership( const std::vector<Vector3> & frontierPts,
									const std::string & chunkName );

	int		getBSPNodeCount() const;
	int		getBSPNode( int cursor, int depth, BSPNodeInfo & bni ) const;

	int		getPolygonCount() const;
	float	getGirth( int set ) const		{ return 0.f; }
	int		getVertexCount(int polygon) const;
	float	getMinHeight(int polygon) const;
	float	getMaxHeight(int polygon) const;
	int		getSet(int polygon) const;
	void	getVertex(int polygon, int vertex, Vector2& v, int& adjacency,
		bool & adjToAnotherChunk) const;
	void	setAdjacency( int polygon, int vertex, int newAdj );

	int		findWaypoint( const Vector3& v, float girth ) const;

	struct IProgress
	{
		virtual void onProgress(const char* phase, int current) = 0;
	};

	void	setProgressMonitor(IProgress* pProgress) { pProgress_ = pProgress; }

private:
	
	struct BSPNode
	{
		float	borderOffset[8];
		float	splitOffset;
		int		splitNormal;
		int		parent;
		int		front;
		int		back;
		bool	waypoint;
		int		waypointIndex;
		Vector2	centre;
		float	minHeight;
		float	maxHeight;

		mutable int		baseX;
		mutable int		baseZ;
		mutable int		width;
		mutable std::vector<BOOL> pointInNodeFlags;

		void	updatePointInNodeFlags() const;
		BOOL	pointInNode( int x, int z ) const;

		void	calcCentre();
		bool	pointInNode(const Vector2&) const;
	};

	struct SplitDef
	{
		static const int VERTICAL_SPLIT = 8;
		int 	normal;
		float 	value;
		union 
		{
			float position[2];
			float heights[2];
		};
	};

	struct PointDef
	{
		int		angle;
		float	offset;
		float	t;
		float	minHeight;
		float	maxHeight;

		bool operator<(const PointDef& v) const;
		bool operator==(const PointDef& v) const;
		bool nearlySame(const PointDef& v) const;
	};

	struct VertexDef
	{
		VertexDef() : pos( 0.f, 0.f ), adjNavPoly( 0 ), angles( 0 ) { }

		Vector2		pos;
		int			adjNavPoly;
		bool		adjToAnotherChunk;
		int			angles;
	};
	
	struct PolygonDef
	{
		PolygonDef() :
			minHeight( 0.f ), maxHeight( 0.f ), set( 0 ) { }

		std::vector<VertexDef>		vertices;
		float						minHeight;
		float						maxHeight;
		int							set;

		bool ptNearEnough( const Vector3 & pt ) const;

#ifdef DEBUG_UNRECIPROCATED_ADJACENCY
		// this is for debugging purposes only
		int							originalId;
#endif
	};
	
	struct EdgeDef
	{
		Vector2	from;
		Vector2 to;
		int		id;

		bool operator<(const EdgeDef& v) const;
		bool operator==(const EdgeDef& v) const;
	};
	
	struct ChunkDef
	{
		std::string				id;
		std::vector<PlaneEq>	planes;
	};

	AdjGridElt *			adjGrids_[WaypointFlood::MAX_HEIGHTS];
	float *					hgtGrids_[WaypointFlood::MAX_HEIGHTS];
	unsigned int			gridX_;
	unsigned int			gridZ_;
	Vector3					gridMin_;
	float					gridResolution_;
	IProgress*				pProgress_;

	std::vector<BSPNode>	bsp_;
	std::vector<PolygonDef>	polygons_;
	std::set<PointDef>		points_;
	std::set<EdgeDef>		edges_;
	
	Vector3 WaypointGenerator::toVector3( const Vector2 & v, const PolygonDef & polygon,
					 Chunk * pChunk ) const;

	bool	nodesSame( const int index1, const int index2 );
	void 	initBSP();
	void	processNode( std::vector<int> & indexStack );
	bool	findDispoints( const BSPNode & frontNode );
	void 	calcSplitValue( const BSPNode &, SplitDef & );
	void	refineNodeEdges( BSPNode* node ) const;
	void	reduceNodeHeight( BSPNode & node );
	std::pair<float,int> averageHeight(const BSPNode &);
	void	doBestHorizontalSplit( int index, std::vector<int> & indexStack );
	bool	splitNode(int, const SplitDef&, bool replace = false);
	bool	calcEdge(int, int, PointDef&, PointDef&) const;
	void	generatePoints();
	void	generatePolygons();
	void	generateAdjacencies();
	void	invertPoint(PointDef&) const;
	void	pointToVertex(const PointDef&, Vector2&) const;
	bool	isConvexJoint(int a1, int a2, int b) const;
	int		findPolygonVertex(int, const Vector2&) const;
	bool	joinPolygon(int);
	void	joinPolygons();
	void	fixupAdjacencies(int, int, int);

};

#endif
