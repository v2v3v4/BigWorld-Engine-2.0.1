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

#include "visual_mesh.hpp"
#include "skin.hpp"

#include "moo/vertex_formats.hpp"

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
	typedef std::vector< std::string > NodeVector;

	VisualEnvelope();
	~VisualEnvelope();

	bool init( Skin& skin, Mesh& mesh );

	virtual bool save( DataSectionPtr spVisualSection, DataSectionPtr spExistingVisual, const std::string& primitiveFile,
		bool useIdentifier );

	virtual bool savePrimXml( const std::string& xmlFile );

	virtual bool isVisualEnvelope() { return true; }

	size_t boneNodesCount() { return boneNodes_.size(); }

	template<class T>
	void copyVertex( const BloatVertex& inVertex, T& outVertex ) const;
	
	template<class T>
	void copyVerts( BVVector& inVerts, std::vector<T>& outVerts );

private:
#pragma pack(push, 1)

	//TODO: use the Moo vertex formats..
	struct VertexXYZNUVI
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float index;
	};

	struct VertexXYZNUV2I
	{
		float pos[3];
		float normal[3];
		float uv[2];
		float uv2[2];
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
	typedef std::vector<VertexXYZNUV2I> UV2VertexVector;

	typedef std::vector<VertexXYZNUVIIIWW_v2> BlendedVertexVector;	
	typedef std::vector<VertexXYZNUV2IIIWW> UV2BlendedVertexVector;

	typedef std::vector<VertexXYZNUVITB> TBVertexVector;
	typedef std::vector<VertexXYZNUVIIIWWTB_v2> TBBlendedVertexVector;

	template<class T> void normaliseBoneWeights(int vertexIndex, T& v);
	void		createVertexList( VertexVector& vertices );
	void		createVertexList( BlendedVertexVector& vertices );
	void		createVertexList( UV2VertexVector& vertices );
	void		createVertexList( UV2BlendedVertexVector& vertices );
	void		createVertexList( TBVertexVector& vertices );
	void		createVertexList( TBBlendedVertexVector& vertices );
	void		writeMorphTargets( QuickFileWriter& f );
	void		writeMorphTargetsXml( DataSectionPtr spFile );

	bool		collectInitialTransforms( Skin& skin );
	void		normaliseInitialTransforms();
	void		initialPoseVertices();
	void		relaxedPoseVertices();

	NodeVector	boneNodes_;

	typedef std::vector< BoneVertex > BoneVVector;
	BoneVVector		boneVertices_;

	typedef std::vector< matrix4<float> >	MatrixVector;
	MatrixVector	initialTransforms_;

	matrix4<float>	initialObjectTransform_;

	VisualEnvelope( const VisualEnvelope& );
	VisualEnvelope& operator=( const VisualEnvelope& );
};


/**
 *	Templated function for assigning and normalising bone indices and weights.
 *
 *	@param	vertexIndex	The index of the bone vertex.
 *	@param	v			The vertex being assigned bone indices and weights.
 */
