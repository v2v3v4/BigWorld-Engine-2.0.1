/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLORA_LIGHT_MAP_HPP
#define FLORA_LIGHT_MAP_HPP

#include "light_map.hpp"
#include "terrain/base_terrain_block.hpp"
#include "moo/device_callback.hpp"

class Chunk;

namespace Terrain
{
	class BaseTerrainBlock;
	typedef SmartPointer<BaseTerrainBlock> BaseTerrainBlockPtr;
}

/**
 *	This class creates a light map
 *	for the flora shader.
 */
class FloraLightMap : public EffectLightMap
{
public:
	FloraLightMap( class Flora* flora );
	~FloraLightMap();

	void update( float gametime );
	void setTransform( int constBase, const Matrix& vertexToWorld );
private:
	void setProjection(Terrain::BaseTerrainBlockPtr pValidBlock,
		const Matrix& blockTransform);
	virtual void createTransformSetter();
	
	bool							inited_;	
	Terrain::BaseTerrainBlockPtr	blocks_[4];
	Chunk*							chunks_[4];
	Matrix							worldToLight_;		

	Flora*							flora_;
};

#endif