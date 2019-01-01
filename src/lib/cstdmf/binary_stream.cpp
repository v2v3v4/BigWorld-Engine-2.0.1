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
#include "binary_stream.hpp"
#include <stdlib.h>

// Static return buffer returned from retrieve() when not enough data for
// request.
char BinaryIStream::errBuf[ BS_BUFF_MAX_SIZE ];
