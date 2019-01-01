/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHADOW_CASTER_COMMON_HPP
#define SHADOW_CASTER_COMMON_HPP


#include "moo/effect_material.hpp"

class MaterialDrawOverride;
typedef SmartPointer<class DataSection> DataSectionPtr;
/**
*	This class contains the common properties for the shadows
*/
class ShadowCasterCommon
{
public:
	ShadowCasterCommon();
	~ShadowCasterCommon();
	void init( DataSectionPtr pConfigSection );
	MaterialDrawOverride* pCasterOverride() { return pCasterOverride_.get(); }
	MaterialDrawOverride* pReceiverOverride() { return pReceiverOverride_.get(); }
	Moo::EffectMaterialPtr  pTerrainReceiver() { return terrainReceiver_; }
	Moo::EffectMaterialPtr  pTerrain2Receiver() { return terrain2Receiver_; }

	float shadowIntensity() { return shadowIntensity_; }
	float shadowFadeStart() { return shadowFadeStart_; };
	float shadowDistance() { return shadowDistance_; }
	uint32 shadowBufferSize() { return shadowBufferSize_; }
	int	  shadowQuality() { return shadowQuality_; }

	void  shadowQuality( int shadowQuality) { shadowQuality_ = shadowQuality; }
private:
	typedef std::auto_ptr<MaterialDrawOverride> MaterialDrawOverridePtr;
	MaterialDrawOverridePtr pCasterOverride_;
	MaterialDrawOverridePtr pReceiverOverride_;
	Moo::EffectMaterialPtr terrainReceiver_;
	Moo::EffectMaterialPtr terrain2Receiver_;

	float	shadowIntensity_;

	float	shadowDistance_;
	float	shadowFadeStart_;
	uint32	shadowBufferSize_;

	int		shadowQuality_;
};


#endif // SHADOW_CASTER_COMMON_HPP
