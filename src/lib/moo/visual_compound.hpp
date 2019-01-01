/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_COMPOUND_HPP
#define VISUAL_COMPOUND_HPP

#include "moo/device_callback.hpp"
#include "moo/primitive.hpp"
#include "moo/vertex_declaration.hpp"
#include "moo/effect_material.hpp"
#include "math/boundbox.hpp"
#include "cstdmf/bgtask_manager.hpp"

namespace Moo
{

struct VertexXYZNUVTBPC;


class TransformHolder;
class EffectLightingSetter;

/**
 *	This class implements the visual compound, the visual compound tries to 
 *	combine visual instances into one object to limit the number
 *	of draw calls made by the engine. Each visual compound handles the instances
 *	for one unique visual.
 *
 */
class VisualCompound : public DeviceCallback, public BackgroundTask
{
public:
	VisualCompound();
	~VisualCompound();

	bool init( const std::string& visualResourceName );
	const std::string& resourceName() { return resourceName_; }

	TransformHolder* addTransform( const Matrix& transform, uint32 batchCookie );

	bool draw( EffectMaterialPtr pMaterialOverride = NULL );
	void updateDrawCookie();

	bool valid() const { return valid_; };

	void deleteManagedObjects();

	SimpleMutex& mutex() { return mutex_; }

	typedef std::vector< TransformHolder* > TransformHolders;

	/**
	 *	This class implements the batches used by the visual compound
	 *	a batch is a grouping of objects contained in an area, i.e. in 
	 *	a chunk.
	 */
	class Batch
	{
	public:
		Batch( VisualCompound* pVisualCompound ) : 
		  preloaded_( false ),
		  drawCookie_( -1 ),
		  pVisualCompound_( pVisualCompound ),
		  pLightContainer_( new LightContainer ),
		  pSpecLContainer_( new LightContainer ),
		  boundingBox_(BoundingBox::s_insideOut_)
		  {}
		~Batch();
		void draw( uint32 sequence );
		void clearSequences();
		LightContainerPtr pLightContainer()	{ return pLightContainer_; }
		LightContainerPtr pSpecLContainer()	{ return pSpecLContainer_; }
		uint32 drawCookie() const { return drawCookie_; }
		void drawCookie(uint32 cookie) { drawCookie_ = cookie; }
		TransformHolder* add( const Matrix& transform );
		void del( TransformHolder* transformHolder );


		typedef std::vector< PrimitiveGroup > PrimitiveGroups;

		const PrimitiveGroups& primitiveGroups() const { return primitiveGroups_; }
		void primitiveGroups(const PrimitiveGroups& pgs) { primitiveGroups_ = pgs; }

		VertexBuffer& vertexBuffer() { return vertexBuffer_; }
		void vertexBuffer(const VertexBuffer& vb) { vertexBuffer_ = vb; }
		IndexBuffer& indexBuffer() { return indexBuffer_; }
		void indexBuffer(const IndexBuffer& ib) { indexBuffer_ = ib; }

		const BoundingBox& boundingBox() const { return boundingBox_; }
		void boundingBox( const BoundingBox& box ) { boundingBox_ = box; }

		typedef std::vector<uint8> Sequences;
		const Sequences& sequences() { return sequences_; }

		const TransformHolders& transformHolders() const { return transformHolders_; }

		void invalidate();
		
		bool preloaded() { return preloaded_; }
		void preload();
	private:
		bool	preloaded_;
		PrimitiveGroups primitiveGroups_;
		TransformHolders transformHolders_;
		uint32	drawCookie_;
		Sequences sequences_;
		VertexBuffer vertexBuffer_;
		IndexBuffer indexBuffer_;
		VisualCompound* pVisualCompound_;
		LightContainerPtr pLightContainer_;
		LightContainerPtr pSpecLContainer_;
		BoundingBox	boundingBox_;
	};

	void dirty( Batch* pBatch );

	static VisualCompound* get( const std::string& visualName );
	static TransformHolder* add( const std::string& visualName, const Matrix& transform, uint32 batchCookie );
	static void drawAll( EffectMaterialPtr pMaterialOverride = NULL );
	static void fini();

	static void grabDelMutex();
	static void giveDelMutex();

	static void disable( bool val ) { disable_ = val; }
	static bool disable( ) { return disable_; }
	typedef std::vector< VertexXYZNUVTBPC > VerticesHolder;
private:
	VerticesHolder		sourceVerts_;
	IndicesHolder		sourceIndices_;
	uint32				nSourceVerts_;
	uint32				nSourceIndices_;
	std::vector< PrimitiveGroup > sourcePrimitiveGroups_;
	BoundingBox			sourceBB_;
	BoundingBox			bb_;

	VertexDeclaration*	pDecl_;

	bool loadVertices( BinaryPtr pVertexData );
	bool loadIndices( BinaryPtr pIndexData );
	bool loadMaterials( DataSectionPtr pGeometrySection );

	bool wantsDraw();

	void doBackgroundTask( BgTaskManager & mgr );
	bool update();
	Batch* getNextDirtyBatch();

	typedef std::map< uint32, Batch* > BatchMap;
	BatchMap		batchMap_;

	bool valid_;

	std::vector< EffectMaterialPtr > materials_;
	std::vector< EffectLightingSetter* > lightingSetters_;

	typedef std::vector<Batch*> BatchVector;
	BatchVector renderBatches_;
	BatchVector dirtyBatches_;

	uint32		drawCookie_;
	uint32		updateCookie_;

	void		invalidate();

	std::string resourceName_;

	bool taskAdded_;

	SimpleMutex	mutex_;

	static bool disable_;

	VisualCompound( const VisualCompound& );
	VisualCompound& operator=( const VisualCompound& );
};

/**
 * This class holds the transform for a single object in a visual compound.
 */
class TransformHolder : public Aligned
{
	public:
		TransformHolder( const Matrix& transform, VisualCompound::Batch* pBatch, uint32 sequence ) : 
			transform_( transform ),
			pBatch_( pBatch ),
			sequence_( sequence ) {}
		VisualCompound::Batch* pBatch() const { return pBatch_; }
		void pBatch( VisualCompound::Batch* pBatch ) { pBatch_ = pBatch; }
		const Matrix& transform() { return transform_; }
		void draw() { pBatch_->draw( sequence_ ); }
		void del();
		uint32 sequence() const { return sequence_; }
		void sequence( uint32 val ) { sequence_ = val; }
	private:
        Matrix	transform_;
		uint32 sequence_;
		VisualCompound::Batch*	pBatch_;
};

}


#endif // VISUAL_COMPOUND_HPP
