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
#include "py_fs_command_handler.hpp"

namespace Scaleform
{

PyFSCommandHandler::PyFSCommandHandler( PyObject * callback ):
	pCallback_( callback )
{
};


PyFSCommandHandler::~PyFSCommandHandler()
{
}


void PyFSCommandHandler::Callback(GFxMovieView* pmovie, const char* command, const char* args)
{
	BW_GUARD;
	PyObject* pyArgs = Py_BuildValue("(s s)", command, args);
	Py_XINCREF( pCallback_.get() );
	Script::call( pCallback_.getObject(), pyArgs, "PyFSCommandHandler::Callback", false );
}

}	// namespace Scaleform
#endif //#if SCALEFORM_SUPPORT