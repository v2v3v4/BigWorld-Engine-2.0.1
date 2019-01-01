/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STIPPLE_DRAW_OVERRIDE_HPP
#define STIPPLE_DRAW_OVERRIDE_HPP

/**
 *	This class implements the stipple draw override, which
 *	is a global rendering style that attempts to draw any
 *	visual stippled.
 */
#include "material_draw_override.hpp"

class StippleDrawOverride : public ReferenceCount
{
public:
	StippleDrawOverride();
	~StippleDrawOverride();

	void init( DataSectionPtr pConfigSection );

	void begin();
	void end();
private:
	MaterialDrawOverride* pDrawOverride_;

	StippleDrawOverride( const StippleDrawOverride& );
	StippleDrawOverride& operator=( const StippleDrawOverride& );
};


#endif // STIPPLE_DRAW_OVERRIDE_HPP
