/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "stdafx.h"

#include "resmgr/bwresource.hpp"

TEST( BWResource_InitArgCArgV )
{
	BWResource bwResource;
	BWResource::init( 0, NULL );

	BWResource::fini();
	DebugMsgHelper::fini();
}

TEST( BWResource_InitArgCArgV2 )
{
	BWResource bwResource;
	BWResource::init( 0, NULL );

	BWResource::fini();
	DebugMsgHelper::fini();
}


TEST( BWResource_InitArgCArgVTwice )
{
	BWResource * pResource = new BWResource();

	BWResource::init( 0, NULL );

	delete pResource;
	BWResource::fini();

	pResource = new BWResource();
	BWResource::init( 0, NULL );
	delete pResource;
	BWResource::fini();
	
	DebugMsgHelper::fini();
}

// test_bwresource.cpp
