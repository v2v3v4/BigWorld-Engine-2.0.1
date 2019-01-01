/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_LIGHT_HPP
#define EDITOR_CHUNK_LIGHT_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "chunk/chunk_light.hpp"


/**
 * We extend Aligned here, as all the editor chunk lights have Matrix members
 */
template<class BaseLight>
class EditorChunkLight : public EditorChunkSubstance<BaseLight>, public Aligned
{
public:
	virtual void edPreDelete()
	{
		BW_GUARD;

		markInfluencedChunks();
		EditorChunkSubstance<BaseLight>::edPreDelete();
	}

	virtual void edPostCreate()
	{
		BW_GUARD;

		markInfluencedChunks();
		EditorChunkSubstance<BaseLight>::edPostCreate();
	}

	virtual bool edShouldDraw()
	{
		BW_GUARD;

		// Override EditorChunkSubstance's method since it's tied to 'Scenery'.
		return BaseLight::edShouldDraw();
	}

	virtual void markInfluencedChunks()
	{
		BW_GUARD;

		if (pChunk_)
			StaticLighting::markChunk( pChunk_ );
	}

	virtual bool load( DataSectionPtr pSection )
	{
		BW_GUARD;

		if (EditorChunkSubstance<BaseLight>::load( pSection ))
		{
			loadModel();
			return true;
		}
		else
		{
			return false;
		}
	}
	
	virtual void draw()
	{
		BW_GUARD;

		if (!edIsTooDistant() || WorldManager::instance().drawSelection())
		{
			// Draw the light proxy if it's not too far away from teh camera.
			EditorChunkSubstance<BaseLight>::draw();
		}
	}

	virtual bool usesLargeProxy() const = 0;

	ModelPtr reprModel() const 
	{ 
		BW_GUARD;

		if (usesLargeProxy())
		{
			return model_; 
		}
		else
		{
			return modelSmall_; 
		}

		return NULL;
	}
	
	virtual void syncInit();

	virtual bool edIsSnappable() { return false; }

	virtual std::string edAssetName() const { return sectName(); }

protected:
	ModelPtr model_;
	ModelPtr modelSmall_;
	virtual void loadModel() = 0;
	Matrix	transform_;
};


/**
 * A ChunkLight containing a light that moo knows about, ie everything but
 * ambient
 */
template<class BaseLight>
class EditorChunkMooLight : public EditorChunkLight<BaseLight>
{
public:
	EditorChunkMooLight() : staticLight_( true ) {}

	virtual void toss( Chunk * pChunk )
	{
		BW_GUARD;

		if (pChunk_ != NULL)
		{
			StaticLighting::StaticChunkLightCache & clc =
				StaticLighting::StaticChunkLightCache::instance( *pChunk_ );

			clc.lights()->removeLight( pLight_ );
		}

		this->EditorChunkSubstance<BaseLight>::toss( pChunk );

		if (pChunk_ != NULL)
		{
			StaticLighting::StaticChunkLightCache & clc =
				StaticLighting::StaticChunkLightCache::instance( *pChunk_ );

			if (staticLight())
				clc.lights()->addLight( pLight_ );
		}
	}

	virtual bool load( DataSectionPtr pSection )
	{
		BW_GUARD;

		staticLight_ = pSection->readBool( "static", true );
		return EditorChunkLight<BaseLight>::load( pSection );
	}

	virtual bool edShouldDraw()
	{
		BW_GUARD;

		if( !EditorChunkLight<BaseLight>::edShouldDraw() )
			return false;
		if (!OptionsLightProxies::visible())
			return false;

		bool drawStatic = OptionsLightProxies::staticVisible();
		if (drawStatic && staticLight())
			return true;

		bool drawDynamic = OptionsLightProxies::dynamicVisible();
		if (drawDynamic && dynamicLight())
			return true;

		bool drawSpecular = OptionsLightProxies::specularVisible();
		if (drawSpecular && specularLight())
			return true;

		if( drawStatic && drawDynamic && drawSpecular && 
			!staticLight() && !dynamicLight() && !specularLight() )
			return true;

		return false;
	}

	void staticLight( const bool s )
	{
		BW_GUARD;

		if (s != staticLight_)
		{
			if (pChunk_)
			{
				StaticLighting::StaticChunkLightCache & clc =
					StaticLighting::StaticChunkLightCache::instance( *pChunk_ );

				if (s)
					clc.lights()->addLight( pLight_ );
				else
					clc.lights()->removeLight( pLight_ );

				markInfluencedChunks();
			}

			staticLight_ = s;
		}
	}

	bool staticLight() const	{ return staticLight_; }

	// Get and Set functions for AccessorDataProxy
	bool staticLightGet() const				{ return staticLight(); }
	bool staticLightSet( const bool& b )	{ staticLight( b ); loadModel(); return true; }

	bool dynamicLightGet() const			{ return dynamicLight(); }
	bool dynamicLightSet( const bool& b )	{ dynamicLight( b ); loadModel(); return true; }

	bool specularLightGet() const			{ return specularLight(); }
	bool specularLightSet( const bool& b )	{ specularLight( b ); loadModel(); return true; }
protected:
	bool	staticLight_;
};


/**
 * A ChunkLight containing a light that moo knows about, and exists somewhere
 * in the world, ie, neither ambient nor directional
 */
