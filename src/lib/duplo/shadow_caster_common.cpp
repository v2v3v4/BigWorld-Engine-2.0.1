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
#include "shadow_caster_common.hpp"

#include "material_draw_override.hpp"

/**
 * Constructor, initialises to default values and adds watches.
 */
ShadowCasterCommon::ShadowCasterCommon() : 
shadowIntensity_( 0.4f ),
shadowDistance_( 10.f ),
shadowFadeStart_( 9.f ),
shadowBufferSize_( 1024 ),
shadowQuality_( 0 )
{
	BW_GUARD;
	MF_WATCH( "Render/shadows/intensity", shadowIntensity_, Watcher::WT_READ_WRITE, "Sets the intensity, or darkness, of the shadows" );
	MF_WATCH( "Render/shadows/quality", shadowQuality_, Watcher::WT_READ_WRITE, "Quality setting for shadows (0,1,2)" );
	MF_WATCH( "Render/shadows/distance", shadowDistance_, Watcher::WT_READ_WRITE, "Distance to which shadows are drawn." );
	MF_WATCH( "Render/shadows/fadeStart", shadowFadeStart_, Watcher::WT_READ_WRITE, "Distance at which shadows begin to fade out." );
	MF_WATCH( "Render/shadows/bufferSize", shadowBufferSize_, Watcher::WT_READ_ONLY, "Resolution of the shadow buffer (buffer is always square)" );
}

/**
 * Destructor.
 */
ShadowCasterCommon::~ShadowCasterCommon()
{
}

/**
*	This method inits the common properties used by the shadows from a xml section.
*	@param pConfigSection the root section to read the properties from (ie engine_config.xml)
*/
void ShadowCasterCommon::init( DataSectionPtr pConfigSection )
{
	BW_GUARD;
	shadowIntensity_ = pConfigSection->readFloat( "intensity", shadowIntensity_ );
	shadowDistance_ = pConfigSection->readFloat( "distance", shadowDistance_ );
	shadowFadeStart_ = pConfigSection->readFloat( "fadeStart", shadowFadeStart_ );
	shadowBufferSize_ = pConfigSection->readInt( "bufferSize", shadowBufferSize_ );
	shadowQuality_ = pConfigSection->readInt( "quality", shadowQuality_ );
	pCasterOverride_.reset(new MaterialDrawOverride( pConfigSection->readString( "modelCasters" ), false ));
	pReceiverOverride_.reset(new MaterialDrawOverride( pConfigSection->readString( "modelReceivers" ), false ));
	terrainReceiver_ = new Moo::EffectMaterial();
	terrainReceiver_->initFromEffect( pConfigSection->readString( "terrainReceiver" ) );
	terrain2Receiver_ = new Moo::EffectMaterial();
	terrain2Receiver_->initFromEffect( pConfigSection->readString( "terrain2Receiver" ) );
}

// shadow_caster.cpp