/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOADING_COLUMN_HPP
#define LOADING_COLUMN_HPP

#include "cstdmf/stdmf.hpp"

#include <list>

class Chunk;
class ChunkSpace;
class ServerGeometryMapping;

enum LoadingColumnState
{
	UNLOADED,
	LOADING_OUTSIDE,
	LOADING_INSIDE,
	LOADED
};


/**
 *	This class tracks the loading of one column. That is, one outdoor chunk and
 *	the overlapping indoor chunks.
 */
class LoadingColumn
{
public:
	LoadingColumn();
	~LoadingColumn();

	bool stepLoad( ServerGeometryMapping * pMapping, int perpPos,
			bool isVerticalEdge, bool & rAnyColumnsLoaded );
	bool stepUnload( ServerGeometryMapping * pMapping, int perpPos,
			bool isVerticalEdge );

	void prepareNewlyLoadedChunksForDelete( ServerGeometryMapping * pMapping );

	bool isLoading() const
	{
		return (state_ == LOADING_INSIDE) || (state_ == LOADING_OUTSIDE);
	}

	// The position along the edge of the loading chunk/column.
	int		pos_;

	Chunk * pOutsideChunk_;
	LoadingColumnState state_;

private:
	// Loading
	void loadOutsideChunk( ServerGeometryMapping * pMapping,
		int edgePos, bool isVerticalEdge );
	void loadOverlappers();
	bool areOverlappersLoaded() const;

	void bindOutsideChunk( ServerGeometryMapping * pMapping );
	void bindOverlappers( ServerGeometryMapping * pMapping );

	// Unloading
	void unloadOverlappers( ServerGeometryMapping * pMapping );
	void unloadOutsideChunk( ServerGeometryMapping * pMapping );
	void cancelLoadingOverlappers( ServerGeometryMapping * pMapping );
	void cancelLoadingOutside( ServerGeometryMapping * pMapping );
};

#endif // LOADING_COLUMN_HPP
