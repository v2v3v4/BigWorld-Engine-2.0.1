/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_DOMINANT_TEXTURE_MAP_HPP
#define TERRAIN_DOMINANT_TEXTURE_MAP_HPP

#include "terrain_map.hpp"
#include "terrain_map_holder.hpp"

namespace Terrain
{
	class TerrainTextureLayer;
	typedef uint8 MaterialIndex;

	// types
	typedef SmartPointer<TerrainTextureLayer> TerrainTextureLayerPtr;
	typedef std::vector<TerrainTextureLayerPtr> TextureLayers;
}

namespace Moo
{	
	//Specialise Image for MaterialIndex type to avoid attempting
	//bicubic interpolation when using parametric indices.  It makes
	//no sense to interpolate material indices.
	template <> inline Terrain::MaterialIndex
		Image<Terrain::MaterialIndex>::getBicubic(float x, float y) const
    {
		return get(int(x), int(y));
	}

	template <> inline double
		Image<Terrain::MaterialIndex>::getBicubic(double x, double y) const
    {
		return (double)get( int(x), int(y) );
	}
}

namespace Terrain
{
    /**
     *  This class allows access to material data of terrain blocks.
     */
    class DominantTextureMap : public TerrainMap<MaterialIndex>
    {
    public:
		DominantTextureMap();

		DominantTextureMap(
			TextureLayers&		layerData,
			float               sizeMultiplier = 0.5f );

        /**
         *  This function interprets the terrain material as a material kind.
         *
         *  @returns            The material kind at x,z
         */
        uint32 materialKind( float x, float z ) const;

        /**
         *  This function interprets the terrain material as a texture name.
         *
         *  @returns            The texture name at x,z
         */
		const std::string& texture( float x, float z ) const;

		virtual uint32 width() const;
		virtual uint32 height() const;

#ifdef EDITOR_ENABLED
		virtual bool lock(bool readOnly);		
		virtual ImageType &image();
		virtual bool unlock();
		virtual bool save(DataSectionPtr) const;
#endif
		virtual ImageType const &image() const;
	
	protected:
		void image( const ImageType& newImage );
		const std::vector<std::string>& textureNames() const;
		void textureNames( const std::vector<std::string>& names );

	private:
		MaterialIndex materialIndex( float x, float z ) const;
		
		// stub definition
		virtual bool load(DataSectionPtr dataSection, std::string *error = NULL)
		{ return false; };

	    Moo::Image<MaterialIndex>	image_;
		std::vector<std::string>	textureNames_;
    };


    /**
     *  A TerrainHoleMapIter can be used to iterate over a DominantTextureMap.
     */
    typedef TerrainMapIter<DominantTextureMap>      DominantTextureMapIter;


    /**
     *  A DominantTextureMapHolder can be used to lock/unlock a DominantTextureMap.
     */
    typedef TerrainMapHolder<DominantTextureMap>    DominantTextureMapHolder;
}

#endif // TERRAIN_DOMINANT_TEXTURE_MAP_HPP
