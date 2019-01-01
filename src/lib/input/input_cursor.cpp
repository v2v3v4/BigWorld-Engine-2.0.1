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

#include "input_cursor.hpp"


PY_TYPEOBJECT( InputCursor )

PY_BEGIN_METHODS( InputCursor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( InputCursor )
PY_END_ATTRIBUTES()


/**
 *	Default constructor.
 */
InputCursor::InputCursor( PyTypePlus * pType ) :
	PyObjectPlus( pType )
{}

/**
 *	Base class focus method, which does nothing
 */
void InputCursor::focus( bool focus )
{}


/**
 *	Base class activation method, which does nothing
 */
void InputCursor::activate()
{}


/**
 *	Base class deactivation method, which does nothing
 */
void InputCursor::deactivate()
{}

// input_cursor.cpp
