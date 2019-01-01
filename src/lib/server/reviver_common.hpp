/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REVIVER_COMMON_HPP
#define REVIVER_COMMON_HPP

// This file contains information used by both the Reviver process and by
// Reviver subjects.

#include "cstdmf/stdmf.hpp"

typedef uint8 ReviverPriority;
const ReviverPriority REVIVER_PING_NO  = 0;
const ReviverPriority REVIVER_PING_YES = 1;

#endif // REVIVER_COMMON_HPP
