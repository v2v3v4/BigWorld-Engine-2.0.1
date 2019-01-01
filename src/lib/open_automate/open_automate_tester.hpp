/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef OPEN_AUTOMATE_TESTER
#define OPEN_AUTOMATE_TESTER

#include "OpenAutomate.h"

/*
This file contains functions used to interanlly test FantasyDemo using the OpenAutomate
callbacks. We override the function table of open automate during the testOAInit.
*/
//TBD - build a proxy based test from this --> this is required as when restarting it is hard to store information in this module as it restarts.
//This will require using some rpc between the client and the proxy and therefore was currently postponed.

oaBool testOAInit(const oaChar *init_str, oaVersion *version);

#endif
/*OPEN_AUTOMATE_TESTER*/
