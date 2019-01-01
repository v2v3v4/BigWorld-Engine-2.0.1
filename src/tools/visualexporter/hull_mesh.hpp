/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HULL_MESH_HPP
#define HULL_MESH_HPP

#include "mfxnode.hpp"
#include "visual_mesh.hpp"
#include "visual_portal.hpp"
#include "math/planeeq.hpp"
#include <map>

typedef SmartPointer<class HullMesh> HullMeshPtr;

/**
 *	This class ...
 *
 *	@todo Document this class.
 */
class HullMesh : public VisualMesh
{
public:
	void save( DataSectionPtr pSection );
	bool hasPortal() const	{	return !portals_.empty();	}
private:
	std::vector<PlaneEq>		planeEqs_;
	typedef std::map<int,VisualPortalPtr> PortalMap;
	PortalMap portals_;

	int addUniquePlane( const PlaneEq& peq );
	int findPlane( const PlaneEq& peq );
	bool isPortal( uint materialIndex );

	struct BiIndex
	{
		BiIndex();
		BiIndex(uint i1, uint i2);

		uint& operator[](uint index);
		const uint& operator[](uint& index);

		uint i1;
		uint i2;
	};

	struct TriIndex
	{
		TriIndex();
		TriIndex(uint i1, uint i2, uint i3);

		uint& operator[](uint index);
		const uint& operator[](uint& index);

		uint i1;
		uint i2;
		uint i3;
	};

	void		extractPlanesAndPortals	(	VertexVector& vv, std::vector<TriIndex>& faces );
	Triangle*	findOriginalTriangle( TriIndex& face );
	
	bool	makeConvex				(	VertexVector& vertices, std::vector<TriIndex>& faces ) const;
	bool	mcCreateMaxTetrahedron	(	const VertexVector& vertices, std::vector<uint>& verts,
										std::vector<TriIndex>& faces, const float epsilon = 0.0001f ) const;
	void	mcRemoveVertex			(	const uint vertIndex, const VertexVector& vertices,
										std::vector<uint>& verts, const float epsilon = 0.0001f ) const;
	bool	mcRemoveInsideVertices	(	const VertexVector& vertices, std::vector<uint>& verts,
										const std::vector<TriIndex>& faces ) const;
	uint	mcFindNextVert			(	const VertexVector& vertices, const std::vector<uint>& verts,
										const std::vector<TriIndex>& faces ) const;
	void	mcExpandHull			(	const uint vertIndex, const VertexVector& vertices,
										std::vector<uint>& verts, std::vector<TriIndex>& faces ) const;
	bool	mcIsInternalEdge		(	std::vector<TriIndex>& faces, const std::vector<uint>& frontFacing,
										const uint vertIndex1, const uint vertIndex2) const;
	void	mcAddFaces				(	const uint vertIndex, const std::vector<uint>& horizonVerts,
										std::vector<TriIndex>& faces ) const;
};

#endif // HULL_MESH_HPP
