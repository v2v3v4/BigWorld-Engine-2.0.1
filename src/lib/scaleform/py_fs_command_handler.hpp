/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_FS_COMMAND_HANDLER_HPP
#define PY_FS_COMMAND_HANDLER_HPP
#if SCALEFORM_SUPPORT

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"
#include <GFxPlayer.h>

namespace Scaleform
{
	/**
	 * This class stores a callback pointer, and implements the FSCommandHandler
	 * interface.
	 */
	class PyFSCommandHandler : public GFxFSCommandHandler
	{
	public:
		PyFSCommandHandler( PyObject * pCallback );
		~PyFSCommandHandler();
		void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg);
	private:
		PyObjectPtr pCallback_;
	};

}	// namespace Scaleform

#endif //#if SCALEFORM_SUPPORT
#endif //PY_FS_COMMAND_HANDLER_HPP