/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_UMBRA_HPP
#define CHUNK_UMBRA_HPP

#include "umbra_config.hpp"
#include "resmgr/datasection.hpp"
#include "cstdmf/bw_functor.hpp"


#if UMBRA_ENABLE

#include <umbraLibrary.hpp>

namespace Umbra
{
	class Commander;
};
class ChunkCommander;
class ChunkUmbraServices;
class Chunk;

/**
 *	This class creates and destroys umbra objects it also contains
 *	the umbra commander for the chunks
 */
class ChunkUmbra
{
public:
	ChunkUmbra( DataSectionPtr configSection = NULL );
	~ChunkUmbra();
	static void init( DataSectionPtr configSection = NULL );
	static void fini();
	static Umbra::Commander* pCommander(); 
	static void terrainOverride( SmartPointer< BWBaseFunctor0 > pTerrainOverride );
	static void repeat();
	static void tick();
	static bool softwareMode();
	static bool clipPlaneSupported();
	static void minimiseMemUsage();
private:
	static ChunkUmbra* s_pInstance_;
	ChunkCommander* pCommander_;
	ChunkUmbraServices*	pServices_;
	bool softwareMode_;
	bool clipPlaneSupport_;
};

class UmbraPortal
{
public:
	UmbraPortal(Chunk* pChunk)
	{
		pChunk_ = pChunk;
		reflectionPortal_ = false;
	}
	UmbraPortal(const std::vector<Vector3>& vertices,
				const std::vector<uint32>& triangles,
				Chunk* pChunk)
	{
		vertices_ = vertices;
		triangles_.reserve(triangles.size()); 
 		std::vector<uint32>::const_iterator it = triangles.begin(); 

		while (it != triangles.end()) 
		{ 
			triangles_.push_back( (uint16)*(it++)); 
		} 

		pChunk_ = pChunk;
		reflectionPortal_ = false;
	}

	int vertexCount() { return vertices_.size(); }
	int triangleCount() { return triangles_.size()/3; }
	Vector3* vertices() { return &vertices_[0]; }
	uint16* triangles() { return &triangles_[0]; } 

	Chunk* chunk() { return pChunk_; }

	bool reflectionPortal_;
private:
	std::vector<Vector3> vertices_;
	std::vector<uint16> triangles_; 
	Chunk* pChunk_;
};

/**
 * This class implments helper functionality for the umbra integration.
 */
class UmbraHelper
{
public:
	static UmbraHelper& instance();
	~UmbraHelper();

	bool drawTestModels() const;
	void drawTestModels(bool b);

	bool drawWriteModels() const;
	void drawWriteModels(bool b);

	bool drawObjectBounds() const;
	void drawObjectBounds( bool b );

	bool drawVoxels() const;
	void drawVoxels( bool b );

	bool drawSilhouettes() const;
	void drawSilhouettes( bool b );

	bool drawQueries() const;
	void drawQueries( bool b );

	bool occlusionCulling() const { return occlusionCulling_; } 
	void occlusionCulling(bool b) { occlusionCulling_ = b; }

	bool umbraEnabled() const { return umbraEnabled_; }
	void umbraEnabled(bool b) { umbraEnabled_ = b; }

	bool flushTrees() const { return flushTrees_; }
	void flushTrees(bool b) { flushTrees_ = b; }

	bool depthOnlyPass() const { return depthOnlyPass_; }
	void depthOnlyPass(bool b) { depthOnlyPass_ = b; }

	bool wireFrameTerrain() const { return wireFrameTerrain_; }
	void wireFrameTerrain( bool wireFrameTerrain ) { wireFrameTerrain_ = wireFrameTerrain; }

private:
	void init();
	void fini();

	class Statistic;

	UmbraHelper();
	
	bool occlusionCulling_;
	bool umbraEnabled_;
	bool flushTrees_;
	bool depthOnlyPass_;
	bool wireFrameTerrain_;

	friend ChunkUmbra;
};

#endif

#endif
/*chunk_umbra.hpp*/
