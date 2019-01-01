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
#include "terrain_settings.hpp"
#include "resmgr/auto_config.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "cstdmf/watcher.hpp"
#include "manager.hpp"

#ifndef MF_SERVER
#include "terrain1/terrain_renderer1.hpp"
#include "terrain2/terrain_renderer2.hpp"
#include "terrain_graphics_options.hpp"
#endif

using namespace Terrain;

namespace
{
	// This function returns the smallest n such that 2^n >= x.
	uint32 log2RoundUp(uint32 x)
	{
		uint32 result = 0;
		while ((x >> result) > 1)
			result++;
		return result;
	}

	// This helper method reads a int value from a datasection, using
	// a second section for defaults
	int readIntSetting( std::string sectionName, DataSectionPtr pSection,
		DataSectionPtr pDefaultsSection )
	{
		if (pDefaultsSection)
			return pSection->readInt( sectionName, 
				pDefaultsSection->readInt( sectionName ) );
		return pSection->readInt( sectionName );
	}

	// This helper method reads a float value from a datasection, using
	// a second section for defaults
	float readFloatSetting( std::string sectionName, DataSectionPtr pSection,
		DataSectionPtr pDefaultsSection )
	{
		if (pDefaultsSection)
			return pSection->readFloat( sectionName, 
				pDefaultsSection->readFloat( sectionName ) );
		return pSection->readFloat( sectionName );
	}

	// This helper method reads a bool value from a datasection, using
	// a second section for defaults
	bool readBoolSetting( std::string sectionName, DataSectionPtr pSection,
		DataSectionPtr pDefaultsSection )
	{
		if (pDefaultsSection)
			return pSection->readBool( sectionName, 
				pDefaultsSection->readBool( sectionName ) );
		return pSection->readBool( sectionName );
	}

	// This helper method reads the level distances from a datasection
	// using another datasection for default values
	void readLevelDistances( DataSectionPtr pSection, 
		DataSectionPtr pDefaultsSection, uint32 numTerrainLods,
		std::vector<float>& distances )
	{
		// clear the distances
		distances.clear();

		// Distance count is numlods - 1 as the last distance does not lod out
		const uint32 numDistances = numTerrainLods - 1;

		// read each distance
		for ( uint32 i = 0; i < numDistances; i++ )
		{
			char buffer[64];
			bw_snprintf( buffer, ARRAY_SIZE(buffer), 
				"lodInfo/lodDistances/distance%u", i );
			distances.push_back( readFloatSetting( buffer, 
				pSection, pDefaultsSection ) );
		}
	}

	// This helper method writes the level distances to a datasection
	void saveLevelDistances( DataSectionPtr pSection, 
		std::vector<float>& distances )
	{
		// read each distance
		for ( uint32 i = 0; i < distances.size(); i++ )
		{
			char buffer[64];
			bw_snprintf( buffer, ARRAY_SIZE(buffer), 
				"lodDistances/distance%u", i );
			pSection->writeFloat( buffer, distances[i] );
		}
	}

	// The last lod is set to be really far away, Not FLT_MAX as shaders may 
	// have different max limits to regular floats
	const float LAST_LOD_DIST = 1000000.f;
};

#ifndef MF_SERVER
uint32 TerrainSettings::s_topVertexLod_ = 0;
bool TerrainSettings::s_useLodTexture_ = true;
bool TerrainSettings::s_constantLod_ = false;
bool TerrainSettings::s_doBlockSplit_ = true;
bool TerrainSettings::s_loadDominantTextureMaps_ = true;
#else
bool TerrainSettings::s_loadDominantTextureMaps_ = false;
#endif // MF_SERVER

// Constructor
TerrainSettings::TerrainSettings()
{
	BW_GUARD;	
#ifndef MF_SERVER
	static bool firstTime = true;
	if (firstTime)
	{
		firstTime = false;
		MF_WATCH( "Render/Terrain/Terrain2/lodStartLevel", s_topVertexLod_ );

		MF_WATCH( "Render/Terrain/Terrain2/Use lod texture", s_useLodTexture_ );
		MF_WATCH( "Render/Terrain/Terrain2/Do block split", s_doBlockSplit_ );
		MF_WATCH( "Render/Terrain/Terrain2/Do constant LOD", s_constantLod_ );
	}
#endif

#if defined( MF_SERVER ) || defined( EDITOR_ENABLED )
	serverHeightMapLod_ = 0;
#endif
}

