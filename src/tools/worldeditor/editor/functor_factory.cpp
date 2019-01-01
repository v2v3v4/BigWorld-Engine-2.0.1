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
#include "worldeditor/editor/item_view.hpp"
#include "gizmo/item_functor.hpp"
#include "gizmo/general_properties.hpp"
#include "chunk/chunk_item.hpp"


DECLARE_DEBUG_COMPONENT2( "Editor", 0 )


// -----------------------------------------------------------------------------
// Section: MatrixScaler
// -----------------------------------------------------------------------------

PY_FACTORY( MatrixScaler, Functor )

/**
 *	Factory method
 */
PyObject * MatrixScaler::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject * pPyRev = NULL;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_TypeError, "MatrixScaler() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "MatrixScaler() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	ChunkItemPtr pItem = items[0];
	if (pItem->chunk() == NULL)
	{
		PyErr_Format( PyExc_ValueError, "MatrixScaler() "
			"Item to move is not in the scene" );
		return NULL;
	}

	return new MatrixScaler( MatrixProxy::getChunkItemDefault( pItem ) );
}

// -----------------------------------------------------------------------------
// Section: MatrixRotator
// -----------------------------------------------------------------------------

PY_FACTORY( MatrixRotator, Functor )

/**
 *	Factory method
 */
PyObject * MatrixRotator::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject * pPyRev = NULL;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_TypeError, "MatrixRotator() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "MatrixRotator() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	ChunkItemPtr pItem = items[0];
	if (pItem->chunk() == NULL)
	{
		PyErr_Format( PyExc_ValueError, "MatrixRotator() "
			"Item to move is not in the scene" );
		return NULL;
	}

	return new MatrixRotator( MatrixProxy::getChunkItemDefault( pItem ) );
}


// -----------------------------------------------------------------------------
// Section: DynamicFloatDevice
// -----------------------------------------------------------------------------

PY_FACTORY( DynamicFloatDevice, Functor )

/**
 *	Factory method
 */
PyObject * DynamicFloatDevice::pyNew( PyObject * args )
{
	/*PyObject * pPyRev = NULL;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_TypeError, "DynamicFloatDevice() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "DynamicFloatDevice() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	ChunkItemPtr pItem = items[0];
	if (pItem->chunk() == NULL)
	{
		PyErr_Format( PyExc_ValueError, "DynamicFloatDevice() "
			"Item to set radius is not in the scene" );
		return NULL;
	}

	return new DynamicFloatDevice(
		MatrixProxy::getChunkItemDefault( pItem ),
		FloatProxy::getChunkItemDefault( pItem ) );*/
	return NULL;
}




// -----------------------------------------------------------------------------
// Section: TeeFunctor
// -----------------------------------------------------------------------------

class TeeFunctor : public ToolFunctor
{
	Py_Header( TeeFunctor, ToolFunctor )

private:
	ToolFunctor* f1_;
	ToolFunctor* f2_;
	KeyCode::Key altKey_;

	ToolFunctor* activeFunctor() const {
		BW_GUARD;

		if (InputDevices::isKeyDown( altKey_ ))
			return f2_;
		else
			return f1_;
	}

public:
	TeeFunctor( ToolFunctor* f1, ToolFunctor* f2, KeyCode::Key altKey, PyTypePlus * pType = &s_type_ )
		: f1_(f1), f2_(f2), altKey_(altKey) {}

	virtual void update( float dTime, Tool& tool ) {
		BW_GUARD;

		activeFunctor()->update( dTime, tool );
	}
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool ) {
		BW_GUARD;

		return activeFunctor()->handleKeyEvent( event, tool );
	}
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool ) {
		BW_GUARD;

		return activeFunctor()->handleMouseEvent( event, tool );
	}
	virtual bool applying() const {
		BW_GUARD;

		return activeFunctor()->applying();
	}

	PY_FACTORY_DECLARE()
};

PY_TYPEOBJECT( TeeFunctor )

PY_BEGIN_METHODS( TeeFunctor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( TeeFunctor )
PY_END_ATTRIBUTES()

PY_FACTORY( TeeFunctor, Functor )

PyObject * TeeFunctor::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject* f1;
	PyObject* f2;
	int i;
	if (!PyArg_ParseTuple( args, "OOi", &f1, &f2, &i ) ||
		!ToolFunctor::Check( f1 ) ||
		!ToolFunctor::Check( f2 ))
	{
		PyErr_SetString( PyExc_TypeError, "TeeFunctor() "
			"expects two ToolFunctors and a key" );
		return NULL;
	}

	return new TeeFunctor( 
		static_cast<ToolFunctor*>( f1 ),
		static_cast<ToolFunctor*>( f2 ),
		static_cast<KeyCode::Key>( i )
		);
}



// -----------------------------------------------------------------------------
// Section: PipeFunctor
// -----------------------------------------------------------------------------

class PipeFunctor : public ToolFunctor
{
	Py_Header( PipeFunctor, ToolFunctor )

private:
	ToolFunctor* f1_;
	ToolFunctor* f2_;

public:
	PipeFunctor( ToolFunctor* f1, ToolFunctor* f2, PyTypePlus * pType = &s_type_ )
		: f1_(f1), f2_(f2) {}

	virtual void update( float dTime, Tool& tool ) {
		BW_GUARD;

		f1_->update( dTime, tool );
		f2_->update( dTime, tool );
	}
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool ) {
		BW_GUARD;

		bool handled = f1_->handleKeyEvent( event, tool );
		if (!handled)
			handled = f2_->handleKeyEvent( event, tool );
		return handled;
	}
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool ) {
		BW_GUARD;

		bool handled = f1_->handleMouseEvent( event, tool );
		if (!handled)
			handled = f2_->handleMouseEvent( event, tool );
		return handled;
	}
	virtual bool applying() const {
		BW_GUARD;

		return f1_->applying() || f2_->applying();
	}

	PY_FACTORY_DECLARE()
};

PY_TYPEOBJECT( PipeFunctor )

PY_BEGIN_METHODS( PipeFunctor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PipeFunctor )
PY_END_ATTRIBUTES()

PY_FACTORY( PipeFunctor, Functor )

PyObject * PipeFunctor::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject* f1;
	PyObject* f2;
	
	if (!PyArg_ParseTuple( args, "OO", &f1, &f2 ) ||
		!ToolFunctor::Check( f1 ) ||
		!ToolFunctor::Check( f2 ))
	{
		PyErr_SetString( PyExc_TypeError, "TeeFunctor() "
			"expects two ToolFunctors" );
		return NULL;
	}

	return new PipeFunctor( 
		static_cast<ToolFunctor*>( f1 ),
		static_cast<ToolFunctor*>( f2 )
		);
}
