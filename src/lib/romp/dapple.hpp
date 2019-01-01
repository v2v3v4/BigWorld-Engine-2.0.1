/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DAPPLE_HPP
#define DAPPLE_HPP

#include "cstdmf/stdmf.hpp"
#include "moo/managed_texture.hpp"
#include "moo/material.hpp"
#include "sky_light_map.hpp"

class Dapple : public SkyLightMap::IContributor
{
public:
	Dapple();
	void dapple(float sunAngle);
	bool dappleEnabled();

	void activate( const class EnviroMinder& enviro,
		DataSectionPtr pSpaceSettings,
		SkyLightMap* skyLightMap );
	void deactivate( const class EnviroMinder& enviro,
		SkyLightMap* skyLightMap );

	//SkyLightMap::Contributor interface
	bool needsUpdate();
	void render(
		SkyLightMap* lightMap,
		Moo::EffectMaterial* material,
		float sunAngle  );
private:
	Moo::EffectMaterial* dappleMaterial_;
	Moo::BaseTexturePtr pDappleTex_;
	Vector2 spaceMin_;
	Vector2 spaceMax_;
};

#endif