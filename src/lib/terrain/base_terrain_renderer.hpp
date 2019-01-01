/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef TERRAIN_BASE_TERRAIN_RENDERER_HPP
#define TERRAIN_BASE_TERRAIN_RENDERER_HPP


#include "moo/forward_declarations.hpp"
#include "moo/effect_material.hpp"

namespace Terrain
{

class EffectMaterial;
class BaseRenderTerrainBlock;

class BaseTerrainRenderer : public SafeReferenceCount
{
public:
	/**
	 *  This method sets the current terrain renderer instance. At the moment,
	 *  TerrainRenderer1 and TerrainRenderer2 call this method when they are
	 *  inited to set themselves as the current renderer.
	 *
	 *  @param newInstance      pointer to the current terrain renderer, or 
	 *                          NULL to disable terrain rendering.
	 */
	static void instance( BaseTerrainRenderer* newInstance );

	/**
	 *  Singleton instance get method.
	 *
	 *  @return          instance of the current terrain renderer. If no
	 *                   terrain renderer has been set, it returs a pointer
	 *                   to a dummy renderer that does nothing.
	 */
	static BaseTerrainRenderer* instance();
	
	/**
	 *  Get the version of the terrain renderer.
	 *
	 *  @return          terrain renderer version, 0 if no terrain renderer
	 *                   instance has been set.
	 */
	virtual uint32 version() = 0;

	/**
	*	This function adds a block for drawing.
	*
	*	@param pBlock			The block to render
	*	@param transform		The transformation matrix to apply when drawing.
	*/
	virtual void addBlock(	BaseRenderTerrainBlock*	pBlock,	
							const Matrix&			transform) = 0;

	/**
	 *  This function draws the list of terrain blocks.
	 *
	 *  @param pOverride    Override material to use for each block.
	 *	@param clearList	Clear the list of blocks to render.
	 */
	virtual void drawAll(	Moo::EffectMaterialPtr pOverride = NULL, 
							bool clearList = true ) = 0;

	/**
	*	This function asks the renderer to immediately draw the terrain
	*	block. The block is not added to the render list. The caller must 
	*	maintain render context transform state. By default the cached
	*	lighting is not used - the caller must set lighting before call.
	*
	*	@param pBlock				The block to render.
	*	@param transform			World transform for block.
	*	@param altMat				An alternative drawing material.
	*	@param useCachedLighting	Use cached lighting state or renderer 
	*								lighting state. Used by Terrain2 only.
	*/
	virtual void drawSingle(BaseRenderTerrainBlock*	pBlock,	
							const Matrix&			transform, 
							Moo::EffectMaterialPtr	altMat = NULL,
							bool					useCachedLighting = false ) 
							= 0;

	/**
	 *  Clear the list of terrain blocks to be renderered.
	 */
	virtual void clearBlocks() = 0;

	/**
	 *  Method to find out if the terrain should be drawn or not.
	 *
	 *  @return          true if the terrain should be drawn, false otherwise.
	 */
	virtual bool canSeeTerrain() const = 0;

	/**
	 *  Tell the renderer that it should/shouldn't use specular lighting.
	 *  This method is used by the chunk photographer class in WE.
	 *
	 *  @param          true to use specular in the terrain, false otherwise.
	 */
	void enableSpecular(bool enable) { specularEnabled_ = enable; }

	/**
	 *  Method to find out if the terrain should be drawn with specular
	 *  highlights or not. This method is used by the chunk photographer class
	 *  in WE.
	 *
	 *  @return          true if using specular in the terrain, false otherwise.
	 */
	bool specularEnabled() const { return specularEnabled_; }

	/**
	 *	This method sets the visibility flag for the renderer.
	 */
	void isVisible( bool value ) { isVisible_ = value; }

	/**
	*	This method reads a flag that indicates whether or not hole map is used 
	*	during terrain texture rendering.
	*/
	bool holeMapEnabled() const { return holeMapEnabled_ ; }
	bool enableHoleMap( bool value ) ;

protected:
	BaseTerrainRenderer() :
		isVisible_( false ),
		specularEnabled_( true ),
		holeMapEnabled_( true )	   
	{}

	bool isVisible_;

private:
	bool specularEnabled_;
	bool holeMapEnabled_;
};

typedef SmartPointer<BaseTerrainRenderer> BaseTerrainRendererPtr;

}; // namespace Terrain

#endif // TERRAIN_BASE_TERRAIN_RENDERER_HPP