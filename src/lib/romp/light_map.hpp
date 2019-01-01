/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIGHTMAP_HPP
#define LIGHTMAP_HPP

#include "moo/forward_declarations.hpp"
#include "moo/render_target.hpp"
#include "romp/texture_feeds.hpp"

namespace Moo
{	
	class EffectMaterial;
}

/**
 *	This class defines a render target based light map,
 *	and the interface to set vertex shader constants
 */
class LightMap : public Moo::DeviceCallback
{
public:
	LightMap( const std::string& className );
	virtual ~LightMap();
	//last update time
	float lastTime()	{ return lastTime_; }	
	void orthogonalProjection(float xExtent, float zExtent, Matrix& ret);

	void createUnmanagedObjects();
	void deleteUnmanagedObjects();

	virtual void activate();
	virtual void deactivate();
protected:
	virtual bool init(const DataSectionPtr pSection);	
	bool needsUpdate(float gameTime);

	virtual void createLightMapSetter( const std::string& textureFeedName );
	virtual void createTransformSetter() = 0;

	void linkTextures();
	void deLinkTextures();

	//update due to time tolerance
	float lastTime_;
	float timeTolerance_;

	//render target variables
	int	width_;
	int	height_;	
	Moo::RenderTargetPtr pRT_;
	bool active_;

	//identifiers
	std::string renderTargetName_;
	std::string effectTextureName_;
	std::string effectTransformName_;

	Moo::EffectConstantValuePtr transformSetter_;
	Moo::EffectConstantValuePtr lightMapSetter_;
	PyTextureProviderPtr textureProvider_;
};


/**
 *	This class implements an effect-based light map.
 */
class EffectLightMap : public LightMap
{
public:
	void setLightMapProjection(const Matrix& m);
protected:
	EffectLightMap( const std::string& className );
	bool init(const DataSectionPtr pSection);	

	Moo::EffectMaterialPtr		material_;
};


#endif