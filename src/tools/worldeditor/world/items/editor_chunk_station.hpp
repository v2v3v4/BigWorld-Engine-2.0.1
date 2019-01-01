/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_STATION_HPP
#define EDITOR_CHUNK_STATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "chunk/chunk_stationnode.hpp"


/**
 * This is an individual node within a waypoint station point / patrol point graph.
 */
class EditorChunkStationNode : public EditorChunkSubstance<ChunkStationNode>, public Aligned
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkStationNode )
public:
	EditorChunkStationNode();
	~EditorChunkStationNode();

	virtual bool edAffectShadow() const {	return false; }

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );

	virtual void toss( Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );

	virtual const Matrix & edTransform();

	virtual bool edTransform( const Matrix & m, bool transient );

	virtual std::string edDescription();

	virtual bool edEdit( class GeneralEditor & editor );

	virtual bool edIsSnappable() { return false; }

	virtual bool edCanDelete();

	virtual void edPreDelete();

	virtual void draw();

    virtual bool edShouldDraw();

	virtual bool edIsEditable() const;

    ChunkLink::Direction getLinkDirection( EditorChunkStationNode const * other ) const;

    EditorChunkLinkPtr getLink(size_t idx) const;

    ChunkLinkPtr createLink( ChunkLink::Direction, EditorChunkStationNode* other );  

    virtual bool isEditorChunkStationNode() const;

    bool deleted() const;

    void deleted(bool del);

    void split(EditorChunkStationNode &other);

    virtual bool isValid(std::string &failureMsg) const;

    virtual void makeDirty();

    static void linkClonedNodes
    (
        std::vector<EditorChunkStationNodePtr> const    &cloneNodes,
        std::vector<EditorChunkStationNodePtr>          &newNodes
    );

    static void enableDraw(bool enable);

	virtual void syncInit();
	
protected:
	virtual bool loadName( DataSectionPtr pSection, Chunk * pChunk );

    virtual ChunkLinkPtr createLink() const;

    bool setUserString(std::string const &str);

    std::string getUserString() const;


    void getNeighbourHoodList(std::vector<UniqueID> &neighbours) const;

    void makeNeighboursDirty(std::vector<UniqueID> const &neighbours) const;    

	void syncLinksWithGraph();

private:
	EditorChunkStationNode( const EditorChunkStationNode& );
	EditorChunkStationNode& operator=( const EditorChunkStationNode& );

	virtual const char * sectName() const;
	virtual const char * drawFlag() const;
	virtual ModelPtr reprModel() const;

	Matrix			transform_;
    bool            deleted_;
    static bool     s_enableDraw_;

	ModelPtr model_;
};

typedef SmartPointer<EditorChunkStationNode> EditorChunkStationNodePtr;


#include "gizmo/general_properties.hpp"


/**
 * Station Graphs as properties
 *
 * We extend textproperty, so that borland doesn't need tobe modified to handle
 * the property.
 */
class StationGraphProperty : public TextProperty
{
public:
	StationGraphProperty( const std::string& name, StringProxyPtr graph );

private:
	GENPROPERTY_VIEW_FACTORY_DECLARE( StationGraphProperty )
};


#include "gizmo/current_general_properties.hpp"


class CurrentStationGraphProperties : public PropertyCollator<StationGraphProperty>
{
public:
	CurrentStationGraphProperties( StationGraphProperty& prop ) : PropertyCollator<StationGraphProperty>( prop )
	{
	}

	static GeneralProperty::View * create( StationGraphProperty & prop )
	{
		BW_GUARD;

		return new CurrentStationGraphProperties( prop );
	}

private:
	static struct ViewEnroller
	{
		ViewEnroller()
		{
			StationGraphProperty_registerViewFactory(
				GeneralProperty::nextViewKindID(), &create );
		}
	}	s_viewEnroller;
};


#endif // EDITOR_CHUNK_STATION_HPP
