/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_VISUAL_CONTEXT_HPP
#define EFFECT_VISUAL_CONTEXT_HPP

#include "cstdmf/singleton.hpp"
#include "visual.hpp"

namespace Moo
{

/**
 * This class is a helper class for setting up automatic constants while rendering
 * a visual using effect files. The automatic constants are referenced by using
 * the d3dx effect file semantics.
 */
class EffectVisualContext : public Singleton< EffectVisualContext >
{
public:
	EffectVisualContext();
	~EffectVisualContext();

	void initConstants();

	void pRenderSet( Visual::RenderSet* pRenderSet );
	Visual::RenderSet* pRenderSet() const { return pRenderSet_; }
	const Matrix& invWorld() { return invWorld_; }
	float invWorldScale() { return invWorldScale_; }
	DX::CubeTexture* pNormalisationMap() { return pNormalisationMap_.pComObject(); };
	void tick( float dTime );
	void staticLighting( bool value ) { staticLighting_ = value; }
	bool staticLighting( ) const { return staticLighting_; }
	void isOutside( bool value ) { isOutside_ = value; }
	bool isOutside( ) const { return isOutside_; }
	
	void overrideConstants(bool value)	{ overrideConstants_ = value; }
	bool overrideConstants() const { return overrideConstants_; }

private:
	Visual::RenderSet*		pRenderSet_;
	Matrix					invWorld_;
	float					invWorldScale_;
	bool					staticLighting_;
	bool					isOutside_;
	bool					overrideConstants_;
	bool					inited_;
	ComObjectWrap<DX::CubeTexture> pNormalisationMap_;
	void createNormalisationMap();

	typedef std::map< EffectConstantValuePtr*, EffectConstantValuePtr > ConstantMappings;
	ConstantMappings constantMappings_;
};


/**
 *	Helper class for automatic renderset setup / removal.
 */
class EffectVisualContextSetter
{
public:
	EffectVisualContextSetter( Visual::RenderSet* pRenderSet )
	{
		EffectVisualContext::instance().pRenderSet( pRenderSet );
	}
	~EffectVisualContextSetter()
	{
		EffectVisualContext::instance().pRenderSet( NULL );
	}
};

}





#endif // EFFECT_VISUAL_CONTEXT_HPP
