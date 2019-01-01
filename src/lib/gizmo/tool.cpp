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
#include "tool.hpp"
#include "tool_view.hpp"
#include "physics2/worldtri.hpp"
#include "input/py_input.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/dogWatch.hpp"
#include "chunk/chunk_item.hpp"
//#include "chunks/editor_chunk.hpp"

DECLARE_DEBUG_COMPONENT2( "Tool", 0 );

static DogWatch s_toolLocate( "tool_locate" );
static DogWatch s_toolView( "tool_draw" );
static DogWatch s_toolFunctor( "tool_apply_function" );

#ifndef CODE_INLINE
#include "tool.ipp"
#endif

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE Tool::

PY_TYPEOBJECT( Tool )

PY_BEGIN_METHODS( Tool )
	PY_METHOD( addView )
	PY_METHOD( delView )
	PY_METHOD( beginUsing )
	PY_METHOD( endUsing )
	PY_METHOD( handleKeyEvent )
	PY_METHOD( handleMouseEvent )
	PY_METHOD( handleContextMenu )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Tool )
	PY_ATTRIBUTE( size )
	PY_ATTRIBUTE( strength )
	PY_ATTRIBUTE( locator )
	PY_ATTRIBUTE( functor )
	PY_ATTRIBUTE( applying )
PY_END_ATTRIBUTES()

PY_FACTORY( Tool, WorldEditor )


/**
 *	Constructor.
 */
Tool::Tool( ToolLocatorPtr locator,
			 ToolViewPtr view,
			 ToolFunctorPtr functor,
			 PyTypePlus * pType ):
	PyObjectPlus( pType ),
	locator_( locator ),
	functor_( functor ),
	size_( 1.f ),
	strength_( 25.f ),
	currentChunk_( NULL )
{
	BW_GUARD;

	if (view) addView( "default", view );
}

void Tool::onPush()
{
	BW_GUARD;

	currentChunk_ = NULL;
	relevantChunks_.clear();
}

void Tool::onPop()
{
}


void Tool::onBeginUsing()
{
	BW_GUARD;

	if (functor_)
		functor_->onBeginUsing(*this);
}


void Tool::onEndUsing()
{
	BW_GUARD;

	if (functor_)
		functor_->onEndUsing(*this);
}


/**
 *	This method is the entry point for calculating the position
 *	of the tool.  Most of the work is done in the tool's locator.
 *
 *	@param	worldRay	The camera's world ray.
 */
void Tool::calculatePosition( const Vector3& worldRay )
{
	BW_GUARD;

	s_toolLocate.start();

	relevantChunks_.clear();

	if ( locator_ )
	{
		locator_->calculatePosition( worldRay, *this );
		this->findRelevantChunks();
	}

	s_toolLocate.stop();
}


/**
 *	This method updates the tool.  It delegates the call
 *	to the tool's functor.
 *
 *	@param	dTime	The change in time since the last frame.
 */
void Tool::update( float dTime )
{
	BW_GUARD;

	s_toolFunctor.start();

	if ( functor_ )
		functor_->update( dTime, *this );

	s_toolFunctor.stop();
}


/**
 *	This method renders the tool.  It delegates the call
 *	to all of the tool's views.
 */
void Tool::render()
{
	BW_GUARD;

	s_toolView.start();

	ToolViewPtrs::iterator it = view_.begin();
	ToolViewPtrs::iterator end = view_.end();

	while ( it != end )
	{
		(it++)->second->render( *this );
	}

	s_toolView.stop();
}


/**
 *	This method overrides InputHandler's method.
 *
 *	@see InputHandler::handleKeyEvent
 */
bool Tool::handleKeyEvent( const KeyEvent & event )
{
	BW_GUARD;

	if ( functor_ )
		return functor_->handleKeyEvent( event, *this );
	return false;
}


/**
 *	This method overrides InputHandler's method.
 *
 *	@see InputHandler::handleMouseEvent
 */
bool Tool::handleMouseEvent( const MouseEvent & event )
{
	BW_GUARD;

	if ( functor_ )
		return functor_->handleMouseEvent( event, *this );
	return false;
}


/**
 *	This method can be called from python to tell the tool to 
 *	display its relevant context menu.
 */
bool Tool::handleContextMenu()
{
	BW_GUARD;

	if ( functor_ )
		return functor_->handleContextMenu( *this );
	return false;
}


/**
 *	This method sets the tool's locator.
 *
 *	@param	spl		The new locator for the tool.
 */
void Tool::locator( ToolLocatorPtr spl )
{
	locator_ = spl;
}


/**
 *	This method retrives the vector of views for the tool.
 *
 *	@return	The map of views.
 */
const ToolViewPtrs& Tool::view() const
{
	return view_;
}


/**
 *	This method retrives the a view for the tool.
 *
 *	@return	The desired view, if it exists.
 */
ToolViewPtr& Tool::view( const std::string& name )
{
	BW_GUARD;

	return view_[name];
}



/**
 *	This method adds a new view to the tool
 *
 *	@param	spv		The new view for the tool.
 */
void Tool::addView( const std::string& name, ToolViewPtr spv )
{
	BW_GUARD;

	if ( spv.hasObject() )
		view_[name] = spv;
}


/**
 *	This method deletes a view from the tool
 *
 *	@param	spv		The view to remove.
 */
