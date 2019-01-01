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
#include "visual_splitter.hpp"

#include "morph_vertices.hpp"
#include "node.hpp"
#include "node_catalogue.hpp"
#include "primitive_file_structs.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/dataresource.hpp"
#include "resmgr/xml_section.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

namespace Moo
{

// -----------------------------------------------------------------------------
// Section: VisualSplitter
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
VisualSplitter::VisualSplitter(uint32 nodeLimit/* = 17*/)
: nodeLimit_( nodeLimit )
{
}

/**
 *	Destructor.
 */
VisualSplitter::~VisualSplitter()
{
}

/**
 *	This method opens the visual for processing.
 *	@param resourceName the resource name and path of the visual to process
 */
void VisualSplitter::open( const std::string& resourceName )
{
	BW_GUARD;
	resourceName_ = resourceName;
	DataSectionPtr pSection = BWResource::openSection( resourceName );
	if (pSection)
	{
		pSection_ = new XMLSection("visual");
		for (int32 i = 0; i < pSection->countChildren(); i++)
		{
			copySections( pSection_, pSection->openChild( i ) );
		}

		std::string primitivesResource = BWResource::removeExtension(resourceName) + ".primitives";
		DataSectionPtr pPrimitives = BWResource::openSection( primitivesResource );
		if (pPrimitives)
		{
			DataSectionPtr pNodes = pSection_->openSection( "node" );
			pNode_ = new Node();
			if (pNodes)
			{
				pNode_->loadRecursive( pNodes );
			}

			// Check if we have any nodes, no point in splitting static objects.
			std::vector<DataSectionPtr> renderSets;
			pSection_->openSections( "renderSet", renderSets );

			for (uint32 i = 0; i < renderSets.size(); i++)
			{
				readRenderSet( renderSets[i], pPrimitives );
			}
		}
	}
}

/**
 * This method splits the rendersets in the visual.
 */
bool VisualSplitter::split()
{
	BW_GUARD;
	for (uint i = 0; i < pRenderSets_.size(); i++)
	{
		if( !pRenderSets_[i]->compute() )
			return false;
	}
    return true;
}

/**
 *	This method saves the split visual
 *	@param resourceName the name of the visual file to save.
 */
void VisualSplitter::save( const std::string& resourceName )
{
	BW_GUARD;
	// Open the out visual file and remove any sections already in this file.
	DataSectionPtr pVisual = BWResource::openSection( resourceName, true );
	while (pVisual->countChildren())
	{
		pVisual->deleteSection(pVisual->openChild(0)->sectionName());
	}

	// Create the primitive file for this visual
	std::string primitivesResource = BWResource::removeExtension(resourceName) + ".primitives";
	DataSectionPtr pPrimitives = new BinSection( primitivesResource, new BinaryBlock( 0, 0, "BinaryBlock/VisualSplitter" ) );

	// We iterate over all the sections in the old visual file and copy any data that is not
	// a renderset over. When we encounter the rendersets we insert our newly created rendersets.
	bool renderSetsWritten = false;
	for (int i = 0; i < pSection_->countChildren(); i++)
	{
		DataSectionPtr pSect = pSection_->openChild( i );
		if (pSect->sectionName() == "renderSet")
		{
			if (!renderSetsWritten)
			{
				renderSetsWritten = true;
				for (uint j = 0; j < pRenderSets_.size(); j++)
				{
					pRenderSets_[j]->save(pVisual, pPrimitives);
				}
			}
		}
		else
		{
			copySections( pVisual, pSect );
		}
	}

	// Save the primitives and visual files.
	pPrimitives->save( primitivesResource );
	pVisual->save();
}

/*
 *	Helper function that makes a copy of a visual section and all its sub sections.
 */
void VisualSplitter::copySections( DataSectionPtr pDest, DataSectionPtr pSrc )
{
	BW_GUARD;
	DataSectionPtr pSect = pDest->newSection( pSrc->sectionName() );
	pSect->setString( pSrc->asString() );
	for (int i = 0; i < pSrc->countChildren(); i++)
	{
		copySections( pSect, pSrc->openChild( i ) );
	}
}

namespace
{

/*
 *	This helper function creates a new vertex section and 
 *	converts the vertex format from VertexType to OutType
 */
template <typename OutType, typename VertexType>
BinaryPtr convertVerts( const VertexHeader* pVH, uint32 inDataSize, 
							const std::string& formatName )
{
	// Allocate memory for the vertex section
	uint32 newLen = inDataSize + sizeof(OutType) * pVH->nVertices_ - sizeof(VertexType) * pVH->nVertices_;
	char* pData = new char[newLen];

	// Create the new vertex header
	VertexHeader* pVHo = reinterpret_cast<VertexHeader*>(pData);
	bw_snprintf( pVHo->vertexFormat_, sizeof(pVHo->vertexFormat_), "%s", formatName.c_str() );
	pVHo->nVertices_ = pVH->nVertices_;

	// Copy the vertex data
	OutType* pOutVertices = reinterpret_cast<OutType*>(pVHo + 1);
	const VertexType* pVertices = reinterpret_cast<const VertexType*>(pVH + 1);

	for (int i = 0; i < pVH->nVertices_; i++)
	{
		*(pOutVertices++) = *(pVertices++);
	}

	// Copy any extra data (i.e. if we have morph targets stored after vertex data)
	uint32 remainder = inDataSize - sizeof(VertexHeader) - sizeof(VertexType) * pVH->nVertices_;

	if (remainder)
	{
		memcpy( pOutVertices, pVertices, remainder );
	}

	// Create a new binary block and delete our temporary memory
	BinaryPtr pBlock = new BinaryBlock( pData, newLen, "BinaryBlock/VisualSplitter" );
	delete [] pData;

	return pBlock;
}

/*
 *	Create a new vertex section from an old one and convert the vertex
 *	data if needed
 */
BinaryPtr vertsBinSection( BinaryPtr pOldVerts )
{
	const VertexHeader* pVH = reinterpret_cast<const VertexHeader*>(pOldVerts->data());
	std::string vFormat( pVH->vertexFormat_ );

	BinaryPtr pVerts;
	if (vFormat == "xyznuviiiww_v2")
	{
		pVerts = convertVerts<VertexXYZNUVIIIWW, VertexXYZNUVIIIWW_v2>( pVH, pOldVerts->len(), "xyznuviiiww" );
	}
	else if (vFormat == "xyznuviiiwwtb_v2")
	{
		pVerts = convertVerts<VertexXYZNUVIIIWWTB, VertexXYZNUVIIIWWTB_v2>( pVH, pOldVerts->len(), "xyznuviiiwwtb" );
	}
	else
	{
		pVerts = new BinaryBlock( pOldVerts->data(), pOldVerts->len(), "BinaryBlock/VisualSplitter" );
	}
	return pVerts;
}

}

/**
 *	This class implements the RenderSet interface, this implementation
 *	makes a straight copy of the renderset.
 */
class RenderSetCopy : public VisualSplitter::RenderSet
{
public:
	RenderSetCopy( DataSectionPtr pRenderSet, DataSectionPtr pPrimitives ) :
	  pRenderSet_( pRenderSet ),
	  pPrimitives_( pPrimitives )
	{
	}

	/*
	 * This method would normally do the calculations, since we are only copying
	 * we don't need to do anything.
	 */
	virtual bool compute()
	{
		return true;
	}

