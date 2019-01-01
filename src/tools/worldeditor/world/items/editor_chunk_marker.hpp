/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_MARKER_HPP
#define EDITOR_CHUNK_MARKER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "chunk/chunk_marker.hpp"
#include "resmgr/string_provider.hpp"


/**
 *	This class implements markers (possible spawn points for entities)
 *	Marker is an editor and server concept, so the client does not need it.
 */
class EditorChunkMarker : public EditorChunkSubstance<ChunkMarker>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkMarker )
public:
	EditorChunkMarker();
	~EditorChunkMarker();

	virtual void draw();

	bool load( DataSectionPtr pSection, std::string* errorString = NULL );
	void edMainThreadLoad();

	virtual void toss( Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual bool edEdit( class GeneralEditor & editor );
	virtual bool edCanDelete();
	virtual void edPreDelete();
	virtual void edPostClone( EditorChunkItem* srcItem );

	std::string edDescription() { return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MARK/ED_DESCRIPTION"); }

	std::string getCategory() const { return category_; }
	bool setCategory(const std::string & newCategory);

	virtual void setParent(ChunkItemTreeNodePtr parent);
	
	virtual void syncInit();
	
private:
	EditorChunkMarker( const EditorChunkMarker& );
	EditorChunkMarker& operator=( const EditorChunkMarker& );

	virtual const char * sectName() const { return "marker"; }
	virtual const char * drawFlag() const { return "render/drawEntities"; }

	virtual ModelPtr reprModel() const;

	bool loadEntityDescription();
	void findCommonProperties();

	bool loadEntityDescription_;

	void syncReprModel(bool createNewModelOnly = false);

	EditorChunkEntityPtr edEntity_;
    class SuperModel * superModel_;

	typedef std::vector< SmartPointer< class Fashion > > FashionVector;
	FashionVector fv_;

	enum MType
	{
		MType_Unknown,
		MType_Entity,
		MType_Marker,
		MType_MarkerWithoutEntity
	};
	MType mtype_;
};


typedef SmartPointer<EditorChunkMarker> EditorChunkMarkerPtr;



#endif // EDITOR_CHUNK_MARKER_HPP
