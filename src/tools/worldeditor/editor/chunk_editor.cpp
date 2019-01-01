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
#include "worldeditor/editor/chunk_editor.hpp"
#include "worldeditor/editor/item_view.hpp"
#include "worldeditor/world/editor_chunk.hpp"


// -----------------------------------------------------------------------------
// Section: ChunkEditor
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( ChunkEditor )

PY_BEGIN_METHODS( ChunkEditor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ChunkEditor )
	PY_ATTRIBUTE( contents )
	PY_ATTRIBUTE( isWriteable )
PY_END_ATTRIBUTES()

PY_FACTORY( ChunkEditor, WorldEditor )


/**
 *	Constructor.
 */
ChunkEditor::ChunkEditor( Chunk * pChunk, PyTypePlus * pType ) :
	GeneralEditor( pType ),
	pChunk_( pChunk )
{
	BW_GUARD;

	lastItemName_ = Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/CHUNK_EDITOR/CHUNK_IDENTIFIER", pChunk->identifier() );

	// ask the EditorChunkCache to take care of this for us
	EditorChunkCache::instance( *pChunk_ ).edEdit( *this );

	constructorOver_ = true;
}

/**
 *	Destructor.
 */
ChunkEditor::~ChunkEditor()
{
}


/**
 *	Get an attribute for python
 */
PyObject * ChunkEditor::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	return GeneralEditor::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int ChunkEditor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return GeneralEditor::pySetAttribute( attr, value );
}

/**
 *	Get all the items in this chunk as a group for Python
 */
PyObject * ChunkEditor::pyGet_contents()
{
	BW_GUARD;

	std::vector<ChunkItemPtr> items;
	EditorChunkCache::instance( *pChunk_ ).allItems( items );
	return new ChunkItemGroup( items );
}

/**
 *	Get whether or not this chunk is writable
 */
PyObject * ChunkEditor::pyGet_isWriteable()
{
	BW_GUARD;

	return Script::getData(
		EditorChunkCache::instance( *pChunk_ ).edIsWriteable() );
}

/**
 *	Static python factory method
 */
PyObject * ChunkEditor::pyNew( PyObject * args )
{
	BW_GUARD;

	// parse arguments
	PyObject * pPyRev = NULL;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_TypeError, "ChunkEditor() "
			"expects a Revealer argument" );
		return NULL;
	}

	ChunkItemRevealer * pRevealer =
		static_cast<ChunkItemRevealer*>( pPyRev );

	/*
	// make sure there's only one item
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "ChunkEditor() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	// make sure it hasn't already been 'deleted'
	ChunkItemPtr pItem = items[0];
	if (pItem->chunk() == NULL)
	{
		PyErr_Format( PyExc_ValueError, "ChunkEditor() "
			"Item must be in the scene" );
		return NULL;
	}

	// ok, create an editor for it's chunk then
	return new ChunkEditor( pItem->chunk() );
	*/

	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	// Check items
	for (uint i = 0; i < items.size(); i++)
	{
		if (items[i]->chunk() == NULL)
		{
			PyErr_Format( PyExc_ValueError, "ChunkEditor() "
				"Item must be in the scene" );
			return NULL;
		}
	}

	if (items.size() == 1)
	{
		return new ChunkEditor( items[0]->chunk() );
	}
	else
	{
		PyObject *t;
		t = PyTuple_New( items.size() );
		for (uint i = 0; i < items.size(); i++)
		{
			PyTuple_SetItem( t, i, new ChunkEditor( items[i]->chunk() ) );
		}
		return t;
	}
}

// chunk_editor.cpp
