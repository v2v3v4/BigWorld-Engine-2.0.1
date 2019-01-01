/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_SET_HEIGHT_FUNCTOR_HPP
#define TERRAIN_SET_HEIGHT_FUNCTOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/terrain/terrain_functor.hpp"
#include "moo/image.hpp"


/**
 *	This class sets the height to a constant value
 */
class TerrainSetHeightFunctor : public TerrainFunctor
{
	Py_Header( TerrainSetHeightFunctor, TerrainFunctor )

public:
	explicit TerrainSetHeightFunctor( PyTypePlus * pType = &s_type_ );

	void	update( float dTime, Tool & t );
	bool	applying() const;
	void	getBlockFormat( EditorChunkTerrain const & chunkTerrain,
				TerrainUtils::TerrainFormat& format ) const;
	void	onFirstApply( EditorChunkTerrain & chunkTerrain );	
	void	applyToSubBlock( EditorChunkTerrain&,
				const Vector3 & toolOffset, const Vector3 & chunkOffset,
				const TerrainUtils::TerrainFormat & format,
				int32 minx, int32 minz, int32 maxx, int32 maxz );
	void	onApplied( Tool & t );
	void	onLastApply( EditorChunkTerrain & chunkTerrain );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( relative_, relative );
	PY_RW_ATTRIBUTE_DECLARE( height_, height );

	PY_FACTORY_DECLARE()

private:
	typedef Moo::Image< uint8 >				VisitedImage;
	typedef SmartPointer< VisitedImage >	VisitedImagePtr;
	typedef std::map<Terrain::EditorBaseTerrainBlock *, VisitedImagePtr >	VisitedMap;

	float		height_;
	int			relative_;
	bool		applying_;
	VisitedMap	poles_;

	FUNCTOR_FACTORY_DECLARE( TerrainSetHeightFunctor() )
};


#endif // TERRAIN_SET_HEIGHT_FUNCTOR_HPP
