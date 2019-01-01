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
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include <dsound.h>
//#define THREADED_WORK

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <algorithm>

#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/memory_counter.hpp"
#include "resmgr/bwresource.hpp"
//#include "profile.hpp"

#ifdef XACT_ENABLED
#pragma comment( lib, "xactguids.lib" )
#endif//XACT_ENABLED

//#include "dspmanager.hpp"
//#include "/mf/bigworld/audio/dsstdfx.h"
//#include "/mf/bigworld/audio/voicefx.h"

#ifndef _RELEASE
#include "moo/moo_math.hpp"
#include "moo/render_context.hpp"
#include "romp/geometrics.hpp"
#include "romp/font.hpp"
#include "ashes/gui_shader.hpp"
#include "ashes/simple_gui.hpp"
#include "ashes/text_gui_component.hpp"
#endif

#include "romp/time_of_day.hpp"
#include "romp/progress.hpp"

#include "soundmgr.hpp"
#ifndef CODE_INLINE
#include "soundmgr.ipp"
#endif
