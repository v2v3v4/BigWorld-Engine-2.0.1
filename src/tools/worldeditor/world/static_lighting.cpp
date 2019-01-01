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
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "math/colour.hpp"

using namespace StaticLighting;


// -----------------------------------------------------------------------------
// Section: markChunk
// -----------------------------------------------------------------------------

void StaticLighting::markChunk( Chunk* chunk )
{
	WorldManager::instance().dirtyLighting( chunk );
}

// -----------------------------------------------------------------------------
// Section: findLightsInfluencing
// -----------------------------------------------------------------------------

bool StaticLighting::findLightsInfluencing( Chunk* forChunk, Chunk* inChunk,
						   StaticLighting::StaticLightContainer* lights,
						   std::set<Chunk*>& searchedChunks,
						   int currentDepth)
{
	BW_GUARD;

	// Add all the lights in inChunk
	StaticLightContainer* currentLights = StaticChunkLightCache::instance( *inChunk ).lights();

	if (forChunk == inChunk)
	{
		// Adding our own lights, just add em all
		lights->addLights( currentLights );
	}
	else
	{
		// Adding someone elses lights, check they can reach forChunk first
		lights->addLights( currentLights, lightingBoundingBox(forChunk->boundingBox()) );
	}

	// Mark that we've now done inChunk
	searchedChunks.insert( inChunk );

	// If we're up to our max portal traversal count, don't search through the
	// connected portals
	if (currentDepth == forChunk->space()->staticLightingPortalDepth())
		return true;

	// Call for each of our connected chunks that havn't yet been searched
	for (Chunk::piterator pit = inChunk->pbegin(); pit != inChunk->pend(); pit++)
	{
		if (!pit->hasChunk())
			continue;

		if (!pit->pChunk->isBound())
			return false;

		// We've already marked it, skip
		if (searchedChunks.find( pit->pChunk ) != searchedChunks.end())
			continue;

		if (!findLightsInfluencing( forChunk, pit->pChunk, lights, searchedChunks, currentDepth + 1 ))
			return false;
	}

	return true;
}


// -----------------------------------------------------------------------------
// Section: StaticLightContainer
// -----------------------------------------------------------------------------

StaticLightContainer::StaticLightContainer() : ambient_( D3DCOLOR( 0x00000000 ) )
{
}

void StaticLightContainer::addLights( StaticLightContainer* from )
{
	BW_GUARD;

	directionalLights_.insert( directionalLights_.end(),
		from->directionalLights_.begin(), from->directionalLights_.end() );

	omniLights_.insert( omniLights_.end(),
		from->omniLights_.begin(), from->omniLights_.end() );

	spotLights_.insert( spotLights_.end(),
		from->spotLights_.begin(), from->spotLights_.end() );
}



/**
 * Functor to add a light if it intersects the boundingbox
 */
template<class LightType>
class LightInserter : public std::unary_function<LightType, void>
{
private:
	const BoundingBox& bb_;
	std::vector<LightType>& lights_;
public:
	LightInserter( std::vector<LightType>& lights, const BoundingBox& bb )
		: lights_( lights ), bb_( bb )
	{}

    void operator() (LightType l)
	{
		BW_GUARD;

		if (l->intersects( bb_ ))
			lights_.push_back( l );
	}
};


void StaticLightContainer::addLights( StaticLightContainer* from, const BoundingBox& bb )
{
	BW_GUARD;

	directionalLights_.insert( directionalLights_.end(),
		from->directionalLights_.begin(), from->directionalLights_.end() );

    LightInserter<Moo::OmniLightPtr> omniInserter( omniLights_, bb );
	std::for_each( from->omniLights_.begin(), from->omniLights_.end(), omniInserter);

    LightInserter<Moo::SpotLightPtr> spotInserter( spotLights_, bb );
	std::for_each( from->spotLights_.begin(), from->spotLights_.end(), spotInserter);
}

