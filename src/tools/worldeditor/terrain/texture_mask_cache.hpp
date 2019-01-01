/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_MASK_CACHE_HPP
#define TEXTURE_MASK_CACHE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"

#include "worldeditor/terrain/terrain_paint_brush.hpp"

#include "cstdmf/singleton.hpp"

#include <map>


/**
 *	This represents a pixel in a mask image.
 */
#pragma pack(1)
struct MaskPixel
{
	uint8				fuzziness_;	// fuzziness in terms of mask 0 = off, 255 = fully on
	uint8				maxValue_;	// maximum value that a pixel can have if painted using mask
};
#pragma pack()


typedef Moo::Image< MaskPixel > MaskImage;


/**
 *	This is the mask image, plus any other bits of information needed when
 *  using the results of a mask operation.
 */
struct MaskResult
{
	MaskImage			image_;				// the image of MaskPixels.
	std::vector<bool>	maskTexLayer_;		// which layers do we mask against?
};


/**
 *	This is used to generate mask images which can be used for painting.
 */
class TextureMaskCache : public Singleton< TextureMaskCache >
{
public:

	TextureMaskCache();
	~TextureMaskCache();

	static void beginFuzziness(
		Terrain::EditorBaseTerrainBlockPtr terrain,
		const TerrainPaintBrush & brush,
		std::vector<bool> &		maskLayers	);

	static MaskPixel
	fuzziness(
		Terrain::EditorBaseTerrainBlockPtr terrain,
		const Vector3 &			terrainPos,
		const Vector3 &			pos,
		const TerrainPaintBrush & brush,
		const std::vector<bool>	&maskLayers );

	static void endFuzziness(
		Terrain::EditorBaseTerrainBlockPtr terrain,
		const TerrainPaintBrush & brush,
		const std::vector<bool>	&maskLayers );

	void clear();

	MaskResult *mask( Terrain::EditorBaseTerrainBlockPtr terrain );

	void changedLayers( Terrain::EditorBaseTerrainBlockPtr terrain );

	TerrainPaintBrushPtr paintBrush() const;
	void paintBrush( TerrainPaintBrushPtr brush );

protected:
	typedef std::map< Terrain::EditorBaseTerrainBlockPtr, MaskResult > CacheMap;

	MaskResult *generate( Terrain::EditorBaseTerrainBlockPtr terrain );

private:
	TerrainPaintBrushPtr	paintBrush_;
	CacheMap				cache_;
};


#endif // TEXTURE_MASK_CACHE_HPP