void Tool::delView( ToolViewPtr spv )
{
	BW_GUARD;

	ToolViewPtrs::iterator it;
	for (it = view_.begin(); it != view_.end(); it++)
	{
		if (it->second.getObject() == spv) break;
	}

	if (it != view_.end())
	{
		view_.erase( it );
	}
}


/**
 *	This method deletes a view from the tool, by name.
 *
 *	@param	name	The name of the view to remove.
 */
void Tool::delView( const std::string & name )
{
	BW_GUARD;

	view_.erase( name );
}


/**
 *	This method sets the tool's functor.
 *
 *	@param	spf		The new functor for the tool.
 */
void Tool::functor( ToolFunctorPtr spf )
{
	functor_ = spf;
}


/**
 *	This method returns the tool's locator.
 *
 *	@return The tool's locator.
 */
const ToolLocatorPtr Tool::locator() const
{
	return locator_;
}


/**
 *	This method returns the tool's functor.
 *
 *	@return The tool's functor.
 */
const ToolFunctorPtr Tool::functor() const
{
	return functor_;
}

extern void findRelevantChunks( ToolPtr tool, float buffer = 0.f );

/**
 *	This method finds all the chunks that a tool covers.
 *
 *	The relevant chunks are stored in the tool's relevantChunks vector.
 */
void Tool::findRelevantChunks( float buffer /* = 0.f */)
{
	BW_GUARD;

	::findRelevantChunks( this, buffer );
	/*
	if ( locator() )
	{
		float halfSize = size() / 2.f;
		Vector3 start( locator()->transform().applyToOrigin() -
						Vector3( halfSize, 0.f, halfSize ) );
		Vector3 end( locator()->transform().applyToOrigin() +
						Vector3( halfSize, 0.f, halfSize ) );

		EditorChunk::findOutsideChunksWithinBox(
			BoundingBox( start, end ), relevantChunks() );

		currentChunk() = 
			EditorChunk::findOutsideChunk( locator()->transform().applyToOrigin() );
	}
	*/
}


/**
 *	Get an attribute for python
 */
PyObject * Tool::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	// try our normal attributes
	PY_GETATTR_STD();

	// try one of the view names
	ToolViewPtrs::iterator vit = view_.find( attr );
	if (vit != view_.end())
	{
		PyObject * ret = vit->second.getObject();
		Py_INCREF( ret );
		return ret;
	}

	// ask our base class
	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	Set an attribute for python
 */
int Tool::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	// try our normal attributes
	PY_SETATTR_STD();

	// ask our base class
	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Add a view for python
 */
PyObject * Tool::py_addView( PyObject * args )
{
	BW_GUARD;

	char noName[32];

	PyObject	* pView;
	char * name = noName;

	// parse args
	if (!PyArg_ParseTuple( args, "O|s", &pView, &name ) ||
		!ToolView::Check( pView ))
	{
		PyErr_SetString( PyExc_TypeError, "Tool.py_addView() "
			"expects a ToolView and optionally a name" );
		return NULL;
	}

	// make up a name if none was set
	if (name == noName || !name[0])
	{
		name = noName;
		bw_snprintf( noName, sizeof( noName ), "S%08X", (int)pView );
	}

	this->addView( name, static_cast<ToolView*>( pView ) );

	Py_Return;
}

/**
 *	Del a view for python
 */
PyObject * Tool::py_delView( PyObject * args )
{
	BW_GUARD;

	if (PyTuple_Size( args ) == 1)
	{
		PyObject * pItem = PyTuple_GetItem( args, 0 );
		if (ToolView::Check( pItem ))
		{
			this->delView( static_cast<ToolView*>( pItem ) );
			Py_Return;
		}
		if (PyString_Check( pItem ))
		{
			this->delView( PyString_AsString( pItem ) );
			Py_Return;
		}
	}

	PyErr_SetString( PyExc_TypeError, "ToolView.py_delView "
		"expects a ToolView or a string" );
	return NULL;
}


/**
 *	Flag that the tool is about to be used.
 */
PyObject * Tool::py_beginUsing( PyObject * args )
{
	BW_GUARD;

	this->onBeginUsing();
	Py_Return;
}


/**
 *	Flag that the tool is stopped being used.
 */
PyObject * Tool::py_endUsing( PyObject * args )
{
	BW_GUARD;

	this->onEndUsing();
	Py_Return;
}

/**
 *	Handle a right click call from python
 */
PyObject * Tool::py_handleContextMenu( PyObject * args )
{
	BW_GUARD;

	return Script::getData( this->handleContextMenu() );
}


/**
 *	Python factory method
 */
PyObject * Tool::pyNew( PyObject * args )
{
	BW_GUARD;

	PyObject * l = NULL, * v = NULL, * f = NULL;
	if (!PyArg_ParseTuple( args, "|OOO", &l, &v, &f ) ||
		(l != NULL && !ToolLocator::Check( l )) ||
		(v != NULL && !ToolView::Check( v )) ||
		(f != NULL && !ToolFunctor::Check( f )))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.Tool() "
			"expects an optional ToolLocator, ToolView, and ToolFunctor" );
		return NULL;
	}

	return new Tool(
		static_cast<ToolLocator*>(l),
		static_cast<ToolView*>(v),
		static_cast<ToolFunctor*>(f) );
}

// tool.cpp
