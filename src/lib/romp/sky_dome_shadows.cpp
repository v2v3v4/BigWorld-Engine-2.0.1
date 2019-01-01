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
#include "cstdmf/debug.hpp"
#include "moo/moo_dx.hpp"
#include "sky_dome_shadows.hpp"
#include "enviro_minder.hpp"
#include "time_of_day.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

static bool s_showRenders = false;

// -----------------------------------------------------------------------------
// Section: Environment Shadow Draw Constant
// -----------------------------------------------------------------------------
/**
 *	This class tells sky domes and other environmental objects to draw using their
 *	shadowing shaders, instead of their standard shader.
 */
class EnvironmentShadowDrawEnable : public Moo::EffectConstantValue
{
public:
	EnvironmentShadowDrawEnable():
	  value_( false )
	{		
	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		pEffect->SetInt(constantHandle, value_ ? 1 : 0);
		return true;
	}
	void value( bool value )
	{
		value_ = value;
	}
private:
	bool value_;
};

// destroyed by EffectConstantValue::fini
static EnvironmentShadowDrawEnable* s_shadowDrawConstant = NULL;



// -----------------------------------------------------------------------------
// Section: Sky Dome Shadows.
// -----------------------------------------------------------------------------
bool SkyDomeShadows::isAvailable()
{
	return true;
}


/**
 *	Constructor.
 */
SkyDomeShadows::SkyDomeShadows(EnviroMinder& enviroMinder):	
	enviroMinder_( enviroMinder )
{
	if (!s_shadowDrawConstant)
	{
		s_shadowDrawConstant = new EnvironmentShadowDrawEnable;
		*Moo::EffectConstantValue::get("EnvironmentShadowDraw") = 
			s_shadowDrawConstant;
	}
}


/**
 *	Destructor.
 */
SkyDomeShadows::~SkyDomeShadows()
{
}


/**
 *	This method draws all the sky boxes.  It uses the automatic effect
 *	variable "EnvironmentShadowDraw" to signify to these
 *	visuals that they should draw themselves using their
 *	shadowing shaders instead of their ordinary ones.
 *
 *	@return	int	the number of pixels in the region we are drawing to.
 */
void SkyDomeShadows::render( SkyLightMap* lightMap, Moo::EffectMaterialPtr material, float sunAngle  )
{
	s_shadowDrawConstant->value(true);
	enviroMinder_.drawSkyDomes();
	s_shadowDrawConstant->value(false);
}


std::ostream& operator<<(std::ostream& o, const SkyDomeShadows& t)
{
	o << "SkyDomeShadows\n";
	return o;
}

// sky_dome_shadows.cpp
