/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_PHOTOGRAPHER_HPP
#define CHUNK_PHOTOGRAPHER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "resmgr/datasection.hpp"
#include "moo/render_target.hpp"
#include "romp/time_of_day.hpp"
#include "cstdmf/singleton.hpp"


class ChunkPhotographer : public Aligned, public Singleton< ChunkPhotographer >
{
public:
	ChunkPhotographer();

	static bool photograph( class Chunk& );

private:
	bool takePhoto(class Chunk&);
	bool beginScene();
	void setStates(class Chunk&);
	void renderScene(class Chunk&);
	void resetStates();
	void endScene();

	Moo::RenderTargetPtr			pRT_;
	int								width_;
	int								height_;
	Moo::LightContainerPtr			lighting_;
	OutsideLighting					chunkLighting_;
	Moo::LightContainerPtr			savedLighting_;
	OutsideLighting*				savedChunkLighting_;
	float							oldFOV_;
	Matrix							oldInvView_;
	Chunk *							pOldChunk_;
};


#endif // CHUNK_PHOTOGRAPHER_HPP