TerrainSettings::~TerrainSettings()
{
	BW_GUARD;	
#ifndef MF_SERVER
	if (Manager::pInstance() != NULL)
		TerrainGraphicsOptions::instance()->delSettings( this );
#endif
}

/**
 *	Initialises the settings according to the data section (usually the terrain
 *  section of the space.settings file) and inits the appropriate terrain
 *  renderer when not in the server.
 *
 *  @param settings		DataSection containing the desired terrain settings.
 *	@return true if successful
 */
bool TerrainSettings::init( DataSectionPtr settings )
{
	BW_GUARD;	
#ifndef MF_SERVER
	TerrainGraphicsOptions::instance()->delSettings( this );
#endif

	bool ret = false;
	if ( settings == NULL )
	{
		// no settings, so assume old terrain
		version_ = 100;
#ifndef MF_SERVER
		uvProjections_ = false;
#endif // MF_SERVER
	}
	else
	{
		version_ = settings->readInt( "version", 0 );
#ifndef MF_SERVER
		uvProjections_ = ( version_ != 100 );
#endif // MF_SERVER
	}

	MF_ASSERT( version_ == 100 || version_ == 200 );

#ifndef MF_SERVER
	// Init the values approprate to the renderer.
	if ( version_ == 100 && TerrainRenderer1::init() )
	{
		pRenderer_ = TerrainRenderer1::instance();

		// Must initialise these values for the old terrain as well to avoid
		// having to code special cases depending on the terrain version. The
		// old terrain map sizes are fixed at 25x25 height poles.
		heightMapSize_ =  25; 
		normalMapSize_ =  25; 
		holeMapSize_ =    25; 
		shadowMapSize_ =  25; 
		blendMapSize_ =   25; 
		ret = true;
	}
	else if ( version_ == 200 && TerrainRenderer2::init() )
	{
		// Load up version 2 specific terrain data
		const DataSectionPtr pTerrain2Defaults = Manager::instance().pTerrain2Defaults();

		pRenderer_ = TerrainRenderer2::instance();

		// Unit settings
		heightMapSize_ = readIntSetting( "heightMapSize",
			settings, pTerrain2Defaults ); 
		normalMapSize_ = readIntSetting( "normalMapSize",
			settings, pTerrain2Defaults ); 
		holeMapSize_ = readIntSetting( "holeMapSize",
			settings, pTerrain2Defaults ); 
		shadowMapSize_ = readIntSetting( "shadowMapSize",
			settings, pTerrain2Defaults ); 
		blendMapSize_ = readIntSetting( "blendMapSize",
			settings, pTerrain2Defaults ); 

		// calculate the number of vertex lods
		numVertexLods_ = log2RoundUp(heightMapSize_);

		MF_ASSERT((1 << numVertexLods_) == heightMapSize_);

		// read the lod distances
		readLevelDistances( settings, pTerrain2Defaults, numVertexLods_,
			savedVertexLodValues_ );

		// lod vertex information
		vertexLod_.startBias( readFloatSetting( "lodInfo/startBias",
			settings, pTerrain2Defaults ) ); 
		vertexLod_.endBias( readFloatSetting( "lodInfo/endBias", 
			settings, pTerrain2Defaults ) );

		// read lod texture information
		lodTextureStart_ = readFloatSetting( "lodInfo/lodTextureStart", 
			settings, pTerrain2Defaults ); 
		lodTextureDistance_	= readFloatSetting( 
			"lodInfo/lodTextureDistance", settings, pTerrain2Defaults ); 
		blendPreloadDistance_= readFloatSetting( 
			"lodInfo/blendPreloadDistance", settings, pTerrain2Defaults );

		// normal map lod
		lodNormalStart_ = readFloatSetting( "lodInfo/lodNormalStart", 
			settings, pTerrain2Defaults );
		lodNormalStart_ = std::max( lodNormalStart_, 
			lodTextureStart_ + lodTextureDistance_ );
		lodNormalDistance_	= readFloatSetting( 
			"lodInfo/lodNormalDistance", settings, pTerrain2Defaults ); 
		normalPreloadDistance_= readFloatSetting( 
			"lodInfo/normalPreloadDistance", settings, pTerrain2Defaults );


		// Height map lods
		defaultHeightMapLod_ = readIntSetting(
			"lodInfo/defaultHeightMapLod", settings, pTerrain2Defaults );
		detailHeightMapDistance_ = readFloatSetting(
			"lodInfo/detailHeightMapDistance", settings, pTerrain2Defaults );

		applyLodModifier(TerrainGraphicsOptions::instance()->lodModifier());
		ret = true;
	}
	else
	{
		BaseTerrainRenderer::instance( NULL );
		ERROR_MSG( "Couldn't find a valid terrain renderer for this space.\n" );
	}

	TerrainGraphicsOptions::instance()->addSettings( this );

	directSoundOcclusion_ = settings->readFloat( "soundOcclusion/directOcclusion", 0.0f );
	reverbSoundOcclusion_ = settings->readFloat( "soundOcclusion/reverbOcclusion", 0.0f );

#endif // MF_SERVER

	// Server and editor specific settings for terrain 2
#if defined( MF_SERVER ) || defined( EDITOR_ENABLED )
	if ( version_ == 200 )
	{
		// Load up version 2 specific terrain data
		const DataSectionPtr pTerrain2Defaults = Manager::instance().pTerrain2Defaults();

		//Load base height map lod setting
		serverHeightMapLod_ = readIntSetting( "lodInfo/server/heightMapLod",
			settings, Manager::instance().pTerrain2Defaults() );
	}
#endif

	return ret;
}

