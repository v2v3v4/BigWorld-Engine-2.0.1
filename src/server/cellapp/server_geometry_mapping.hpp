/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_GEOMETRY_MAPPING_HPP
#define SERVER_GEOMETRY_MAPPING_HPP

#include "loading_edge.hpp"

#include "chunk/geometry_mapping.hpp"

#include "math/vector3.hpp"
#include "math/rectt.hpp"

class Chunk;
class ServerGeometryMappings;
class Vector3;


/**
 *	This class handles the loading of one GeometryMapping in a Space.
 */
class ServerGeometryMapping : public GeometryMapping
{
public:
	ServerGeometryMapping( ChunkSpacePtr pSpace, const Matrix & m,
		const std::string & path, DataSectionPtr pSettings,
		ServerGeometryMappings & mappings, const Vector3 & initialPoint );
	~ServerGeometryMapping();

	bool tick( const BW::Rect & rect, bool unloadOnly );
	bool hasFullyLoaded();
	bool isFullyUnloaded() const;

	void calcLoadedRect( BW::Rect & loadedRect,
		   bool isFirst, bool isLast ) const;

	void prepareNewlyLoadedChunksForDelete();

	void bind( Chunk * pChunk );
	void unbindAndUnload( Chunk * pChunk );

	Chunk * findOrAddOutsideChunk( int xGrid, int yGrid );

private:
	void postBoundChunk( Chunk * pChunk );
	void preUnbindChunk( Chunk * pChunk );

   	LoadingEdge edge_[4];	// in order L,B,R,T

	enum EdgeType
	{
		LEFT = 0,
		BOTTOM = 1,
		RIGHT = 2,
		TOP = 3
	};

	// Loaded rect is (left, bottom) to (right, top):
	//  (edge_[0].currPos_, edge_[1].currPos_) to:
	//  (edge_[2].currPos_, edge_[3].currPos_)

	// Destination rect to be loaded is (left, bottom) to (right, top):
	//  (edge_[0].destPos_, edge_[1].destPos_) to:
	//  (edge_[2].destPos_, edge_[3].destPos_)

	bool currLoadedAll_;	// do we think we have it all loaded

	ServerGeometryMappings & mappings_;
};

#endif // SERVER_GEOMETRY_MAPPING_HPP
