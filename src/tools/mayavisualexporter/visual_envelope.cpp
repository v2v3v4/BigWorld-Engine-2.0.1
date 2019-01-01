/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#pragma warning (disable:4786)

//~ #include "mfxexp.hpp"
#include "visual_envelope.hpp"
#include "expsets.hpp"
#include "utility.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/quick_file_writer.hpp"
#include "resmgr/xml_section.hpp"

#ifndef CODE_INLINE
#include "visual_envelope.ipp"
#endif


static inline uint32 packNormal( const Point3&n )
{
	if (ExportSettings::instance().useLegacyOrientation())
	{
		// This legacy code came from the 1.7.2 release
		return	( ( ( (uint32)(n.z * 511.0f) )  & 0x3ff ) << 22L ) |
				( ( ( (uint32)(n.y * 1023.0f) ) & 0x7ff ) << 11L ) |
				( ( ( (uint32)(-n.x * 1023.0f) ) & 0x7ff ) <<  0L );
	}
	else
	{
		return	( ( ( (uint32)(-n.z * 511.0f) )  & 0x3ff ) << 22L ) |
				( ( ( (uint32)(n.y * 1023.0f) ) & 0x7ff ) << 11L ) |
				( ( ( (uint32)(n.x * 1023.0f) ) & 0x7ff ) <<  0L );
	}
}


// -----------------------------------------------------------------------------
// Section: VisualEnvelope
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
VisualEnvelope::VisualEnvelope()
{
}


/**
 *	Destructor.
 */
VisualEnvelope::~VisualEnvelope()
{
}

bool VisualEnvelope::collectInitialTransforms( Skin& skin )
{
	bool ret = true;

	for( uint32 i = 0; i < skin.numberOfBones(); ++i )
	{
		initialTransforms_.push_back( skin.transform( i, false ) );
	}
	return ret;
}

void VisualEnvelope::normaliseInitialTransforms()
{
	MatrixVector::iterator it = initialTransforms_.begin();
	MatrixVector::iterator end = initialTransforms_.end();

	while (it != end)
	{
		matrix4<float> m = *it++;
		m = normaliseMatrix( m );
	}
}

void VisualEnvelope::initialPoseVertices()
{
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();

	while (it!=end)
	{
		BloatVertex& bloatV = *it++;
		BoneVertex& boneV = boneVertices_[ bloatV.vertexIndex ];
		bloatV.pos = /*initialTransforms_[ boneV.index1 ] */ boneV.position;
	}
}

void VisualEnvelope::relaxedPoseVertices()
{
	MatrixVector invert;

	for (uint i = 0; i < initialTransforms_.size(); i++)
	{
		matrix4<float> m = initialTransforms_[i].inverse();
		invert.push_back( m );
	}

	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();

	while (it!=end)
	{
		BloatVertex& bloatV = *it++;
		BoneVertex& boneV = boneVertices_[ bloatV.vertexIndex ];
		bloatV.pos = invert[ boneV.index1 ] * boneV.position * boneV.weight1 +
					 invert[ boneV.index2 ] * boneV.position * boneV.weight2 +
					 invert[ boneV.index3 ] * boneV.position * boneV.weight3;
	}
}

void VisualEnvelope::createVertexList( VertexVector& vertices )
{
	VertexXYZNUVI v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();

	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();

		if (ExportSettings::instance().useLegacyOrientation())
		{
			// This legacy code came from the 1.7.2 release
			v.pos[0] = -bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = bv.pos.z;
			v.normal[0] = -bv.normal.x;
			v.normal[1] = bv.normal.y;
			v.normal[2] = bv.normal.z;
		}
		else
		{
			v.pos[0] = bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = -bv.pos.z;
			v.normal[0] = bv.normal.x;
			v.normal[1] = bv.normal.y;
			v.normal[2] = -bv.normal.z;
		}

		v.uv[0] = bv.uv.u;
		v.uv[1] = bv.uv.v;
		v.index = (float) (3 * boneVertices_[ bv.vertexIndex ].index1);
		//~ assert( (v.index / 3) < boneNodes_.size() );
		vertices.push_back( v );
	}
}

