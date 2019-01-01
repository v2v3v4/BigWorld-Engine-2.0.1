/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FROZEN_MODULES_HPP
#define FROZEN_MODULES_HPP

#include "res_mgr_import.hpp"

#ifdef USE_RES_MGR_IMPORT_HOOK

#include "Python.h"

PyAPI_DATA(struct _frozen *) PyImport_FrozenModules;

/*
 * This structure contains modules we've frozen because
 * they are loaded before the ResMgrImportHook is put into
 * sys.paths
 */
extern struct _frozen BigWorldFrozenModules[];

#endif // USE_RES_MGR_IMPORT_HOOK

#endif // FROZEN_MODULES_HPP
