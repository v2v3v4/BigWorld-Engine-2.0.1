/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_MARKER_CLUSTER_HPP
#define EDITOR_CHUNK_MARKER_CLUSTER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "worldeditor/world/items/editor_chunk_item_tree_node.hpp"
#include "chunk/chunk_marker_cluster.hpp"
#include "resmgr/string_provider.hpp"


/**
 *	This class implements markers (possible spawn points for entities)
 *	Marker is an editor and server concept, so the client does not need it.
 */
class EditorChunkMarkerCluster : public EditorChunkSubstance<ChunkMarkerCluster> 
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkMarkerCluster )
public:
	EditorChunkMarkerCluster();
	~EditorChunkMarkerCluster();

	virtual void draw();

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );

	virtual void toss( Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual bool edEdit( class GeneralEditor & editor );
	virtual bool edCanDelete();
	virtual void edPreDelete();
	virtual void edPostClone( EditorChunkItem* srcItem );
	std::string edDescription() { return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MARK_CLUSTER/ED_DESCRIPTION"); }

	float getAvailableMarkersF() const { return (float)getAvailableMarkers(); }
	bool setAvailableMarkersF(const float& number) { return setAvailableMarkers((uint32)number); }

	int getAvailableMarkers() const { return availableMarkers_; }
	bool setAvailableMarkers(const int& number);

	std::string numberChildrenAsString();

	void setParent(ChunkItemTreeNodePtr parent);

	virtual void onRemoveChild();

private:
	EditorChunkMarkerCluster( const EditorChunkMarkerCluster& );
	EditorChunkMarkerCluster& operator=( const EditorChunkMarkerCluster& );

	virtual const char * sectName() const { return "marker_cluster"; }
	virtual const char * drawFlag() const { return "render/drawEntities"; }

	virtual ModelPtr reprModel() const;

	Matrix transform_;
	ModelPtr model_;
};


typedef SmartPointer<EditorChunkMarkerCluster> EditorChunkMarkerClusterPtr;


#endif // EDITOR_CHUNK_MARKER_CLUSTER_HPP
