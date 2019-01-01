/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_SETTINGS_HPP
#define TERRAIN_TERRAIN_SETTINGS_HPP

#include "resmgr/datasection.hpp"

#ifndef MF_SERVER
#include "terrain2/terrain_vertex_lod.hpp"
#endif

//namespace Moo
//{
//	class GraphicsSetting;
//}

namespace Terrain
{

class BaseTerrainRenderer;
typedef SmartPointer<BaseTerrainRenderer> BaseTerrainRendererPtr;

/**
 *  This class keeps the current terrain settings, usually read from the file
 *  space.settings, keeps track of other runtime settings, and inits the
 *  appropriate terrain render in the 'init' method.
 */
class TerrainSettings : public SafeReferenceCount
{
public:
	TerrainSettings();
	~TerrainSettings();

	// called to set the settings (and initialise the renderer in the
	// client/tools)
	bool init( DataSectionPtr settings );

	// terrain space.settings methods
	uint32	version() const;

	// other terrain-related settings
	static bool	loadDominantTextureMaps();
	static void	loadDominantTextureMaps( bool doLoad );

#ifndef MF_SERVER
	uint32	heightMapSize() const;
	uint32	normalMapSize() const;
	uint32	holeMapSize() const;
	uint32	shadowMapSize() const;
	uint32	blendMapSize() const;

	bool	uvProjections() const;

	inline uint32 numVertexLods() const;

	float	vertexLodStartBias() const;
	void	vertexLodStartBias( float value );
    float	vertexLodEndBias() const;
    void	vertexLodEndBias( float value );

	float	lodTextureStart() const;
	void	lodTextureStart( float value );
	float	lodTextureDistance() const;
	void	lodTextureDistance( float value );
	float	blendPreloadDistance() const;
	void	blendPreloadDistance( float value );

	float	lodNormalStart() const;
	void	lodNormalStart( float value );
	float	lodNormalDistance() const;
	void	lodNormalDistance( float value );
	float	normalPreloadDistance() const;
	void	normalPreloadDistance( float value );

	inline float 	absoluteBlendPreloadDistance() const;
	inline float	absoluteNormalPreloadDistance() const;

	inline uint32	defaultHeightMapLod() const;
	inline void		defaultHeightMapLod( uint32 value );

	inline float	detailHeightMapDistance() const;

	const TerrainVertexLod& vertexLod() const { return vertexLod_; }

	float	directSoundOcclusion() const;
	void	directSoundOcclusion( float value );
	float	reverbSoundOcclusion() const;
	void	reverbSoundOcclusion( float value );

	void	setActiveRenderer();

	BaseTerrainRendererPtr pRenderer();
	// settings that are applied globally
    static uint32	topVertexLod();
	static void		topVertexLod(uint32 level);

	static bool useLodTexture();
	static void useLodTexture( bool state );

	static bool constantLod();
	static void constantLod( bool state );

	static bool doBlockSplit();

	void	applyLodModifier(float modifier);

#endif // !MF_SERVER

#if defined( MF_SERVER ) || defined( EDITOR_ENABLED )
	uint32 serverHeightMapLod() const	{ return serverHeightMapLod_; }

#endif // MF_SERVER || EDITOR_ENABLED

#ifdef EDITOR_ENABLED
	// Save the settings
	void	save( DataSectionPtr pSpaceTerrainSection );
	void	initDefaults();

	void	version(uint32 value);
	void	heightMapSize(uint32 value);
	void	normalMapSize(uint32 value);
	void	holeMapSize(uint32 value);
	void	shadowMapSize(uint32 value);
	void	blendMapSize(uint32 value);

	static  void fini();

	static SmartPointer<TerrainSettings> defaults();

	float	vertexLodDistance( uint32 lod ) const;
	void	vertexLodDistance( uint32 lod, float value );
#endif

private:
	// terrain space.settings values
	// These two are used for both new and old terrain
	uint32 version_;

	// other terrain settings
	static bool s_loadDominantTextureMaps_;
#ifndef MF_SERVER

	uint32 heightMapSize_;
	uint32 normalMapSize_;
	uint32 holeMapSize_;
	uint32 shadowMapSize_;
	uint32 blendMapSize_;

	TerrainVertexLod	vertexLod_;
	std::vector<float>	savedVertexLodValues_;

	bool uvProjections_;

	uint32 numVertexLods_;

	// lod texture start draw
	float	lodTextureStart_;
	// lod texture distance till 100% blended
	float	lodTextureDistance_;
	// extra buffer to load blends before they are rendered.
	float	blendPreloadDistance_;

	float	lodNormalStart_;
	float	lodNormalDistance_;
	float	normalPreloadDistance_;

	// The first lod level to render
    static uint32	s_topVertexLod_;

	// Enable or disable lod texture feature
	static bool s_useLodTexture_;

	// Enable to have every block at the same LOD.
	static bool s_constantLod_;

	// Do vertex lod on half terrain blocks
	static bool s_doBlockSplit_;

	// Lod of the default height map that gets loaded with every block.
	uint32	defaultHeightMapLod_;

	// Detail height map will be loaded when less than this distance from camera.
	float detailHeightMapDistance_;

	// sound properties
	float	directSoundOcclusion_;
	float	reverbSoundOcclusion_;

	BaseTerrainRendererPtr pRenderer_;
#endif // !MF_SERVER

#if defined( MF_SERVER ) || defined( EDITOR_ENABLED )
	// The height map load that the server should load.
	uint32 serverHeightMapLod_;
#endif // MF_SERVER || EDITOR_ENABLED
};

#ifndef MF_SERVER
/**
*	Current vertex lod count getter.
*	
&	@returns		Current vertex lod count.
*/
inline uint32 TerrainSettings::numVertexLods() const
{
	return numVertexLods_;
}

/**
*	Get the default height map lod. 
*/
uint32 TerrainSettings::defaultHeightMapLod() const
{
	return defaultHeightMapLod_;
}

/**
*	Set the default height map lod. 
*	@param	value	The new value of default height map LOD.
*/
void TerrainSettings::defaultHeightMapLod( uint32 value )
{
	defaultHeightMapLod_ = value;
}

/**
* Detail height map will be loaded when less than this distance from camera.
*/
float TerrainSettings::detailHeightMapDistance() const
{
	return detailHeightMapDistance_;
}

/**
 * Returns the absolute (total) blend pre load distance
 */
inline float TerrainSettings::absoluteBlendPreloadDistance() const
{
	return lodTextureStart_ + lodTextureDistance_ + blendPreloadDistance_;
}

/**
 *	Returns the absolute (total) normal preload distance
 */

inline float TerrainSettings::absoluteNormalPreloadDistance() const
{
	return lodNormalStart_ + lodNormalDistance_ + normalPreloadDistance_;
}

#endif

} // namespace Terrain

#endif //  TERRAIN_TERRAIN_SETTINGS_HPP
