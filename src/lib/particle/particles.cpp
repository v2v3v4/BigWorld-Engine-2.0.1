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
#include "particles.hpp"

#ifndef CODE_INLINE
#include "particles.ipp"
#endif

// Avoid LNK4221 warning - no public symbols. This happens when this file is
// compiled in non-editor build
extern const int dummyPublicSymbol = 0;
