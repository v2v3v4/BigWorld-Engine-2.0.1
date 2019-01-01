/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WP_MARKER_HPP
#define WP_MARKER_HPP

#include "chunk/chunk_item.hpp"
#include "chunk/chunk.hpp"

#include "marker_girth_info.hpp"

/**
 *	This class is a set of connected waypoints in a chunk.
 *	It may have connections to other waypoint sets when its chunk is bound.
 */

struct MarkerItem
{
	Vector3 position;
	std::string category;
};

class WPMarker : public ChunkItem, public Aligned
{
	// defines the static chunkItemFactory and
	// the create method.
	DECLARE_CHUNK_ITEM( WPMarker )

public:
	WPMarker();
	~WPMarker();

	virtual void toss( Chunk * pChunk );
	virtual void draw();

	bool load( DataSectionPtr pSection );
	const Vector3 & position() const	{ return position_; }

	static std::vector<MarkerItem> markers_;

private:
	std::string typeName_;
	DataSectionPtr pProps_;
	void finishLoad();
	WPMarker( const WPMarker& );
	WPMarker& operator=( const WPMarker& );

	Vector3		position_;
	std::string category_;

	Matrix		transform_;
	class SuperModel			* pSuperModel_;
};

typedef SmartPointer<WPMarker> WPMarkerPtr;

typedef std::vector<WPMarkerPtr> WPMarkers;


/**
 *	This class is a cache of all the entities in a given chunk
 */
class WPMarkerCache : public ChunkCache
{
public:
	WPMarkerCache( Chunk & chunk );
	~WPMarkerCache();

	WPMarkers::iterator begin()	{ return markers_.begin(); }
	WPMarkers::iterator end()	{ return markers_.end(); }

	void add( WPMarkerPtr e );
	void del( WPMarkerPtr e );

	static Instance<WPMarkerCache> instance;

private:
	WPMarkers markers_;
};

#endif