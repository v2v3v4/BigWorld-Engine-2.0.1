/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_EXTERNAL_INTERFACE_HPP
#define PY_EXTERNAL_INTERFACE_HPP
#if SCALEFORM_SUPPORT

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"
#include <GFxPlayer.h>

namespace Scaleform
{
	/**
	 * This class stores a callback pointer, and implements the ExternalInterface
	 * interface.
	 */
	class PyExternalInterface : public GFxExternalInterface
	{
	public:
		PyExternalInterface( PyObject * pCallback );
		~PyExternalInterface();
		void Callback(GFxMovieView* pmovie, const char* pcommand, const GFxValue* args, UInt argCount);
	private:
		PyObjectPtr pCallback_;
	};
}	// namespace Scaleform

#endif //#if SCALEFORM_SUPPORT
#endif //PY_EXTERNAL_INTERFACE_HPP