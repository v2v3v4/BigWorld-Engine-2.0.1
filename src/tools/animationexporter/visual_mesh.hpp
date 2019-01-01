/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_MESH_HPP
#define VISUAL_MESH_HPP


#include <vector>
#include <set>
#include "max.h"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"
#include "math/boundbox.hpp"


class QuickFileWriter;
typedef SmartPointer<class VisualMesh> VisualMeshPtr;

/**
 *	This class is used to convert 3dsMax objects into BigWorld visual
 *	mesh object.
 */
class VisualMesh : public ReferenceCount
{
public:
	/**
	 *	The vertex format captured from 3dsMax.
	 */
	struct BloatVertex
	{
		Point3	pos;
		Point3	normal;
		Point2	uv;
		int		vertexIndex;
		int		smoothingGroup;
		bool operator == ( const BloatVertex& bv ) const 
		{ 
			return  this->pos == bv.pos &&
			this->normal == bv.normal &&
			this->uv == bv.uv &&
			this->vertexIndex == bv.vertexIndex &&
			(this->smoothingGroup & bv.smoothingGroup);
		};
	};

	/**
	 *	The triangle format captured from 3dsMax.
	 */
	struct Triangle
	{
		int		index[3];
		int		realIndex[3];
		int		smoothingGroup;
		int		materialIndex;
	};

	/**
	 *	The material format captured from 3dsMax.
	 */
	struct Material
	{
		Material()
		: identifier( "empty" ),
		  hasMap( false ),
		  selfIllum( 0 ),
		  inUse( false )
		{
		}
		std::string identifier;
		bool hasMap;
		std::string mapIdentifier;
		float selfIllum;
		Color ambient;
		Color diffuse;
		Color specular;
		bool inUse;
		bool operator== (const Material& m) const
		{
			return identifier == m.identifier &&
				hasMap == m.hasMap &&
				mapIdentifier == m.mapIdentifier &&
				selfIllum == m.selfIllum;

		}
	};

	// Type definitions
	typedef std::vector< BloatVertex >  BVVector;
	typedef std::vector< Triangle > TriangleVector;
	typedef std::vector< Material > MaterialVector;

	// Constructor / destructor
	VisualMesh();
	virtual ~VisualMesh();

	// Public interface
	bool				init(INode* node);
	virtual	void		save( DataSectionPtr spVisualSection, const std::string& primitiveFile );
	const BoundingBox&	bb() const { return bb_; };
	int					nMorphAnims() const;
	void				exportMorphAnims(class BinaryFile& animFile);

	void				duplicateMorphNames( std::set<std::string>& morphNames, std::set<std::string>& duplicates );

protected:
	struct VertexXYZNUV
	{
		float pos[3];
		float normal[3];
		float uv[2];
	};

	struct VertexHeader
	{
		char	vertexFormat[ 64 ];
		int		nVertices;
	};

	struct IndexHeader
	{
		char	indexFormat[ 64 ];
		int		nIndices;
		int		nTriangleGroups;
	};

	struct PrimitiveGroup
	{
		int startIndex;
		int nPrimitives;
		int startVertex;
		int nVertices;
	};

	struct MorphVertex
	{
		Point3	delta;
		int		vertexIndex;
	};
	
	struct MorphTarget
	{
		std::string	identifier;
		int			channelIndex;
		std::vector< MorphVertex > verts;
		std::vector< float > animationChannel;
	};

	struct ExportMorphHeader
	{
		int version;
		int nMorphTargets;
	};

	struct ExportMorphTarget
	{
		char	identifier[64];
		int		channelIndex;
		int		nVertices;
	};

	struct ExportMorphVertex
	{
		float	delta[3];
		int		index;
	};

	typedef std::vector<uint16> IndexVector;
	typedef std::vector<PrimitiveGroup> PGVector;
	typedef std::vector<VertexXYZNUV> VertexVector;

	void	captureMorphTargets( INode* node );
	uint16	addVertex( const BloatVertex& bv );
	void	createNormals( );
	float	normalAngle( const Triangle& tri, uint32 index );
	void	addNormal( const Point3& normal, int realIndex, int smoothingGroup, int index );
	void	addMaterial( StdMat* mtl );
	void	gatherMaterials( INode* node );
	void	flipTriangleWindingOrder();
	void	sortTriangles();
	void	createPrimitiveGroups( PGVector& primGroups, IndexVector& indices );
	void	createVertexList( VertexVector& vertices );
	void	writeMorphTargets( QuickFileWriter& f );

	VisualMesh( const VisualMesh& );
	VisualMesh& operator=( const VisualMesh& );

	// Member variables
	typedef std::vector<int> RemapVector;
	std::vector<MorphTarget> morphTargets_;
	RemapVector		materialRemap_;

	std::string		identifier_;
	BoundingBox		bb_;
	BVVector		vertices_;
	TriangleVector	triangles_;
	MaterialVector	materials_;
};


#ifdef CODE_INLINE
#include "visual_mesh.ipp"
#endif

#endif // VISUAL_MESH_HPP