	/**
	 *	This method saves the renderset copy.
	 */
	virtual void save( DataSectionPtr pVisualSection, DataSectionPtr pPrimitives )
	{
		BW_GUARD;
		if (pRenderSet_ && pPrimitives_)
		{
			// copy the renderset sections in the visual file.
			VisualSplitter::copySections( pVisualSection, pRenderSet_ );
			// Read and create a copy of the vertex and index resources used by the
			// visual file.
			std::vector<DataSectionPtr> geometrySections;
			pRenderSet_->openSections( "geometry", geometrySections );
			for (uint i = 0; i  < geometrySections.size(); i++)
			{
				std::string verts = geometrySections[i]->readString( "vertices" );
				std::string prim = geometrySections[i]->readString( "primitive" );
				BinaryPtr pVerts = pPrimitives_->readBinary( verts );
				BinaryPtr pPrims = pPrimitives_->readBinary( prim );

                pPrimitives->writeBinary( verts, vertsBinSection( pVerts ) );

				std::vector<std::string> streams;
				geometrySections[i]->readStrings( "stream", streams );

				for (uint i=0; i<streams.size(); i++)
				{
					BinaryPtr pStream = pPrimitives_->readBinary( streams[i] );
					if (pStream)
					{
						pPrimitives->writeBinary( streams[i], new BinaryBlock( pStream->data(), pStream->len(), "BinaryBlock/VisualSplitter" ) );
					}
				}

                pPrimitives->writeBinary( prim, new BinaryBlock( pPrims->data(), pPrims->len(), "BinaryBlock/VisualSplitter" ) );
			}
		}
	}
private:
	DataSectionPtr pRenderSet_;
	DataSectionPtr pPrimitives_;
};


/**
 *	This class implements the RenderSet interface, this implementation splits the renderset
 *	into multiple rendersets so that bone limits are preserved.
 */
class RenderSetSplit : public VisualSplitter::RenderSet
{
public:
	RenderSetSplit( DataSectionPtr pRenderSet, DataSectionPtr pPrimitives, uint32 nodeLimit ) :
	  pRenderSet_( pRenderSet ),
	  pPrimitives_( pPrimitives ),
	  nodeLimit_( nodeLimit )
	{
		BW_GUARD;
		pRenderSet->readStrings( "node", nodes_ );
	}

	/**
	 *	This interface is the base class for the vertex interface.
	 */
	class VertexHolder : public ReferenceCount
	{
	public:
		virtual ~VertexHolder() {}
		// This method returns the number of bone indices used by a vertex.
		virtual uint nIndices( uint32 vertex ) const = 0;
		// This method returns the bone index referenced by a vertex;
		virtual uint index( uint32 vertex, uint32 index ) const = 0;
		// This method creates a copy of the vertices as defined by the param mappings
		// The mappings vector contains the old index of the vertices in sequence.
		virtual VertexHolder* copyRemapped( const std::vector<uint16>& mappings ) const = 0;
		// This method remaps the bone indices referenced by the vertices.
		// The boneList vector contains the new bone index for each bone.
		virtual void remapBoneIndices( const std::vector<uint32>& boneList ) = 0;
		// This method creates the binary section for the vertices.
		virtual BinaryPtr createVertexSection() const = 0;
		virtual uint32 nVertices() const = 0;
		// Helper method for remapping vertices (or anything for that matter) according to the 
		// mappings.
		template<typename Type> static void remap( std::vector<Type>& out, const std::vector<Type>& in,
			const std::vector<uint16>& mappings )
		{
			for (uint32 i = 0; i < mappings.size(); i++)
				out.push_back( in[mappings[i]] );
		}
		// Helper method to create the binary data for the vertices.
		template<typename Type, typename OutType> static BinaryPtr createVertexSection( 
			const std::vector<Type>& in, const std::string& formatName )
		{
			uint32 len = sizeof(VertexHeader) + sizeof(OutType) * in.size();
			char* data = new char[len];
            VertexHeader* pVH = reinterpret_cast<VertexHeader*>(data);
			OutType* pVertices = reinterpret_cast<OutType*>(pVH+1);
			bw_snprintf( pVH->vertexFormat_, sizeof(pVH->vertexFormat_), "%s", formatName.c_str() );
			pVH->nVertices_ = in.size();
			for (uint32 i = 0; i < in.size(); i++)
			{
				pVertices[i] = in[i];
			}

			BinaryPtr pData = new BinaryBlock( data, len, "BinaryBlock/VisualSplitter" );
			delete [] data;
			return pData;
		}
		uint streamCount( ) const { return extraStreams_.size(); }
		const VertexHolder* stream( uint i ) const { return extraStreams_[i].get(); }

		void addStream( VertexHolder* newStream )
		{
			extraStreams_.push_back( newStream );
		}
	private:
		std::vector<SmartPointer<VertexHolder>> extraStreams_;
	};

	template< typename VertexType, typename IndexType = uint8 >
	class StreamHolder : public VertexHolder
	{
	public:
		StreamHolder( const VertexType* pVerts, uint32 nVertices, 
			const std::string& formatName ) :
			formatName_( formatName )
		{
			verts_.assign( pVerts, pVerts + nVertices );
		}

		// This method returns the number of bone indices for the vertex 
		//	specified by vertex.
		virtual uint nIndices( uint32 vertex ) const
		{
			return 0;
		}

		// This method returns the bone index specified by index of the 
		// vertex specified by vertex
		virtual uint index( uint32 vertex, uint32 index ) const
		{
			return 0;
		}

		// This method remaps all the bone indices to the ones specified in boneList
		virtual void remapBoneIndices( const std::vector<uint32>& boneList )
		{
		}

		virtual VertexHolder* copyRemapped( const std::vector<uint16>& mappings ) const
		{
			BW_GUARD;
			std::vector< VertexType > out;
			remap( out, verts_, mappings );

			VertexHolder* pOut = new StreamHolder<VertexType>( &out.front(), out.size(),
				formatName_ );

			return pOut;
		}

		// This method creates the binary data for the vertices
		virtual BinaryPtr createVertexSection() const
		{
			BW_GUARD;
			uint32 len = sizeof(VertexType) * verts_.size();
			char* data = new char[len];
			VertexType* pVertices = reinterpret_cast<VertexType*>(data);

			memcpy( pVertices, &verts_.front(), sizeof(VertexType) * verts_.size() );

			BinaryPtr pData = new BinaryBlock( data, len, "BinaryBlock/VisualSplitter" );
			
			delete [] data;
			return pData;
		}

		// this method returns the number of vertices in this vertexholder
		virtual uint32 nVertices() const
		{
			return verts_.size();
		}
	private:
		std::vector< VertexType > verts_;
		std::string formatName_;
	};

	/**
	 *	This class implements the VertexHolder interface for blended vertices.
	 */
	template< typename VertexType, typename OutVertexType, uint8 indexSize, typename IndexType = uint8 >
	class MultiBlendHolder : public VertexHolder
	{
	public:
		MultiBlendHolder( const VertexType* pVerts, uint32 nVertices, 
			const std::string& formatName ) :
			formatName_( formatName )
		{
			verts_.assign( pVerts, pVerts + nVertices );
		}