template<class T>
void VisualEnvelope::normaliseBoneWeights(int vertexIndex, T& v)
{
	// Check that the weights are in range
	MF_ASSERT(
		boneVertices_[ vertexIndex ].weight1 >= 0.0f &&
		boneVertices_[ vertexIndex ].weight1 <= 1.0f &&
		boneVertices_[ vertexIndex ].weight2 >= 0.0f &&
		boneVertices_[ vertexIndex ].weight2 <= 1.0f &&
		boneVertices_[ vertexIndex ].weight3 >= 0.0f &&
		boneVertices_[ vertexIndex ].weight3 <= 1.0f &&
		"VisualEnvelope::createVertexList - Bone weights outside [0..1] range!" );

	// Check that a valid bone is being assigned.  If a bone is not valid, set
	// the index to the root node and set the weighting to zero
	if( boneVertices_[ vertexIndex ].index1 >= (int)boneNodes_.size() )
	{
		v.index = 0;
		boneVertices_[ vertexIndex ].weight1 = 0.0f;

		float tempTotal =
			boneVertices_[ vertexIndex ].weight2 +
			boneVertices_[ vertexIndex ].weight3;
		if (tempTotal > 0.0f)
			boneVertices_[ vertexIndex ].weight2 /= tempTotal;

		boneVertices_[ vertexIndex ].weight3 =
			1.0f - boneVertices_[ vertexIndex ].weight2;
	}
	else
		v.index = boneVertices_[ vertexIndex ].index1;

	if( boneVertices_[ vertexIndex ].index2 >= (int)boneNodes_.size() )
	{
		v.index2 = 0;
		boneVertices_[ vertexIndex ].weight2 = 0.0f;

		float tempTotal = boneVertices_[ vertexIndex ].weight1 +
			boneVertices_[ vertexIndex ].weight3;
		if (tempTotal > 0.0f)
			boneVertices_[ vertexIndex ].weight1 /= tempTotal;

		boneVertices_[ vertexIndex ].weight3 =
			1.0f - boneVertices_[ vertexIndex ].weight1;
	}
	else
		v.index2 = boneVertices_[ vertexIndex ].index2;

	if( boneVertices_[ vertexIndex ].index3 >= (int)boneNodes_.size() )
	{
		v.index3 = 0;
		boneVertices_[ vertexIndex ].weight3 = 0.0f;

		float tempTotal = boneVertices_[ vertexIndex ].weight1 +
			boneVertices_[ vertexIndex ].weight2;
		if (tempTotal > 0.0f)
			boneVertices_[ vertexIndex ].weight1 /= tempTotal;

		boneVertices_[ vertexIndex ].weight2 =
			1.0f - boneVertices_[ vertexIndex ].weight1;
	}
	else
		v.index3 = boneVertices_[ vertexIndex ].index3;

	// If all bones are out of range, set weight1 to 1.0f and all other
	// weights to zero
	if (v.index == 0 && v.index2 == 0 && v.index3 == 0)
	{
		boneVertices_[ vertexIndex ].weight1 = 1.0f;
		boneVertices_[ vertexIndex ].weight2 = 0.0f;
		boneVertices_[ vertexIndex ].weight3 = 0.0f;
	}

	// When calculating the integer weightings, we determine the remainders
	// for each weight.
	float weight1Rem = 255.f * boneVertices_[ vertexIndex ].weight1 -
						(int)(255.f * boneVertices_[ vertexIndex ].weight1);
	float weight2Rem = 255.f * boneVertices_[ vertexIndex ].weight2 -
						(int)(255.f * boneVertices_[ vertexIndex ].weight2);
	float weight3Rem = 255.f * boneVertices_[ vertexIndex ].weight3 -
						(int)(255.f * boneVertices_[ vertexIndex ].weight3);

	// There are three possible sums:
	//
	//		1)	W1remainder + W2remainder + W3remainder = 0.0f
	//
	//		2)	W1remainder + W2remainder + W3remainder = 1.0f
	//
	//		3)	W1remainder + W2remainder + W3remainder = 2.0f
	//
	//		4)	W1remainder + W2remainder + W3remainder = 3.0f
	//
	// Note that case 4) is actually case one suffering from round off error
	//
	// Case 1)
	if ( weight1Rem + weight2Rem + weight3Rem < 0.1f )
	{
		v.weight = (uint8) (255.f * boneVertices_[ vertexIndex ].weight1);
		v.weight2 = (uint8) (255.f * boneVertices_[ vertexIndex ].weight2);

		// Sanity check
		MF_ASSERT(
			v.weight +
			v.weight2 +
			(uint8) (255.f * boneVertices_[ vertexIndex ].weight3)
				== 255);
	}
	// If 2) find the largest of the three remanders and round it up.  Round
	// the other 2 remainders down
	else if ( weight1Rem + weight2Rem + weight3Rem < 1.1f )
	{
		if ( weight1Rem > weight2Rem )
		{
			if ( weight1Rem > weight3Rem )
			{
				v.weight = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight1) + 1.0f);
				v.weight2 = (uint8) (255.f * boneVertices_[ vertexIndex ].weight2);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) (255.f * boneVertices_[ vertexIndex ].weight3)
						== 255);
			}
			else
			{
				v.weight = (uint8) (255.f * boneVertices_[ vertexIndex ].weight1);
				v.weight2 = (uint8) (255.f * boneVertices_[ vertexIndex ].weight2);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) ((255.f * boneVertices_[ vertexIndex ].weight3) + 1.0f)
						== 255);
			}
		}
		else
		{
			if ( weight2Rem > weight3Rem )
			{
				v.weight = (uint8) (255.f * boneVertices_[ vertexIndex ].weight1);
				v.weight2 = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight2) + 1.0f);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) (255.f * boneVertices_[ vertexIndex ].weight3)
						== 255);
			}
			else
			{
				v.weight = (uint8) (255.f * boneVertices_[ vertexIndex ].weight1);
				v.weight2 = (uint8) (255.f * boneVertices_[ vertexIndex ].weight2);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) ((255.f * boneVertices_[ vertexIndex ].weight3) + 1.0f)
						== 255);
			}
		}
	}
	// If 3) find the smallest of the three remanders and round it down.
	// Round the other 2 remainders up
	else if ( weight1Rem + weight2Rem + weight3Rem < 2.1f )
	{
		if ( weight1Rem < weight2Rem )
		{
			if ( weight1Rem < weight3Rem )
			{
				v.weight = (uint8) (255.f * boneVertices_[ vertexIndex ].weight1);
				v.weight2 = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight2) + 1.0f);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) ((255.f * boneVertices_[ vertexIndex ].weight3) + 1.0f)
						== 255);
			}
			else
			{
				v.weight = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight1) + 1.0f);
				v.weight2 = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight2) + 1.0f);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) (255.f * boneVertices_[ vertexIndex ].weight3)
						== 255);
			}
		}
		else
		{
			if ( weight2Rem < weight3Rem )
			{
				v.weight = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight1) + 1.0f);
				v.weight2 = (uint8) (255.f * boneVertices_[ vertexIndex ].weight2);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) ((255.f * boneVertices_[ vertexIndex ].weight3) + 1.0f)
						== 255);
			}
			else
			{
				v.weight = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight1) + 1.0f);
				v.weight2 = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight2) + 1.0f);

				// Sanity check
				MF_ASSERT(
					v.weight +
					v.weight2 +
					(uint8) (255.f * boneVertices_[ vertexIndex ].weight3)
						== 255);
			}
		}
	}
	// Case 4), which is the same as case 1 above
	else if ( weight1Rem + weight2Rem + weight3Rem < 3.1f )
	{
		v.weight = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight1) + 1.0f);
		v.weight2 = (uint8) ((255.f * boneVertices_[ vertexIndex ].weight2) + 1.0f);

		// Sanity check
		MF_ASSERT(
			v.weight +
			v.weight2 +
			(uint8) ((255.f * boneVertices_[ vertexIndex ].weight3) + 1.0f)
				== 255);
	}
}


#ifdef CODE_INLINE
#include "visual_envelope.ipp"
#endif

#endif // VISUAL_ENVELOPE_HPP
