/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _CHUNK_VIEW_HEADER
#define _CHUNK_VIEW_HEADER

#include <vector>
#include <string>

#include "waypoint_view.hpp"
#include "resmgr/datasection.hpp"

class ChunkView : public IWaypointView
{
public:
	ChunkView();
	~ChunkView();

	void clear();
	bool load( Chunk * pChunk );

	uint32	gridX() const;
	uint32 	gridZ() const;
	uint8 	gridMask(int x, int z) const;
	Vector3	gridMin() const;
	Vector3 gridMax() const;
	float	gridResolution() const;

	int		getPolygonCount() const;
	int		getBSPNodeCount() const;
	float	getGirth( int set ) const;
	int		getVertexCount(int polygon) const;
	float	getMinHeight( int polygon ) const;
	float	getMaxHeight( int polygon ) const;
	int		getSet( int polygon ) const;
	void	getVertex( int polygon, int vertex, 
		Vector2& v, int& adjacency, bool & adjToAnotherChunk ) const;
	void	setAdjacency( int polygon, int vertex, int newAdj );

	int		findWaypoint( const Vector3&, float girth ) const;

private:
	Vector3		gridMin_;
	Vector3		gridMax_;

	struct VertexDef
	{
		Vector2 position;
		int		adjacentID;
		bool	adjToAnotherChunk;
	};

	struct PolygonDef
	{
		float minHeight;
		float maxHeight;
		std::vector<VertexDef> vertices;
		int set;
	};

	struct SetDef
	{
		SetDef() : girth( 0.f ) { }
		float girth;
	};

	std::vector<PolygonDef> polygons_;
	std::vector<SetDef> sets_;
	int		nextID_;
	bool	minMaxValid_;

	void parseNavPoly( DataSectionPtr pSect, int set, Chunk * pChunk = NULL );
};

#endif
