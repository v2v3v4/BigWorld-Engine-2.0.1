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
#include "cstdmf.hpp"

/// cstdmf library Singleton
BW_SINGLETON_STORAGE( CStdMf );

CStdMf::CStdMf()
{
	DebugFilter::instance();
	DogWatchManager::instance();
	Diary::instance();
	BgTaskManager::instance();
}

CStdMf::~CStdMf()
{
	BgTaskManager::fini();
	Diary::fini();
	DogWatchManager::fini();
	DebugFilter::fini();
}


// cstdmf.cpp