void StaticLightContainer::removeLight( Moo::DirectionalLightPtr pDirectional )
{
	BW_GUARD;

	DirectionalLightVector::iterator i = std::find( directionalLights_.begin(),
		directionalLights_.end(),
		pDirectional );

	if (i != directionalLights_.end())
		directionalLights_.erase( i );
}

void StaticLightContainer::removeLight( Moo::OmniLightPtr pOmni )
{
	BW_GUARD;

	OmniLightVector::iterator i = std::find( omniLights_.begin(),
		omniLights_.end(),
		pOmni );

	if (i != omniLights_.end())
		omniLights_.erase( i );
}

void StaticLightContainer::removeLight( Moo::SpotLightPtr pSpot )
{
	BW_GUARD;

	SpotLightVector::iterator i = std::find( spotLights_.begin(),
		spotLights_.end(),
		pSpot );

	if (i != spotLights_.end())
		spotLights_.erase( i );
}

bool StaticLightContainer::empty()
{
	return (directionalLights_.empty() &&
		spotLights_.empty() &&
		omniLights_.empty() && 
		ambient_ == 0x00000000);
}


namespace
{

static Vector3 x(2.f, 0.f, 0.f);
static Vector3 y(0.f, 2.f, 0.f);
static Vector3 z(0.f, 0.f, 2.f);


/** Simple collision callback to find out if point a is visible from point b */
class VisibilityCollision : public CollisionCallback
{
public:
	VisibilityCollision() : gotone_( false ) { }

	int operator()( const ChunkObstacle & co,
		const WorldTriangle & hitTriangle, float dist )
	{
		BW_GUARD;

		if (!co.pItem()->pOwnSect() ||
			( co.pItem()->pOwnSect()->sectionName() != "model" &&
			 co.pItem()->pOwnSect()->sectionName() != "speedtree" &&
			co.pItem()->pOwnSect()->sectionName() != "shell" ))
		{
			return COLLIDE_ALL;
		}

		// if it's not transparent, we can stop now
		if (!hitTriangle.isTransparent() && co.pItem()->edAffectShadow()) 
		{ 
			gotone_ = true; 
			return COLLIDE_STOP; 
		}

		// otherwise we have to keep on going
		return COLLIDE_ALL;
	}

	bool gotone()			{ return gotone_; }
private:
	bool gotone_;
};


/** Simple collision callback to clip the target point to the source point
 using collision scene */
class ClosestObstacleCollision : public CollisionCallback
{
public:
	ClosestObstacleCollision() : gotone_( false ) { }

	int operator()( const ChunkObstacle & co,
		const WorldTriangle & hitTriangle, float dist )
	{
		BW_GUARD;

		if (!co.pItem()->pOwnSect() ||
			( co.pItem()->pOwnSect()->sectionName() != "model" &&
			 co.pItem()->pOwnSect()->sectionName() != "speedtree" &&
			co.pItem()->pOwnSect()->sectionName() != "shell" ))
		{
			return COLLIDE_ALL;
		}

		// if it's not transparent, we can stop now
		if (!hitTriangle.isTransparent() && co.pItem()->edAffectShadow()) 
		{ 
			gotone_ = true; 
			dist_ = dist;
			return COLLIDE_BEFORE; 
		}

		// otherwise we have to keep on going
		return COLLIDE_ALL;
	}

	bool gotone()			{ return gotone_; }
	float dist() const		{ return dist_; }
private:
	bool gotone_;
	float dist_;
};


D3DCOLOR combineColours( D3DCOLOR a, D3DCOLOR b )
{
	return Colour::getUint32(Colour::getVector4( a ) + Colour::getVector4( b ));
}


bool isVisibleFrom( Vector3 vertex, Vector3 light )
{
	// check the vertex is visible from the light
	BW_GUARD;

	VisibilityCollision v;
	ChunkManager::instance().cameraSpace()->collide( 
		vertex,
		light,
		v );

	return !v.gotone();
}


Vector3 clipPoint( const Vector3& src, Vector3 dest )
{
	BW_GUARD;

	ClosestObstacleCollision c;
	ChunkManager::instance().cameraSpace()->collide( 
		src,
		dest,
		c );

	if (c.gotone())
	{
		Vector3 normal = dest - src;
		normal.normalise();
		dest = src + normal * c.dist() * 0.99f;
	}

	return dest;
}

}

