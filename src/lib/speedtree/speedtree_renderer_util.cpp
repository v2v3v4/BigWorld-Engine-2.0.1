/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/***
 *

This file contains the implementations of various speedtree renderer utilities.

 *
 ***/

#include "speedtree_config.hpp"

// BW Tech Hearders
#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"

#if SPEEDTREE_SUPPORT // -------------------------------------------------------

#include "resmgr/bwresource.hpp"

// SpeedTree API
#include <SpeedTreeRT.h>
#include <SpeedTreeAllocator.h>

#include "speedtree_renderer_util.hpp"

#ifdef OPT_BUFFERS

namespace speedtree {	

//TODO: ensure all of these are managed pool....

// The common vertex/index buffer storage:
// Fronds/Branches:
CommonTraits::VertexBufferPtr	CommonTraits::s_vertexBuffer_;
STVector< BranchVertex >		CommonTraits::s_vertexList_;

CommonTraits::IndexBufferPtr	CommonTraits::s_indexBuffer_;
STVector< uint32 >				CommonTraits::s_indexList_;

// Leaves:
LeafTraits::VertexBufferPtr		LeafTraits::s_vertexBuffer_;
STVector< LeafVertex >			LeafTraits::s_vertexList_;

LeafTraits::IndexBufferPtr		LeafTraits::s_indexBuffer_;
STVector< uint32 >				LeafTraits::s_indexList_;
}

#endif // OPT_BUFFERS

