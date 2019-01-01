/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_LOD_MAP2_HPP
#define TERRAIN_LOD_MAP2_HPP

namespace Terrain
{

/**
 *	This class implements the lod texture for the terrain version 2
 */
class TerrainLodMap2 : public SafeReferenceCount
{
public:

	TerrainLodMap2();
	~TerrainLodMap2();

	/**
	 *  Load lod map from a section
	 */
	bool load( DataSectionPtr pLodMapSection );
	bool save( DataSectionPtr parentSection, const std::string& name ) const;
	
	/**
	 *  Get the lod map
	 */
	ComObjectWrap<DX::Texture>	pTexture() const { return pLodMap_; }
	void pTexture( ComObjectWrap<DX::Texture> texture ) { pLodMap_ = texture; } 

	/**
	 * Get the lod map size
	 */
	uint32						size() const;
private:

	ComObjectWrap<DX::Texture>  pLodMap_;
	uint32 size_;
};

typedef SmartPointer<TerrainLodMap2> TerrainLodMap2Ptr;

}


#endif // TERRAIN_LOD_MAP2_HPP
