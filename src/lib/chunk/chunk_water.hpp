/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WATER_HPP
#define CHUNK_WATER_HPP

#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include "chunk_item.hpp"
#include "chunk_vlo.hpp"

#include "romp/water.hpp"

/**
 *	This class is a body of water as a chunk item
 */
class ChunkWater : public VeryLargeObject
{
public:
	ChunkWater( );
	ChunkWater( std::string uid );
	~ChunkWater();

	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual void draw( ) {}
	virtual void draw( Chunk* pChunk );
	virtual void sway( const Vector3 & src, const Vector3 & dst, const float diameter );
	static void simpleDraw( bool state );

#ifdef EDITOR_ENABLED
	virtual void dirty();
	virtual void edCommonChanged() {}
#endif //EDITOR_ENABLED

	virtual BoundingBox chunkBB( Chunk* pChunk );

	virtual void syncInit(ChunkVLO* pVLO);
protected:
	Water *				pWater_;
	Chunk*	pChunk_;

	Water::WaterState	config_;

private:

	static bool create( Chunk * pChunk, DataSectionPtr pSection, std::string uid );
	static VLOFactory	factory_;
	static bool s_simpleDraw;
	virtual bool addYBounds( BoundingBox& bb ) const;
};


#endif // CHUNK_WATER_HPP
