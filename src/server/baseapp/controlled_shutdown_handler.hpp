/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLLED_SHUTDOWN_HANDLER_HPP
#define CONTROLLED_SHUTDOWN_HANDLER_HPP

#include "network/basictypes.hpp"
#include "network/misc.hpp"

class Bases;
class SqliteDatabase;

namespace ControlledShutdown
{

void start( SqliteDatabase * pSecondaryDB,
			const Bases & bases,
			Mercury::ReplyID replyID,
			const Mercury::Address & srcAddr );

} // namespace ControlledShutdown


#endif // CONTROLLED_SHUTDOWN_HANDLER_HPP
