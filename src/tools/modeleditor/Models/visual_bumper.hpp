/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_BUMPER_HPP
#define VISUAL_BUMPER_HPP

#include "resmgr/quick_file_writer.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/morph_vertices.hpp"
#include "moo/index_buffer.hpp"
#include "moo/primitive_file_structs.hpp"
#include "moo/vertex_streams.hpp"
#include "exporter_common/nvmeshmender.h"


/**
 * This class is used to add tangent and binormal information to models,
 * that do not have this information for their vertices.
 * It modifies the on disk state of the primitive file. Models cannot be
 * unbumped.
 *
 * TODO: Needs to be integrated with visual loading.
 */
class VisualBumper
{
public:
	VisualBumper( DataSectionPtr pVisual, const std::string& primitivesName );
	
	bool bumpVisual();

private:

	typedef std::vector< Moo::PrimitiveGroup > PGVector;
	typedef std::vector< Moo::MorphTargetPtr > MorphTargetVector;

	template< typename BumpedVertexType >
	bool savePrimitiveFile( const std::vector< BumpedVertexType >& bumpedVertices,
							const std::vector<Moo::VertexStreamPtr>& streams,
							const Moo::IndicesHolder& newIndices, 
							const PGVector& newPrimGroups );

	template< typename VertexType, typename BumpedVertexType >
	void getMorphTargets( const VertexType* pVertices, int nVertices );
	void writeMorphTargets( QuickFileWriter& qfw );
	void updateMorphTargets( int newStartVertex, int oldStartVertex, 
							const std::vector< unsigned int >& mappingNewToOld );

	template< typename VertexType, typename BumpedVertexType >
	bool makeBumped( const VertexType* pVertices, int nVertices, 
					 std::vector<Moo::VertexStreamPtr>& streams,
					 const Moo::IndicesHolder& indices, 
					 const PGVector& primGroups, 
					 bool hasMorphTargets );


	void createPrimitiveGroups( size_t nBumpedVertices, 
								const std::vector< uint32 >& bumpedIndices, 
								const std::vector< size_t >& material, 
								PGVector& newPrimGroups, 
								Moo::IndicesHolder& newIndices );

	bool updatePrimitives();

	std::string vertGroup_;
	std::vector<std::string> streams_;
	std::string indicesGroup_;
	std::string format_;
	
	DataSectionPtr pVisual_;
	DataSectionPtr pPrimFile_;
	std::string primitivesName_;

	MorphTargetVector morphTargets_;
	int morphVersion_;
};

#ifdef CODE_INLINE
#include "visual_bumper.ipp"
#endif

#endif // VISUAL_BUMPER_HPP
