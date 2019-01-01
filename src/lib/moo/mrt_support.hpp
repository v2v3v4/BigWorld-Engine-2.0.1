/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MRT_SUPPORT_HPP
#define MRT_SUPPORT_HPP

#include "effect_manager.hpp"
#include "managed_effect.hpp"
#include "cstdmf/init_singleton.hpp"

namespace Moo
{

/**
 *	MRT management.
 */
class MRTSupport : private Moo::EffectManager::IListener, public InitSingleton<MRTSupport>
{
public:
	MRTSupport();
	bool isEnabled();
	void bind();
	void unbind();
	bool bound() const	{ return bound_; }

	//InitSingleton methods
	bool doInit();
	bool doFini();

	//IListener methods
	void onSelectPSVersionCap(int psVerCap);
private:
	/**
	 * Texture setter is an effect constant binding that also holds a reference
	 * to a texture.
	 */
	class TextureSetter : public Moo::EffectConstantValue
	{
	public:
		bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle);
		void map(DX::BaseTexture* pTexture );
		ComObjectWrap<DX::BaseTexture> map();
	private:
		ComObjectWrap<DX::BaseTexture> map_;
	};

	static void configureKeywordSetting(Moo::EffectMacroSetting & setting);
	static Moo::EffectMacroSetting::EffectMacroSettingPtr s_mrtSetting_;
	SmartPointer<TextureSetter> mapSetter_;
	bool bound_;
};

}	//namespace

#endif	//MRT_SUPPORT_HPP
