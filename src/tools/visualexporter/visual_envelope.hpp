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
	// Typedefs
	typedef std::vector< INode* > NodeVector;

	VisualEnvelope();
	~VisualEnvelope();

	bool init( INode* node, MFXNode* root );

	virtual bool save(
		DataSectionPtr pVisualSection,
		DataSectionPtr spExistingVisual,
		const std::string& primitiveFile,
		bool useIdentifier );
	virtual bool savePrimXml( const std::string& xmlFile );
	virtual void add( VisualMesh & destMesh, int forceMeshIndex = -1 ) { }
	virtual bool isVisualEnvelope() { return true; }

	size_t boneNodesCount() { return boneNodes_.size(); }

private:
#pragma pack(push, 1)
	struct VertexXYZNUVI
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float index;
	};

	struct VertexXYZNUVIIIWW_v2
	{
		float pos[3];
		uint32 normal;
		float uv[2];
		uint8  index;
		uint8  index2;
		uint8  index3;
		uint8  weight;
		uint8  weight2;
	};

	struct VertexXYZNUV2I
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float uv2[2];
		float index;
	};

	struct VertexXYZNUV2IIIWW
	{
		float pos[3];
		uint32 normal;
		float uv[2];
		float uv2[2];
		uint8  index;
		uint8  index2;
		uint8  index3;
		uint8  weight;
		uint8  weight2;
	};

	struct VertexXYZNUVITB
	{
		float pos[3];
		uint32 normal;
		float uv[2];
		float index;
		uint32 tangent;
		uint32 binormal;

	};

	struct VertexXYZNUVIIIWWTB_v2
	{
		float pos[3];
		uint32 normal;
		float uv[2];
		uint8  index;
		uint8  index2;
		uint8  index3;
		uint8  weight;
		uint8  weight2;
		uint32 tangent;
		uint32 binormal;
	};
#pragma pack(pop)

	typedef std::vector<VertexXYZNUVI> VertexVector;
	typedef std::vector<VertexXYZNUVIIIWW_v2> BlendedVertexVector;
	typedef std::vector<VertexXYZNUVITB> TBVertexVector;
	typedef std::vector<VertexXYZNUVIIIWWTB_v2> TBBlendedVertexVector;

	int			boneIndex( INode* node );
	void		createVertexList( VertexVector& vertices );
	void		createVertexList( BlendedVertexVector& vertices );
	void		createVertexList( TBVertexVector& vertices );
	void		createVertexList( TBBlendedVertexVector& vertices );

	void		getScaleValues( std::vector< Point3 > & scales );
	void		scaleBoneVertices( std::vector< Point3 >& scales );
	bool		collectInitialTransforms(IPhysiqueExport* phyExport);
	bool		collectInitialTransforms(class ISkin* pSkin);
	void		normaliseInitialTransforms();
	void		initialPoseVertices();
	void		relaxedPoseVertices();
	void		prepareMorphTargets();

	NodeVector	boneNodes_;

	struct BoneVertex
	{
		BoneVertex( const Point3& position, int index, int index2 = 0, int index3 = 0, float weight = 1, float weight2 = 0, float weight3 = 0 )
		: position_( position ),
		  boneIndex_( index ),
		  boneIndex2_( index2 ),
		  boneIndex3_( index3 ),
		  weight_( weight ),
		  weight2_( weight2 ),
		  weight3_( weight3 )
		{ }
		Point3	position_;
		int		boneIndex_;
		int		boneIndex2_;
		int		boneIndex3_;
		float	weight_;
		float	weight2_;
		float	weight3_;
	};

	typedef std::vector< BoneVertex > BoneVVector;
	BoneVVector		boneVertices_;

	typedef std::vector< Matrix3 >	MatrixVector;
	MatrixVector	initialTransforms_;

	Matrix3			initialObjectTransform_;

	VisualEnvelope( const VisualEnvelope& );
	VisualEnvelope& operator=( const VisualEnvelope& );
};

#endif // VISUAL_ENVELOPE_HPP
