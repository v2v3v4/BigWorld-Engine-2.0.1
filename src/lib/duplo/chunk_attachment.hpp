/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_ATTACHMENT_HPP
#define CHUNK_ATTACHMENT_HPP

#include "py_attachment.hpp"
#include "chunk_embodiment.hpp"



/**
 *	This class is a chunk item that has an attachment point.
 *	It makes many calls through to the item that it has attached.
 */
class ChunkAttachment : public ChunkDynamicEmbodiment, public MatrixLiaison,
	public Aligned
{
public:
	ChunkAttachment();
	ChunkAttachment( PyAttachmentPtr pAttachment );
	~ChunkAttachment();

	virtual void tick( float dTime );
	virtual void draw();
	virtual void toss( Chunk * pChunk );

	void enterSpace( ChunkSpacePtr pSpace, bool transient );
	void leaveSpace( bool transient );

	virtual void move( float dTime );

	virtual const Matrix & worldTransform() const	{ return worldTransform_; }
	
	virtual void localBoundingBox( BoundingBox & bb, bool skinny = false ) const;
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false ) const;
	virtual void localVisibilityBox( BoundingBox & vbb, bool skinny = false ) const;
	virtual void worldVisibilityBox( BoundingBox & vbb, const Matrix& world, bool skinny = false ) const;

	virtual void drawBoundingBoxes( const BoundingBox &bb, const BoundingBox &vbb, const Matrix &spaceTrans ) const {}

	virtual const Matrix & getMatrix() const		{ return worldTransform_; }
	virtual bool setMatrix( const Matrix & m );

	PyAttachment * pAttachment() const	{ return (PyAttachment*)&*pPyObject_; }

	static int convert( PyObject * pObj, ChunkEmbodimentPtr & pNew,
		const char * varName );

	virtual bool addYBounds( BoundingBox & bb ) const;

	virtual bool reflectionVisible() { return true; }

private:
	bool			needsSync_;
	Matrix			worldTransform_;
	bool			inited_;

};



#endif // CHUNK_ATTACHMENT_HPP
