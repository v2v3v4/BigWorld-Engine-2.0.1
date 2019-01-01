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

#include "tool_functor.hpp"
#include "tool.hpp"
#include "input/input.hpp"
#include "input/py_input.hpp"


//----------------------------------------------------
//	Section : ToolFunctor ( Base class )
//----------------------------------------------------

/// static factory map initialiser
template<> FunctorFactory::ObjectMap * FunctorFactory::pMap_;

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE ToolFunctor::

PY_TYPEOBJECT( ToolFunctor )

PY_BEGIN_METHODS( ToolFunctor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ToolFunctor )
PY_END_ATTRIBUTES()

ToolFunctor::ToolFunctor( PyTypePlus * pType ):
	PyObjectPlus( pType )
{
}


ToolFunctor::~ToolFunctor()
{
}



void ToolFunctor::update( float dTime, Tool& tool ) 
{ 
}


bool ToolFunctor::handleContextMenu( Tool& tool ) 
{ 
	return false; 
};


bool ToolFunctor::applying() const 
{ 
	return false; 
}


void ToolFunctor::onBeginUsing(Tool &tool)
{
}


void ToolFunctor::onEndUsing(Tool &tool)
{
}


PY_SCRIPT_CONVERTERS( ToolFunctor )


//----------------------------------------------------
//	Section : ScriptedToolFunctor
//----------------------------------------------------

//#undef PY_ATTR_SCOPE
//#define PY_ATTR_SCOPE ScriptedToolFunctor::

PY_TYPEOBJECT( ScriptedToolFunctor )

PY_BEGIN_METHODS( ScriptedToolFunctor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ScriptedToolFunctor )
	PY_ATTRIBUTE( script )
PY_END_ATTRIBUTES()

PY_FACTORY_NAMED( ScriptedToolFunctor, "ScriptedFunctor", Functor )

/// declare our C++ factory method
FUNCTOR_FACTORY( ScriptedToolFunctor )

ScriptedToolFunctor::ScriptedToolFunctor( PyTypePlus * pType ):
	ToolFunctor( pType ),
	pScriptObject_( NULL )
{
}


void ScriptedToolFunctor::update( float dTime, Tool& tool )
{
	BW_GUARD;

	if (pScriptObject_)
	{
		Script::call(
			PyObject_GetAttrString( pScriptObject_.getObject(), "update" ),
			Py_BuildValue( "(fO)", dTime, static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::update",
			true );
	}
}



/**
 *	This method handles key events for the ScriptedToolFunctor.
 *	It does this by delegating to our script object.
 *
 *	@param event	the key event to process.
 *	@param tool		the tool to use.
 */
bool ScriptedToolFunctor::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	bool handled = false;
	if (pScriptObject_)
	{
		PyObject * pResult = Script::ask(
			PyObject_GetAttrString( pScriptObject_.getObject(), "onKeyEvent" ),
			Py_BuildValue( "(OO)",
				&*SmartPointer<PyObject>( Script::getData( event ), true ),
				static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::handleKeyEvent",
			true );

		Script::setAnswer( pResult, handled,
			"ScriptedToolFunctor::handleKeyEvent" );
	}

	return handled;
}


/**
 *	This method handles mouse events for the ScriptedToolFunctor.
 *	It does this by delegating to our script object.
 *
 *	@param event	the mouse event to process.
 *	@param tool		the tool to use.
 */
bool ScriptedToolFunctor::handleMouseEvent( const MouseEvent & event, Tool& tool )
{
	BW_GUARD;

	bool handled = false;

	if (pScriptObject_)
	{
		PyObject * pResult = Script::ask(
			PyObject_GetAttrString( pScriptObject_.getObject(), "onMouseEvent" ),
			Py_BuildValue( "(OO)",
				&*SmartPointer<PyObject>( Script::getData( event ), true ),
				static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::handleMouseEvent",
			true );

		Script::setAnswer( pResult, handled,
			"ScriptedToolFunctor::handleMouseEvent" );
	}

	return handled;
}


/**
 *	This method handles right mouse button click for the ScriptedToolFunctor.
 *	It does this by delegating to our script object.
 *
 *	@param event	the mouse event to process.
 *	@param tool		the tool to use.
 */
bool ScriptedToolFunctor::handleContextMenu( Tool& tool )
{
	BW_GUARD;

	bool handled = false;

	if (pScriptObject_)
	{
		PyObject * pResult = Script::ask(
			PyObject_GetAttrString( pScriptObject_.getObject(), "onContextMenu" ),
			Py_BuildValue( "(O)",
				static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::handleContextMenu",
			true );

		Script::setAnswer( pResult, handled,
			"ScriptedToolFunctor::handleContextMenu" );
	}

	return handled;
}


/** 
 *	This gets called when the functor's tool is about to be used.
 */
void ScriptedToolFunctor::onBeginUsing(Tool &tool)
{
	BW_GUARD;

	if (pScriptObject_)
	{
		Script::call
		(
			PyObject_GetAttrString( pScriptObject_.getObject(), "onBeginUsing" ),
			Py_BuildValue( "(O)",
				static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::onBeginUsing",
			true
		);
	}
}


/** 
 *	This gets called when the functor's tool is not being used any more.
 */
void ScriptedToolFunctor::onEndUsing(Tool &tool)
{
	BW_GUARD;

	if (pScriptObject_)
	{
		Script::call
		(
			PyObject_GetAttrString( pScriptObject_.getObject(), "onEndUsing" ),
			Py_BuildValue( "(O)",
				static_cast<PyObject*>( &tool ) ),
			"ScriptedToolFunctor::onEndUsing",
			true
		);
	}
}


/**
 *	Get an attribute for python
 */
PyObject * ScriptedToolFunctor::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	// try our normal attributes
	PY_GETATTR_STD();

	// ask our base class
	return ToolFunctor::pyGetAttribute( attr );
}


/**
 *	Set an attribute for python
 */
int ScriptedToolFunctor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	// try our normal attributes
	PY_SETATTR_STD();

	// ask our base class
	return ToolFunctor::pySetAttribute( attr, value );
}


/**
 *	Static python factory method
 */
PyObject * ScriptedToolFunctor::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject * pInst;
	if (!PyArg_ParseTuple( args, "O", &pInst ) ||
		!PyInstance_Check( pInst ))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.ScriptedFunctor() "
			"expects a class instance object" );
		return NULL;
	}

	This * pFun = new This();
	pFun->pScriptObject_ = pInst;
	return pFun;
}

// Link in some other functors
extern int MatrixMover_token;
int additional_functor_tokens = MatrixMover_token;
