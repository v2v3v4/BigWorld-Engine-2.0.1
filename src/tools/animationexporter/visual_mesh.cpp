/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4530 )

#include <strstream>
#include <algorithm>
#include "mfxexp.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/quick_file_writer.hpp"
#include "utility.hpp"
#include "expsets.hpp"
#include "cstdmf/binaryfile.hpp"
#include "wm3.h"

#include "visual_mesh.hpp"

#ifndef CODE_INLINE
#include "visual_mesh.ipp"
#endif


/**
 *	Helper method that returns the current time.
 *	@return The current time.
 */
TimeValue timeNow()
{
	return ExportSettings::instance().staticFrame() * GetTicksPerFrame();
}

// -----------------------------------------------------------------------------
// Section: VisualMesh
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
VisualMesh::VisualMesh()
{
}


/**
 *	Destructor.
 */
VisualMesh::~VisualMesh()
{
}


/**
 *	This method finds the angle between two edges in a triangle.
 *
 *	@param tri	The triangle.
 *	@param index The index of the vertex whose angle we want.
 *	@return The angle (in radians) between the two edges.
 */
float VisualMesh::normalAngle( const Triangle& tri, uint32 index )
{
	Point3 v1 = vertices_[tri.index[(index + 1 ) % 3]].pos - vertices_[tri.index[index]].pos;
	Point3 v2 = vertices_[tri.index[(index + 2 ) % 3]].pos - vertices_[tri.index[index]].pos;


	float len = Length( v1 );
	if (len == 0)
		return 0;
	v1 /= len;

	len = Length( v2 );
	if (len == 0)
		return 0;
	v2 /= len;

	float normalAngle = DotProd( v1, v2 );

	normalAngle = min( 1.f, max( normalAngle, -1.f ) );

	return acosf( normalAngle );	
}


/**
 *	This method adds a normal to all vertices that should be influenced by it.
 *
 *	@param normal			The normal to add.
 *	@param realIndex			The index of the vertex in the max mesh.
 *	@param smoothingGroup	The smoothinggroup of this normal.
 *	@param index				The index of the vertex this normal has to influence.
 */
void VisualMesh::addNormal( const Point3& normal, int realIndex, int smoothingGroup, int index )
{
	for (uint i = 0; i < vertices_.size(); i++)
	{
		BloatVertex& bv = vertices_[i];
		if ((bv.vertexIndex == realIndex && 
			(bv.smoothingGroup & smoothingGroup)) || 
			index == i )
		{
			bv.normal += normal;
		}
	}
}	


/**
 *	This method creates vertex normals for all vertices in the vertex list.
 */
void VisualMesh::createNormals( )
{
	TriangleVector::iterator it = triangles_.begin();
	TriangleVector::iterator end = triangles_.end();

	while (it!=end)
	{
		Triangle& tri = *it++;
		Point3 v1 = vertices_[tri.index[1]].pos - vertices_[tri.index[0]].pos;
		Point3 v2 = vertices_[tri.index[2]].pos - vertices_[tri.index[0]].pos;
		Point3 normal = v1^v2;
		normal = Normalize( normal );

		addNormal( normal * normalAngle( tri, 0), tri.realIndex[0], tri.smoothingGroup, tri.index[0] );
		addNormal( normal * normalAngle( tri, 1), tri.realIndex[1], tri.smoothingGroup, tri.index[1] );
		addNormal( normal * normalAngle( tri, 2), tri.realIndex[2], tri.smoothingGroup, tri.index[2] );
	}

	BVVector::iterator vit = vertices_.begin();
	BVVector::iterator vend = vertices_.end();
	while (vit != vend)
	{
		BloatVertex& bv = *vit++;
		bv.normal = Normalize( bv.normal );
	}
}


/**
 *	This method takes a max material and stores the material data we want to export.
 *
 *	@param mtl	The max material to export.
 */
