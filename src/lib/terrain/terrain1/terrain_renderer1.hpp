/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_RENDERER1_HPP
#define TERRAIN_TERRAIN_RENDERER1_HPP

#include "../base_terrain_renderer.hpp"

namespace Moo
{
	class VertexDeclaration;	
}

namespace Terrain
{

class EffectFileTextureSetter;
class TerrainBlock1;
typedef SmartPointer<TerrainBlock1> TerrainBlock1Ptr;

/**
 *	This singleton class is the terrain renderer class, it contains the
 *	methods for rendering the old terrain blocks
 */
class TerrainRenderer1 : public BaseTerrainRenderer
{
public:
	~TerrainRenderer1();

	static TerrainRenderer1* instance();

	static bool init();
	
	// BaseTerrainRenderer interface implementation
	uint32 version() { return 100; };

	void addBlock(	BaseRenderTerrainBlock*	pBlock,	
					const Matrix&			transform );
	void drawAll(	Moo::EffectMaterialPtr pOverride = NULL, bool clearList = true );
	void drawSingle(BaseRenderTerrainBlock*	pBlock,	
					const Matrix&			transform, 
					Moo::EffectMaterialPtr	altMat = NULL,
					bool					useCachedLighting = false );
	void clearBlocks();

	bool canSeeTerrain() const { return isVisible_; }

private:
	bool initInternal();

	TerrainRenderer1();

	static TerrainRenderer1*	s_instance_;

	typedef std::pair<Matrix, TerrainBlock1 *>		BlockInPlace;
	typedef AVectorNoDestructor< BlockInPlace >		BlockVector;	

	class Renderer : public ReferenceCount
	{
	public:
		virtual ~Renderer() {};
		virtual void draw( BlockVector& blocks, Moo::EffectMaterialPtr alternateMaterial=NULL ) = 0;
	};

	class EffectFileRenderer : public Renderer
	{
	public:
		EffectFileRenderer();
		~EffectFileRenderer();

		void			draw( BlockVector& blocks, Moo::EffectMaterialPtr alternateEffect=NULL );		
		bool			init();

	protected:
		void			setInitialRenderStates();
		void			sortBlocks( BlockVector& blocks );
		void			renderBlocks( BlockVector& blocks );
		void			textureTransform( const Matrix& world, Matrix& ret ) const;

	private:
		Moo::VertexDeclaration*		pDecl_;
		Moo::EffectMaterialPtr		material_;
		EffectFileTextureSetter*	textureSetter_;

		Moo::EffectConstantValuePtr sunAngleSetter_;
		Moo::EffectConstantValuePtr penumbraSizeSetter_;
		Moo::EffectConstantValuePtr terrainTextureTransformSetter_;
		Moo::EffectConstantValuePtr fogTextureTransformSetter_;
		Moo::EffectConstantValuePtr fogGradientTextureSetter_;
	};	

	BlockVector blocks_;
	SmartPointer<Renderer> renderer_;		
};

};

#endif // TERRAIN_TERRAIN_RENDERER1_HPP
