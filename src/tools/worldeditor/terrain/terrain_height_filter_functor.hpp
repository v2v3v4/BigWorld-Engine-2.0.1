/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HEIGHT_FILTER_FUNCTOR_HPP
#define TERRAIN_HEIGHT_FILTER_FUNCTOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/misc/matrix_filter.hpp"
#include "worldeditor/terrain/terrain_functor.hpp"
#include "terrain/terrain_height_map.hpp"


/**
 *	This class applies a filter to height poles' height
 */
class TerrainHeightFilterFunctor : public TerrainFunctor
{
	Py_Header( TerrainHeightFilterFunctor, TerrainFunctor )

public:
	enum
	{
		FLAT_FALLOFF = 0,
		LINEAR_FALLOFF = 1,
		CURVE_FALLOFF = 2,
		AVERAGE_FALLOFF = 3
	};

	explicit TerrainHeightFilterFunctor( PyTypePlus *pType = &s_type_ );
	~TerrainHeightFilterFunctor();

	void	update( float dTime, Tool& t );
	bool	handleKeyEvent( const KeyEvent & event, Tool & t );
	bool	applying() const;
	void	getBlockFormat( EditorChunkTerrain const & pEct,
				TerrainUtils::TerrainFormat & format ) const;
	void	onFirstApply( EditorChunkTerrain & ect );	
	void	onBeginApply( EditorChunkTerrain & ect );
	void	applyToSubBlock( EditorChunkTerrain &,
				const Vector3 & toolOffset, const Vector3 & chunkOffset,
				const TerrainUtils::TerrainFormat & format,
				int32 minx, int32 minz, int32 maxx, int32 maxz );
	void	onEndApply( EditorChunkTerrain & ect );
	void	onApplied( Tool & t );
	void	onLastApply( EditorChunkTerrain & pEct );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( applyRate_, applyRate );
	PY_RW_ATTRIBUTE_DECLARE( useFalloff_, falloff );
	PY_RW_ATTRIBUTE_DECLARE( useStrength_, strengthMod );
	PY_RW_ATTRIBUTE_DECLARE( useDTime_, framerateMod );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, filterIndex, index )

	int filterIndex() const	{ return filterIndex_; }
	void filterIndex( int i );

	PY_RW_ATTRIBUTE_DECLARE( filter_.constant_, constant )
	PY_RO_ATTRIBUTE_DECLARE( filter_.name_, name )

	PY_FACTORY_DECLARE()

private:
	bool considerNeighbours(
		EditorChunkTerrain & ect, const TerrainUtils::TerrainFormat & format );

private:
	float					avgHeight_;
	float					falloff_;
	float					strength_;

	bool					applying_;
	float					appliedFor_;

	float					applyRate_;		// apply this many times per second
	int						useFalloff_;	// should use falloff circle? (no;linear;squared)
	bool					useStrength_;	// should multiply constant by strength?
	bool					useDTime_;		// should multiply constant by dtime?

	int						filterIndex_;

	Terrain::TerrainHeightMap::ImageType singleHeightMapCache_;

	std::vector< EditorChunkTerrain * > lockMap_;

	SpaceHeightMapPtr		spaceHeightMap_;

	MatrixFilter::FilterDef filter_;

	FUNCTOR_FACTORY_DECLARE( TerrainHeightFilterFunctor() )
};


#endif // TERRAIN_HEIGHT_FILTER_FUNCTOR_HPP