void VisualEnvelope::createVertexList( UV2VertexVector& vertices )
{
	VertexXYZNUV2I v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();

	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();

		if (ExportSettings::instance().useLegacyOrientation())
		{
			// This legacy code came from the 1.7.2 release
			v.pos[0] = -bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = bv.pos.z;
			v.normal[0] = -bv.normal.x;
			v.normal[1] = bv.normal.y;
			v.normal[2] = bv.normal.z;
		}
		else
		{
			v.pos[0] = bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = -bv.pos.z;
			v.normal[0] = bv.normal.x;
			v.normal[1] = bv.normal.y;
			v.normal[2] = -bv.normal.z;
		}

		v.uv[0] = bv.uv.u;
		v.uv[1] = bv.uv.v;
		
		v.uv2[0] = bv.uv2.u;
		v.uv2[1] = bv.uv2.v;

		v.index = (float) (3 * boneVertices_[ bv.vertexIndex ].index1);
		//~ assert( (v.index / 3) < boneNodes_.size() );
		vertices.push_back( v );
	}
}

template<class T>
void VisualEnvelope::copyVertex( const BloatVertex& inVertex, T& outVertex ) const
{
	if (ExportSettings::instance().useLegacyOrientation())
	{
		// This legacy code came from the 1.7.2 release
		outVertex.pos[0] = -inVertex.pos.x;
		outVertex.pos[1] = inVertex.pos.y;
		outVertex.pos[2] = inVertex.pos.z;
	}
	else
	{
		outVertex.pos[0] = inVertex.pos.x;
		outVertex.pos[1] = inVertex.pos.y;
		outVertex.pos[2] = -inVertex.pos.z;
	}

	outVertex.normal = packNormal( inVertex.normal );
	outVertex.uv[0] = inVertex.uv.u;
	outVertex.uv[1] = inVertex.uv.v;
	outVertex.tangent = packNormal( inVertex.tangent );
	outVertex.binormal = packNormal( inVertex.binormal );
	outVertex.index = (float) (3 * boneVertices_[ inVertex.vertexIndex ].index1);
}


template<class T>
void VisualEnvelope::copyVerts( BVVector& inVerts, std::vector<T>& outVerts )
{
	T v;
	BVVector::iterator it = inVerts.begin();
	BVVector::iterator end = inVerts.end();

	// loop through all the verts, copying the required data.
	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();

		// retreive the vertex data from the bloated format.
		copyVertex<T>( bv, v );

		// add to the vertex list
		outVerts.push_back( v );
	}
}


void VisualEnvelope::createVertexList( TBVertexVector& vertices )
{
	copyVerts( vertices_, vertices );
}

void VisualEnvelope::createVertexList( BlendedVertexVector& vertices )
{
	VertexXYZNUVIIIWW_v2 v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();
	int vNum = 0;
	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();

		if (ExportSettings::instance().useLegacyOrientation())
		{
			// This legacy code came from the 1.7.2 release
			v.pos[0] = -bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = bv.pos.z;
		}
		else
		{
			v.pos[0] = bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = -bv.pos.z;
		}


		v.normal = packNormal( bv.normal );
		v.uv[0] = bv.uv.u;
		v.uv[1] = bv.uv.v;

		normaliseBoneWeights<VertexXYZNUVIIIWW_v2>(bv.vertexIndex, v);

		vertices.push_back( v );
	}
}


void VisualEnvelope::createVertexList( UV2BlendedVertexVector& vertices )
{
	VertexXYZNUV2IIIWW v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();
	int vNum = 0;
	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();

		if (ExportSettings::instance().useLegacyOrientation())
		{
			// This legacy code came from the 1.7.2 release
			v.pos[0] = -bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = bv.pos.z;
		}
		else
		{
			v.pos[0] = bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = -bv.pos.z;
		}


		v.normal = packNormal( bv.normal );
		v.uv[0] = bv.uv.u;
		v.uv[1] = bv.uv.v;

		v.uv2[0] = bv.uv2.u;
		v.uv2[1] = bv.uv2.v;


		normaliseBoneWeights<VertexXYZNUV2IIIWW>(bv.vertexIndex, v);

		vertices.push_back( v );
	}
}

