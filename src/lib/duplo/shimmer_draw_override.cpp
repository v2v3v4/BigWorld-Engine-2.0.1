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
#include "shimmer_draw_override.hpp"
#include "material_draw_override.hpp"

// -----------------------------------------------------------------------------
// Section: ShimmerDrawOverride
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ShimmerDrawOverride::ShimmerDrawOverride():
pDrawOverride_(NULL)
{
}


/**
 *	Destructor.
 */
ShimmerDrawOverride::~ShimmerDrawOverride()
{
}


void ShimmerDrawOverride::begin()
{
	BW_GUARD;
	if (Moo::Visual::s_pDrawOverride)
		return;

	if (!pDrawOverride_)
	{
		pDrawOverride_ = new MaterialDrawOverride( "shaders/std_effects/shimmer/shimmer", true );
	}

	Moo::Visual::s_pDrawOverride = pDrawOverride_;	
}

void ShimmerDrawOverride::end()
{
	Moo::Visual::s_pDrawOverride = NULL;
}

// shimmer_draw_override.cpp
