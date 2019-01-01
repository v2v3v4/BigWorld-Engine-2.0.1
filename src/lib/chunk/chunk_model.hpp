/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_MODEL_HPP
#define CHUNK_MODEL_HPP

#include "chunk_item.hpp"
#include "borrowed_light_combiner.hpp"
#include "model/super_model.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/guard.hpp"

#include "model/material_override.hpp"
#include "moo/moo_math.hpp"
#include "moo/animation.hpp"

#include "fmodsound/sound_occluder.hpp"

#include "umbra_config.hpp"

typedef SmartPointer<class SuperModelAnimation> SuperModelAnimationPtr;
typedef std::vector< SmartPointer< class Fashion > > FashionVector;

class ModelCompoundInstance;
typedef SmartPointer<ModelCompoundInstance> ModelCompoundInstancePtr;

/**
 *	This class represents a material override from the chunk perspective
 */
class ChunkMaterial : public Fashion
{
public:
	ChunkMaterial( Moo::EffectMaterialPtr material ) : material_( material ) {}
	Moo::EffectMaterialPtr material_;

private:
	MaterialOverride override_;

	~ChunkMaterial();
	virtual void dress( SuperModel & superModel );
	virtual void undress( SuperModel & superModel );
};

typedef SmartPointer<ChunkMaterial> ChunkMaterialPtr;

/**
 *	This class defines a chunk item inhabitant that
 *	displays a super model ( a .model file ).
 *
 *	Additionally, you can specify a single animation
 *	that is to play, by specifying an &lt;animation&gt; tag
 *	and a &lt;name&gt; sub-tag.
 */
class ChunkModel : public ChunkItem, public Aligned
{
	DECLARE_CHUNK_ITEM( ChunkModel )
	DECLARE_CHUNK_ITEM_ALIAS( ChunkModel, shell )
public:
	ChunkModel();
	~ChunkModel();

	bool load( DataSectionPtr pSection, Chunk * pChunk = NULL );

	virtual void toss( Chunk * pChunk );
	void toss( Chunk * pChunk, SuperModel* extraModel );
	
	virtual void draw();
	virtual void lend( Chunk * pLender );

	virtual const char * label() const;

	virtual bool reflectionVisible() { return reflectionVisible_; }

	bool getReflectionVis() const { return reflectionVisible_; }
	bool setReflectionVis( const bool& reflVis ) { reflectionVisible_=reflVis; return true; }
	BoundingBox localBB() const
	{
		BoundingBox bb;
		if( pSuperModel_ )
			pSuperModel_->localBoundingBox( bb );
		return bb;
	}
protected:
	class SuperModel			* pSuperModel_;

	SuperModelAnimationPtr		pAnimation_;

	uint32						tickMark_;
	uint64						lastTickTimeInMS_;

	std::map<std::string,SuperModelDyePtr> tintMap_;
	std::vector<ChunkMaterialPtr> materialOverride_;

	float						animRateMultiplier_;

	FashionVector				fv_;

	Matrix						transform_;

	std::string					label_;

	ModelCompoundInstancePtr	pModelCompound_;

	bool						reflectionVisible_;

	BorrowedLightCombiner		borrowedLightCombiner_;

	mutable bool				calculateIsShellModel_;
	mutable bool				cachedIsShellModel_;

	virtual void				syncInit();

#if UMBRA_ENABLE
	bool						umbraOccluder_;
	std::string					umbraModelName_;
#endif

#if FMOD_SUPPORT
    SoundOccluder soundOccluder_;
#endif

	/**
	 * Add a fashion for static lighting, loading the values from resName
	 *
	 * This is virtual so EditorChunkModel can do it it's own way
	 */
	virtual void addStaticLighting( DataSectionPtr ds, Chunk* pChunk  );

	virtual bool addYBounds( BoundingBox& bb ) const;

	bool isShellModel( const DataSectionPtr pSection ) const;

	void tickAnimation();
};

#endif CHUNK_MODEL_HPP