void VisualEnvelope::createVertexList( TBBlendedVertexVector& vertices )
{
	VertexXYZNUVIIIWWTB_v2 v;
	BVVector::iterator it = vertices_.begin();
	BVVector::iterator end = vertices_.end();
	int vNum = 0;
	while (it!=end)
	{
		BloatVertex& bv = *it++;
		bv.pos *= ExportSettings::instance().unitScale();
		if (ExportSettings::instance().useLegacyOrientation())
		{
			// This legacy code came from the 1.7.2 release
			v.pos[0] = -bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = bv.pos.z;
		}
		else
		{
			v.pos[0] = bv.pos.x;
			v.pos[1] = bv.pos.y;
			v.pos[2] = -bv.pos.z;
		}

		v.normal = packNormal( bv.normal );
		v.uv[0] = bv.uv.u;
		v.uv[1] = bv.uv.v;

		normaliseBoneWeights<VertexXYZNUVIIIWWTB_v2>(bv.vertexIndex, v);

		v.tangent = packNormal( bv.tangent );
		v.binormal = packNormal( bv.binormal );
		vertices.push_back( v );
	}
}

void VisualEnvelope::writeMorphTargets( QuickFileWriter& f )
{
	BlendShapes blendShapes;

	if( object.isNull() )
		return;

	int count = 0;
	for (uint c=0; c<blendShapes.count(); c++)
	{
		blendShapes.initialise( c );
		count += blendShapes.countTargets();
	}

	ExportMorphHeader mh;
	mh.nMorphTargets = count;
	mh.version = 0x100;

	f << mh;

	int channel = 0;

	for (uint i = 0; i < blendShapes.count(); i++)
	{
		blendShapes.initialise( i );

		if (!blendShapes.hasBlendShape( object ))
			continue;

		// Only a single base is supported at present, so zero is passed for base
		for (uint j = 0; j < blendShapes.numTargets( 0 ); j++)
		{

			// Only a single base is supported at present, so zero is passed for base
			BlendShapes::Object& blendShape = blendShapes.target( 0, j );

			ExportMorphTarget emt;
			emt.channelIndex = channel++;//mt.channelIndex;
			bw_snprintf( emt.identifier, sizeof(emt.identifier), "%s", blendShape.name.c_str() );
			std::vector< ExportMorphVertex > emvs;

			//~ std::vector< MorphVertex >::iterator it = mt.verts.begin();
			//~ std::vector< MorphVertex >::iterator end = mt.verts.end();

			for( uint32 k = 0; k < blendShape.vertices.size(); ++k )
			{
				// Only a single base is supported at present, so zero is passed for base
				vector3<float> delta = blendShapes.delta( 0, j, k );

				ExportMorphVertex emv;
				if (ExportSettings::instance().useLegacyOrientation())
				{
					// This legacy code came from the 1.7.2 release
					// mmm... no idea what the 100 is for??? (below)
					emv.delta[0] = -delta.x * ExportSettings::instance().unitScale();
					emv.delta[1] = delta.y * ExportSettings::instance().unitScale();
					emv.delta[2] = delta.z * ExportSettings::instance().unitScale();
				}
				else
				{
					// mmm... no idea what the 100 is for??? (below)
					emv.delta[0] = delta.x * ExportSettings::instance().unitScale();
					emv.delta[1] = delta.y * ExportSettings::instance().unitScale();
					emv.delta[2] = -delta.z * ExportSettings::instance().unitScale();
				}

				for( int l = 0; l < (int)vertices_.size(); l++ )
				{
					if( vertices_[l].vertexIndex == k )
					{
						emv.index = l;
						emvs.push_back( emv );
					}
				}
			}

			//~ while (it != end)
			//~ {
				//~ MorphVertex& mv = *it++;
				//~ ExportMorphVertex emv;
				//~ emv.delta[0] = mv.delta.x * 100 * ExportSettings::instance().unitScale();
				//~ emv.delta[1] = mv.delta.y * 100 * ExportSettings::instance().unitScale();
				//~ emv.delta[2] = mv.delta.z * 100 * ExportSettings::instance().unitScale();

				//~ for (int j = 0; j < (int)vertices_.size(); j++)
				//~ {
					//~ if (vertices_[j].vertexIndex == mv.vertexIndex)
					//~ {
						//~ emv.index = j;
						//~ emvs.push_back( emv );
					//~ }
				//~ }
			//~ }
			emt.nVertices = emvs.size();
			f << emt;
			f << emvs;
		}
	}
}

