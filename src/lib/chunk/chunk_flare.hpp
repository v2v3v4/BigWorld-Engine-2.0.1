/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_FLARE_HPP
#define CHUNK_FLARE_HPP

#include "chunk_item.hpp"
#include "umbra_config.hpp"
#include "romp/lens_effect.hpp"

/**
 *	This class is a chunk item that tends a lens flare
 */
class ChunkFlare : public ChunkItem
{
	DECLARE_CHUNK_ITEM( ChunkFlare )
public:
	ChunkFlare();
	~ChunkFlare();

	bool load( DataSectionPtr pSection )	{ MF_ASSERT(0); return false; }
	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual void draw();

	static void ignore( bool state );

protected:
	Vector3				position_;
	Vector3				colour_;
	bool				colourApplied_;
	bool				type2_;
	virtual void syncInit();
	LensEffects			lensEffects_;

private:
	
	static bool s_ignore;
};


#endif // CHUNK_FLARE_HPP
