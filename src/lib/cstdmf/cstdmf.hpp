/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CSTDMF_HPP
#define CSTDMF_HPP

#include "singleton.hpp"
#include "dprintf.hpp"
#include "dogwatch.hpp"
#include "diary.hpp"
#include "bgtask_manager.hpp"

class CStdMf : public Singleton<CStdMf>
{
public:
						CStdMf();
	virtual				~CStdMf();

	DebugFilter&		debugFilter();
	DogWatchManager&	dogWatchManager();
	Diary&				diary();
	BgTaskManager&		bgTaskManager();
};


inline DebugFilter& CStdMf::debugFilter()
{
	return DebugFilter::instance();
}


inline DogWatchManager& CStdMf::dogWatchManager()
{
	return DogWatchManager::instance();
}


inline Diary& CStdMf::diary()
{
	return Diary::instance();
}


inline BgTaskManager& CStdMf::bgTaskManager()
{
	return BgTaskManager::instance();
}


#endif // CSTDMF_HPP
