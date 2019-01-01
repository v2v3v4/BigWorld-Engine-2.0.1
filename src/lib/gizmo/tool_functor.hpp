/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BRUSH_FUNCTOR_HPP
#define BRUSH_FUNCTOR_HPP

#include "cstdmf/named_object.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "input/input.hpp"

class KeyEvent;
class MouseEvent;
class Tool;

/**
 *	This class keeps the factory methods for all types of Tool Functors
 */
class ToolFunctor;
typedef NamedObject<ToolFunctor * (*)()> FunctorFactory;

#define FUNCTOR_FACTORY_DECLARE( CONSTRUCT )								\
	static FunctorFactory s_factory;										\
	virtual FunctorFactory & factory() { return s_factory; }				\
	static ToolFunctor * s_create() { return new CONSTRUCT; }				\

#define FUNCTOR_FACTORY( CLASS )											\
	FunctorFactory CLASS::s_factory( #CLASS, CLASS::s_create );				\


/**
 *	The ToolFunctor class handles inputs and performs some operation.
 */
class ToolFunctor : public PyObjectPlus
{
	Py_Header( ToolFunctor, PyObjectPlus )

public:
	ToolFunctor( PyTypePlus * pType = &s_type_ );
	virtual ~ToolFunctor();

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool ) = 0;
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool ) = 0;
	virtual bool handleContextMenu( Tool& tool );
	virtual bool applying() const;
	virtual void onBeginUsing(Tool &tool);
	virtual void onEndUsing(Tool &tool);
};


typedef SmartPointer<ToolFunctor>	ToolFunctorPtr;

PY_SCRIPT_CONVERTERS_DECLARE( ToolFunctor )


/**
 *	The scriptedToolFunctor delegates all toolFunctor
 *	methods to a script object.
 */
class ScriptedToolFunctor : public ToolFunctor
{
	Py_Header( ScriptedToolFunctor, ToolFunctor )
public:
	ScriptedToolFunctor( PyTypePlus * pType = &s_type_ );

	void update( float dTime, Tool& tool );
	bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	bool handleMouseEvent( const MouseEvent & event, Tool& tool );
	bool handleContextMenu( Tool& tool );
	void onBeginUsing(Tool &tool);
	void onEndUsing(Tool &tool);

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	/**
	 *	Python attributes
	 */
	PY_FACTORY_DECLARE()

	PY_RW_ATTRIBUTE_DECLARE( pScriptObject_, script )
private:
	///pluggable script event handling
	SmartPointer<PyObject>		pScriptObject_;

	FUNCTOR_FACTORY_DECLARE( ScriptedToolFunctor() )
};

typedef SmartPointer<ScriptedToolFunctor>	ScriptedToolFunctorPtr;


#endif