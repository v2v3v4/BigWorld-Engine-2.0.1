/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TOOL_HPP_
#define TOOL_HPP_

#include <vector>

#include "pyscript/pyobject_plus.hpp"
#include "input/input.hpp"
#include "input/py_input.hpp"
#include "tool_view.hpp"
#include "tool_locator.hpp"
#include "tool_functor.hpp"

//this kind of vector is only good for one frame ( not even ?),
//since we can't guarantee the existence of the chunk.
class Chunk;
typedef std::vector< Chunk* >	ChunkPtrVector;
typedef Chunk*	ChunkPtr;

class ChunkFinder;

/**
 *	The Tool class is an almagamation of View, Locator and Functor.
 *	This class exposes common Tool properties, as well as these
 *	plugin logic units.  All methods are delegated through the
 *	plugin units.
 */
class Tool : public InputHandler, public PyObjectPlus
{
	Py_Header( Tool, PyObjectPlus )

public:
	Tool( ToolLocatorPtr locator,
		ToolViewPtr view,
		ToolFunctorPtr functor,
		PyTypePlus * pType = &s_type_ );

	virtual ~Tool()	{};

	virtual void onPush();
	virtual void onPop();

	virtual void onBeginUsing();
	virtual void onEndUsing();

	virtual void size( float s );
	virtual float size() const;

	virtual void strength( float s );
	virtual float strength() const;

	virtual const ToolLocatorPtr locator() const;
	virtual void locator( ToolLocatorPtr spl );

	virtual const ToolViewPtrs& view() const;
	virtual ToolViewPtr& view( const std::string& name );
	virtual void addView( const std::string& name, ToolViewPtr spv );
	virtual void delView( ToolViewPtr spv );
	virtual void delView( const std::string& name );

	virtual const ToolFunctorPtr functor() const;
	virtual void functor( ToolFunctorPtr spf );

	virtual ChunkPtrVector& relevantChunks();
	virtual const ChunkPtrVector& relevantChunks() const;

	virtual ChunkPtr& currentChunk();
	virtual const ChunkPtr& currentChunk() const;

	virtual void calculatePosition( const Vector3& worldRay );
	virtual void update( float dTime );
	virtual void render();
	virtual bool handleKeyEvent( const KeyEvent & event );
	virtual bool handleMouseEvent( const MouseEvent & event );
	virtual bool handleContextMenu();
	virtual bool applying() const;

	void			findRelevantChunks( float buffer = 0.f );

	//-------------------------------------------------
	//Python Interface
	//-------------------------------------------------
	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_METHOD_DECLARE( py_addView )
	PY_METHOD_DECLARE( py_delView )

	PY_METHOD_DECLARE( py_beginUsing )
	PY_METHOD_DECLARE( py_endUsing )

	PY_AUTO_METHOD_DECLARE( RETDATA, handleKeyEvent, ARG( KeyEvent, END ) )
	PY_AUTO_METHOD_DECLARE( RETDATA, handleMouseEvent, ARG( MouseEvent, END ) )
	PY_METHOD_DECLARE( py_handleContextMenu )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, size, size )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, strength, strength )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( ToolLocatorPtr, locator, locator )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( ToolFunctorPtr, functor, functor )

	PY_RO_ATTRIBUTE_DECLARE( applying(), applying )

	PY_FACTORY_DECLARE()

private:
	float			size_;
	float			strength_;
	ChunkPtr		currentChunk_;
	ChunkPtrVector	relevantChunks_;

	ToolViewPtrs	view_;
	ToolLocatorPtr	locator_;
	ToolFunctorPtr	functor_;
};

typedef SmartPointer<Tool>	ToolPtr;

class ChunkFinder
{
public:
	virtual void findChunks(ToolPtr tool);
};


#ifdef CODE_INLINE
#include "tool.ipp"
#endif

#endif