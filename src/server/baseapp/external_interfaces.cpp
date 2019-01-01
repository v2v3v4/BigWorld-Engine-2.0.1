/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// this file groups all external interfaces, so that the makefile
// doesn't have to include them from other directories.

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "../baseappmgr/baseappmgr_interface.cpp"
#include "../cellapp/cellapp_interface.cpp"
#include "../cellappmgr/cellappmgr_interface.cpp"
#include "../dbmgr/db_interface.cpp"
#include "connection/client_interface.cpp"
