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
#include "mrt_support.hpp"

// -----------------------------------------------------------------------------
// Section: class MRTSupport
// -----------------------------------------------------------------------------

BW_INIT_SINGLETON_STORAGE( Moo::MRTSupport )

namespace Moo
{

Moo::EffectMacroSetting::EffectMacroSettingPtr MRTSupport::s_mrtSetting_ = 
	new Moo::EffectMacroSetting(
		"MRT_DEPTH", "Advanced Post Processing", "USE_MRT_DEPTH",
		&MRTSupport::configureKeywordSetting);

bool MRTSupport::TextureSetter::operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
{
	//If this assert goes off, then a shader is trying to fetch "DepthTex", but
	//MRTSupport::bind() was not called first - meaning the second render target
	//may well be set on the device currently, which is bad.
	MF_ASSERT_DEV( !MRTSupport::instance().isEnabled() || MRTSupport::instance().bound() == true )

	if (map_.hasComObject())
		pEffect->SetTexture( constantHandle, map_.pComObject() );
	else
		pEffect->SetTexture( constantHandle, NULL );
	return true;
}


MRTSupport::MRTSupport():
	bound_( false )
{
}


void MRTSupport::TextureSetter::map(DX::BaseTexture* pTexture )
{
	map_ = pTexture;
}

ComObjectWrap<DX::BaseTexture> MRTSupport::TextureSetter::map()
{
	return map_;
}

/**
  *
  */
void MRTSupport::configureKeywordSetting(Moo::EffectMacroSetting & setting)
{	
	bool supported = Moo::rc().mrtSupported();
	//Moo::rc().supportsTextureFormat( D3DFMT_R32F, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING );
	  
	setting.addOption("ON", "On", supported, "1");
	setting.addOption("OFF", "Off", true, "0");

	MF_ASSERT( MRTSupport::pInstance() )
	Moo::EffectManager::instance().addListener( MRTSupport::pInstance() );
}


bool MRTSupport::doInit()
{
	bound_ = false;
	mapSetter_ = new TextureSetter();
	*Moo::EffectConstantValue::get( "DepthTex" ) = mapSetter_;
	return true;
}

/**
 *
 */
bool MRTSupport::doFini()
{
	Moo::EffectManager::instance().delListener( MRTSupport::pInstance() );
	s_mrtSetting_ = NULL;
	*Moo::EffectConstantValue::get( "DepthTex" ) = NULL;
	mapSetter_ = NULL;
	return false;
}


void MRTSupport::onSelectPSVersionCap(int psVerCap)
{	
	if (psVerCap < 3 && s_mrtSetting_->activeOption()==0)
		s_mrtSetting_->selectOption(1); //disable
}


bool MRTSupport::isEnabled()
{
	return s_mrtSetting_->activeOption() == 0;
}

void MRTSupport::bind()
{
	if ( !MRTSupport::isEnabled() )
	{
		return;
	}

	MF_ASSERT_DEV( !bound_ )

	//save the current render target state, this will keep a copy of RT2
	//if one is currently set (if not, someone else has pushRenderTarget'd
	//already, so it is still saved)
	rc().pushRenderTarget();

	//unbind RT2 from the render context, as we are now allowing access
	//to it via samplers.
	rc().setRenderTarget(1,NULL);

	mapSetter_->map( Moo::rc().secondRenderTargetTexture().pComObject() );

	bound_ = true;
}


void MRTSupport::unbind()
{
	if ( !MRTSupport::isEnabled() )
	{
		return;
	}

	MF_ASSERT_DEV( bound_ )

	mapSetter_->map( NULL );
	Moo::rc().popRenderTarget();
	bound_ = false;
}


}	//namespace Moo