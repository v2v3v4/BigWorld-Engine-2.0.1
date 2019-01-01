/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_ENVELOPE_HPP
#define VISUAL_ENVELOPE_HPP

#include "mfxnode.hpp"
#include "phyexp.h"
#include "visual_mesh.hpp"

typedef SmartPointer<class VisualEnvelope> VisualEnvelopePtr;

/**
 *	This class ...
 *
 *	@todo Document this class.
 */
class VisualEnvelope : public VisualMesh
{
public:
	VisualEnvelope();
	~VisualEnvelope();

	bool init( INode* node, MFXNode* root );
	void save( DataSectionPtr spVisualSection, const std::string& primitiveFile );

private:
	struct VertexXYZNUVI
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float index;
	};

	typedef std::vector<VertexXYZNUVI> VertexVector;

	int			boneIndex( INode* node );
	void		createVertexList( VertexVector& vertices );

	void		getScaleValues( std::vector< Point3 > & scales );
	void		scaleBoneVertices( std::vector< Point3 >& scales );
	bool		collectInitialTransforms(IPhysiqueExport* phyExport);
	void		normaliseInitialTransforms();
	void		initialPoseVertices();
	void		relaxedPoseVertices();

	typedef std::vector< INode* > NodeVector;
	NodeVector	boneNodes_;

	struct BoneVertex
	{
		BoneVertex( const Point3& position, int index )
		: position_( position ),
		  boneIndex_( index )
		{ }
		Point3	position_;
		int		boneIndex_;
	};

	typedef std::vector< BoneVertex > BoneVVector;
	BoneVVector		boneVertices_;

	typedef std::vector< Matrix3 >	MatrixVector;
	MatrixVector	initialTransforms_;

	Matrix3			initialObjectTransform_;


	VisualEnvelope( const VisualEnvelope& );
	VisualEnvelope& operator=( const VisualEnvelope& );
};


#ifdef CODE_INLINE
#include "visual_envelope.ipp"
#endif

#endif // VISUAL_ENVELOPE_HPP