void VisualMesh::addMaterial( StdMat* mtl )
{
	Material m;
	m.ambient = mtl->GetAmbient( timeNow() );
	m.diffuse = mtl->GetDiffuse( timeNow() );
	m.specular = mtl->GetSpecular( timeNow() );
	m.selfIllum = mtl->GetSelfIllum( timeNow() );
	m.identifier = mtl->GetName();

	if( mtl->MapEnabled( ID_DI ) )
	{
		Texmap *map = mtl->GetSubTexmap( ID_DI );
		if( map )
		{
			if( map->ClassID() == Class_ID( BMTEX_CLASS_ID, 0x00 ) )
			{
				std::string textureName = unifySlashes(toLower(((BitmapTex *)map)->GetMapName()));
				textureName = BWResolver::dissolveFilename( textureName );
				if (textureName.length())
				{
					m.hasMap = true;
					m.mapIdentifier = textureName;
				}
			}
		}
	}

	uint i;
	for (i = 0; i < materials_.size(); i++)
	{
		if (materials_[i] == m)
			break;
	}
	materialRemap_.push_back( i );
	if (i == materials_.size())		
		materials_.push_back( m );
}


/**
 *	This method gathers all the materials contained in a specific node.
 *
 *	@param node	The node to gather materials from.
 */
void VisualMesh::gatherMaterials( INode* node )
{
	Mtl* mtl = node->GetMtl();
	if (mtl)
	{
		if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
		{
			addMaterial( (StdMat*)mtl );
		}
		else if (mtl->NumSubMtls())
		{
			for (int i = 0; i < mtl->NumSubMtls(); i++)
			{
				Mtl* submat = mtl->GetSubMtl( i );
				if (submat && submat->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
				{
					addMaterial( (StdMat*)submat );
				}
				else
				{
					Material m;
					m.identifier = submat ? submat->GetName() : "Default";
					materialRemap_.push_back( materials_.size() );
					materials_.push_back( m );
				}
			}
		}
		else
		{
			Material m;
			m.identifier = mtl->GetName();
			materialRemap_.push_back( materials_.size() );
			materials_.push_back( m );
		}
	}
	else
	{
		Material m;
		m.identifier = "Default";
		materialRemap_.push_back( materials_.size() );
		materials_.push_back( m );
	}

	if (!materialRemap_.size())
	{
		materialRemap_.push_back( 0 );
	}
}


/**
 *	This method swaps the triangle winding order for all triangles.
 */
void VisualMesh::flipTriangleWindingOrder()
{
	TriangleVector::iterator it = triangles_.begin();
	TriangleVector::iterator end = triangles_.end();

	while (it!=end)
	{
		Triangle& tri = *it++;
		swap( tri.index[1], tri.index[2] );
		swap( tri.realIndex[1], tri.realIndex[2] );
	}

}


/**
 *	A helper compare function that orders two triangles based on their material index.
 *
 *	@param	t1	The first triangle.
 *	@param	t2	The second triangle.
 *	@return	Does t1 preceed t2 based on their material index.
 */
bool triangleSorter( const VisualMesh::Triangle& t1, const VisualMesh::Triangle& t2 )
{
	return t1.materialIndex < t2.materialIndex;
}


/**
 *	This method sorts the triangles.
 */
void VisualMesh::sortTriangles()
{
	std::sort( triangles_.begin(), triangles_.end(), triangleSorter );
}


/**
 *	This method creates our primitive groups.
 *
 *	@param primGroups the output primitivegroups
 *	@param indices the outpur indices
 */
void VisualMesh::createPrimitiveGroups( PGVector& primGroups, IndexVector& indices )
{
	if (triangles_.size())
	{
		int mIndex = triangles_.front().materialIndex;

		int firstVertex = triangles_.front().index[0];
		int lastVertex = triangles_.front().index[0];
		int firstTriangle = 0;

		for (uint i = 0; i <= triangles_.size(); i++)
		{
			Triangle& tri = triangles_[i];
			if (i == triangles_.size() || tri.materialIndex != mIndex)
			{
				PrimitiveGroup pg;
				pg.startVertex = firstVertex;
				pg.nVertices = lastVertex - firstVertex + 1;
				pg.startIndex = firstTriangle * 3;
				pg.nPrimitives = i - firstTriangle;
				primGroups.push_back( pg );	
				firstVertex = tri.index[0];
				lastVertex = tri.index[0];
				firstTriangle = i;
				materials_[ mIndex ].inUse = true;
				mIndex = tri.materialIndex;
			}
			if (i!=triangles_.size())
			{
				indices.push_back( tri.index[0] );
				indices.push_back( tri.index[1] );
				indices.push_back( tri.index[2] );
				firstVertex = min( firstVertex, min( tri.index[0], min( tri.index[1], tri.index[2] ) ) );
				lastVertex  = max( lastVertex,  max( tri.index[0], max( tri.index[1], tri.index[2] ) ) );
			}
		}
	}
}


/**
 *	This method creates our output vertex list.
 *
 *	@param	vertices The output vertex list.
 */
void VisualMesh::createVertexList( VertexVector& vertices )
{
	VertexXYZNUV v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();
	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();
		v.pos[0] = bv.pos.x;
		v.pos[1] = bv.pos.z;
		v.pos[2] = bv.pos.y;
		v.normal[0] = bv.normal.x;
		v.normal[1] = bv.normal.z;
		v.normal[2] = bv.normal.y;
		v.uv[0] = bv.uv.x;
		v.uv[1] = bv.uv.y;
		vertices.push_back( v );
	}
}