		// This method returns the number of bone indices for the vertex 
		//	specified by vertex.
		virtual uint nIndices( uint32 vertex ) const
		{
			BW_GUARD;
			// The number of indices that are used is specified by the weights,
			// the last weight is specified by 255 - weight_ - weight1_
			const VertexType& vert = verts_[vertex];
			uint nWeights = 1;
			if (vert.weight2_ > 0)
			{
				nWeights++;
				if ((vert.weight_ + vert.weight2_) < 253)
					nWeights++;
			}
			return nWeights;
		}

		// This method returns the bone index specified by index of the 
		// vertex specified by vertex
		virtual uint index( uint32 vertex, uint32 index ) const
		{
			BW_GUARD;
			const VertexType& vert = verts_[vertex];
			if (index == 0)
				return vert.index_ / indexSize;
			if (index == 1)
				return vert.index2_ / indexSize;
			return vert.index3_ / indexSize;
		}

		// This method creates a new vertexholder with the vertices specified in mappings
		// in the order specified by mappings.
		virtual VertexHolder* copyRemapped( const std::vector<uint16>& mappings ) const
		{
			BW_GUARD;
			std::vector< VertexType > out;
			remap( out, verts_, mappings );
			VertexHolder* pOut = new MultiBlendHolder<VertexType, OutVertexType, indexSize>( &out.front(), out.size(),
				formatName_ );

			//remap the extra streams...
			for (uint i=0; i<streamCount(); i++)
			{
				VertexHolder* newStream = stream(i)->copyRemapped( mappings );
				pOut->addStream( newStream );
			}
			return pOut;
		}

		// This method remaps the bone index value to the value defined in boneList.
		static uint32 remapBone( uint32 value, const std::vector<uint32>& boneList )
		{
			value /= indexSize;
			if (value < boneList.size())
				return boneList[value] * indexSize;
			return 0;
		}

		// This method remaps all the bone indices to the ones specified in boneList
		virtual void remapBoneIndices( const std::vector<uint32>& boneList )
		{
			BW_GUARD;
			for (uint32 i = 0; i < verts_.size();i++)
			{
				VertexType& vert = verts_[i];
                vert.index_ = IndexType(remapBone( uint32(vert.index_), boneList ));
				// If an index is not used, copy the first index to it, just in case
				// we run into precision problems with the weights.
				if (nIndices(i) > 1)
					vert.index2_ = IndexType(remapBone( uint32(vert.index2_), boneList ));
				else
					vert.index2_ = vert.index_;
				if (nIndices(i) > 2)
					vert.index3_ = IndexType(remapBone( uint32(vert.index3_), boneList ));
				else
					vert.index3_ = vert.index_;
			}
		}

		// This method creates the binary data for the vertices
		virtual BinaryPtr createVertexSection() const
		{
			BW_GUARD;
			return VertexHolder::createVertexSection<VertexType, OutVertexType>( verts_, formatName_ );
		}

		// this method returns the number of vertices in this vertexholder
		virtual uint32 nVertices() const
		{
			return verts_.size();
		}
	private:
		std::vector< VertexType > verts_;
		std::string formatName_;
	};

	/**
	 *	This class implements the VertexHolder interface for rigid vertices.
	 */
	template< typename VertexType, typename OutVertexType, uint indexSize, typename IndexType = float >
	class SingleBlendHolder : public VertexHolder
	{
	public:
		SingleBlendHolder( const VertexType* pVerts, uint32 nVertices, 
			const std::string& formatName ) :
			formatName_( formatName )
		{
			BW_GUARD;
			verts_.assign( pVerts, pVerts + nVertices );
		}

		// This method returns the number of bone indices for this vertex,
		// as this is a rigid vertex, it always returns one
		virtual uint nIndices( uint32 vertex ) const
		{
			return 1;
		}

		// This method returns the bone index specified by index of the 
		// vertex specified by vertex, it always returns the same index
		// as each rigid vertex only has one index.
		virtual uint index( uint32 vertex, uint32 index ) const
		{
			const VertexType& vert = verts_[index];
			return uint(vert.index_ / indexSize);
		}


		// This method creates a new vertexholder with the vertices specified in mappings
		// in the order specified by mappings.
		virtual VertexHolder* copyRemapped( const std::vector<uint16>& mappings ) const
		{
			BW_GUARD;
			std::vector< VertexType > out;
			remap( out, verts_, mappings );
			VertexHolder* pOut = new SingleBlendHolder<VertexType, OutVertexType, indexSize>( &out.front(), out.size(),
				formatName_ );
			return pOut;
		}

		// This method remaps the bone index value to the value defined in boneList.
		static uint32 remapBone( uint32 value, const std::vector<uint32>& boneList )
		{
			value /= indexSize;
			if (value < boneList.size())
				return boneList[value] * indexSize;
			return 0;
		}

		// This method remaps all the bone indices to the ones specified in boneList
		virtual void remapBoneIndices( const std::vector<uint32>& boneList )
		{
			BW_GUARD;
			for (uint32 i = 0; i < verts_.size();i++)
			{
				VertexType& vert = verts_[i];
                vert.index_ = IndexType(remapBone( uint32(vert.index_), boneList ));
			}
		}

		// This method creates the binary data for the vertices
		virtual BinaryPtr createVertexSection() const
		{
			BW_GUARD;
			return VertexHolder::createVertexSection<VertexType, OutVertexType>( verts_, formatName_ );
		}

		// this method returns the number of vertices in this vertexholder
		virtual uint32 nVertices() const
		{
			return verts_.size();
		}
	private:
		std::vector< VertexType > verts_;
		std::string formatName_;
	};

	/**
	 * This helper class holds the indices and primitive group loaded from disk.
	 */
	class IndexHolder : public ReferenceCount
	{
	public:
		IndexHolder( const uint16* pIndices, uint32 nIndices, 
			const PrimitiveGroup* pPrimGroups, uint32 nPrimGroups )
		{
			BW_GUARD;
			indices_.assign( pIndices, pIndices + nIndices );
			primitiveGroups_.assign( pPrimGroups, pPrimGroups + nPrimGroups );
		}

		// This method returns the number of primitive groups in these indices.
		uint32 nPrimitiveGroups() { return primitiveGroups_.size(); }

		// This method creates a copy of the vertices used by a primitive group.
		void primGroupIndices( uint32 pg, std::vector<uint16>& out )
		{
			BW_GUARD;
			const PrimitiveGroup& group = primitiveGroups_[pg];
			out.assign( indices_.begin() + group.startIndex_, 
				indices_.begin() + group.startIndex_ + group.nPrimitives_ * 3 );
		}

	private:
		std::vector< uint16 > indices_;
		std::vector< PrimitiveGroup > primitiveGroups_;
	};

	
	/**
	 * This helper class holds the morph targets loaded from disk.
	 */
	class MorphTargetsHolder : public ReferenceCount
	{
	private:
		std::string vertexFormat_;							// Vertex format
		MorphVertices::MorphTargetVector morphTargets_;		// List of morph targets


	public:
		// Type definitions
		typedef SmartPointer<MorphTargetsHolder> MorphTargetsHolderPtr;

		// Constructor
		MorphTargetsHolder( const MorphHeader* pMH, std::string vertexFormat )
		{
			BW_GUARD;
			vertexFormat_ = vertexFormat;

			if (pMH)
				loadMorphTargets( pMH );
		}