#ifndef MF_SERVER
void TerrainSettings::setActiveRenderer()
{
	BW_GUARD;
	BaseTerrainRenderer::instance( pRenderer_.getObject() );
}

BaseTerrainRendererPtr TerrainSettings::pRenderer()
{
	return pRenderer_;
}

void TerrainSettings::applyLodModifier(float modifier)
{
	BW_GUARD;
	vertexLod_.clear();

	// The minimum distance between lod levels is the distance from a blocks
	// corner to where the block starts geomorphing
	const float minDistance = 
		ceilf(sqrtf(SUB_BLOCK_SIZE_METRES * SUB_BLOCK_SIZE_METRES * 2) / 
			vertexLod_.startBias());

	// The first lod distance does not matter it can lod out as early 
	// as it wants.
	float lastValue = -minDistance;

	// Iterate over our lod values and scale them by the lod modifier.
	for (uint32 i = 0; i < savedVertexLodValues_.size(); i++)
	{
		// Make sure our lod level delta is big enough
		float value = floorf(savedVertexLodValues_[i] * modifier);
		if ((value - lastValue) < minDistance)
			value = lastValue + minDistance;

		vertexLod_.push_back( value );
		lastValue = value;
	}
}

#endif

#ifdef EDITOR_ENABLED

namespace
{
	static TerrainSettingsPtr s_pDefaults;
}

/**
 *	Defaults getter
 *
 *  @returns		The TerrainSettings instance.
 */
/*static*/ TerrainSettingsPtr TerrainSettings::defaults()
{
	BW_GUARD;
	if (!s_pDefaults.exists())
	{
		s_pDefaults = new TerrainSettings;
		s_pDefaults->initDefaults();
	}
	return s_pDefaults;
}

void TerrainSettings::initDefaults()
{
	BW_GUARD;
	this->init( Manager::instance().pTerrain2Defaults() );
}

