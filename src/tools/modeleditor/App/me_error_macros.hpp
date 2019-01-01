/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "appmgr/commentary.hpp"

#define ME_INFO_MSG( msg ) \
	INFO_MSG( msg ); \
	Commentary::instance().addMsg( msg, Commentary::COMMENT );

#define ME_WARNING_MSG( msg ) \
	WARNING_MSG( msg ); \
	Commentary::instance().addMsg( msg, Commentary::WARNING );
	
#define ME_INFO_MSGW( msg ) \
	INFO_MSG( bw_wtoacp( msg ).c_str() ); \
	Commentary::instance().addMsg( msg, Commentary::COMMENT );

#define ME_WARNING_MSGW( msg ) \
	WARNING_MSG( bw_wtoacp( msg ).c_str() ); \
	Commentary::instance().addMsg( msg, Commentary::WARNING );
