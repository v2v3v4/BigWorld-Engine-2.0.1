/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHIMMER_DRAW_OVERRIDE_HPP
#define SHIMMER_DRAW_OVERRIDE_HPP

/**
 *	This class implements the shimmer draw override, which
 *	is a global rendering style that attempts to draw any
 *	visual into the shimmer channel.
 */

#include "material_draw_override.hpp"

class ShimmerDrawOverride : public ReferenceCount
{
public:
	ShimmerDrawOverride();
	~ShimmerDrawOverride();

	void init( DataSectionPtr pConfigSection );

	void begin();
	void end();
private:
	MaterialDrawOverride* pDrawOverride_;	

	ShimmerDrawOverride( const ShimmerDrawOverride& );
	ShimmerDrawOverride& operator=( const ShimmerDrawOverride& );
};


#endif // SHIMMER_DRAW_OVERRIDE_HPP