void TerrainSettings::save(DataSectionPtr pSection)
{
	BW_GUARD;
	MF_ASSERT( pSection );
	pSection->delChildren();

	pSection->writeInt( "version", version_ );

	// Only write out other settings for new terrain
	if (version_ == 200)
	{
		// Write unit setup
		pSection->writeInt( "heightMapSize", heightMapSize_ );
		pSection->writeInt( "normalMapSize", normalMapSize_ );
		pSection->writeInt( "holeMapSize", holeMapSize_ );
		pSection->writeInt( "shadowMapSize", shadowMapSize_ );
		pSection->writeInt( "blendMapSize", blendMapSize_ );

		// Write lod information
		DataSectionPtr pLodSection = pSection->newSection( "lodInfo" );
		if (pLodSection)
		{
			pLodSection->writeFloat( "startBias", vertexLod_.startBias() );
			pLodSection->writeFloat( "endBias", vertexLod_.endBias() );
			pLodSection->writeFloat( "lodTextureStart", lodTextureStart_ );
			pLodSection->writeFloat( "lodTextureDistance", 
				lodTextureDistance_ );
			pLodSection->writeFloat( "blendPreloadDistance", 
				blendPreloadDistance_ );

			// normal map lod
			pLodSection->writeFloat( "lodNormalStart", 
				lodNormalStart_ );
			pLodSection->writeFloat( "lodNormalDistance", 
				lodNormalDistance_ );
			pLodSection->writeFloat( "normalPreloadDistance", 
				normalPreloadDistance_ );
			pLodSection->writeInt("defaultHeightMapLod", defaultHeightMapLod_ );
			pLodSection->writeFloat("detailHeightMapDistance", detailHeightMapDistance_ );

			saveLevelDistances( pLodSection, savedVertexLodValues_ );

			// TODO: It would be good to save serverHeightMapLod_ too.
			pLodSection->writeInt( "server/heightMapLod", serverHeightMapLod_ );
		}

	}

	pSection->writeFloat( "soundOcclusion/directOcclusion", directSoundOcclusion_ );
	pSection->writeFloat( "soundOcclusion/reverbOcclusion", reverbSoundOcclusion_ );
}

void TerrainSettings::version(uint32 value)
{
	version_ = value;
}

void TerrainSettings::heightMapSize(uint32 value)
{
	heightMapSize_ = value;

	// recalculate the number of vertex lods
	numVertexLods_ = log2RoundUp(heightMapSize_);
}

void TerrainSettings::normalMapSize(uint32 value)
{
	normalMapSize_ = value;
}

void TerrainSettings::holeMapSize(uint32 value)
{
	holeMapSize_ = value;
}

void TerrainSettings::shadowMapSize(uint32 value)
{
	shadowMapSize_ = value;
}

void TerrainSettings::blendMapSize(uint32 value)
{
	blendMapSize_ = value;
}

void TerrainSettings::fini()
{
	s_pDefaults = NULL;
}

float TerrainSettings::vertexLodDistance( uint32 lod ) const
{
	if (lod < savedVertexLodValues_.size())
	{
		return savedVertexLodValues_[lod];
	}
	return LAST_LOD_DIST;
}

void TerrainSettings::vertexLodDistance( uint32 lod, float value )
{
	if (lod < savedVertexLodValues_.size())
	{
		savedVertexLodValues_[lod] = value;
		applyLodModifier(TerrainGraphicsOptions::instance()->lodModifier());
	}
}
#endif

/**
 *	Current terrain version getter.
 *
 *  @returns		Current terrain version.
 */
uint32 TerrainSettings::version() const
{
	return version_;
}

/**
 *	Returns whether loading dominant texture maps is enabled or not.
 *
 *  @returns		True if loading dominant texture maps is enabled, false otherwise.
 */
bool TerrainSettings::loadDominantTextureMaps()
{
	return s_loadDominantTextureMaps_;
}

/**
 *	Sets whether dominant texture maps should be loaded or not.
 *
 *  @param doLoad		True to enable loading dominant texture maps, false to disable.
 */
void TerrainSettings::loadDominantTextureMaps( bool doLoad )
{
	s_loadDominantTextureMaps_ = doLoad;
}

#ifndef MF_SERVER
/**
 *	Current terrain uv projection availability.
 *
 *  @returns		True if the current terrain supports uv projections.
 */
bool TerrainSettings::uvProjections() const
{
	return uvProjections_;
}


/**
 *	Current terrain height map size getter.
 *
 *  @returns		Current terrain height map size.
 */
uint32 TerrainSettings::heightMapSize() const
{
	return heightMapSize_;
}


/**
 *	Current terrain normal map size getter.
 *
 *  @returns		Current terrain normal map size.
 */
uint32 TerrainSettings::normalMapSize() const
{
	return normalMapSize_;
}


/**
 *	Current terrain hole map size getter.
 *
 *  @returns		Current terrain hole map size.
 */
uint32 TerrainSettings::holeMapSize() const
{
	return holeMapSize_;
}


/**
 *	Current terrain shadow map size getter.
 *
 *  @returns		Current terrain shadow map size.
 */
uint32 TerrainSettings::shadowMapSize() const
{
	return shadowMapSize_;
}


/**
 *	Current terrain blend map size getter.
 *
 *  @returns		Current terrain blend map size.
 */