void VisualEnvelope::writeMorphTargetsXml( DataSectionPtr spFile )
{
	BlendShapes blendShapes;

	if( object.isNull() )
		return;

	int count = 0;
	for (uint c=0; c<blendShapes.count(); c++)
	{
		blendShapes.initialise( c );
		count += blendShapes.countTargets();
	}

	ExportMorphHeader mh;
	mh.nMorphTargets = count;
	mh.version = 0x100;

	// Print Morph Header contents here
	DataSectionPtr pMH = spFile->newSection( "MorphHeader" );
	pMH->writeInt( "NumMorphTargets", mh.nMorphTargets );
	pMH->writeInt( "Version", mh.version );

	int channel = 0;

	for (uint i = 0; i < blendShapes.count(); i++)
	{
		blendShapes.initialise( i );

		if (!blendShapes.hasBlendShape( object ))
			continue;

		// Only a single base is supported at present, so zero is passed for base
		for (uint j = 0; j < blendShapes.numTargets( 0 ); j++)
		{
			// Only a single base is supported at present, so zero is passed for base
			BlendShapes::Object& blendShape = blendShapes.target( 0, j );

			ExportMorphTarget emt;
			emt.channelIndex = channel++;
			bw_snprintf( emt.identifier, sizeof(emt.identifier), "%s", blendShape.name.c_str() );
			std::vector< ExportMorphVertex > emvs;

			for( uint32 k = 0; k < blendShape.vertices.size(); ++k )
			{
				// Only a single base is supported at present, so zero is passed for base
				vector3<float> delta = blendShapes.delta( 0, j, k );

				ExportMorphVertex emv;
				if (ExportSettings::instance().useLegacyOrientation())
				{
					// This legacy code came from the 1.7.2 release
					// mmm... no idea what the 100 is for??? (below)
					emv.delta[0] = -delta.x * ExportSettings::instance().unitScale();
					emv.delta[1] = delta.y * ExportSettings::instance().unitScale();
					emv.delta[2] = delta.z * ExportSettings::instance().unitScale();
				}
				else
				{
					// mmm... no idea what the 100 is for??? (below)
					emv.delta[0] = delta.x * ExportSettings::instance().unitScale();
					emv.delta[1] = delta.y * ExportSettings::instance().unitScale();
					emv.delta[2] = -delta.z * ExportSettings::instance().unitScale();
				}

				for( int l = 0; l < (int)vertices_.size(); l++ )
				{
					if( vertices_[l].vertexIndex == k )
					{
						emv.index = l;
						emvs.push_back( emv );
					}
				}
			}

			emt.nVertices = emvs.size();

			DataSectionPtr pExportMorphTarget = spFile->newSection( "ExportMorphTarget" );
			pExportMorphTarget->writeString( "Identifier", emt.identifier );
			pExportMorphTarget->writeInt( "ChannelIndex", emt.channelIndex );
			pExportMorphTarget->writeInt( "NumVertices", emt.nVertices );

			DataSectionPtr pExportMorphVertices = spFile->newSection( "ExportMorphVertices" );
			std::vector< ExportMorphVertex >::iterator emvIt;
			for (emvIt = emvs.begin(); emvIt != emvs.end(); ++emvIt)
			{
				DataSectionPtr pEMV = pExportMorphVertices->newSection( "ExportMorphVertex" );
				pEMV->writeInt( "Index", emvIt->index );
				pEMV->writeVector3( "Delta", Vector3(emvIt->delta[0], emvIt->delta[1], emvIt->delta[2]) );
			}
		}
	}
}

