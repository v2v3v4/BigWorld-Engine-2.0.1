/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENVIRONMENT_CUBE_MAP_HPP
#define ENVIRONMENT_CUBE_MAP_HPP

#include "moo/cube_render_target.hpp"
#include "moo/effect_constant_value.hpp"

/**
 *	This class exposes the CubeMap render of the environment
 */
class EnvironmentCubeMap
{
public:
	EnvironmentCubeMap( uint16 textureSize = 0, uint8 numFacesPerFrame = 0 );

	void update( float dTime, bool defaultNumFaces = true, uint8 numberOfFaces = 1, uint32 drawSelection = 0x80000000  );

	void numberOfFacesPerFrame( uint8 numberOfFaces );

	Moo::CubeRenderTargetPtr cubeRenderTarget() { return pCubeRenderTarget_; }
private:
	Moo::EffectConstantValuePtr pCubeRenderTargetEffectConstant_;
	Moo::CubeRenderTargetPtr pCubeRenderTarget_;

	uint8 environmentCubeMapFace_;
	uint8 numberOfFacesPerFrame_;
};

#endif // ENVIRONMENT_CUBE_MAP_HPP
