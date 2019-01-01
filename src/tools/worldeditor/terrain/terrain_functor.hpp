/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_FUNCTOR_HPP
#define TERRAIN_FUNCTOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/terrain/mouse_drag_handler.hpp"
#include "gizmo/tool_functor.hpp"
#include <set>


/**
 *	This is a base class for ToolFunctors that operate on terrain (texture
 *	layers, heights or holes).
 */
class TerrainFunctor : public ToolFunctor
{
public:
	explicit TerrainFunctor( PyTypePlus * pType = &s_type_ );

	virtual bool handleKeyEvent( const KeyEvent & keyEvent, Tool & tool );
	virtual bool handleMouseEvent( const MouseEvent & keyEvent, Tool & tool );
	virtual bool handleContextMenu( Tool & tool );

protected:
	void apply( Tool & t, bool allVerts = false );

	virtual void getBlockFormat( 
		const EditorChunkTerrain &		chunkTerrain,
		TerrainUtils::TerrainFormat	&	format ) const = 0;

	virtual void onFirstApply( EditorChunkTerrain & chunkTerrain ) = 0;

	virtual void onBeginApply( EditorChunkTerrain & chunkTerrain );

	virtual void applyToSubBlock(
		EditorChunkTerrain &				chunkTerrain,
		const Vector3 &						toolEffset, 
		const Vector3 &						chunkOffset,
		const TerrainUtils::TerrainFormat &	format,
		int32								minx, 
		int32								minz, 
		int32								maxx, 
		int32								maxz ) = 0;

	virtual void onEndApply( EditorChunkTerrain & chunkTerrain );

	virtual void onApplied( Tool & t ) = 0;

	virtual void onLastApply( EditorChunkTerrain & chunkTerrain ) = 0;	

	virtual bool showWaitCursorOnLastApply() const;

	void beginApplying();
	bool newTouchedChunk( EditorChunkTerrain * pChunkTerrain );
	void endApplying();	

	MouseDragHandler & dragHandler();

private:
	typedef std::set< EditorChunkTerrain* > ECTSet;

	ECTSet				touchedChunks_;
	MouseDragHandler	dragHandler_;
};

#endif // TERRAIN_FUNCTOR_HPP
