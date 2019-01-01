/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __TERRAIN_BLENDS_HPP__
#define __TERRAIN_BLENDS_HPP__

#ifndef MF_SERVER
#include "moo/com_object_wrap.hpp"
#include "moo/moo_dx.hpp"
#include "resource.hpp"
#include "dominant_texture_map2.hpp"
#include "terrain_renderer2.hpp"

namespace Terrain
{
	class TerrainBlendsResource;
	class TerrainBlock2;
	class TerrainSettings;
	
	typedef SmartPointer<TerrainBlendsResource> TerrainBlendsResourcePtr;
	typedef SmartPointer<DominantTextureMap2>	DominantTextureMap2Ptr;
	typedef SmartPointer<TerrainSettings>		TerrainSettingsPtr;

	struct CombinedLayer
	{
		CombinedLayer();

		uint32						width_;
		uint32						height_;
		ComObjectWrap<DX::Texture>	pBlendTexture_;
		TextureLayers				textureLayers_;
		bool						smallBlended_;
	};

	struct TerrainBlends : SafeReferenceCount
	{	
		TerrainBlends();
		virtual ~TerrainBlends();
		
		bool init( TerrainBlock2& owner );
		void createCombinedLayers( bool compressTextures,
									 DominantTextureMap2Ptr* newDominantTexture);
		
		TextureLayers			    textureLayers_;
		std::vector<CombinedLayer>  combinedLayers_;    	
	};

	class TerrainBlendsResource : public Resource< TerrainBlends >
	{
	public:
		TerrainBlendsResource( TerrainBlock2& owner );
		
		// Specialise evaluate and loading methods
		inline ResourceRequired		evaluate( uint8 renderTextureMask );
		virtual bool				load();

		virtual void preAsyncLoad();
		virtual void postAsyncLoad();

#ifdef EDITOR_ENABLED
		virtual bool rebuild(	bool compressTextures, 
								DominantTextureMap2Ptr* newDominantTexture );
		virtual void unload();
		virtual ResourceState		getState();
		virtual ObjectTypePtr		getObject();
#endif

	protected:
		virtual void				startAsyncTask();

		TerrainBlock2&				owner_;
	};

	inline ResourceRequired TerrainBlendsResource::evaluate( 
													uint8 renderTextureMask )
	{
		required_ = RR_No;

		if (renderTextureMask & 
			(TerrainRenderer2::RTM_PreLoadBlend | TerrainRenderer2::RTM_DrawBlend))
		{
			required_ = RR_Yes;
		}

		return required_;
	}
}

#endif // MF_SERVER

#endif