		// Returns a morph targets holder with the same vertexFormat_ as this object
		MorphTargetsHolderPtr cloneFormatOnly()
		{
			BW_GUARD;
			return new MorphTargetsHolder( NULL, this->vertexFormat_ );
		}


		// Add a morph target to the list
		void addMorphTarget(MorphTargetBase::MVVector& morphTargetVertices, 
			std::string identifier, int channelIndex)
		{
			BW_GUARD;
			// Create the morph target header
			MorphTargetHeader MTH;
			MTH.channelIndex_ = channelIndex;
			MTH.nVertices_ = morphTargetVertices.size();
			bw_snprintf( MTH.identifier_, sizeof(MTH.identifier_), "%s", 
				identifier.c_str() );

			// Get a pointer to the start of the morph targer vertex list
			const MorphTargetBase::MorphVertex* pMV = 
				reinterpret_cast< const MorphTargetBase::MorphVertex* >( 
					&morphTargetVertices.front() );

			// Create a morph target
			MorphTargetPtr pMT = createMorphTarget(&MTH, pMV);
			MF_ASSERT_DEV(pMT && 
				"Moo::RenderSetSplit::MorphTargetsHolder::loadMorphTargets: "
				"Failed to create morph target!\n")

			// Add the morph target to the list if valid
			if (pMT)
				morphTargets_.push_back(pMT);
		}

		// Clear the morph targets
		void clear()
		{
			BW_GUARD;
			morphTargets_.clear();
		}

		// Remap the morph vertex indices
		void remapIndices(std::vector<uint16>& indexRemap)
		{
			BW_GUARD;
			// For each morph target
			for (uint i = 0; i < morphTargets_.size(); i++)
			{
				// For each morph vertex
				MorphTargetBase::MVVector& morphTargetVertices = 
					morphTargets_[i]->morphVertices();
				for (uint j = 0; j < morphTargetVertices.size(); j++)
				{
					// Find the index in indexRemap
					std::vector<uint16>::iterator it;
					it = std::find(indexRemap.begin(), indexRemap.end(), 
						morphTargetVertices[j].index_);
					MF_ASSERT_DEV(it != indexRemap.end() &&
						"Moo::RenderSetSplit::MorphTargetsHolder::loadMorphTargets: "
						"Failed to find index in indexRemap!\n")

					// Remap to the index position
					if( it != indexRemap.end() )
						morphTargetVertices[j].index_ = it - indexRemap.begin();
					else
						morphTargetVertices[j].index_ = 0; // 0 should be valid?
				}
			}
		}
		
		// Returns the number of morph targets
		int morphTargetCount()
		{
			return morphTargets_.size();
		}

		// Returns the ith morph target or NULL if out of bounds
		MorphTargetPtr getMorphTarget(uint i)
		{
			if (i >= morphTargets_.size())
				return NULL;
			else
				return morphTargets_[i];
		}

		// Return the vertex format
		std::string getVertexFormat()
		{
			return vertexFormat_;
		}

	private:
		// Loads in the morph targets
		void loadMorphTargets( const MorphHeader* pMH )
		{
			BW_GUARD;
			// Load in the first morph target header
			const MorphTargetHeader* pMTH = 
					reinterpret_cast< const MorphTargetHeader* >( pMH + 1 );

			// Load in each morph target
			for (int i = 0; i < pMH->nMorphTargets_; i++)
			{
				// Load in the morph target vertices
				const MorphTargetBase::MorphVertex* pMV = 
						reinterpret_cast< const MorphTargetBase::MorphVertex* >
							( pMTH + 1 );

				// Create a new morph target
				MorphTargetPtr pMT = createMorphTarget(pMTH, pMV);
				MF_ASSERT_DEV(pMT && 
					"Moo::RenderSetSplit::MorphTargetsHolder::loadMorphTargets: "
					"Failed to create morph target!\n")

				// Add the morph target to the list if valid
				if (pMT)
					morphTargets_.push_back(pMT);

				// Load the next morph target
				pMTH = reinterpret_cast< const MorphTargetHeader* >
						( pMV + pMTH->nVertices_ );
			}
		}