bool VisualEnvelope::init( Skin& skin, Mesh& mesh )
{
	bool ret = false;

	if( skin.count() < 1 || mesh.positions().size() != skin.numberOfVertices( mesh.fullName() ) )
		return ret;

	VisualMesh::init( mesh );

	std::vector<BoneVertex>& vertices = skin.vertices( mesh.fullName() );

	for( std::vector<BoneVertex>::iterator it = vertices.begin(); it != vertices.end(); ++it )
		boneVertices_.push_back( *it );

	collectInitialTransforms( skin );
	bb_.setBounds(	reinterpret_cast<const Vector3&>( vertices_[0].pos ),
					reinterpret_cast<const Vector3&>( vertices_[0].pos ) );
	for (int i = 1; i < (int)vertices_.size(); i++)
	{
		bb_.addBounds( reinterpret_cast<const Vector3&>( vertices_[i].pos ) );
	}

	flipTriangleWindingOrder();
	removeDuplicateVertices();
	sortTriangles();

	BoneSet& bones = skin.boneSet( mesh.fullName() );

	for( uint32 i = 0; i < bones.bones.size(); ++i )
		boneNodes_.push_back( bones.bones[i] );

	if ( ExportSettings::instance().bumpMapped() )
	{
		makeBumped();
	}

	removeDuplicateVertices();
	sortTriangles();

	ret = true;
	return ret;
}

bool VisualEnvelope::save( DataSectionPtr spVisualSection, DataSectionPtr spExistingVisual, const std::string& primitiveFile,
	bool useIdentifier )
{
	DataSectionPtr spFile = BWResource::openSection( primitiveFile );
	if (!spFile)
		spFile = new BinSection( primitiveFile, new BinaryBlock( 0, 0, "BinaryBlock/VisualEnvelope" ) );

	if (spFile)
	{
		PGVector primGroups;
		Moo::IndicesHolder indices;

		if (!createPrimitiveGroups( primGroups, indices ))
			return false;

		if (indices.entrySize() != 2)
		{
			::MessageBox(
				0,
				L"Skinned models cannot have more than 65535 vertices!",
				L"Vertex count error!",
				MB_ICONERROR );
			return false;
		}

		Moo::IndexHeader ih;
		bw_snprintf( ih.indexFormat_, sizeof(ih.indexFormat_), "list" );
		ih.nIndices_ = indices.size();
		ih.nTriangleGroups_ = primGroups.size();

		QuickFileWriter f;
		f << ih;
		f.write( indices.indices(), indices.size() * indices.entrySize() );
		f << primGroups;

		spFile->writeBinary( identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".indices", f.output() );

		f = QuickFileWriter();

		std::vector<Vector2> uv2;
		std::vector<DWORD> colours;
		{
			if (!ExportSettings::instance().bumpMapped())
			{
				BlendedVertexVector vertices;
				createVertexList( vertices );
				Moo::VertexHeader vh;
				vh.nVertices_ = vertices.size();
				bw_snprintf( vh.vertexFormat_, sizeof(vh.vertexFormat_), "xyznuviiiww_v2" );
				f << vh;
				f << vertices;
			}
			else
			{
				TBBlendedVertexVector vertices;
				createVertexList( vertices );
				Moo::VertexHeader vh;
				vh.nVertices_ = vertices.size();
				bw_snprintf( vh.vertexFormat_, sizeof(vh.vertexFormat_), "xyznuviiiwwtb_v2" );
				f << vh;
				f << vertices;
			}
		}

		if( object.isNull() == false )
		{
			writeMorphTargets( f );
		}
		
		std::string vertName;
		if( object.isNull() == false )
			vertName = identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".mvertices";
		else
			vertName = identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".vertices";
		
		spFile->writeBinary( vertName, f.output() );

		// Write extra vertex data streams
		std::vector<std::string> streams;
		if (dualUV_)
		{
			// Retrieve the second UV channel.
			BVVector::iterator it = vertices_.begin();
			BVVector::iterator end = vertices_.end();
			
			while (it!=end)
			{
				BloatVertex& bv = *it++;
				Vector2 uv;
				uv.x = bv.uv2.u;
				uv.y = bv.uv2.v;
				uv2.push_back( uv );
			}
			
			f = QuickFileWriter();
			f << uv2;

			streams.push_back( identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".uv2" );

			spFile->writeBinary( streams.back(), f.output() );
		}

		if (vertexColours_)
		{
			// Retrieve the vertex colour info.
			
			BVVector::iterator it = vertices_.begin();
			BVVector::iterator end = vertices_.end();
			while (it!=end)
			{
				BloatVertex& bv = *it++;
				DWORD colour = D3DCOLOR_COLORVALUE(bv.colour.x,bv.colour.y,bv.colour.z,bv.colour.w);
				colours.push_back( colour );
			}

			f = QuickFileWriter();
			f << colours;

			streams.push_back( identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".colour" );

			spFile->writeBinary( streams.back(), f.output() );
		}

		spFile->save( primitiveFile );

		DataSectionPtr spRenderSet = spVisualSection->newSection( "renderSet" );
		if (spRenderSet)
		{
			spRenderSet->writeBool( "treatAsWorldSpaceObject", true );
			for( int i = 0; i < (int)boneNodes_.size(); i++ )
			{
				DataSectionPtr spNode = spRenderSet->newSection( "node" );
				if (spNode)
				{
					spNode->setString( (boneNodes_[i].substr( boneNodes_[i].find_last_of( "|" ) + 1 ) + "_BlendBone") );
				}
			}

			DataSectionPtr spGeometry = spRenderSet->newSection( "geometry" );
			if (spGeometry)
			{
				spGeometry->writeString( "vertices", vertName );

				spGeometry->writeStrings( "stream", streams );

				spGeometry->writeString( "primitive", identifier_.substr( identifier_.find_last_of( "|" ) + 1 ) + ".indices" );

				MaterialVector::iterator it = materials_.begin();
				MaterialVector::iterator end = materials_.end();

				uint32 primGroup = 0;
				for (uint32 i = 0; i < materials_.size(); i++)
				{
					Material& mat = materials_[i];
					if (mat.inUse) mat.save( spGeometry, spExistingVisual, primGroup++, true, (int)boneNodes_.size(),
						Material::SOFT_SKIN, boneNodes_.size() <= 17  );
				}
			}
		}
	}
	return true;
}