template<class BaseLight>
class EditorChunkPhysicalMooLight : public EditorChunkMooLight<BaseLight>
{
public:
	virtual void markInfluencedChunks()
	{
		BW_GUARD;

		if (pChunk_)
			StaticLighting::markChunks( pChunk_, pLight_ );
	}
};


/**
 *	This class is the editor version of a chunk directional light
 */
class EditorChunkDirectionalLight :
	public EditorChunkMooLight<ChunkDirectionalLight>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkDirectionalLight )
public:

	virtual bool load( DataSectionPtr pSection );
	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edEdit( class GeneralEditor & editor );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual const char * sectName() const { return "directionalLight"; }
	virtual const char * drawFlag() const { return "render/drawChunkLights"; }

	virtual bool usesLargeProxy() const { return false; }
	
	Moo::DirectionalLightPtr	pLight()	{ return pLight_; }

	float getMultiplier() const { return pLight_->multiplier(); }
	bool setMultiplier( const float& m ) { pLight_->multiplier(m); markInfluencedChunks(); return true; }

protected:
	virtual void loadModel();

};


/**
 *	This class is the editor version of a chunk omni light
 */
class EditorChunkOmniLight : public EditorChunkPhysicalMooLight<ChunkOmniLight>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkOmniLight )
public:

	virtual bool load( DataSectionPtr pSection );
	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edEdit( class GeneralEditor & editor );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual const char * sectName() const { return "omniLight"; }
	virtual const char * drawFlag() const { return "render/drawChunkLights"; }

	virtual bool usesLargeProxy() const;
	
	Moo::OmniLightPtr	pLight()	{ return pLight_; }

	float getMultiplier() const { return pLight_->multiplier(); }
	bool setMultiplier( const float& m ) { pLight_->multiplier(m); markInfluencedChunks(); return true; }

	bool setPriority( const int& priority ) { pLight_->priority(priority); markInfluencedChunks(); return true; }
	int getPriority() const { return pLight_->priority(); }

protected:
	virtual void loadModel();

};


/**
 *	This class is the editor version of a chunk spot light
 */
class EditorChunkSpotLight : public EditorChunkPhysicalMooLight<ChunkSpotLight>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkSpotLight )
public:

	virtual bool load( DataSectionPtr pSection );
	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edEdit( class GeneralEditor & editor );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual const char * sectName() const { return "spotLight"; }
	virtual const char * drawFlag() const { return "render/drawChunkLights"; }

	virtual bool usesLargeProxy() const;
	
	Moo::SpotLightPtr	pLight()	{ return pLight_; }

	float getMultiplier() const { return pLight_->multiplier(); }
	bool setMultiplier( const float& m ) { pLight_->multiplier(m); markInfluencedChunks(); return true; }

	bool setPriority( const int& priority ) { pLight_->priority(priority); markInfluencedChunks(); return true; }
	int getPriority() const { return pLight_->priority(); }

	virtual bool edShouldDraw();

protected:
	virtual void loadModel();

};


/**
 *	This class is the editor version of a chunk omni light
 */
class EditorChunkPulseLight : public EditorChunkPhysicalMooLight<ChunkPulseLight>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkPulseLight )
public:
	typedef std::vector<Vector2> Frames;

	virtual bool load( DataSectionPtr pSection );
	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edEdit( class GeneralEditor & editor );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual const char * sectName() const { return "pulseLight"; }
	virtual const char * drawFlag() const { return "render/drawChunkLights"; }

	virtual bool usesLargeProxy() const;
	
	Moo::OmniLightPtr	pLight()	{ return pLight_; }

	float getMultiplier() const { return pLight_->multiplier(); }
	bool setMultiplier( const float& m ) { pLight_->multiplier(m); markInfluencedChunks(); return true; }

	bool setPriority( const int& priority ) { pLight_->priority(priority); markInfluencedChunks(); return true; }
	int getPriority() const { return pLight_->priority(); }

	float getTimeScale() const	{ return timeScale_; }
	bool setTimeScale( const float& timeScale );

	float getDuration() const	{ return duration_; }
	bool setDuration( const float& duration );

	std::string getAnimation() const	{ return animation_; }
	bool setAnimation( const std::string& animation );

	Frames& getFrames()	{ return frames_;	}
	bool setFrames( const Frames& frames );

	void onFrameChanged();

	virtual bool edShouldDraw();

protected:
	virtual void loadModel();

	float timeScale_;
	float duration_;
	std::string animation_;
	Frames frames_;
};


/**
 *	This class is the editor version of a chunk ambient light
 */

class EditorChunkAmbientLight :
	public EditorChunkLight<ChunkAmbientLight>
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkAmbientLight )
public:

	bool load( DataSectionPtr pSection );
	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edEdit( class GeneralEditor & editor );

	virtual const Matrix & edTransform();
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual const char * sectName() const { return "ambientLight"; }
	virtual const char * drawFlag() const { return "render/drawChunkLights"; }

	virtual bool usesLargeProxy() const;
	
	EditorChunkAmbientLight *	pLight()	{ return this; }

	const Moo::Colour & colour() const			{ return colour_; }
	void colour( const Moo::Colour & c );

	virtual void toss( Chunk * pChunk );

	float getMultiplier() const { return multiplier(); }
	bool setMultiplier( const float& m );

	virtual bool edShouldDraw();

protected:
	virtual void loadModel();

};


#endif // EDITOR_CHUNK_LIGHT_HPP
