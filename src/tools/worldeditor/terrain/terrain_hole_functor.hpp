/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HOLE_FUNCTOR_HPP
#define TERRAIN_HOLE_FUNCTOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/terrain/terrain_functor.hpp"


/**
 *	This class either cuts out or fills in a height poles' hole.
 */
class TerrainHoleFunctor : public TerrainFunctor
{
	Py_Header( TerrainHoleFunctor, TerrainFunctor )

public:
	explicit TerrainHoleFunctor( PyTypePlus * pType = &s_type_ );

	void	update( float dTime, Tool& t );
	bool	applying() const;
	void	getBlockFormat( const EditorChunkTerrain & chunkTerrain,
				TerrainUtils::TerrainFormat & format ) const;
	void	onFirstApply( EditorChunkTerrain & chunkTerrain );
	void	applyToSubBlock( EditorChunkTerrain &,
				const Vector3 & toolEffset, const Vector3 & chunkOffset,
				const TerrainUtils::TerrainFormat & format,
				int32 minx, int32 minz, int32 maxx, int32 maxz );
	void	onApplied( Tool & t );
	void	onLastApply( EditorChunkTerrain& chunkTerrain );

	PyObject *		pyGetAttribute( const char * pAttr );
	int				pySetAttribute( const char * pAttr, PyObject * pValue );

	PY_RW_ATTRIBUTE_DECLARE( fillNotCut_, fillNotCut )

	PY_FACTORY_DECLARE()
private:
	float	falloff_;
	bool	fillNotCut_;
	bool	applying_;

	FUNCTOR_FACTORY_DECLARE( TerrainHoleFunctor() )
};


#endif // TERRAIN_HOLE_FUNCTOR_HPP