/**
 * This method saves the visual envelope in XML format.  Note that this
 * method is intended for debugging purposes only.
 *
 * @param	xmlFile	The filename of the XML file.
 */
bool VisualEnvelope::savePrimXml( const std::string& xmlFile )
{
	DataSectionPtr spFileRoot = BWResource::openSection( xmlFile );
	if (!spFileRoot)
		spFileRoot = new XMLSection( "VisualEnvelopePrimXml" );

	DataSectionPtr spFile = spFileRoot->newSection( identifier_ );

	PGVector primGroups;
	Moo::IndicesHolder indices;

	if (!createPrimitiveGroups( primGroups, indices ))
		return false;

	if (indices.entrySize() != 2)
	{
		return false;
	}
	
	Moo::IndexHeader ih;
	bw_snprintf( ih.indexFormat_, sizeof(ih.indexFormat_), "list" );
	ih.nIndices_ = indices.size();
	ih.nTriangleGroups_ = primGroups.size();

	// Print Index Header contents here
	DataSectionPtr pIH = spFile->newSection( "IndexHeader" );
	pIH->writeString( "IndexFormat", ih.indexFormat_ );
	pIH->writeInt( "NumIndices", ih.nIndices_ );
	pIH->writeInt( "NumPrimGroups", ih.nTriangleGroups_ );

	// Write out the indices
	DataSectionPtr pIndices = spFile->newSection( "Indices" );
	for (uint32 i = 0; i < indices.size(); ++i)
	{
		DataSectionPtr pIndex = pIndices->newSection( "Index" );
		pIndex->setInt( (uint32)indices[ i ] );
	}

	DataSectionPtr pPrimGroups = spFile->newSection( "PrimGroups" );
	PGVector::iterator pgvIt;
	for (pgvIt = primGroups.begin(); pgvIt != primGroups.end(); ++pgvIt)
	{
		DataSectionPtr pPrimGroup = pPrimGroups->newSection( "PrimGroup" );
		pPrimGroup->writeInt( "NumPrimGroups", pgvIt->nPrimitives_ );
		pPrimGroup->writeInt( "NumVertices", pgvIt->nVertices_ );
		pPrimGroup->writeInt( "StartIndex", pgvIt->startIndex_ );
		pPrimGroup->writeInt( "StartVertex", pgvIt->startVertex_ );
	}

	if (!ExportSettings::instance().bumpMapped())
	{
		BlendedVertexVector vertices;
		createVertexList( vertices );
		Moo::VertexHeader vh;
		vh.nVertices_ = vertices.size();
		bw_snprintf( vh.vertexFormat_, sizeof(vh.vertexFormat_), "xyznuviiiww_v2" );

		// Print Vertex Header contents here
		DataSectionPtr pVH = spFile->newSection( "VertexHeader" );
		pVH->writeString( "VertexFormat", vh.vertexFormat_ );
		pVH->writeInt( "NumVertices", vh.nVertices_ );

		DataSectionPtr pVertices = spFile->newSection( "Vertices" );
		BlendedVertexVector::iterator bvvIt;
		for (bvvIt = vertices.begin(); bvvIt != vertices.end(); ++bvvIt)
		{
			DataSectionPtr pVertex = pVertices->newSection( "Vertex" );
			pVertex->writeVector3( "Pos", Vector3(bvvIt->pos[0], bvvIt->pos[1], bvvIt->pos[2]) );
			pVertex->writeLong( "Normal", bvvIt->normal );
			pVertex->writeVector2( "UV", Vector2(bvvIt->uv[0], bvvIt->uv[1]) );
			pVertex->writeInt( "Index1", bvvIt->index );
			pVertex->writeInt( "Index2", bvvIt->index2 );
			pVertex->writeInt( "Index3", bvvIt->index3 );
			pVertex->writeInt( "Weight", bvvIt->weight );
			pVertex->writeInt( "Weight2", bvvIt->weight2 );
		}
	}
	else
	{
		TBBlendedVertexVector vertices;
		createVertexList( vertices );
		Moo::VertexHeader vh;
		vh.nVertices_ = vertices.size();
		bw_snprintf( vh.vertexFormat_, sizeof(vh.vertexFormat_), "xyznuviiiwwtb_v2" );

		// Print Vertex Header contents here
		DataSectionPtr pVH = spFile->newSection( "VertexHeader" );
		pVH->writeString( "VertexFormat", vh.vertexFormat_ );
		pVH->writeInt( "NumVertices", vh.nVertices_ );

		DataSectionPtr pVertices = spFile->newSection( "Vertices" );
		TBBlendedVertexVector::iterator tbbvvIt;
		for (tbbvvIt = vertices.begin(); tbbvvIt != vertices.end(); ++tbbvvIt)
		{
			DataSectionPtr pVertex = pVertices->newSection( "Vertex" );
			pVertex->writeVector3( "Pos", Vector3(tbbvvIt->pos[0], tbbvvIt->pos[1], tbbvvIt->pos[2]) );
			pVertex->writeLong( "Normal", tbbvvIt->normal );
			pVertex->writeVector2( "UV", Vector2(tbbvvIt->uv[0], tbbvvIt->uv[1]) );
			pVertex->writeInt( "Index1", tbbvvIt->index );
			pVertex->writeInt( "Index2", tbbvvIt->index2 );
			pVertex->writeInt( "Index3", tbbvvIt->index3 );
			pVertex->writeInt( "Weight", tbbvvIt->weight );
			pVertex->writeInt( "Weight2", tbbvvIt->weight2 );
			pVertex->writeInt( "Tangent", tbbvvIt->tangent );
			pVertex->writeInt( "Binormal", tbbvvIt->binormal );
		}
	}

	if( object.isNull() == false )
	{
		writeMorphTargetsXml( spFile );
	}

	spFileRoot->save( xmlFile );

	return true;
}