/**
 *	This method saves the visual mesh.
 *
 *	@param spVisualSection DataSection of the .visual file.
 *	@param primitiveFile The filename of the primitive record.
 */
void VisualMesh::save( DataSectionPtr spVisualSection, const std::string& primitiveFile )
{
	DataSectionPtr spFile = BWResource::openSection( primitiveFile );
	if (!spFile)
		spFile = new BinSection( primitiveFile, new BinaryBlock( 0, 0, "BinaryBlock/VisualMesh" ) );

	if (spFile)
	{
		flipTriangleWindingOrder();
		sortTriangles();

		PGVector primGroups;
		IndexVector indices;

		createPrimitiveGroups( primGroups, indices );

		IndexHeader ih;
		bw_snprintf( ih.indexFormat, sizeof(ih.indexFormat), "list" );
		ih.nIndices = indices.size();
		ih.nTriangleGroups = primGroups.size();

		QuickFileWriter f;
		f << ih;
		f << indices;
		f << primGroups;

		spFile->writeBinary( identifier_ + ".indices", f.output() );

		f = QuickFileWriter();

		VertexVector vertices;
		createVertexList( vertices );
		VertexHeader vh;
		vh.nVertices = vertices.size();
		bw_snprintf( vh.vertexFormat, sizeof(vh.vertexFormat), "xyznuv" );
		f << vh;
		f << vertices;

		if (morphTargets_.size())
		{
			writeMorphTargets( f );
		}

		if (morphTargets_.size())
			spFile->writeBinary( identifier_ + ".mvertices", f.output() );
		else
			spFile->writeBinary( identifier_ + ".vertices", f.output() );

		spFile->save( primitiveFile );

		DataSectionPtr spRenderSet = spVisualSection->newSection( "renderSet" );
		if (spRenderSet)
		{
			spRenderSet->writeBool( "treatAsWorldSpaceObject", false );
			spRenderSet->writeString( "node", identifier_ );
			DataSectionPtr spGeometry = spRenderSet->newSection( "geometry" );
			if (spGeometry)
			{
				if (morphTargets_.size())
					spGeometry->writeString( "vertices", identifier_ + ".mvertices" );
				else
					spGeometry->writeString( "vertices", identifier_ + ".vertices" );
				spGeometry->writeString( "primitive", identifier_ + ".indices" );

				MaterialVector::iterator it = materials_.begin();
				MaterialVector::iterator end = materials_.end();

				uint32 primGroup = 0;
				for (uint32 i = 0; i < materials_.size(); i++)
				{
					Material& mat = materials_[i];;
					if (mat.inUse)
					{
						DataSectionPtr spPG = spGeometry->newSection( "primitiveGroup" );
						if (spPG)
						{
							spPG->setInt( primGroup++ );
							spPG->writeString( "material/identifier", mat.identifier );
							spPG->writeFloat( "material/selfIllumination", mat.selfIllum );
							if (mat.hasMap)
								spPG->writeString( "material/texture", mat.mapIdentifier );
						}
					}
				}
			}
		}
	}
}


/**
 *	This method adds a vertex to our vertex list.
 *
 *	@param bv Vertex to add to the vertexlist.
 *	@return Index of the vertex in the vector.
 */
uint16 VisualMesh::addVertex( const VisualMesh::BloatVertex& bv )
{
	BVVector::iterator it = std::find( vertices_.begin(), vertices_.end(), bv );

	if (it != vertices_.end())
	{
		it->smoothingGroup |= bv.smoothingGroup;
		uint16 i = 0;
		while (it != vertices_.begin())
		{
			++i;
			--it;
		}
		return i;
	}
	else
	{
		vertices_.push_back( bv );
		return vertices_.size() - 1;
	}
}


