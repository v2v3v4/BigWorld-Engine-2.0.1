/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELLAPP_PROFILE_HPP
#define CELLAPP_PROFILE_HPP

#include "cstdmf/profile.hpp"


// __kyl__(30/7/2007) Special profile thresholds for T2CN
extern TimeStamp 	g_profileInitGhostTimeLevel;
extern int			g_profileInitGhostSizeLevel;
extern TimeStamp 	g_profileInitRealTimeLevel;
extern int			g_profileInitRealSizeLevel;
extern TimeStamp 	g_profileOnloadTimeLevel;
extern int			g_profileOnloadSizeLevel;
extern int			g_profileBackupSizeLevel;


extern ProfileVal SCRIPT_CALL_PROFILE;

extern ProfileVal ON_MOVE_PROFILE;
extern ProfileVal SHUFFLE_ENTITY_PROFILE;
extern ProfileVal SHUFFLE_TRIGGERS_PROFILE;
extern ProfileVal SHUFFLE_AOI_TRIGGERS_PROFILE;

extern ProfileVal CLIENT_UPDATE_PROFILE;
extern ProfileVal TRANSIENT_LOAD_PROFILE;

namespace CellProfileGroup
{
void init();
}

#ifdef CODE_INLINE
#include "profile.ipp"
#endif

#endif // CELLAPP_PROFILE_HPP
