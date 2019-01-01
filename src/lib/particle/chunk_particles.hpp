/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_PARTICLES_HPP
#define CHUNK_PARTICLES_HPP

#include "chunk/chunk_item.hpp"
#include "math/matrix_liason.hpp"

class MetaParticleSystem;
typedef SmartPointer<MetaParticleSystem> MetaParticleSystemPtr;

/**
 *	This class is a chunk item that tends a particle system.
 */
class ChunkParticles : public ChunkItem, public MatrixLiaison
{
public:
	ChunkParticles();
	~ChunkParticles();

	bool load( DataSectionPtr pSection );

	virtual void load( const std::string& resourceName );

	virtual void draw();

	virtual void toss( Chunk * pChunk );
	virtual void tick( float dTime );

	virtual void drawBoundingBoxes( const BoundingBox &bb, const BoundingBox &vbb, const Matrix &spaceTrans ) const;

	virtual const char * label() const;

	bool getReflectionVis() const { return reflectionVisible_; }
	bool setReflectionVis( const bool& reflVis ) { reflectionVisible_=reflVis; return true; }

	virtual const Matrix & getMatrix() const	{ return worldTransform_; }
	virtual bool setMatrix( const Matrix & m )	{ worldTransform_ = m; return true; }

	virtual bool reflectionVisible() { return reflectionVisible_; }
protected:
	virtual bool addYBounds( BoundingBox& bb ) const;

	std::string				resourceID_;
	DataSectionPtr			resourceDS_;
	Matrix					localTransform_;
	Matrix					worldTransform_;
	MetaParticleSystemPtr	system_;
	uint32					staggerIdx_;
	float					seedTime_;

private:
	static ChunkItemFactory::Result create( Chunk * pChunk, DataSectionPtr pSection );
	static ChunkItemFactory	factory_;

	bool			reflectionVisible_;
};


#endif // CHUNK_PARTICLES_HPP
