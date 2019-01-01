/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATIC_LIGHTING_HPP
#define STATIC_LIGHTING_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_space.hpp"
#include <set>


namespace StaticLighting
{

	/** Mark the chuck as dirty, so static lighting will be recalculated */
	void markChunk( Chunk* chunk );

	BoundingBox	lightingBoundingBox( const BoundingBox& chunkBB );

	/**
	 * Mark all the chunks the light influences as dirty
	 *
	 * LightType must be a pointer to a class with an intersects( BoundingBox )
	 * method, which both SpotLight and OmniLight have
	 */
	template<class LightType>
	void markChunks( Chunk* srcChunk, LightType light,
		std::set<Chunk*>& markedChunks = std::set<Chunk*>(),
		uint32 currentDepth = 0 )
	{
		markedChunks.insert( srcChunk );

		markChunk( srcChunk );

		// Stop if we've reached the maximum portal traversal depth
		if (currentDepth == srcChunk->space()->staticLightingPortalDepth())
			return;

		for (Chunk::piterator pit = srcChunk->pbegin(); pit != srcChunk->pend(); pit++)
		{
			if (!pit->hasChunk() || !pit->pChunk->isBound())
				continue;

			// Don't mark outside chunks
			if (!srcChunk->space()->staticLightingOutside() && 
				pit->pChunk->isOutsideChunk())
				continue;

			// We've already marked it, skip
			if (markedChunks.find( pit->pChunk ) != markedChunks.end())
				continue;

			if (!light->intersects( lightingBoundingBox(pit->pChunk->boundingBox()) ))
				continue;

			// TODO: Check that we can actually see the portal. Only really of value if
			// we get lights going across > 2 chunks.

			markChunks( pit->pChunk, light, markedChunks, currentDepth + 1 );
		}

	}

	class StaticLightContainer;

	/**
	 * Find all lights influencing forChunk, enabling it's lighting to be
	 * recalculated
	 */
	bool findLightsInfluencing( Chunk* forChunk, Chunk* inChunk,
							StaticLightContainer* lights,
							std::set<Chunk*>& searchedChunks = std::set<Chunk*>(),
							int currentDepth = 0);


	/**
	 * It will generate the volume for soft shadow for
	 * the given light position. It will also take collision
	 * scene into account.
	 */
	class LightVolume
	{
	public:
		LightVolume( const Vector3& position );

		Vector3 volume_[6];
	};

	class StaticLightContainer
	{
	public:
		StaticLightContainer();

		/** Add all lights from from */
		void addLights( StaticLightContainer* from );
		/** Add all lights in from, provided they can influence what's in bb */
		void addLights( StaticLightContainer* from, const BoundingBox& bb );

		void ambient( Moo::Colour colour )	{ ambient_ = colour; }
		Moo::Colour ambient()				{ return ambient_; };

		std::vector< Moo::DirectionalLightPtr >& directionals()
			{ return directionalLights_; }
		void					addLight( Moo::DirectionalLightPtr pDirectional )
			{ directionalLights_.push_back( pDirectional ); }
		void					removeLight( Moo::DirectionalLightPtr pDirectional );

		std::vector< Moo::OmniLightPtr >& omnis()
			{ return omniLights_; }
		void					addLight( Moo::OmniLightPtr pOmni )
			{ omniLights_.push_back( pOmni ); }
		void					removeLight( Moo::OmniLightPtr pOmni );

		std::vector< Moo::SpotLightPtr >& spots()
			{ return spotLights_; }
		void					addLight( Moo::SpotLightPtr pSpot )
			{ spotLights_.push_back( pSpot ); }
		void					removeLight( Moo::SpotLightPtr pSpot );

		/** If there are any lights in the container */
		bool empty();

		void rebuildLightVolumes();

		D3DCOLOR calcLight( const Vector3& vertexPos, const Vector3& vertexNormal );

	private:
		Moo::Colour				ambient_;
		std::vector< Moo::DirectionalLightPtr >	directionalLights_;
		std::vector< Moo::OmniLightPtr > omniLights_;
		std::vector< Moo::SpotLightPtr > spotLights_;
		std::vector< LightVolume > omniLightVolumes_;
		std::vector< LightVolume > spotLightVolumes_;
	};

	class StaticChunkLightCache : public ChunkCache
	{
	public:
		StaticChunkLightCache( Chunk & chunk );

		static void touch( Chunk & chunk );

		StaticLightContainer*	lights()		{ return &lights_; }

		/** Mark all chunks this set of lights influence as dirty */
		void markInfluencedChunksDirty();

		static Instance<StaticChunkLightCache>	instance;
	private:
		StaticLightContainer lights_;

		Chunk & chunk_;
	};

}


#endif // STATIC_LIGHTING_HPP
