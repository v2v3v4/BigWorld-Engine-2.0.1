/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VK_MAP__HPP
#define VK_MAP__HPP

#include "key_code.hpp"
#include <windows.h>

namespace VKMap 
{
	KeyCode::Key fromVKey( USHORT vkey, bool extended );
	KeyCode::Key fromRawKey( RAWKEYBOARD* rkb );
	USHORT toVKey( KeyCode::Key key );
}

#endif // VK_MAP__HPP
