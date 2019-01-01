/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_LINK_HPP
#define EDITOR_CHUNK_LINK_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "worldeditor/world/items/editor_chunk_station.hpp"
#include "worldeditor/world/items/chunk_link_segment.hpp"
#include "chunk/chunk_link.hpp"
#include "appmgr/options.hpp"
#include "moo/moo_math.hpp"
#include "moo/base_texture.hpp"
#include "moo/index_buffer.hpp"
#include "moo/vertex_buffer.hpp"


/**
 *  This class represents a link between items.  These links need to be created
 *  by the objects that have links, they are not saved.
 */
class EditorChunkLink : public EditorChunkSubstance<ChunkLink>, public Aligned
{
    DECLARE_EDITOR_CHUNK_ITEM(EditorChunkLink)

public:
    EditorChunkLink();
    ~EditorChunkLink();

	static void flush( float dtime, bool clearOnly = false );

	virtual bool edAffectShadow() const {	return false; }

    /*virtual*/ void draw();

    /*virtual*/ char const *sectName() const;

    /*virtual*/ const char * drawFlag() const;

    /*virtual*/ ModelPtr reprModel() const;

	/*virtual*/ bool autoAddToSceneBrowser() const { return false; }

    /*virtual*/ const Matrix & edTransform();

    /*virtual*/ void toss(Chunk *chunk);

    /*virtual*/ void tick(float dtime);

    /*virtual*/ void addAsObstacle();

    /*virtual*/ std::vector<std::string>
    edCommand
    (
        std::string     const&path
    ) const;

    /*virtual*/ bool
    edExecuteCommand
    (
        std::string     const &path,
        std::vector<std::string>::size_type index
    );

	/*virtual*/ bool edShouldDraw();

    virtual float
    collide
    (
        Vector3         const &source,
        Vector3         const &dir,
        WorldTriangle   &wt
    ) const;

    Vector3 midPoint(Chunk *&chunk) const;

	virtual bool edFrozen() const;

    /*virtual*/ void edBounds(BoundingBox &bb) const;

	/*virtual*/ void edSelectedBox( BoundingBox & bbRet ) const;

    /*virtual*/ bool isEditorChunkLink() const;

	/*virtual*/ bool edIsSnappable() { return false; }

    /*virtual*/ bool edCanAddSelection() const;

    /*virtual*/ bool isValid(std::string &failureMsg) const;

    /*virtual*/ void makeDirty();

    static void enableDraw(bool enable);

	void chunkLoaded(const std::string& chunkId);

	void chunkTossed(const std::string& chunkId);
	
	virtual void syncInit();
	
protected:
    // Neighbor links to this one are links before and after this link, as
    // well as this link that go through nodes that only have a two links.
    // The second parameter is true if the link goes is in the same order
    // as the original link (e.g. this->start == link->end) or false if in the
    // opposite order (e.g. this->start == link->start.)
    typedef std::pair<EditorChunkLink *, bool> Neighbor;

	// constants, potentially used by derived classes
    static const float MAX_SEG_LENGTH;
    static const float MIN_SEG_LENGTH;
    static const float LINK_THICKNESS;
    static const float SEGMENT_SPEED;
	static const float HEIGHT_BUFFER;
    static const float NEXT_HEIGHT_SAMPLE;
    static const float NEXT_HEIGHT_SAMPLE_MID;
    static const float MAX_SEARCH_HEIGHT;
    static const float AIR_THRESHOLD;
    static const float BB_OFFSET;
	static const float VERTICAL_LINK_EPSILON;

	Chunk* outsideChunk() const;

	bool addYBounds( BoundingBox& ) const	{	return false;	}

    void recalcMesh(Vector3 const &s, Vector3 const &e, bool updateCollisions = true);

	void alignLineWithXAxis(
		Matrix& align, Matrix& invAlign, Vector3 startPoint,
		Vector3 endPoint) const;

    virtual bool getEndPoints(Vector3 &s, Vector3 &e, bool absoluteCoords) const;

	bool isEitherEndTransient();

    float heightAtPos(float x, float y, float z, bool *foundHeight = NULL) const;

    Chunk *getChunk(int x, int y) const;

    void addToLentChunks();

    void removeFromLentChunks();

    EditorChunkStationNodePtr startNode() const;

    EditorChunkStationNodePtr endNode() const;

    void neighborhoodLinks(std::vector<Neighbor> &neighbors) const;

    void deleteCommand();

    void swapCommand();

    void bothDirCommand();

    void swapRunCommand();

    void bothDirRunCommand();

    void splitCommand();

    void validateCommand();

    void deleteLinkCommand();

    void loadGraph();

	void directionalTexture( ComObjectWrap<DX::BaseTexture> texture );
	void noDirectionTexture( ComObjectWrap<DX::BaseTexture> texture );
	void materialEffect( Moo::EffectMaterialPtr effect );
	Moo::EffectMaterialPtr materialEffect();

    void drawImmediate();

    static bool enableDraw();

private:
    EditorChunkLink(EditorChunkLink const &);
    EditorChunkLink &operator=(EditorChunkLink const &);

	void batch( bool colourise, bool frozen = false );

protected:
	mutable bool						highlight_;			  // Is the link being highlighted?

private:
	Chunk *								pLastObstacleChunk_;  // Keeps track of where the obstacle was last added
    float                               yOffset_;             // offset above ground
    Vector3                             lastStart_;           // last render start point
    Vector3                             lastEnd_;             // last render end point
    std::vector<Vector3>				polyline_;            // The link polyline
    std::vector<ChunkLinkSegmentPtr>    meshes_;              // mesh used to draw
    mutable Matrix                      bbTransform_;         // bounding box local transform
	mutable Matrix						bbInvTransform_;      // inverse of bbTransform_
    mutable Matrix                      bbShellTransform_;    // only used in shells
    mutable Matrix                      lineTransform_;       // line local transform
	mutable Matrix						lineInvTransform_;    // inverse of lineTransform_
    ComObjectWrap<DX::BaseTexture>		directionalTexture_;  // texture used to directional links draw with
    ComObjectWrap<DX::BaseTexture>		noDirectionTexture_;  // texture used to draw links without direction
    Moo::EffectMaterialPtr				meshEffect_;          // effect file used to draw mesh
    std::vector<ChunkPtr>               lentChunks_;          // chunks lent out to
	std::set<std::string>				unlentChunks_;        // chunks that still require lending
	BoundingBox							bb_;                  // bounding box in chunk
    float                               minY_;                // minimum y coord (helps when going over mountains)
    float                               maxY_;                // maximum y coord (helps when going over mountains)
    bool                                needRecalc_;          // is a recalc needed due to a non-loaded chunk?
    float                               midY_;                // height at around the mid point.
	Moo::VertexBuffer                   vertexBuffer_;        // vertex buffer for mesh
	int									vertexBufferSize_;
	Moo::IndexBuffer                    indexBuffer_;         // index buffer for mesh
	Moo::VertexBuffer                   lineVertexBuffer_;    // vertex buffer for line
	Moo::IndexBuffer                    lineIndexBuffer_;     // index buffer for line

	static bool							s_linkCollide_;       // optimisation when finding the heightAtPos
    static bool                         s_enableDraw_;        // drawing enabled?

	// Statics used in batched link rendering
	static float s_totalTime_;
};


typedef SmartPointer<EditorChunkLink>   EditorChunkLinkPtr;


#endif // EDITOR_CHUNK_LINK_HPP
