/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "terrain_chunk_texture_tool_view.hpp"

#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "worldeditor/terrain/editor_chunk_terrain_projector.hpp"

#include "chunk/base_chunk_space.hpp"


#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE TerrainChunkTextureToolView::


PY_TYPEOBJECT( TerrainChunkTextureToolView )

PY_BEGIN_METHODS( TerrainChunkTextureToolView )

PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( TerrainChunkTextureToolView )
	PY_ATTRIBUTE( numPerChunk )
PY_END_ATTRIBUTES()


PY_FACTORY_NAMED( TerrainChunkTextureToolView, "TerrainChunkTextureToolView", View )


VIEW_FACTORY( TerrainChunkTextureToolView )


/**
 *	Constructor.
 *
 *	@param resourceID		The id of the texture to show.
 *	@param pType			The Python type.
 */
TerrainChunkTextureToolView::TerrainChunkTextureToolView(
	const std::string &		resourceID,
	PyTypePlus *			pType 
):
	TextureToolView( resourceID, pType ),
	numPerChunk_( 1.f )
{
}


/**
 *	This method renders the terrain texture chunk tool.  It does
 *	this by projecting a texture onto the terrain, making sure
 *	the entire chunk the tool is in is covered, even if the tool
 *	does not fill an entire chunk.
 *
 *	Similarly, if the tool overlaps multiple chunks, then those
 *	chunks will have the texture overlayed.
 *
 *	Some tool locators don't bother filling out the set of relevant
 *	chunks, for example the LineLocator.  If this is the case, then
 *	we just draw on the chunk the tool is in, using currentChunk()
 *
 *	@param	tool	The tool we are viewing.
 */
void TerrainChunkTextureToolView::render( const Tool & tool )
{
	BW_GUARD;

	EditorChunkTerrainPtrs spChunks;

	if (tool.relevantChunks().size())
	{

		ChunkPtrVector::const_iterator it  = tool.relevantChunks().begin();
		ChunkPtrVector::const_iterator end = tool.relevantChunks().end  ();

		while (it != end)
		{
			Chunk * pChunk = *it++;
			
			EditorChunkTerrain * pChunkTerrain = 
				static_cast< EditorChunkTerrain * >(
					ChunkTerrainCache::instance( *pChunk ).pTerrain());

			if (pChunkTerrain != NULL)
			{
				spChunks.push_back( pChunkTerrain );

				float scale = GRID_RESOLUTION / numPerChunk_;               
                // Have to offset by 0.5f to properly align the texture when numPerChunk_ is even
                float scaleOffset = ( numPerChunk_ / 2.0f - int( numPerChunk_ / 2.0f ) + 0.5f ) * scale;

				EditorChunkTerrainProjector::instance().projectTexture(
					pTexture_,
					GRID_RESOLUTION / numPerChunk_,
					0.f,				// Rotation		
					pChunk->transform().applyToOrigin() +
						Vector3( GRID_RESOLUTION/2.0f + scaleOffset, 0.f, GRID_RESOLUTION/2.0f  + scaleOffset),
					D3DTADDRESS_WRAP,	// Wrapping mode
					spChunks,			// Affected chunks
					false );			// Don't show holes

				spChunks.clear();
			}
		}
	}
	else if ( tool.currentChunk() )
	{
		Chunk * pChunk = tool.currentChunk();

		EditorChunkTerrain * pChunkTerrain = 
			static_cast< EditorChunkTerrain* >(
				ChunkTerrainCache::instance( *pChunk ).pTerrain());

		if (pChunkTerrain != NULL)
		{
			spChunks.push_back( pChunkTerrain );

			EditorChunkTerrainProjector::instance().projectTexture(
				pTexture_,
				GRID_RESOLUTION / numPerChunk_,
				0.f,				// Rotation
				pChunk->transform().applyToOrigin() +
					Vector3( GRID_RESOLUTION/2.0f, 0.f, GRID_RESOLUTION/2.0f ),
				D3DTADDRESS_BORDER,	// Wrapping mode
				spChunks,			// Affected chunks
				false );			// Don't show holes
		}
	}
}


/**
 *	This sets the number of textures shown per chunk.
 *
 *	@param num		The number of textures to show per chunk.
 */
void TerrainChunkTextureToolView::numPerChunk( float num )	
{ 
	numPerChunk_ = num; 
}


/**
 *	Static python factory method
 */
PyObject * TerrainChunkTextureToolView::pyNew( PyObject * pArgs )
{
	BW_GUARD;

	char * pTextureName = NULL;
	if (!PyArg_ParseTuple( pArgs, "|s", &pTextureName ))
	{
		PyErr_SetString( PyExc_TypeError, "View.TerrainChunkTextureToolView: "
			"Argument parsing error: Expected an optional texture name" );
		return NULL;
	}

	if (pTextureName != NULL)
	{
		return new TerrainChunkTextureToolView( pTextureName );
	}
	else
	{
		return new TerrainChunkTextureToolView( "resources/maps/gizmo/square.dds" );
	}
}


/**
 *	This gets an attribute for python.
 *
 *	@param attr		The attribute to get.
 *	@return			The attribute.
 */
PyObject * TerrainChunkTextureToolView::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	return TextureToolView::pyGetAttribute( attr );
}


/**
 *	This set an attribute for python.
 *
 *	@param attr		The attribute to set.
 *	@param value	The new value of the attribute.
 */
int TerrainChunkTextureToolView::pySetAttribute( 
	const char *	attr, 
	PyObject *		value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return TextureToolView::pySetAttribute( attr, value );
}