speedtree::LeafRenderData speedtree::createLeafRenderData(
	CSpeedTreeRT      & speedTree,
	CSpeedWind	      & speedWind,
	const std::string & filename,
	const std::string & datapath,
	int                 windAnglesCount,
	float             & leafRockScalar,
	float             & leafRustleScalar )
{
	BW_GUARD;
	LeafRenderData result =
		getCommonRenderData< LeafTraits >( speedTree, datapath );

	// extract geometry for all lod levels
	static CSpeedTreeRT::SGeometry geometry;
	speedTree.GetGeometry( geometry, SpeedTree_LeafGeometry );

	typedef CSpeedTreeRT::SGeometry::SLeaf SLeaf;
	typedef SLeaf::SCard SCard;
	typedef SLeaf::SMesh SMesh;

#ifdef OPT_BUFFERS
	if (geometry.m_nNumLeafLods > 0)
	{
		const SLeaf & testData = geometry.m_pLeaves[0];
		const SCard & testCard = testData.m_pCards[0];
		if (testCard.m_pTexCoords[0] - testCard.m_pTexCoords[2] == 1.0 &&
			testCard.m_pTexCoords[5] - testCard.m_pTexCoords[3] == 1.0)
		{
			speedtreeError(filename.c_str(), 
				"Tree looks like it's not using composite maps.");
		}
	}

	static const int c_vertPerCard = 4;
	static const int c_indPerCard  = 6;
	int leafVerticesCount = 0;
	for (int lod=0; lod<geometry.m_nNumLeafLods; ++lod)
	{
		// we need to go through all leaves data
		// to find out how many vertices we need
		// to reserve for leaf indices and vertices.
		
		const SLeaf & data = geometry.m_pLeaves[lod];

		int leafCount = data.m_nNumLeaves;
		for (int leaf=0; leaf<leafCount; ++leaf)
		{
			int cardIndex = data.m_pLeafCardIndices[leaf];
			const SMesh * leafMesh = data.m_pCards[cardIndex].m_pMesh;
			if (leafMesh != NULL)
			{
				leafVerticesCount += leafMesh->m_nNumVertices;
			}
			else
			{				
				leafVerticesCount += c_vertPerCard;
			}
		}
	}

	result.verts_ = LeafTraits::s_vertexList_.newSlot( leafVerticesCount );
	for (int lod=0; lod<geometry.m_nNumLeafLods; ++lod)
	{
		typedef LeafTraits::RenderDataType::LodData LodData;
		result.lod_.push_back( LodData() );
		LodData & lodData = result.lod_.back();
		const SLeaf & data = geometry.m_pLeaves[lod];

        if (lod == 0)
        {
            leafRockScalar   = data.m_fLeafRockScalar;
            leafRustleScalar = data.m_fLeafRustleScalar;
        }

		int leafCount = data.m_nNumLeaves;
		if (leafCount > 0)
		{
			lodData.index_ = LeafTraits::s_indexList_.newSlot( );
			// build the actual leaf mesh data
			{
				std::vector<const SMesh *> meshes;
				for (int leaf=0; leaf<leafCount; ++leaf)
				{
					// retrieve leaf card										
					int cardIndex = data.m_pLeafCardIndices[leaf];
					const SCard & leafCard = data.m_pCards[cardIndex];
					const SMesh * leafMesh = data.m_pCards[cardIndex].m_pMesh;

					// (in speedtree, z is up)
					Vector3 cardCenter(
						+data.m_pCenterCoords[leaf*3 + 0],
						+data.m_pCenterCoords[leaf*3 + 1],
						-data.m_pCenterCoords[leaf*3 + 2]);
							
					Vector3 cardNormal = Vector3(
						-data.m_pBinormals[leaf*12 + 0],
						-data.m_pBinormals[leaf*12 + 1],
						+data.m_pBinormals[leaf*12 + 2]);

					Vector3 cardTangent = Vector3(
						-data.m_pTangents[leaf*12 + 0],
						-data.m_pTangents[leaf*12 + 1],
						+data.m_pTangents[leaf*12 + 2]);

					Vector3 cardBinormal = Vector3(
						+data.m_pNormals[leaf*12 + 0],
						+data.m_pNormals[leaf*12 + 1],
						-data.m_pNormals[leaf*12 + 2]);

					Matrix cardTransform;
					cardTransform.row(0, Vector4(cardTangent, 0)  );
					cardTransform.row(1, Vector4(cardBinormal, 0) );
					cardTransform.row(2, Vector4(cardNormal, 0)   );
					cardTransform.row(3, Vector4(cardCenter, 1)   );
					
					// no, use leaf card
					//const int baseIndex = std::distance(vbBegin, vbIt);
					const int baseIndex = LeafTraits::s_vertexList_.size() - result.verts_->start();
					// does this card have a 
					// mesh or a billboarded card?
					if (leafMesh != NULL)
					{
						// yes, add mesh vertices
						for (int v=0; v<leafMesh->m_nNumVertices; ++v)
						{
							LeafVertex vert;

							// position (in speedtree, z is up) 
							Vector3 position(
								+leafMesh->m_pCoords[v*3 + 0],
								+leafMesh->m_pCoords[v*3 + 1],
								-leafMesh->m_pCoords[v*3 + 2]);
							vert.position_ = cardTransform.applyPoint( position );

							// normals							
							Vector3 normal(
								+leafMesh->m_pNormals[v*3 + 0],
								+leafMesh->m_pNormals[v*3 + 1],
								-leafMesh->m_pNormals[v*3 + 2]);
							vert.normal_ = cardTransform.applyVector(normal);
							
							#if SPT_ENABLE_NORMAL_MAPS
								Vector3 binormal(
									+leafMesh->m_pBinormals[v*3 + 0],
									+leafMesh->m_pBinormals[v*3 + 1],
									-leafMesh->m_pBinormals[v*3 + 2]);
								vert.binormal_ = cardTransform.applyVector(binormal);
								
								Vector3 tangent(
									+leafMesh->m_pTangents[v*3 + 0],
									+leafMesh->m_pTangents[v*3 + 1],
									-leafMesh->m_pTangents[v*3 + 2]);
								vert.tangent_ = cardTransform.applyVector(tangent);
							#endif // SPT_ENABLE_NORMAL_MAPS
							
							// wind info
							float fWindMatrixIndex1 = 
								float( int( data.m_pWindMatrixIndices[0][leaf] *
											10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight1 = c_fWindWeightCompressionRange * 
														data.m_pWindWeights[0][leaf];
							float fWindMatrixIndex2 = 
								float( int( data.m_pWindMatrixIndices[1][leaf] *
											10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight2 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[1][leaf];

							vert.windInfo_[0] = fWindMatrixIndex1 + fWindMatrixWeight1;
							vert.windInfo_[1] = fWindMatrixIndex2 + fWindMatrixWeight2;

							// texture coords
							vert.texCoords_[0] = leafMesh->m_pTexCoords[v*2 + 0];
							vert.texCoords_[1] = leafMesh->m_pTexCoords[v*2 + 1];

							// rock and rustle info
							vert.rotInfo_[0] = DEG_TO_RAD(leafCard.m_afAngleOffsets[0]);
							vert.rotInfo_[1] = DEG_TO_RAD(leafCard.m_afAngleOffsets[1]);

							// extra info
							vert.extraInfo_[0] = float(leaf % windAnglesCount);
							vert.extraInfo_[1] = c_vertPerCard;
							vert.extraInfo_[2] = data.m_pDimming[leaf];
							//TODO: pack the vertex data better.

							// geometry info
							vert.geomInfo_[0] = 1.0f;
							vert.geomInfo_[1] = 1.0f;

							// pivot into
							vert.pivotInfo_[0] = 0.0f;
							vert.pivotInfo_[1] = 0.0f;

							LeafTraits::s_vertexList_.push_back( vert );
						}
						lodData.index_->count( lodData.index_->count() + leafMesh->m_nNumIndices );

						// add mesh indices
						for (int v=0; v<leafMesh->m_nNumIndices; ++v)
						{
							LeafTraits::s_indexList_.push_back( 
								baseIndex + leafMesh->m_pIndices[v] /*+
								result.verts_->start()*/ );
						}						
					}
					else
					{
						// no, add card vertices
						for (int v=0; v<c_vertPerCard; ++v)
						{
							LeafVertex vert;

							vert.position_ = cardCenter;
							
							vert.normal_ = Vector3(
									+data.m_pNormals[leaf*12 + v*3 + 0],
									+data.m_pNormals[leaf*12 + v*3 + 1],
									-data.m_pNormals[leaf*12 + v*3 + 2]);
							
							#if SPT_ENABLE_NORMAL_MAPS
								vert.tangent_ = Vector3(
									+data.m_pTangents[leaf*12 + v*3 + 0],
									+data.m_pTangents[leaf*12 + v*3 + 1],
									-data.m_pTangents[leaf*12 + v*3 + 2]);

								vert.binormal_ = Vector3(
									+data.m_pBinormals[leaf*12 + v*3 + 0],
									+data.m_pBinormals[leaf*12 + v*3 + 1],
									-data.m_pBinormals[leaf*12 + v*3 + 2]);
							#endif // SPT_ENABLE_NORMAL_MAPS
			
							// wind info
							//TODO: put this in a function
							float fWindMatrixIndex1 = 
								float( int( data.m_pWindMatrixIndices[0][leaf] *
										10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight1 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[0][leaf];
							float fWindMatrixIndex2 = 
								float( int( data.m_pWindMatrixIndices[1][leaf] *
										10.0f / speedWind.GetNumWindMatrices() ) );

							float fWindMatrixWeight2 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[1][leaf];

							vert.windInfo_[0] = fWindMatrixIndex1 + fWindMatrixWeight1;
							vert.windInfo_[1] = fWindMatrixIndex2 + fWindMatrixWeight2;

							// texture coords
							vert.texCoords_[0] = leafCard.m_pTexCoords[v*2 + 0];
							vert.texCoords_[1] = leafCard.m_pTexCoords[v*2 + 1];

							// rock and rustle info
							vert.rotInfo_[0] = DEG_TO_RAD(leafCard.m_afAngleOffsets[0]);
							vert.rotInfo_[1] = DEG_TO_RAD(leafCard.m_afAngleOffsets[1]);

							// extra info
							vert.extraInfo_[0] = float(leaf % windAnglesCount);
							vert.extraInfo_[1] = float(v);
							vert.extraInfo_[2] = data.m_pDimming[leaf];

							// geometry info
							vert.geomInfo_[0] = leafCard.m_fWidth;
							vert.geomInfo_[1] = leafCard.m_fHeight;

							// pivot into
							vert.pivotInfo_[0] = leafCard.m_afPivotPoint[0] - 0.5f;
							vert.pivotInfo_[1] = -(leafCard.m_afPivotPoint[1] - 0.5f);

							//++vbIt;
							LeafTraits::s_vertexList_.push_back( vert );
						}
						lodData.index_->count( lodData.index_->count() 
												+ c_indPerCard );
						
						// add card indices
						static const int indexOffset[] = {0, 1, 2, 0, 2, 3};
						for (int i=0; i<c_indPerCard; ++i)
						{
							LeafTraits::s_indexList_.push_back( baseIndex + indexOffset[i] );
						}
					}
				}
			}
		}
	}

#else

	if ( geometry.m_nNumLeafLods > 0 )
	{
		const SLeaf & testData = geometry.m_pLeaves[0];
		const SCard & testCard = testData.m_pCards[0];
		if ( testCard.m_pTexCoords[0] - testCard.m_pTexCoords[2] == 1.0 &&
			 testCard.m_pTexCoords[5] - testCard.m_pTexCoords[3] == 1.0 )
		{
			speedtreeError(filename.c_str(), 
				"Tree looks like it's not using composite maps.");
		}
	}

	for ( int lod=0; lod<geometry.m_nNumLeafLods; ++lod )
	{
		typedef LeafTraits::RenderDataType::LodData LodData;
		result.lod_.push_back( LodData() );
		LodData & lodData = result.lod_.back();

		typedef VertexBufferWrapper< LeafVertex > VBufferType;
		lodData.vertices_ = new VBufferType;
		lodData.strips_.push_back( new IndexBufferWrapper );

		const SLeaf & data = geometry.m_pLeaves[lod];

        if ( lod == 0 )
        {
            leafRockScalar   = data.m_fLeafRockScalar;
            leafRustleScalar = data.m_fLeafRustleScalar;
        }

		int leafCount = data.m_nNumLeaves;
		if ( leafCount > 0 )
		{
			static const int c_vertPerCard = 4;
			static const int c_indPerCard  = 6;
		
			// we need to go through all leaves data
			// to find out how many vertices we need
			// to reserve for leaf indices and vertices.
			int leafVerticesCount = 0;
			int leafIndicesCount  = 0;
			for ( int leaf=0; leaf<leafCount; ++leaf )
			{
				int cardIndex = data.m_pLeafCardIndices[leaf];
				const SMesh * leafMesh = data.m_pCards[cardIndex].m_pMesh;
				if ( leafMesh != NULL )
				{
					leafVerticesCount += leafMesh->m_nNumVertices;
					leafIndicesCount  += leafMesh->m_nNumIndices;
				}
				else
				{				
					leafVerticesCount += c_vertPerCard;
					leafIndicesCount  += c_indPerCard;
				}
			}

			
				
			// build the actual leaf mesh data
			if ( lodData.vertices_->reset( leafVerticesCount )	&& 
				 lodData.vertices_->lock()						&&
				 lodData.strips_[0]->reset( leafIndicesCount )	&&
				 lodData.strips_[0]->lock() )
			{
				std::vector<const SMesh *> meshes;
			
				IndexBufferWrapper::iterator ibIt = lodData.strips_[0]->begin();
				VBufferType::iterator vbIt        = lodData.vertices_->begin();
				VBufferType::iterator vbBegin     = vbIt;
				
				for ( int leaf=0; leaf<leafCount; ++leaf )
				{
					// retrieve leaf card										
					int cardIndex = data.m_pLeafCardIndices[leaf];
					const SCard & leafCard = data.m_pCards[cardIndex];
					const SMesh * leafMesh = data.m_pCards[cardIndex].m_pMesh;

					// (in speedtree, z is up)
					Vector3 cardCenter(
						+data.m_pCenterCoords[leaf*3 + 0],
						+data.m_pCenterCoords[leaf*3 + 1],
						-data.m_pCenterCoords[leaf*3 + 2]);
							
					Vector3 cardNormal = Vector3(
						-data.m_pBinormals[leaf*12 + 0],
						-data.m_pBinormals[leaf*12 + 1],
						+data.m_pBinormals[leaf*12 + 2]);

					Vector3 cardTangent = Vector3(
						-data.m_pTangents[leaf*12 + 0],
						-data.m_pTangents[leaf*12 + 1],
						+data.m_pTangents[leaf*12 + 2]);

					Vector3 cardBinormal = Vector3(
						+data.m_pNormals[leaf*12 + 0],
						+data.m_pNormals[leaf*12 + 1],
						-data.m_pNormals[leaf*12 + 2]);

					Matrix cardTransform;
					cardTransform.row( 0, Vector4(cardTangent,	0)	);
					cardTransform.row( 1, Vector4(cardBinormal, 0)	);
					cardTransform.row( 2, Vector4(cardNormal,	0)	);
					cardTransform.row( 3, Vector4(cardCenter,	1)	);
					
					// no, use leaf card
					const int baseIndex = std::distance( vbBegin, vbIt );
					
					// does this card have a 
					// mesh or a billboarded card?
					if ( leafMesh != NULL )
					{
						// yes, add mesh vertices
						for ( int v=0; v<leafMesh->m_nNumVertices; ++v )
						{
							// position (in speedtree, z is up) 
							Vector3 position(
								+leafMesh->m_pCoords[v*3 + 0],
								+leafMesh->m_pCoords[v*3 + 1],
								-leafMesh->m_pCoords[v*3 + 2]);
							vbIt->position_ = cardTransform.applyPoint( position );

							// normals							
							Vector3 normal(
								+leafMesh->m_pNormals[v*3 + 0],
								+leafMesh->m_pNormals[v*3 + 1],
								-leafMesh->m_pNormals[v*3 + 2]);
							vbIt->normal_ = cardTransform.applyVector( normal );
							
							#if SPT_ENABLE_NORMAL_MAPS
								Vector3 binormal(
									+leafMesh->m_pBinormals[v*3 + 0],
									+leafMesh->m_pBinormals[v*3 + 1],
									-leafMesh->m_pBinormals[v*3 + 2]);
								vbIt->binormal_ = cardTransform.applyVector( binormal );
								
								Vector3 tangent(
									+leafMesh->m_pTangents[v*3 + 0],
									+leafMesh->m_pTangents[v*3 + 1],
									-leafMesh->m_pTangents[v*3 + 2]);
								vbIt->tangent_ = cardTransform.applyVector( tangent );
							#endif // SPT_ENABLE_NORMAL_MAPS
							
							// wind info
							float fWindMatrixIndex1 = 
								float( int( data.m_pWindMatrixIndices[0][leaf] *
											10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight1 = c_fWindWeightCompressionRange * 
														data.m_pWindWeights[0][leaf];
							float fWindMatrixIndex2 = 
								float( int( data.m_pWindMatrixIndices[1][leaf] *
											10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight2 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[1][leaf];

							vbIt->windInfo_[0] = fWindMatrixIndex1 + fWindMatrixWeight1;
							vbIt->windInfo_[1] = fWindMatrixIndex2 + fWindMatrixWeight2;

							// texture coords
							vbIt->texCoords_[0] = leafMesh->m_pTexCoords[v*2 + 0];
							vbIt->texCoords_[1] = leafMesh->m_pTexCoords[v*2 + 1];

							// rock and rustle info
							vbIt->rotInfo_[0] = DEG_TO_RAD(leafCard.m_afAngleOffsets[0]);
							vbIt->rotInfo_[1] = DEG_TO_RAD(leafCard.m_afAngleOffsets[1]);

							// extra info
							vbIt->extraInfo_[0] = float(leaf % windAnglesCount);
							vbIt->extraInfo_[1] = c_vertPerCard;
							vbIt->extraInfo_[2] = data.m_pDimming[leaf];
							//TODO: pack the vertex data better.

							// geometry info
							vbIt->geomInfo_[0] = 1.0f;
							vbIt->geomInfo_[1] = 1.0f;

							// pivot into
							vbIt->pivotInfo_[0] = 0.0f;
							vbIt->pivotInfo_[1] = 0.0f;

							++vbIt;							
						}
						
						// add mesh indices
						for ( int v=0; v<leafMesh->m_nNumIndices; ++v )
						{
							*ibIt = baseIndex + leafMesh->m_pIndices[v];
							++ibIt;
						}						
					}
					else
					{
						// no, add card vertices
						for ( int v=0; v<c_vertPerCard; ++v )
						{
							vbIt->position_ = cardCenter;
							
							vbIt->normal_ = Vector3(
									+data.m_pNormals[leaf*12 + v*3 + 0],
									+data.m_pNormals[leaf*12 + v*3 + 1],
									-data.m_pNormals[leaf*12 + v*3 + 2]);
							
							#if SPT_ENABLE_NORMAL_MAPS
								vbIt->tangent_ = Vector3(
									+data.m_pTangents[leaf*12 + v*3 + 0],
									+data.m_pTangents[leaf*12 + v*3 + 1],
									-data.m_pTangents[leaf*12 + v*3 + 2]);

								vbIt->binormal_ = Vector3(
									+data.m_pBinormals[leaf*12 + v*3 + 0],
									+data.m_pBinormals[leaf*12 + v*3 + 1],
									-data.m_pBinormals[leaf*12 + v*3 + 2]);
							#endif // SPT_ENABLE_NORMAL_MAPS


					//vbIt->normal_ = Vector3(
					//	-data.m_pBinormals[leaf*12 + v*3 + 0],
					//	-data.m_pBinormals[leaf*12 + v*3 + 1],
					//	+data.m_pBinormals[leaf*12 + v*3 + 2]);

					//vbIt->tangent_ = Vector3(
					//	-data.m_pTangents[leaf*12 + v*3 + 0],
					//	-data.m_pTangents[leaf*12 + v*3 + 1],
					//	+data.m_pTangents[leaf*12 + v*3 + 2]);

					//vbIt->binormal_ = Vector3(
					//	+data.m_pNormals[leaf*12 + v*3 + 0],
					//	+data.m_pNormals[leaf*12 + v*3 + 1],
					//	-data.m_pNormals[leaf*12 + v*3 + 2]);

							// wind info
							//TODO: put this in a function
							float fWindMatrixIndex1 = 
								float( int( data.m_pWindMatrixIndices[0][leaf] *
										10.0f / speedWind.GetNumWindMatrices() ) );
							
							float fWindMatrixWeight1 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[0][leaf];
							float fWindMatrixIndex2 = 
								float( int( data.m_pWindMatrixIndices[1][leaf] *
										10.0f / speedWind.GetNumWindMatrices() ) );

							float fWindMatrixWeight2 = c_fWindWeightCompressionRange *
														data.m_pWindWeights[1][leaf];

							vbIt->windInfo_[0] = fWindMatrixIndex1 + fWindMatrixWeight1;
							vbIt->windInfo_[1] = fWindMatrixIndex2 + fWindMatrixWeight2;

							// texture coords
							vbIt->texCoords_[0] = leafCard.m_pTexCoords[v*2 + 0];
							vbIt->texCoords_[1] = leafCard.m_pTexCoords[v*2 + 1];

							// rock and rustle info
							vbIt->rotInfo_[0] = DEG_TO_RAD(leafCard.m_afAngleOffsets[0]);
							vbIt->rotInfo_[1] = DEG_TO_RAD(leafCard.m_afAngleOffsets[1]);

							// extra info
							vbIt->extraInfo_[0] = float(leaf % windAnglesCount);
							vbIt->extraInfo_[1] = float(v);
							vbIt->extraInfo_[2] = data.m_pDimming[leaf];

							// geometry info
							vbIt->geomInfo_[0] = leafCard.m_fWidth;
							vbIt->geomInfo_[1] = leafCard.m_fHeight;

							// pivot into
							vbIt->pivotInfo_[0] = leafCard.m_afPivotPoint[0] - 0.5f;
							vbIt->pivotInfo_[1] = -(leafCard.m_afPivotPoint[1] - 0.5f);

													
							++vbIt;							
						}
						
						// add card indices
						static const int indexOffset[] = {0, 1, 2, 0, 2, 3};
						for (int i=0; i<c_indPerCard; ++i)
						{
							*ibIt = baseIndex + indexOffset[i];
							++ibIt;
						}
					}
				}
				MF_ASSERT(vbIt == lodData.vertices_->end())
				lodData.vertices_->unlock();
				lodData.strips_[0]->unlock();
			}
			else
			{
				ERROR_MSG(
					"createLeafRenderData: "
					"Could not create/lock vertex buffer\n");
			}
		}
	}

#endif //OPT_BUFFERS

	return result;
}

speedtree::TreePartRenderData< speedtree::BillboardVertex > speedtree::createBillboardRenderData(
	CSpeedTreeRT      & speedTree,
	const std::string & datapath )
{
	BW_GUARD;
	TreePartRenderData< BillboardVertex > result =
		getCommonRenderData< BillboardTraits >( speedTree, datapath );

	// generate billboard geometry
	speedTree.SetLodLevel(0);
	static CSpeedTreeRT::SGeometry geometry;
	speedTree.GetGeometry( geometry, SpeedTree_BillboardGeometry );

	typedef CSpeedTreeRT::SGeometry::S360Billboard S360Billboard;
	S360Billboard & data360 = geometry.m_s360Billboard;
	if (data360.m_nNumImages < 0)
	{
		// If m_s360Billboard.m_nNumImages is < 0,
		// UpdateBillboardGeometry crashes, so return, no billboards here.
		ERROR_MSG( "createBillboardRenderData: tree '%s' does not have billboards. "
			"Please ensure it is exported for realtime.\n", datapath.c_str() );
		return result;
	}

	speedTree.UpdateBillboardGeometry( geometry );

	// horizontal billboard
	typedef CSpeedTreeRT::SGeometry::SHorzBillboard SHorzBillboard;
	SHorzBillboard & dataHor = geometry.m_sHorzBillboard;
	bool drawHorizontalBB = 
		dataHor.m_pCoords    != NULL;

	// 360 degrees billboards
	const int bbCount = data360.m_nNumImages + (drawHorizontalBB ? 1 : 0);

	if (data360.m_pCoords == NULL || data360.m_nNumImages <= 0)
	{
		INFO_MSG( "createBillboardRenderData: tree '%s' does not have billboards, "
			"it will not be rendered if far away.\n", datapath.c_str() );
		return result;
	}

	typedef BillboardTraits::RenderDataType::LodData LodData;
	result.lod_.push_back( LodData() );
	LodData & lodData = result.lod_.back();

	// create vertex buffer
	typedef VertexBufferWrapper< BillboardVertex > VBufferType;
	lodData.vertices_ = new VBufferType;

	static const int vertexCount = 6;

	if ( lodData.vertices_->reset( bbCount*vertexCount ) &&
		 lodData.vertices_->lock() )
	{
		// fill vertex buffer	
		VBufferType::iterator vbIt = lodData.vertices_->begin();
		Vector3 center(0, 0.5f * data360.m_fHeight, 0);
		for ( int bb=0; bb<bbCount; ++bb )
		{
			// matching the billboard rotation with 
			// the tree's took a bit of experimentation. 
			// If, in future versions of speedtree, it 
			// stops matching, this is the place to tweak.
			Matrix bbSpace(Matrix::identity);
			bbSpace.setRotateY( -bb*(2*MATH_PI/data360.m_nNumImages) - (MATH_PI/2) );

			static const short vIdx[vertexCount] = { 0, 1, 2, 0, 2, 3 };

			if ( bb<(bbCount - (drawHorizontalBB ? 1 : 0)) )
			{
				static const float xSignal[4] = { +1.0f, -1.0f, -1.0f, +1.0f };
				static const float ySignal[4] = { +1.0f, +1.0f, -0.0f, -0.0f };

				for ( int v=0; v<vertexCount; ++v )
				{
					Vector3 tangVector = bbSpace.applyToUnitAxisVector(0);
					vbIt->position_.x   = 0.5f * tangVector.x *
										xSignal[vIdx[v]] * data360.m_fWidth;
					vbIt->position_.y   = ySignal[vIdx[v]] * data360.m_fHeight;
					vbIt->position_.z   = 0.5f * tangVector.z *
										xSignal[vIdx[v]] * data360.m_fWidth;

					vbIt->lightNormal_  = vbIt->position_ - center;
					vbIt->lightNormal_.normalise();

					vbIt->alphaNormal_.x = -tangVector.z;
					vbIt->alphaNormal_.y = +tangVector.y;
					vbIt->alphaNormal_.z = +tangVector.x;

					#if SPT_ENABLE_NORMAL_MAPS
						vbIt->tangent_  = Vector3(0.0f, 1.0f, 0.0f);
						vbIt->binormal_ = tangVector;
					#endif // SPT_ENABLE_NORMAL_MAPS

					vbIt->texCoords_[0] = 
						data360.m_pTexCoordTable[(bb)*8 + vIdx[v]*2 + 0];
					vbIt->texCoords_[1] = 
						data360.m_pTexCoordTable[(bb)*8 + vIdx[v]*2 + 1];
					++vbIt;
				}
			}
			else
			{
				for ( int v=0; v<vertexCount; ++v )
				{
					vbIt->position_.x   = +dataHor.m_pCoords[vIdx[v]*3 + 0];
					vbIt->position_.y   = +dataHor.m_pCoords[vIdx[v]*3 + 1];
					vbIt->position_.z   = +dataHor.m_pCoords[vIdx[v]*3 + 2];

					vbIt->lightNormal_  = Vector3(0, 1, 0);
					vbIt->alphaNormal_  = Vector3(0, 1, 0);

					#if SPT_ENABLE_NORMAL_MAPS
						vbIt->tangent_  = Vector3(0.0f, 0.0f, 1.0f);
						vbIt->binormal_ = Vector3(1.0f, 0.0f, 0.0f);
					#endif // SPT_ENABLE_NORMAL_MAPS

					vbIt->texCoords_[0] = dataHor.m_afTexCoords[vIdx[v]*2 + 0];
					vbIt->texCoords_[1] = dataHor.m_afTexCoords[vIdx[v]*2 + 1];
					++vbIt;
				}
			}	
		}
		MF_ASSERT( vbIt == lodData.vertices_->end() );
		lodData.vertices_->unlock();
	}
	else
	{
		ERROR_MSG(
			"createBillboardRenderData: "
			"Could not create/lock vertex buffer\n");
	}

	return result;
}
#endif // SPEEDTREE_SUPPORT
