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
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_view.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/editor/chunk_editor.hpp"
#include "worldeditor/world/world_manager.hpp"


// -----------------------------------------------------------------------------
// Section: ChunkItemEditor
// -----------------------------------------------------------------------------

// declare attributes deferred until chunk item was included
#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE ChunkItemEditor::

PY_RO_ATTRIBUTE_DECLARE( pItem_->edClassName(), className )
PY_RO_ATTRIBUTE_DECLARE( pItem_->edDescription(), description )
PyObject * ChunkItemEditor::pyGet_transform()
{
	BW_GUARD;

	Matrix m = pItem_->chunk()->transform();
	m.preMultiply( pItem_->edTransform() );
	return Script::getData( m );
}
PY_RO_ATTRIBUTE_SET( transform )

// and make the standard python stuff
PY_TYPEOBJECT( ChunkItemEditor )

PY_BEGIN_METHODS( ChunkItemEditor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ChunkItemEditor )
	PY_ATTRIBUTE( className )
	PY_ATTRIBUTE( description )
	PY_ATTRIBUTE( transform )
PY_END_ATTRIBUTES()

PY_FACTORY( ChunkItemEditor, WorldEditor )


/**
 *	Constructor.
 */
ChunkItemEditor::ChunkItemEditor( ChunkItemPtr pItem, PyTypePlus * pType ) :
	GeneralEditor( pType ),
	pItem_( pItem )
{
	BW_GUARD;

	// notify the item's name
	lastItemName_ = Localise(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/ITEM_EDITOR/LAST_ITEM_NAME",
		pItem_->edDescription() );

	this->addProperty( new StaticTextProperty( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/PROPERTIES/ITEM_EDITOR/CHUNK"), 
		new ConstantChunkNameProxy<ChunkItem>( pItem_ ) ) );

	pItem->edEdit( *this );

	constructorOver_ = true;
}

/**
 *	Destructor.
 */
ChunkItemEditor::~ChunkItemEditor()
{
}


/**
 *	Get an attribute for python
 */
PyObject * ChunkItemEditor::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	return GeneralEditor::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int ChunkItemEditor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return GeneralEditor::pySetAttribute( attr, value );
}

/**
 *	Static python factory method
 */
PyObject * ChunkItemEditor::pyNew( PyObject * args )
{
	BW_GUARD;

	// parse arguments
	PyObject * pPyRev = NULL;
	int alwaysChunkItem = 0;
	if (!PyArg_ParseTuple( args, "O|i", &pPyRev, &alwaysChunkItem ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_TypeError, "ChunkItemEditor() "
			"expects a Revealer argument and optionally alwaysChunkItem flag" );
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
		PyErr_Format( PyExc_ValueError, "ChunkItemEditor() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	// make sure it hasn't already been 'deleted'
	ChunkItemPtr pItem = items[0];
	if (pItem->chunk() == NULL)
	{
		PyErr_Format( PyExc_ValueError, "ChunkItemEditor() "
			"Item must be in the scene" );
		return NULL;
	}

	// ok, that's it then
	return new ChunkItemEditor( pItem );
	*/


	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	// Check items
	for (uint i = 0; i < items.size(); i++)
	{
		if (items[i]->chunk() == NULL)
		{
			PyErr_Format( PyExc_ValueError, "ChunkItemEditor() "
				"Item must be in the scene" );
			return NULL;
		}
	}

	if (items.size() == 1)
	{
		if (items[0]->isShellModel() && alwaysChunkItem == 0)
			return new ChunkEditor( items[0]->chunk() );
		else
			return new ChunkItemEditor( items[0] );
	}
	else
	{
		PyObject *t;
		t = PyTuple_New( items.size() );
		for (uint i = 0; i < items.size(); i++)
		{
			GeneralEditor* ge;

			if (items[i]->isShellModel() && alwaysChunkItem == 0)
				ge = new ChunkEditor( items[i]->chunk() );
			else
				ge = new ChunkItemEditor( items[i] );

			PyTuple_SetItem( t, i, ge );
		}
		return t;
	}
}



// item_editor.cpp