LightVolume::LightVolume( const Vector3& position )
{
	BW_GUARD;

	volume_[ 0 ] = clipPoint( position, position + x );
	volume_[ 1 ] = clipPoint( position, position - x );
	volume_[ 2 ] = clipPoint( position, position + y );
	volume_[ 3 ] = clipPoint( position, position - y );
	volume_[ 4 ] = clipPoint( position, position + z );
	volume_[ 5 ] = clipPoint( position, position - z );
}


void StaticLightContainer::rebuildLightVolumes()
{
	BW_GUARD;

	omniLightVolumes_.clear();
	spotLightVolumes_.clear();

	for (std::vector< Moo::OmniLightPtr >::iterator iter = omniLights_.begin();
		iter != omniLights_.end(); ++iter)
	{
		omniLightVolumes_.push_back( LightVolume( (*iter)->worldPosition() ) );
	}

	for (std::vector< Moo::SpotLightPtr >::iterator iter = spotLights_.begin();
		iter != spotLights_.end(); ++iter)
	{
		spotLightVolumes_.push_back( LightVolume( (*iter)->worldPosition() ) );
	}
}


D3DCOLOR StaticLightContainer::calcLight( const Vector3& vertexPos, const Vector3& vertexNormal )
{
	BW_GUARD;

	D3DCOLOR colour = ambient();;

	for (uint32 i = 0; i < omniLights_.size(); ++i)
	{
		Moo::OmniLightPtr omni = omniLights_[ i ];

		Vector3 dirToLight = omni->worldPosition() - vertexPos;
		dirToLight.normalise();

		// early out for back facing tris
		float dot = dirToLight.dotProduct( vertexNormal );

		if (dot <= 0.f)
		{
			continue;
		}

		// early out for out of range vertices
		float maxRadiusSq = omni->outerRadius() * omni->outerRadius();

		if ((vertexPos - omni->worldPosition()).lengthSquared() > maxRadiusSq)
		{
			continue;
		}

		// Offset the vertex 50cm towards the light, so we don't collide with ourself
		// 10cm causes too many shadows on small things
		Vector3 vert = vertexPos + (dirToLight * 0.5);

		float vis = 1.f;

		if (!isVisibleFrom( vert, omni->worldPosition() ))
		{
			if (isVisibleFrom( vert, omniLightVolumes_[i].volume_[0] ) ||
				isVisibleFrom( vert, omniLightVolumes_[i].volume_[1] ) ||
				isVisibleFrom( vert, omniLightVolumes_[i].volume_[2] ) ||
				isVisibleFrom( vert, omniLightVolumes_[i].volume_[3] ) ||
				isVisibleFrom( vert, omniLightVolumes_[i].volume_[4] ) ||
				isVisibleFrom( vert, omniLightVolumes_[i].volume_[5] )
				)
				vis = 0.5f;
			else
				continue;
		}

		// Add the colour to the vertex
		if (dot > 1.f)
			dot = 1.f;

		float dist = (vertexPos - omni->worldPosition()).length();

		if (dist < omni->innerRadius())
		{
			colour = combineColours( colour, omni->colour() * dot * vis * omni->multiplier() );
		}
		else if (dist < omni->outerRadius())
		{
			float falloff = (dist - omni->innerRadius()) / (omni->outerRadius() - omni->innerRadius());
			colour = combineColours( colour, omni->colour() * (1 - falloff) * dot * vis * omni->multiplier() );
		}
	}

	for (uint32 i = 0; i < spotLights_.size(); ++i)
	{
		Moo::SpotLightPtr spot = spotLights_[ i ];

		Vector3 dirToLight = spot->worldPosition() - vertexPos;
		dirToLight.normalise();

		// early out for back facing tris
		float dot = (-spot->worldDirection()).dotProduct( vertexNormal );
		if (dot <= 0.f)
			continue;

		// early out for out of range vertices
		float maxRadiusSq = spot->outerRadius() * spot->outerRadius();
		if ((vertexPos - spot->worldPosition()).lengthSquared() > maxRadiusSq)
			continue;

		float cosAngle = (-spot->worldDirection()).dotProduct( dirToLight );

		if (cosAngle <= spot->cosConeAngle())
			continue;

		// Offset the vertex 50cm towards the light, so we don't collide with ourself
		// 10cm causes too many shadows on small things
		Vector3 vert = vertexPos + (dirToLight * 0.5);

		float vis = 1.f;

		if (!isVisibleFrom( vert, spot->worldPosition() ))
		{
			if (isVisibleFrom( vert, spotLightVolumes_[i].volume_[0] ) ||
				isVisibleFrom( vert, spotLightVolumes_[i].volume_[1] ) ||
				isVisibleFrom( vert, spotLightVolumes_[i].volume_[2] ) ||
				isVisibleFrom( vert, spotLightVolumes_[i].volume_[3] ) ||
				isVisibleFrom( vert, spotLightVolumes_[i].volume_[4] ) ||
				isVisibleFrom( vert, spotLightVolumes_[i].volume_[5] )
				)
				vis = 0.5f;
			else
				continue;
		}

		float coneFalloff = (cosAngle - spot->cosConeAngle()) / (1.f - spot->cosConeAngle());

		float dist = (vertexPos - spot->worldPosition()).length();
		if (dist < spot->innerRadius())
		{
			colour = combineColours( colour, spot->colour() * coneFalloff * dot * vis * spot->multiplier() );
		}
		else if (dist < spot->outerRadius())
		{
			float falloff = (dist - spot->innerRadius()) / (spot->outerRadius() - spot->innerRadius());
			colour = combineColours( colour, spot->colour() * (1 - falloff) * coneFalloff * dot * vis * spot->multiplier() );
		}
	}

	return colour;
}


