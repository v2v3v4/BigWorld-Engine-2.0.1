/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MEMORY_COUNTER_HPP
#define MEMORY_COUNTER_HPP

#include "stdmf.hpp"

#include <string>
#include <vector>
#include <map>
#include <list>

// TODO: Implement these on Win32 on the PC
#define memoryCounterAdd( X )
#define memoryCounterSub( X )

#ifdef _WIN32
#define memoryClaim (void)
#else
#define memoryClaim( ... )
#endif
#define memoryCounterDeclare( X )
#define memoryCounterDefine( X, Y )
#define memoryCounterDefineWithAlias( X, Y, Z )

#define trackAddRef( X, Y )
#define trackDelRef( X, Y )

#endif // MEMORY_COUNTER_HPP
