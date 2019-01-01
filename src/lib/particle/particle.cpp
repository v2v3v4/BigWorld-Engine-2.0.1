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
#include "particle.hpp"
#ifndef CODE_INLINE
#include "particle.ipp"
#endif

namespace 
{
    // The compiler can complain that the .obj file that this file produces
    // has no symbols.  This do-nothing int prevents the warning.
    int noCompilerWarning   = 0;
}