// -----------------------------------------------------------------------------
// Section: StaticChunkLightCache
// -----------------------------------------------------------------------------

StaticChunkLightCache::StaticChunkLightCache( Chunk & chunk ) : chunk_( chunk )
{
}

void StaticChunkLightCache::touch( Chunk & chunk )
{
	BW_GUARD;

	StaticChunkLightCache::instance( chunk );
}

/** Functor to mark chunks as dirty */
template<class LightType>
class ChunkMarker : public std::unary_function<LightType, void>
{
private:
	Chunk* chunk_;
public:
	ChunkMarker( Chunk* chunk ) : chunk_( chunk ) {}

	void operator() (LightType l)
	{
		BW_GUARD;

		markChunks( chunk_, l );
	}
};

void StaticChunkLightCache::markInfluencedChunksDirty()
{
	BW_GUARD;

	markChunk( &chunk_ );

    ChunkMarker<Moo::OmniLightPtr> omniMarker( &chunk_ );
	std::for_each( lights()->omnis().begin(), lights()->omnis().end(), omniMarker);

    ChunkMarker<Moo::SpotLightPtr> spotMarker( &chunk_ );
	std::for_each( lights()->spots().begin(), lights()->spots().end(), spotMarker);
}


/// Static instance accessor initialiser
ChunkCache::Instance<StaticChunkLightCache> StaticChunkLightCache::instance;


// Get the biggest possible bounding box for a chunk and its items
BoundingBox	StaticLighting::lightingBoundingBox( const BoundingBox& chunkBB )
{
	BW_GUARD;

	BoundingBox bb = chunkBB;
	const float HALF_GRID_RESOLUTION = GRID_RESOLUTION * 0.5f;
	bb.expandSymmetrically( HALF_GRID_RESOLUTION, HALF_GRID_RESOLUTION, HALF_GRID_RESOLUTION );

	return bb;
}