uint32 TerrainSettings::blendMapSize() const
{
	return blendMapSize_;
}

/**	
 *	Get the vertex lod start bias
 *	@return the bias
 */
float TerrainSettings::vertexLodStartBias() const
{
	return vertexLod_.startBias();
}

/**	
 *	Set the vertex lod start bias
 *	@param value the start bias
 */
void TerrainSettings::vertexLodStartBias( float value )
{
	vertexLod_.startBias( value );
}

/**	
 *	Get the vertex lod end bias
 *	@return the bias
 */
float TerrainSettings::vertexLodEndBias() const
{
	return vertexLod_.endBias();
}

/**	
 *	Set the vertex lod end bias
 *	@param value the end bias
 */
void TerrainSettings::vertexLodEndBias( float value )
{
	vertexLod_.endBias( value );
}

/**
 *	Get he top vertex lod, i.e. the first lod level to consider
 *	@return the top vertex lod
 */
uint32 TerrainSettings::topVertexLod()
{
	return s_topVertexLod_;
}

void TerrainSettings::topVertexLod(uint32 state)
{
	s_topVertexLod_ = state;
}

/**
 *	Get the distance to where the lod texture blends in
 *	@return distance to where the lod texture blends in
 */
float TerrainSettings::lodTextureStart() const
{
	return lodTextureStart_;
}

/**
 *	Set the distance to where the lod texture blends in
 *	@param value distance to where the lod texture blends in
 */
void TerrainSettings::lodTextureStart( float value )
{
	lodTextureStart_ = value;
}

/**
 *	Get the distance over which the lod texture is blended in
 *	This value is relative to the lod texture start
 *	@return the distance over which the lod texture is blended in
 */
float TerrainSettings::lodTextureDistance() const
{
	return lodTextureDistance_;
}

/**
 *	Set the distance over which the lod texture is blended in
 *	This value is relative to the lod texture start
 *	@param value the distance over which the lod texture is blended in
 */
void TerrainSettings::lodTextureDistance( float value )
{
	lodTextureDistance_ = value;
}

/**
 *	Get the distance at which we preload blends
 *	This distance is relative to the lod texture start and lod texture distance
 *	@return the distance at which we preload blends
 */
float TerrainSettings::blendPreloadDistance() const
{
	return blendPreloadDistance_;
}

/**
 *	Set the distance at which we preload blends
 *	This distance is relative to the lod texture start and lod texture distance
 *	@param value the distance at which we preload blends
 */
void TerrainSettings::blendPreloadDistance( float value )
{
	blendPreloadDistance_ = value;
}

/**
 *	Get whether we want to use lod texture or not
 *	@return true if we want to use the lod texture
 */
bool TerrainSettings::useLodTexture()
{
	return s_useLodTexture_;
}

void TerrainSettings::useLodTexture(bool state)
{
	s_useLodTexture_ = state;
}

/**
 *	Get whether we want to use a constant lod or not
 *	@return true if we want to use constant lod
 */
bool TerrainSettings::constantLod()
{
	return s_constantLod_;
}

void TerrainSettings::constantLod(bool state)
{
	s_constantLod_ = state;
}


bool TerrainSettings::doBlockSplit()
{
	return s_doBlockSplit_;
}

float TerrainSettings::lodNormalStart() const
{
	return lodNormalStart_;
}

void TerrainSettings::lodNormalStart( float value )
{
	lodNormalStart_ = std::max(value, blendPreloadDistance());
}

float TerrainSettings::lodNormalDistance() const
{
	return lodNormalDistance_;
}

void TerrainSettings::lodNormalDistance( float value )
{
	lodNormalDistance_ = value;
}

float TerrainSettings::normalPreloadDistance() const
{
	return normalPreloadDistance_;
}

void TerrainSettings::normalPreloadDistance( float value )
{
	normalPreloadDistance_ = value;
}

float TerrainSettings::directSoundOcclusion() const
{
	return directSoundOcclusion_;
}

void TerrainSettings::directSoundOcclusion( float value )
{
	directSoundOcclusion_ = value;
}

float TerrainSettings::reverbSoundOcclusion() const
{
	return reverbSoundOcclusion_;
}

void TerrainSettings::reverbSoundOcclusion( float value )
{
	reverbSoundOcclusion_ = value;
}

#endif
