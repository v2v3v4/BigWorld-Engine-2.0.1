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
#if SCALEFORM_SUPPORT
#include "py_external_interface.hpp"
#include "util.hpp"

namespace Scaleform
{

PyExternalInterface::PyExternalInterface( PyObject * callback ):
	pCallback_( callback )
{
};


PyExternalInterface::~PyExternalInterface()
{
}


void PyExternalInterface::Callback(GFxMovieView* pmovie, const char* command, const GFxValue* args, UInt argCount)
{
	BW_GUARD;
	PyObject * ret;
	PyObject* tuple = PyTuple_New( argCount );

	for (size_t i=0; i<argCount; i++)
	{
		PyObject * pArg;
		valueToObject( args[i], &pArg );
		PyTuple_SetItem( tuple, i, pArg );
	}

	PyObject* pyArgs = Py_BuildValue("sO", command, tuple );

	Py_XINCREF( pCallback_.get() );
	ret = Script::ask( pCallback_.getObject(), pyArgs, "PyExternalInterface::Callback", false );

	GFxValue gfxRet;
	objectToValue( ret, gfxRet );
	pmovie->SetExternalInterfaceRetVal( gfxRet );
}

} // namespace Scaleform
#endif //#if SCALEFORM_SUPPORT