/**
 *	This method writes the morph targets out to the passed in
 *	file writer.
 *
 *	@param	f	The file writer to write the morph targets to.
 */
void VisualMesh::writeMorphTargets( QuickFileWriter& f )
{
	ExportMorphHeader mh;
	mh.nMorphTargets = morphTargets_.size();
	mh.version = 0x100;

	f << mh;

	for (uint i = 0; i < morphTargets_.size(); i++)
	{
		MorphTarget& mt = morphTargets_[i];
		ExportMorphTarget emt;
		emt.channelIndex = mt.channelIndex;
		bw_snprintf( emt.identifier, sizeof(emt.identifier), "%s", mt.identifier.c_str() );
		std::vector< ExportMorphVertex > emvs;

		std::vector< MorphVertex >::iterator it = mt.verts.begin();
		std::vector< MorphVertex >::iterator end = mt.verts.end();

		while (it != end)
		{
			MorphVertex& mv = *it++;
			ExportMorphVertex emv;
			emv.delta[0] = mv.delta.x * 100 * ExportSettings::instance().unitScale();
			emv.delta[1] = mv.delta.z * 100 * ExportSettings::instance().unitScale();
			emv.delta[2] = mv.delta.y * 100 * ExportSettings::instance().unitScale();

			for (uint j = 0; j < vertices_.size(); j++)
			{
				if (vertices_[j].vertexIndex == mv.vertexIndex)
				{
					emv.index = j;
					emvs.push_back( emv );
				}
			}
		}
		emt.nVertices = emvs.size();
		f << emt;
		f << emvs;
	}
}


/**
 *	This method captures the morph target information for the passed
 *	node using the Morpher modifier.
 *
 *	@param	node	The node to capture the morph targets from.
 */
void VisualMesh::captureMorphTargets( INode* node )
{
	Modifier* mod = MFXExport::findMorphModifier( node );
	if (mod)
	{
		Matrix3 meshMatrix = node->GetObjectTM( timeNow() );
		Matrix3 nodeMatrix = node->GetNodeTM( timeNow() );
		if( !ExportSettings::instance().allowScale() )
			nodeMatrix = normaliseMatrix( nodeMatrix );

		Matrix3 scaleMatrix = meshMatrix * Inverse( nodeMatrix );
		scaleMatrix.SetRow( 3, Point3::Origin );

		MorphR3* morph = static_cast<MorphR3*>(mod);
		for (int i = 0; i < MR3_NUM_CHANNELS; i++)
		{
//#pragma message( "fix morphing!" ) 
//#if 0
			morphChannel& channel = morph->chanBank[i];
			if (channel.mActive)
			{
				MorphTarget mt;
				mt.channelIndex = i;
				mt.identifier = channel.mName;
/*				for (int j = 0; j < channel.nmPts; j++)
				{
					int index = channel.mOptdata[j];
					MorphVertex mv;
					mv.delta = scaleMatrix * channel.mDeltas[index];
					mv.vertexIndex = index;
					mt.verts.push_back( mv );
				}*/
				if (channel.cblock)
				{
					IParamBlock* pb = channel.cblock;
					int firstFrame = ExportSettings::instance().firstFrame();
					int lastFrame = ExportSettings::instance().lastFrame();
					for (int i = firstFrame; i <= lastFrame; i++)
					{
						float f;
						Interval ivalid;
						if (pb->GetValue( 0, i * GetTicksPerFrame(), f, ivalid))
						{
							mt.animationChannel.push_back( min( 100.f, max( 0.f, f ) ) / 100.f );
						}
						else
						{
							mt.animationChannel.push_back( 0.f );
						}

					}
				}
				morphTargets_.push_back( mt );
			}
//#endif
		}
	}
}


/**
 *	This method returns the number of morph target animation channels being
 *	used in total by all the morph targets.
 *
 *	@return	The number of morph target animation channels
 */
int VisualMesh::nMorphAnims() const
{
	int ret = 0;
	for (uint i = 0; i < morphTargets_.size(); i++)
	{
		if (morphTargets_[i].animationChannel.size())
		{
			ret++;
		}
	}
	return ret;
}