		// Creates a morph target of type vertexFormat_
		MorphTargetPtr createMorphTarget( const MorphTargetHeader* pMTH,
										  const MorphTargetBase::MorphVertex* pMV )
		{
			BW_GUARD;
			// Create the appropriate morph target based on the vertex format
			if (vertexFormat_ == "xyznuvi")
			{
				return new MorphTarget< VertexXYZNUVI >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else if (vertexFormat_ == "xyznuvitb")
			{
				return new MorphTarget< VertexXYZNUVITB >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else if (vertexFormat_ == "xyznuviiiww")
			{
				return new MorphTarget< VertexXYZNUVIIIWW >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else if (vertexFormat_ == "xyznuviiiwwtb")
			{
				return new MorphTarget< VertexXYZNUVIIIWWTB >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else if (vertexFormat_ == "xyznuviiiww_v2")
			{
				return new MorphTarget< VertexXYZNUVIIIWW_v2 >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else if (vertexFormat_ == "xyznuviiiwwtb_v2")
			{
				return new MorphTarget< VertexXYZNUVIIIWWTB_v2 >(
						pMV,
						pMTH->nVertices_,
						pMTH->identifier_,
						pMTH->channelIndex_
						);
			}
			else
			{
				WARNING_MSG("Moo::RenderSetSplit::MorphTargetsHolder::createMorphTarget: "
					"Unknown vertex format!\n");
				return NULL;
			}
		}
	};

	typedef std::vector<uint32> BoneRelationship;

	/**
	 *	This helper class handles and manipulates the relationship between 
	 *	different bones in a renderset to ease splitting.
	 */
	class BoneRelation
	{
	public:
		/*
		 *	Constructor for this class.
		 *	param indices is the indices to build the relationship list from
		 *	param pVHolder is the vertices to build the relationship list from
		 */
		BoneRelation( const std::vector<uint16>& indices, 
			SmartPointer<VertexHolder> pVHolder )
		{
			BW_GUARD;
			// Build relationship list.
			// For each triangle add the relationship between its bones
			// to the relationship list.
			for (uint32 i = 0; i < indices.size(); i += 3)
			{
				addRelationship( indices[i], indices[i+1], indices[i+2], 
					pVHolder );
			}

			// For each relationship in the relationship list,
			// see if the relationship is fully contained within
			// another relationship, if so, get rid of the relationship
			// that is fully contained in another one.
			uint32 relationshipCount = relationships_.size();
			for (uint32 i = 0; i < relationshipCount; i++)
			{
				BoneRelationship br = relationships_.front();
				relationships_.erase( relationships_.begin() );
				if (findRelationship( br, false ) == -1)
				{
					relationships_.push_back( br );
				}
			}
		}

		/*
		 *	This method creates a list of node indices by combining 
		 *	relationships until the list contains up to a specified number of 
		 *	nodes. Relationships that are added to the node list will be removed.
		 *
		 *	param nodeLimit the number of nodes allowed in the list
		 *	param nodeList the returned list of node indices
		 *	return false if a relationship contains too many nodes.
		 */
		bool createList(uint32 nodeLimit, std::vector<uint32>& nodeList)
		{
			BW_GUARD;
			// Grab the first relationship
			nodeList = relationships_.back();
			// if the relationship is bigger that nodeLimit, we have failed.
			if (nodeList.size() > nodeLimit )
				return false;
			relationships_.pop_back();

			// find a relationship that when combined with our current nodelist does not
			// exceed nodeLimit
			int index = 0;
			while((index = findAppropriateRelationship(nodeLimit, nodeList)) != -1)
			{
				// Get the appropriate relationship and add its nodes to our ndoeList
				const BoneRelationship& r = relationships_[index];
				for (uint i = 0; i < r.size(); i++)
				{
					if(std::find(nodeList.begin(), nodeList.end(), r[i]) == nodeList.end())
						nodeList.push_back( r[i] );
				}
				// Remove the added relationship.
				relationships_.erase( relationships_.begin() + index );
			}
			return true;
		}

		/* 
		 * check if a vertex index is used by the current nodes.
		 * param pVH the vertices to check
		 * param indexUsed, the list of boneIndices to check, the bone 
		 * indices are set to true for a bone that is used and false for 
		 * one that is not.
		 * param ind the index to check
		 * return true if success
		 */
		static bool checkIndex( SmartPointer<VertexHolder> pVH, 
			const std::vector<bool>& indexUsed, uint16 ind )
		{
			BW_GUARD;
			for (uint32 i = 0; i < pVH->nIndices(ind);i++)
			{
                if (!indexUsed[pVH->index(ind, i)])
					return false;
			}
			return true;
		}

		/*
		 * This method moves the indices that have triangles that reference the
		 * current bonelist to a separate list of indices.
		 * param indices the full index list
		 * param splitIndices the indices that reference the current bone list.
		 * param boneIndices the indices of the bones to check again.
		 * pVHolder the vertices
		 */
		void splitIndices( std::vector<uint16>& indices, std::vector<uint16>& splitIndices,
			const std::vector<uint32>& boneIndices, SmartPointer<VertexHolder> pVHolder,
			SmartPointer<MorphTargetsHolder> pMTHolder, SmartPointer<MorphTargetsHolder> pMTHolderForRS)
		{
			BW_GUARD;
			// create a index list for the nodes, true means the node is included in the list,
			// false means not included.
			std::vector<bool> indexUsed;
			indexUsed.resize(256,false);
			for (uint32 i = 0; i < boneIndices.size(); i++)
			{
				indexUsed[boneIndices[i]] = true;
			}

			// Iterate over our triangles, if all the vertices in the triangle 
			// are affected by all the bones, move them to the split list.
			uint32 index = 0;
			while (index < indices.size())
			{
				if (checkIndex(pVHolder, indexUsed, indices[index]) &&
					checkIndex(pVHolder, indexUsed, indices[index + 1]) &&
					checkIndex(pVHolder, indexUsed, indices[index + 2]))
				{
					splitIndices.insert( splitIndices.end(), indices.begin() + index, indices.begin() + index + 3 );
					indices.erase( indices.begin() + index, indices.begin() + (index + 3) );
				}
				else
				{
					index += 3;
				}
			}

			// If the vertices have morph targets
			if (pMTHolder)
			{
				// Cycle through the morph targets
				for (int i = 0; i < pMTHolder->morphTargetCount(); i++)
				{
					MorphTargetPtr pMT = pMTHolder->getMorphTarget(i);
					MorphTargetBase::MVVector& morphVertexVector = pMT->morphVertices();
					MorphTargetBase::MVVector splitMorphVertexVector;
					
					// Cycle through the vertices of this morph target to see if any
					// of the indices are included with this render set.
					for (uint j = 0; j < morphVertexVector.size(); j++)
					{
						if (std::find(splitIndices.begin(), splitIndices.end(),
									morphVertexVector[j].index_) != splitIndices.end())
						{
							// Add the morph target vertex to the list
							splitMorphVertexVector.push_back(morphVertexVector[j]);
						}
					}

					// Add the morph targets to the render set holder if there are morph
					// vertices in splitMorphVertexVector
					if (splitMorphVertexVector.size())
						pMTHolderForRS->addMorphTarget(splitMorphVertexVector, pMT->identifier(), pMT->index());

					// Clear the list of morph vertices
					splitMorphVertexVector.clear();
				}
			}
		}


		/*
		 * The number of relationships in the list.
		 */
		uint size()
		{
			return relationships_.size();
		}

	private:

		/*
		 *	This method tries to find a relationship that can be combined with the current nodelist
		 *	and not make the nodelist go over nodeLimit. It tries to find the most suitable
		 *	relationship to combine with ie the one that will increase bonecount the least.
		 *	param nodeLimit the max number of nodes in the nodelist
		 *	param nodeList the current list of nodes
		 *	return the index of the most appropriate relationship to combine with or -1 if none found.
		 */
		int	findAppropriateRelationship(uint32 nodeLimit, const std::vector<uint32>& nodeList) const
		{
			BW_GUARD;
			std::vector<uint32>::const_iterator b = nodeList.begin();
			std::vector<uint32>::const_iterator e = nodeList.end();
			int index = -1;
			uint diff = nodeLimit - nodeList.size() + 1;
			// iterate over the relationships to find an appropriate one.
			for (uint i = 0; i < relationships_.size(); i++)
			{
				uint curDiff = 0;
				const BoneRelationship& br = relationships_[i];
				for (uint32 j = 0; j < br.size(); j++)
				{
					if( std::find( b, e, br[j] ) == e )
						curDiff++;
				}
				if (curDiff < diff)
				{
					diff = curDiff;
					index = int(i);
				}
			}

			return index;
		}

		/*
		 *	This method adds unique node relationships based on the triangle described
		 *	by the inputs
		 *	param a, b, c the three indices describing a triangle
		 *	pVHolder the vertices
		 */
		void addRelationship( uint16 a, uint16 b, uint16 c, SmartPointer<VertexHolder> pVHolder )
		{
			BW_GUARD;
			// Create the relationship
			BoneRelationship relationship;
			addRelationship( relationship, a, pVHolder );
			addRelationship( relationship, b, pVHolder );
			addRelationship( relationship, c, pVHolder );

			// If this relationship has not been defined already add it to the list.
			int relindex = findRelationship( relationship );
			if (relindex < 0)
			{
				relationships_.push_back( relationship );
			}
		}

		/*
		 *	This method finds a relationship completely containing another one.
		 *	param relationship the relationship to find a match for.
		 *	param perfectMatch only find relationships that match exactly
		 *  return the index of the found relationship or -1 if none found
		 */
		int findRelationship( const BoneRelationship& relationship, bool perfectMatch = true )
		{
			BW_GUARD;
			// iterate over all relationships
			for (uint i = 0; i < relationships_.size(); i++)
			{
				if (relationships_[i].size() == relationship.size() || !perfectMatch)
				{
					// compare each element of the relationship to see if it can be found
					BoneRelationship::iterator it = relationships_[i].begin();
					BoneRelationship::iterator end = relationships_[i].end();
					bool success = true;
					for (uint32 j = 0; j < relationship.size(); j++)
					{
						if (std::find( it, end, relationship[j]) == end)
						{
							success = false;
							j = relationship.size();
						}
					}
					if (success)
						return i;
				}
			}
			return -1;
		}

		/*
		 *	This method adds the relationship for a vertex to a relationship list.
		 *	param relationship the relationship to add to
		 *	param v the index of the vertex to add the relationship for
		 *	param pVHolder the vertices
		 */
		void addRelationship( BoneRelationship& relationship, uint16 v, 
			SmartPointer<VertexHolder> pVHolder )
		{
			BW_GUARD;
			for (uint i = 0; i < pVHolder->nIndices(v); i++)
			{
				if( std::find( relationship.begin(), relationship.end(),
					pVHolder->index(v, i)) == relationship.end() )
				{
					relationship.push_back( pVHolder->index(v, i) );
				}
			}
		}

		std::vector<BoneRelationship>	relationships_;
	};

	/**
	 *	This class is a helper class for creating a new split renderset
	 */
	class NewRenderSet : public ReferenceCount
	{
	public:
		/*
		 * The constructor
		 * param indices the indices used by this renderset
		 * param pVertices the full vertex list
		 * param boneIndices the bone indices used by this renderset
		 * param nodes the full list of nodes
		 * param vertsName the name of the input vertices
		 * param indsName the name of the input indices
		 * param pgIndex the index of the primitive group
		 * param splitIndex the index of this split
		 * param pMaterialSection the material section used by this renderset
		 */
		NewRenderSet( const std::vector<uint16>& indices, SmartPointer<VertexHolder> pVertices,
				SmartPointer<MorphTargetsHolder> pMTHolderForRS, const std::vector<uint32>& boneIndices,
				const std::vector<std::string>& nodes, 
				const std::string& vertsName, const std::vector<std::string>& streamNames,
				const std::string& indsName, uint32 pgIndex, uint32 splitIndex,
				DataSectionPtr pMaterialSection ) :
			pgIndex_( pgIndex ),
			splitIndex_( splitIndex ),
			pMaterialSection_( pMaterialSection )
		{
			BW_GUARD;
			// Create the new indices for the triangles
			for (uint32 i = 0; i < indices.size(); i++)
			{
				indices_.push_back( getIndex( indices[i] ) );
			}

			// If morph targets
			if (pMTHolderForRS)
			{
				// Remap the indices of the morph
				pMTHolderForRS->remapIndices(indexRemap_);
			}
			
			// Create a new vertex list of vertices used by the triangles
			pVertices_ = pVertices->copyRemapped( indexRemap_ );

			// Create a bone remapping list, each entry in the bone remapper list
			// points to the new bone value for the bone previously residing at 
			// that index
			std::vector<uint32> boneRemapper;
			boneRemapper.resize(nodes.size(), 0);
			for (uint32 i = 0; i < boneIndices.size(); i++)
			{
				boneRemapper[boneIndices[i]] = i;
				nodes_.push_back( nodes[boneIndices[i]] );
			}

			// Remap the bone indices of our new vertices
			pVertices_->remapBoneIndices( boneRemapper );

			// Create the names of the output indices and vertices.
			// To give them unique names each name is made up of 
			// the old name + primitive group index + a letter describing 
			// the split index.
			char postfix[32];
			bw_snprintf( postfix, sizeof(postfix), "%d%c", pgIndex, 'a' + char(splitIndex_) );
			if (vertsName.length() > 9)
			{
				if (pMTHolderForRS)
					vertsName_ = vertsName.substr( 0, vertsName.length() - 10 ) + postfix + ".mvertices";
				else
					vertsName_ = vertsName.substr( 0, vertsName.length() - 9 ) + postfix + ".vertices";
			}

			streamNames_.resize(streamNames.size());
			for (uint i=0; i<streamNames.size(); i++)
			{
				uint off = streamNames[i].find_last_of('.');
				if (off != std::string::npos)
				{
					streamNames_[i] = streamNames[i].substr( 0, off ) + postfix + streamNames[i].substr(off);
				}
			}

			if (indsName.length() > 8)
			{
				indsName_ = indsName.substr( 0, indsName.length() - 8 ) + postfix + ".indices";
			}
		}

		/*
		 * This method saves the split renderset.
		 * param pVisualSection the output visual section
		 * param pPrimitives the output primitives section
		 */
		void save( DataSectionPtr pVisualSection, DataSectionPtr pPrimitives,
				   MorphTargetsHolder::MorphTargetsHolderPtr pMTH)
		{
			BW_GUARD;
			// If there are no morph targets for this render set the vertices
			// section should end in .vertices and not .mvertices
			std::string vertsSectName = vertsName_;
			bool hasMorphTargets = false;
			if (pMTH)
				if (pMTH->morphTargetCount())
					hasMorphTargets = true;
				else
					vertsSectName = vertsName_.substr(0, vertsName_.length() - 10) + ".vertices";
			
			// Create the new renderset section
			DataSectionPtr pRenderSet = pVisualSection->newSection("renderSet");
			if (pRenderSet)
			{
				// write renderset information
				pRenderSet->writeBool( "treatAsWorldSpaceObject", true );
				pRenderSet->writeStrings( "node", nodes_ );
				// create the new geometry section
                DataSectionPtr pGeometry = pRenderSet->newSection( "geometry" );
				if (pGeometry)
				{
					// Save out the vertices and indices names
					pGeometry->writeString( "vertices", vertsSectName );
					pGeometry->writeStrings( "stream", streamNames_ );
					pGeometry->writeString( "primitive", indsName_ );
					// write out the primitive group
					DataSectionPtr pPrimGroup = pGeometry->newSection("primitiveGroup");
					if (pPrimGroup)
					{
						// write out the material
						pPrimGroup->setInt( 0 );
						VisualSplitter::copySections( pPrimGroup, pMaterialSection_ );
					}
				}
			}
			// Save the vertices and indices
			pPrimitives->writeBinary( vertsSectName, pVertices_->createVertexSection() );

			// Save the extra stream info
			for (uint i=0; i<pVertices_->streamCount(); i++)
			{
				const VertexHolder* pStream = pVertices_->stream(i);
				std::string streamSectName = streamNames_[i];
				pPrimitives->writeBinary( streamSectName, pVertices_->stream(i)->createVertexSection() );
			}

			(pMTH && hasMorphTargets) ? writeMorphTargets(pMTH, pPrimitives) : 0;
			writeIndices( pPrimitives );
		}

	private:
		/*
		 * This method remaps the vertex index by compacting the vertex list
		 * to the vertices that are used.
		 */
		uint16 getIndex( uint16 index )
		{
			BW_GUARD;
			// Look for the index.
			std::vector<uint16>::iterator it = 
				std::find( indexRemap_.begin(), indexRemap_.end(), index );
			if (it != indexRemap_.end())
			{
				return uint16( it - indexRemap_.begin());
			}
			// If not found, add it and return the index of this index.
			indexRemap_.push_back( index );
			return uint16(indexRemap_.size() - 1);
		}

		/*
		 * This method creates the binary index list and saves it out to the primitive file.
		 */
		void writeIndices( DataSectionPtr pPrimitives )
		{
			BW_GUARD;
			// The size of the indices
			uint32 indsSize = indices_.size() * sizeof(uint16) + sizeof(IndexHeader) + sizeof(PrimitiveGroup);

			// Allocate data
			char* data = new char[indsSize];
			IndexHeader* pIH = reinterpret_cast<IndexHeader*>(data);
			uint16* pIndices = reinterpret_cast<uint16*>(pIH + 1);
			PrimitiveGroup* pPG = reinterpret_cast<PrimitiveGroup*>(pIndices + indices_.size());

			//Fill in the index header.
			bw_snprintf( pIH->indexFormat_, sizeof(pIH->indexFormat_), "list" );
			pIH->nIndices_ = indices_.size();
			pIH->nTriangleGroups_ = 1;

			// Copy the indices
			memcpy( pIndices, &indices_.front(), sizeof( uint16 ) * indices_.size() );

			// Fill in the primitive group
			pPG->startIndex_ = 0;
			pPG->nPrimitives_ = indices_.size() / 3;
			pPG->startVertex_ = 0;
			pPG->nVertices_ = pVertices_->nVertices();

			// Write the binary data.
			pPrimitives->writeBinary( indsName_, new BinaryBlock(data, indsSize, "BinaryBlock/VisualSplitter") );

			delete [] data;
		}


		/*
		 * This method creates the binary morph target list and saves it out to the primitive file.
		 */
		void writeMorphTargets(MorphTargetsHolder::MorphTargetsHolderPtr pMTH, DataSectionPtr pPrimitives)
		{
			BW_GUARD;
			// Get the vertex data
			BinaryPtr pVerts = pPrimitives->readBinary(vertsName_);
			const char* pVertData = reinterpret_cast<const char*>(pVerts->data());
			int vertDataLength = pVerts->len();
			
			// The size of the morph target data buffer
			uint32 morphTargetDataSize = sizeof(MorphHeader);

			// Add the morph target headers and the morph vertices to the data size
			for (int i = 0; i < pMTH->morphTargetCount(); i++)
				morphTargetDataSize += sizeof(MorphTargetHeader) +
									   sizeof(MorphTargetBase::MorphVertex) * pMTH->getMorphTarget(i)->morphVertices().size();

			// Total data buffer size
			int totalDataSize = vertDataLength + morphTargetDataSize;
			
			// Allocate data
			BinaryPtr dataBlock = new BinaryBlock(NULL, totalDataSize, "BinaryBlock/VisualSplitter");
			char* data = dataBlock->cdata();

			// Copy in the vertex data
			memcpy(data, pVertData, vertDataLength);

			MorphHeader* pMH = reinterpret_cast<MorphHeader*>(data + vertDataLength);
			
			//Fill in the morph header
			pMH->nMorphTargets_ = pMTH->morphTargetCount();
			pMH->version_ = 0x100;
			
			MorphTargetHeader* pMTHeader = reinterpret_cast<MorphTargetHeader*>(pMH + 1);

			// Loop through the morph targets adding each in turn
			for (int i = 0; i < pMTH->morphTargetCount(); i++)
			{
				pMTHeader->channelIndex_ = pMTH->getMorphTarget(i)->index();
				pMTHeader->nVertices_ = pMTH->getMorphTarget(i)->morphVertices().size();
				bw_snprintf( pMTHeader->identifier_, sizeof(pMTHeader->identifier_), "%s", pMTH->getMorphTarget(i)->identifier().c_str() );

				// Morph vertices
				char* pMorphVertices = reinterpret_cast<char*>(pMTHeader + 1);
				
				memcpy(pMorphVertices,
					   &pMTH->getMorphTarget(i)->morphVertices().front(),
					   sizeof(MorphTargetBase::MorphVertex) * pMTHeader->nVertices_);

				pMTHeader = reinterpret_cast<MorphTargetHeader*>(pMorphVertices +
																 sizeof(MorphTargetBase::MorphVertex) * pMTHeader->nVertices_); 
			}

			// Write the binary data.
			pPrimitives->writeBinary( vertsName_, dataBlock);
		}

		std::vector<uint16> indexRemap_;
		std::vector<uint16> indices_;
		SmartPointer<VertexHolder> pVertices_;
		std::string vertsName_;
		std::vector<std::string> streamNames_;
		std::string indsName_;
		uint32 pgIndex_;
		uint32 splitIndex_;
		DataSectionPtr pMaterialSection_;
		std::vector<std::string> nodes_;
	};

	typedef SmartPointer<NewRenderSet> NewRenderSetPtr;

	/*
	 *	This method does the actual splitting work
	 */
	virtual bool compute()
	{
		BW_GUARD;
		// Open up the source geometry sections
		std::vector<DataSectionPtr> geometrySections;
		pRenderSet_->openSections( "geometry", geometrySections );

		// Iterate over the geometry sections and split them.
		for (uint i = 0; i  < geometrySections.size(); i++)
		{
			// Read the vertex names
			DataSectionPtr geomSect = geometrySections[i];
			std::string verts = geomSect->readString( "vertices" );
			
			// Read in the binary file
			BinaryPtr pVerts = pPrimitives_->readBinary( verts );

			std::vector<std::string> streams;
			geomSect->readStrings( "stream", streams );

			// Create the vertex list
			SmartPointer<VertexHolder> pVHolder = readVertices( pVerts, streams );


			// If the value of "vertices" ends with .mvertices, the vertices
			// have morph targets
			MorphTargetsHolder::MorphTargetsHolderPtr pMTHolder = NULL;
			MorphTargetsHolder::MorphTargetsHolderPtr pMTHolderForRS = NULL;
			std::string morphVerts = ".mvertices";
			if ( verts.rfind( morphVerts ) == ( verts.size() - morphVerts.size() ) )
			{
				pMTHolder = readMorphTargets(pVerts);
				pMTHolderForRS = pMTHolder->cloneFormatOnly();
			}

			// Read the index names
			std::string prim = geomSect->readString( "primitive" );
			BinaryPtr pPrims = pPrimitives_->readBinary( prim );

			// Create the index list
			SmartPointer<IndexHolder> pIHolder = readIndices( pPrims );

			// Open the primitive group sections
			std::vector<DataSectionPtr> pgSections;
			geomSect->openSections( "primitiveGroup", pgSections );

			// Iterate over the primitive group sections and split
			for (uint32 j = 0; j < pgSections.size(); j++)
			{
				// Get the primitive group index.
				uint32 pgIndex = pgSections[j]->asInt(j);

				// Store the material section for later
				DataSectionPtr pMaterialSection = 
					pgSections[j]->openSection( "material" );

				// Get the indices used by this primitive group.
				std::vector<uint16> indices;
				pIHolder->primGroupIndices( pgIndex, indices );

                // Create the bone relationships used by this primitive group
				BoneRelation boneRelations( indices, pVHolder );

				// While there are still bone relationships left keep splitting
				uint32 splitIndex = 0;
				while (boneRelations.size())
				{
					// Create list of bones for the first split.
					std::vector<uint32> boneList;
					if (boneRelations.createList( nodeLimit_, boneList ))
					{
						// Create the list of indices to use for this split.
						std::vector<uint16> splitIndices;
						boneRelations.splitIndices( indices, splitIndices, boneList,
							pVHolder, pMTHolder, pMTHolderForRS );

						// Create the new split renderset using the new indices
						pRenderSets_.push_back( 
						new NewRenderSet( splitIndices, pVHolder, pMTHolderForRS, boneList, nodes_,
							verts, streams, prim, j, splitIndex, pMaterialSection ) );

						// If morph targets
						if (pMTHolderForRS)
						{
							vMTHs_.push_back(pMTHolderForRS);
							pMTHolderForRS = pMTHolder->cloneFormatOnly();
						}
						
						splitIndex++;
					}
					else
					{
						ERROR_MSG( "RenderSetSplit::compute - "
							"Unable to split visual as too many bones (%d) are"
							" being used at the same time\n", boneList.size() );
						return false;
					}
				}
			}
		}
		return true;
	}

	/* 
	 *	Save out the new rendersets
	 */
	virtual void save( DataSectionPtr pVisualSection, DataSectionPtr pPrimitives )
	{
		BW_GUARD;
		// iterate over the rendersets and save them
		for( uint32 i = 0; i < pRenderSets_.size(); i++)
		{
			if (vMTHs_.size())
				pRenderSets_[i]->save(pVisualSection, pPrimitives, vMTHs_[i]);
			else
				pRenderSets_[i]->save(pVisualSection, pPrimitives, NULL);
		}
	}
private:

	/*
	 *	Helper method to create the vertex list for the correct format.
	 */
	VertexHolder* readVertices( BinaryPtr pVerts, const std::vector<std::string>& streams )
	{
		BW_GUARD;	
#define CREATE_HOLDER( name, outName, format, outFormat, container, indexSize ) \
	if (std::string(pVh->vertexFormat_) == #name)\
    {\
		pHolder = new container<format, outFormat,indexSize>( \
		reinterpret_cast< const format* >( pVh + 1 ), pVh->nVertices_, #outName );\
    }
		const VertexHeader* pVh = reinterpret_cast<const VertexHeader*>( pVerts->data());
		VertexHolder* pHolder = NULL;
		CREATE_HOLDER( xyznuvi, xyznuvi, VertexXYZNUVI, VertexXYZNUVI, SingleBlendHolder, 3 )
		else CREATE_HOLDER( xyznuvitb, xyznuvitb, VertexXYZNUVITB, VertexXYZNUVITB, SingleBlendHolder, 3 )
		else CREATE_HOLDER( xyznuviiiww, xyznuviiiww, VertexXYZNUVIIIWW, VertexXYZNUVIIIWW, MultiBlendHolder, 3 )
		else CREATE_HOLDER( xyznuviiiwwtb, xyznuviiiwwtb, VertexXYZNUVIIIWWTB, VertexXYZNUVIIIWWTB, MultiBlendHolder, 3 )
		else CREATE_HOLDER( xyznuviiiww_v2, xyznuviiiww, VertexXYZNUVIIIWW_v2, VertexXYZNUVIIIWW, MultiBlendHolder, 1 )
		else CREATE_HOLDER( xyznuviiiwwtb_v2, xyznuviiiwwtb, VertexXYZNUVIIIWWTB_v2, VertexXYZNUVIIIWWTB, MultiBlendHolder, 1 )

		for (uint i=0; i<streams.size(); ++i)
		{
			VertexHolder* pStream;
			const std::string& name = streams[i];
			BinaryPtr pData = pPrimitives_->readBinary( name );

			uint pos = name.find_last_of( '.' );
			if (pos != std::string::npos && pData)
			{
				std::string type = name.substr(pos+1);
				if (type == "uv2")
				{
					const Vector2* pUV2 = (Vector2*)(pData->data());
					pStream = new StreamHolder<Vector2>(
						reinterpret_cast< const Vector2* >( pUV2 ), pVh->nVertices_, "uv2" );
				}
				else if (type == "colour")
				{
					const DWORD* pColour = (DWORD*)(pData->data());
					pStream = new StreamHolder<DWORD>(
						reinterpret_cast< const DWORD* >( pColour ), pVh->nVertices_, "colour" );
				}
			}
			pHolder->addStream( pStream );
		}
		return pHolder;
	}

	/*
	 *	Helper method to create the index list
	 */
	IndexHolder* readIndices( BinaryPtr pIndexData )
	{
		BW_GUARD;
		const IndexHeader* pIH = 
			reinterpret_cast<const IndexHeader*> (pIndexData->data());
		const uint16* pIndices = 
			reinterpret_cast<const uint16*>( pIH + 1 );
		const PrimitiveGroup* pPrimGroups =
			reinterpret_cast<const PrimitiveGroup*>( pIndices + pIH->nIndices_ );
		return new IndexHolder( pIndices, pIH->nIndices_, 
			pPrimGroups, pIH->nTriangleGroups_ );
	}


	/*
	 *	Helper method to create the morph targets holder from the primitives
	 *	vertices section.
	 *	param pVerts The vertex section from the primitives file.
	 */
	MorphTargetsHolder* readMorphTargets(BinaryPtr pVerts)
	{
		BW_GUARD;
		// Get the vertex header to determine the vertex type and the number of vertices
		const VertexHeader* pVHeader = reinterpret_cast<const VertexHeader*>( pVerts->data());
		int nVertices = pVHeader->nVertices_;
		
		// Get the stride of the vertices
		uint vertexStride;
		if (std::string( pVHeader->vertexFormat_ ) == "xyznuvi")
		{
			vertexStride = sizeof(VertexXYZNUVI);
		}
		else if (std::string( pVHeader->vertexFormat_ ) == "xyznuvitb")
		{
			vertexStride = sizeof(VertexXYZNUVITB);
		}
		else if (std::string( pVHeader->vertexFormat_ ) == "xyznuviiiww")
		{
			vertexStride = sizeof(VertexXYZNUVIIIWW);
		}
		else if (std::string( pVHeader->vertexFormat_ ) == "xyznuviiiwwtb")
		{
			vertexStride = sizeof(VertexXYZNUVIIIWWTB);
		}
		else if (std::string( pVHeader->vertexFormat_ ) == "xyznuviiiww_v2")
		{
			vertexStride = sizeof(VertexXYZNUVIIIWW_v2);
		}
		else if (std::string( pVHeader->vertexFormat_ ) == "xyznuviiiwwtb_v2")
		{
			vertexStride = sizeof(VertexXYZNUVIIIWWTB_v2);
		}

		// Get a pointer to the list of vertices
		const char* verts = reinterpret_cast< const char* >( pVHeader + 1 );
		
		// Get the morph header at the end of the vertex list
		const MorphHeader* pMH = reinterpret_cast< const MorphHeader* >( verts + nVertices * vertexStride );
		
		// Return the morph target holder
		return new MorphTargetsHolder( pMH, std::string(pVHeader->vertexFormat_) );
	}


	DataSectionPtr pRenderSet_;
	DataSectionPtr pPrimitives_;
	uint32 nodeLimit_;
	std::vector<std::string> nodes_;
	std::vector<NewRenderSetPtr> pRenderSets_;
	std::vector<MorphTargetsHolder::MorphTargetsHolderPtr> vMTHs_;
};

/*
 *	This method creates a renderset of the correct type depending on wether it needs to be
 *	split or not.
 *	param pRenderSet the renderset to read
 *	param pPrimitives the primitive section for this visual
 */
void VisualSplitter::readRenderSet( DataSectionPtr pRenderSet, DataSectionPtr pPrimitives )
{
	BW_GUARD;
	// Read the nodes for this renderset
	std::vector<DataSectionPtr> nodes;
	pRenderSet->openSections( "node", nodes );

	// If there are more nodes than allowed in this renderset create a splitter
	// otherwise just create a copy of the renderset.
	if (nodes.size() > nodeLimit_)
	{
		pRenderSets_.push_back( new RenderSetSplit(pRenderSet, pPrimitives, nodeLimit_) );
	}
	else
	{
		pRenderSets_.push_back( new RenderSetCopy(pRenderSet, pPrimitives) );
	}
}


}

// visual_splitter.cpp