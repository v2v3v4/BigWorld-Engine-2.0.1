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
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/binaryfile.hpp"
#include "math/boundbox.hpp"
#include "resmgr/primitive_file.hpp"
#include "moo/primitive_file_structs.hpp"
#include "moo/index_buffer.hpp"
#include "moo/vertex_formats.hpp"
#include "matrix3.hpp"
#include "mesh.hpp"
#include "vertex.hpp"
#include "material.hpp"

class QuickFileWriter;
typedef SmartPointer<class VisualMesh> VisualMeshPtr;

class Color
{
public:
	Color( float r = 1.0f, float g = 0.5f, float b = 0.25f ) : r( r ), g( g ), b( b ) {}
	
	float r, g, b;
};

/**
 *	This class ...
 *
 *	@todo Document this class.
 */
class VisualMesh : public ReferenceCount
{
public:
	VisualMesh();
	virtual ~VisualMesh();
	bool init( Mesh& mesh );

	void exportMorphAnims( BinaryFile& animFile, bool stripRefPrefix = false );
	void exportMorphAnimsXml(
		const std::string& animDebugXmlFileName, const std::string& animName,
		bool stripRefPrefix = false );

	//return all resources used
	void resources( std::vector<std::string>& retResources );
	struct BloatVertex
	{
		vector3<float>	pos;
		vector3<float>	normal;
		Point4  colour;
		Point2	uv;
		Point2	uv2;
		int		vertexIndex;
		int		smoothingGroup;
		vector3<float>	binormal;
		vector3<float>	tangent;
		int		meshIndex;			// for creating MeshParticles
		bool operator == ( const BloatVertex& bv ) const 
		{ 
			return  this->pos == bv.pos &&
			this->normal == bv.normal &&
			this->colour == bv.colour &&
			this->uv == bv.uv &&
			this->uv2 == bv.uv2 &&
			this->vertexIndex == bv.vertexIndex &&
			(this->smoothingGroup & bv.smoothingGroup) &&
			this->binormal == bv.binormal &&
			this->tangent == bv.tangent &&
			this->meshIndex == bv.meshIndex;
		};

		Vector3 v3Pos() const	{ return Vector3(pos.x,pos.y,pos.z); }
	};

	typedef std::vector< BloatVertex >  BVVector;

	struct Triangle
	{
		int		index[3];
		int		realIndex[3];
		int		smoothingGroup;
		int		materialIndex;
	};

	typedef std::vector< Triangle > TriangleVector;

	struct Material
	{
		Material()
		: identifier( "empty" ),
		  mapIdMeaning( 0 ),
		  selfIllum( 0 ),
		  inUse( false )
		{
		}

		//return all resources used
		void resources( std::vector<std::string>& retResources );

		std::string identifier;
		std::string mapIdentifier;
		int mapIdMeaning;	// 0 = none, 1 = bitmap, 2 = mfm
		float selfIllum;
		Color ambient;
		Color diffuse;
		Color specular;
		bool inUse;

		bool operator==( const Material& m ) const
		{
			return identifier == m.identifier &&
				mapIdMeaning == m.mapIdMeaning &&
				mapIdentifier == m.mapIdentifier &&
				selfIllum == m.selfIllum;

		}

//		void save( DataSectionPtr spGeometry, uint32 pgid, bool skinned = false, int nodeCount = 0 ) const;
		enum SkinType
		{
			NO_SKIN,
			SOFT_SKIN
		};

		void save( DataSectionPtr pGeometry,
			DataSectionPtr spExistingVisual,
			uint32 pgid,
			bool skinned = false,
			int nodeCount = 0,
			SkinType skin = NO_SKIN,
			bool skinnedShader = false ) const;

		bool findAndCopyExistingMaterial( const std::string& identifier,
				DataSectionPtr pPrimGroup,
				DataSectionPtr spExistingVisual ) const;

		DataSectionPtr findExistingMaterial( const std::string& identifier,
				DataSectionPtr spExistingVisual ) const;
	};

	typedef std::vector< Material > MaterialVector;

	virtual bool save( DataSectionPtr spVisualSection,
		DataSectionPtr spExistingVisual,
		const std::string& primitiveFile,
		bool useIdentifier );

	virtual bool savePrimXml( const std::string& xmlFile );

	virtual bool isVisualEnvelope() { return false; }

	virtual void add( VisualMesh & destMesh, int forceMeshIndex = -1, bool worldSpace = true );

	bool largeIndices() const { return largeIndices_; }

	bool dualUV() const { return dualUV_; }
	void dualUV( bool value ){ dualUV_ = value; }

	bool vertexColours() const { return vertexColours_; }
	void vertexColours( bool value ){ vertexColours_ = value; }

	const BoundingBox& bb() const { return bb_; };

	void snapVertices( bool snapVertices ) { snapVertices_ = snapVertices; }
	
	MObject object; // Maya object for mesh, isNull() returns true except for blend shapes

	const std::string& fullName() { return fullName_; }

	std::string			getIdentifier() { return this->identifier_; }

protected:
	struct MorphVertex
	{
		vector3<float>	delta;
		int		vertexIndex;
	};
	struct MorphTarget
	{
		std::string	identifier;
		int			channelIndex;
		std::vector< MorphVertex > verts;
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
	typedef std::vector<Moo::PrimitiveGroup> PGVector;
	typedef std::vector<Moo::VertexXYZNUV> VertexVector;
	typedef std::vector<Moo::VertexXYZNUV2> UV2VertexVector;
	typedef std::vector<Moo::VertexXYZNDUV> DiffuseVertexVector;
	typedef std::vector<Moo::VertexXYZNUVI> IndexedVertexVector;
	typedef std::vector<Moo::VertexXYZNUVTB> TBVertexVector;

	uint32	addVertex( const BloatVertex& bv );
	float	normalAngle( const Triangle& tri, uint32 index );
	void	addNormal( const vector3<float>& normal, int realIndex, int smoothingGroup, int index );
	void	addMaterial( material& mtl );
	void	gatherMaterials( Mesh& mesh );
	int		findOrAddMaterial( const Material & mat );
	void	flipTriangleWindingOrder();
	void	sortTriangles();
	void	removeDuplicateVertices();
	bool	createPrimitiveGroups( PGVector& primGroups, Moo::IndicesHolder& indices );
	bool	createMeshParticlePrimGroups( PGVector& primGroups );
	void	createVertexList( VertexVector& vertices );
	void	createVertexList( UV2VertexVector& vertices );
	void	createVertexList( DiffuseVertexVector& vertices );
	void	createVertexList( IndexedVertexVector& vertices );
	void	createVertexList( TBVertexVector& vertices );
	void	writeMorphTargets( QuickFileWriter& f );
	void	writeMorphTargetsXml( DataSectionPtr spFile );
	void	makeBumped();

	bool			vertexColours_;
	bool			dualUV_;
	BVVector		vertices_;
	TriangleVector	triangles_;

	MaterialVector	materials_;

	typedef std::vector<int> RemapVector;
	RemapVector		materialRemap_;

	std::vector< MorphTarget > morphTargets_;

	std::string identifier_;
	std::string nodeIdentifier_;
	std::string fullName_;
	BoundingBox bb_;

	matrix3<float>		world_;

	bool		snapVertices_;
	bool		largeIndices_;

	VisualMesh( const VisualMesh& );
	VisualMesh& operator=( const VisualMesh& );
};


std::string stripReferencePrefix(std::string& nodeName);


#ifdef CODE_INLINE
#include "visual_mesh.ipp"
#endif

#endif // VISUAL_MESH_HPP
