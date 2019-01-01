/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_TREE_HPP
#define EDITOR_CHUNK_TREE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk_tree.hpp"
#include "editor_chunk_bsp_holder.hpp"


/**
 *	This class is the editor version of a ChunkTree
 */
class EditorChunkTree : public ChunkTree, EditorChunkBspHolder
{
public:
	EditorChunkTree();
	~EditorChunkTree();

	virtual bool edShouldDraw();
	virtual void draw();

	bool load( DataSectionPtr pSection, Chunk * pChunk );
	/** Called once after loading from the main thread */
	void edPostLoad();

	virtual void toss( Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );
	virtual void edChunkSave();
	virtual void edChunkSaveCData(DataSectionPtr cData);

	virtual const Matrix & edTransform() { return this->transform(); }
	virtual bool edTransform( const Matrix & m, bool transient );
	virtual void edBounds( BoundingBox& bbRet ) const;
	virtual void edSelectedBox( BoundingBox& bbRet ) const; 

	virtual bool edAffectShadow() const;

	virtual bool edEdit( class GeneralEditor & editor );
	virtual Chunk * edDropChunk( const Vector3 & lpos );

	virtual std::string edDescription();

	virtual int edNumTriangles() const;

	virtual int edNumPrimitives() const;

	virtual std::string edAssetName() const;
	virtual std::string edFilePath() const;

	virtual DataSectionPtr pOwnSect()	{ return pOwnSect_; }
	virtual const DataSectionPtr pOwnSect()	const { return pOwnSect_; }

	virtual Moo::LightContainerPtr edVisualiseLightContainer();

	/**
	 * Make sure we've got our own unique lighting data after being cloned
	 */
	void edPostClone( EditorChunkItem* srcItem );

	/** Ensure lighting on the chunk is marked as dirty */
	void edPostCreate();


	/** If we've got a .lighting file, delete it */
	void edPreDelete();

	Vector3 edMovementDeltaSnaps();
	float edAngleSnaps();

private:
	EditorChunkTree( const EditorChunkTree& );
	EditorChunkTree& operator=( const EditorChunkTree& );

	unsigned long getSeed() const;
	bool setSeed( const unsigned long & seed );

	bool getCastsShadow() const;
	bool setCastsShadow( const bool & castsShadow );

	bool			castsShadow_;

	bool			hasPostLoaded_;
	DataSectionPtr	pOwnSect_;

	ModelPtr missingTreeModel_;
	
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkTree )

	// for edDescription
	std::string desc_;

	mutable BoundingBox bspBB_;
};


#endif // EDITOR_CHUNK_TREE_HPP
