/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_DOMINANT_TEXTURE_MAP2_HPP
#define TERRAIN_DOMINANT_TEXTURE_MAP2_HPP

#include "../dominant_texture_map.hpp"


namespace Terrain
{
    /**
     *  This class allows access to material data of terrain blocks.
     */
    class DominantTextureMap2 : public DominantTextureMap
    {
    public:
		DominantTextureMap2();

		DominantTextureMap2(
			TextureLayers&		layerData,
			float               sizeMultiplier = 0.5f );

#ifdef EDITOR_ENABLED
		virtual bool save(DataSectionPtr) const;
#endif
		virtual bool load(DataSectionPtr dataSection, std::string *error = NULL);

    };
}

#endif // TERRAIN_DOMINANT_TEXTURE_MAP2_HPP