/**
 *	This method checks whether there are any duplicate morph target names in the mesh
 *	@param morphNames list of known morph target names. The names in this mesh are added to this list.
 *	@param duplicate list of duplicate morph target names.
 */
void VisualMesh::duplicateMorphNames( std::set<std::string>& morphNames, std::set<std::string>& duplicates )
{
	for (uint i = 0; i < morphTargets_.size(); i++)
	{
		if (morphTargets_[i].animationChannel.size())
		{
			const std::string& id = morphTargets_[i].identifier;
			if (morphNames.find(id) == morphNames.end())
			{
				morphNames.insert( id );
			}
			else
			{
				duplicates.insert( id );
			}
		}
	}
}


/**
 *	This method writes out the morph target animation information to
 *	the passed animation file.
 *
 *	@param	animFile	The file to write the animation to.
 */
void VisualMesh::exportMorphAnims( BinaryFile& animFile )
{
	for (uint i = 0; i < morphTargets_.size(); i++)
	{
		MorphTarget& mt = morphTargets_[i];

		if (mt.animationChannel.size())
		{
			animFile << int(2);
			animFile << mt.identifier;
			animFile.writeSequence( mt.animationChannel );
		}
	}
}


/**
 *	This method initalises the visual mesh.
 *
 *	@param node The node of the mesh we want to export.
 *	@return	Success or failure.
 */
bool VisualMesh::init( INode* node )
{
	bool needDel = false;
	ConditionalDeleteOnDestruct<TriObject> triObject( MFXExport::getTriObject( node, timeNow(), needDel));
	if (! (&*triObject))
		return false;
	triObject.del( needDel );
	Mesh* mesh = &triObject->mesh;

	bool ret = false;
	if (mesh->getNumFaces() && mesh->getNumVerts())
	{
		identifier_ = trimWhitespaces( node->GetName() );
		gatherMaterials( node );
		bool hasUVs = mesh->getNumTVerts() && mesh->tvFace;
		BloatVertex bv;
		bv.normal = Point3::Origin;
		bv.uv = Point2::Origin;

		Matrix3 meshMatrix = node->GetObjectTM( timeNow() );
		Matrix3 nodeMatrix = node->GetNodeTM( timeNow() );
		if( !ExportSettings::instance().allowScale() )
			nodeMatrix = normaliseMatrix( nodeMatrix );

		Matrix3 scaleMatrix = meshMatrix * Inverse( nodeMatrix );

		for (int i = 0; i < mesh->getNumFaces(); i++)
		{
			Face f = mesh->faces[ i ];

			Triangle tr;
			tr.materialIndex = materialRemap_[ f.getMatID() % materialRemap_.size() ];
			tr.smoothingGroup = f.smGroup;
			for (int j = 0; j < 3; j++)
			{
				bv.pos = scaleMatrix * mesh->verts[ f.v[j] ];
				bv.vertexIndex = f.v[j];
				bv.smoothingGroup = f.smGroup;
				if (hasUVs)
				{
					bv.uv = reinterpret_cast<Point2&>( mesh->tVerts[ mesh->tvFace[i].t[j] ] );
					bv.uv.y = 1 - bv.uv.y;
				}
				tr.index[j] = addVertex( bv );
				tr.realIndex[j] = bv.vertexIndex;
			}
			triangles_.push_back( tr );
		}

		captureMorphTargets( node );

		bool nodeMirrored = isMirrored( nodeMatrix );
		bool meshMirrored = isMirrored( meshMatrix );

		if (nodeMirrored ^ meshMirrored)
			flipTriangleWindingOrder();

/*		if (isMirrored( meshMatrix ))
			flipTriangleOrder();

		if (isMirrored( nodeMatrix ))
			flipTriangleWindingOrder();*/

		createNormals( );

		// initialise boundingbox
		bb_.setBounds(
			reinterpret_cast<const Vector3&>( meshMatrix * mesh->verts[ 0 ] ),
			reinterpret_cast<const Vector3&>( meshMatrix * mesh->verts[ 0 ] ));
		for (int i = 1; i < mesh->getNumVerts(); i++)
		{
			bb_.addBounds( reinterpret_cast<const Vector3&>( meshMatrix * mesh->verts[ i ] ) );
		}

		if (nodeMirrored)
			flipTriangleWindingOrder();

		ret = true;
	}

	return ret;
}

// visual_mesh.cpp
