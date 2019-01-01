/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef INPUT_CURSOR_HPP
#define INPUT_CURSOR_HPP

#include "input.hpp"
#include "pyscript/pyobject_plus.hpp"


/**
 *	This abstract base class is an interface for 
 *  all cursor like input handlers.
 */
class InputCursor : public PyObjectPlus, public InputHandler
{
	Py_Header( InputCursor, PyObjectPlus )

public:
	virtual void focus( bool focus );
	virtual void activate();
	virtual void deactivate();
	
protected:
	InputCursor( PyTypePlus * pType = &s_type_ );
};


#endif // INPUT_CURSOR_HPP
