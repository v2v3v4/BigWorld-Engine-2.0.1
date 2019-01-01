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
#include "vertices.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "primitive_file_structs.hpp"
#include "render_context.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/primitive_file.hpp"
#include "software_skinner.hpp"
#include "vertex_formats.hpp"
#include "vertices_manager.hpp"

#include "vertex_streams.hpp"

#ifndef CODE_INLINE
#include "vertices.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

PROFILER_DECLARE( Vertices_setVertices, "Vertices SetVertices" );

#ifndef EDITOR_ENABLED
memoryCounterDefine( vertices, Geometry );
#endif

namespace Moo
{

Vertices::Vertices( const std::string& resourceID, int numNodes )
: resourceID_( resourceID ),
  nVertices_( 0 ),
  vbBumped_( false ),
  pDecl_( NULL ),
  pStaticDecl_( NULL ),
  streams_( NULL ),
  numNodes_( numNodes * 3 /* indices are pre-multiplied by 3 */ )
{
	BW_GUARD;	
}

Vertices::~Vertices()
{
	BW_GUARD;

	if (streams_)
	{
		delete streams_; streams_ = NULL;
	}

	// let the manager know we're gone
	VerticesManager::del( this );
}

template <class VertexType>
void copyVertexPositions( Vertices::VertexPositions& vertexPositions, const VertexType* pVerts, uint32 nVertices )
{
	BW_GUARD;
	vertexPositions.resize( nVertices );
	Vertices::VertexPositions::iterator it = vertexPositions.begin();
	Vertices::VertexPositions::iterator end = vertexPositions.end();
	while (it != end)
	{
		*(it++) = (pVerts++)->pos_;
	}
}

#ifdef EDITOR_ENABLED

template <class VertexType>
void copyVertexNormals( Vertices::VertexNormals& vertexNormals, const VertexType* pVerts, uint32 nVertices )
{
	BW_GUARD;
	vertexNormals.resize( nVertices );
	Vertices::VertexNormals::iterator it = vertexNormals.begin();
	Vertices::VertexNormals::iterator end = vertexNormals.end();
	while (it != end)
	{
		*(it++) = (pVerts++)->normal_;
	}
}

template <class VertexType>
void copyVertexNormals2( Vertices::VertexPositions& vertexNormals, const VertexType* pVerts, uint32 nVertices )
{
	BW_GUARD;
	vertexNormals.resize( nVertices );
	Vertices::VertexPositions::iterator it = vertexNormals.begin();
	Vertices::VertexPositions::iterator end = vertexNormals.end();
	while (it != end)
	{
		*(it++) = (pVerts++)->normal_;
	}
}

template <class VertexType>
void copyTangentSpace( Vertices::VertexNormals& vertexNormals, const VertexType* pVerts, uint32 nVertices )
{
	BW_GUARD;
	vertexNormals.resize( nVertices * 3 );
	Vertices::VertexNormals::iterator it = vertexNormals.begin();
	Vertices::VertexNormals::iterator end = vertexNormals.end();
	while (it != end)
	{
		*(it++) = (pVerts)->normal_;
		*(it++) = (pVerts)->tangent_;
		*(it++) = (pVerts)->binormal_;
		pVerts++;
	}
}
#endif //EDITOR_ENABLED


/**
 * Vertex snapshot specialisation for non-skinned vertices.
 */
class RigidVertexSnapshot : public VertexSnapshot, public Aligned
{
public:
	virtual ~RigidVertexSnapshot() {}

	void init( VerticesPtr pVertices, const Matrix& worldTransform )
	{
		BW_GUARD;
		pVertices_ = pVertices;
		worldViewProj_.multiply( worldTransform, Moo::rc().viewProjection() );
	}

	virtual bool getVertexDepths( uint32 startVertex, uint32 nVertices, float* pOutDepths )
	{
		BW_GUARD;
		const Vertices::VertexPositions& positions = pVertices_->vertexPositions();

		Vector3 vec( worldViewProj_.row(0).w, worldViewProj_.row(1).w, 
			worldViewProj_.row(2).w );
        float d = worldViewProj_.row(3).w;

		float* pDepth = pOutDepths;
		Vertices::VertexPositions::const_iterator it = positions.begin() + startVertex;
		for (uint32 i = 0; i < nVertices; i++)
		{
			*(pDepth++) = vec.dotProduct(*(it++)) + d;
		}
		return true;
	}

	virtual uint32 setVertices( uint32 startVertex, uint32 nVertices, bool staticLighting )
	{
		pVertices_->setVertices( false, staticLighting );
		return startVertex;
	}

	virtual void fini()
	{
		pVertices_ = NULL;
	}
protected:
	VerticesPtr	pVertices_;
	Matrix		worldViewProj_;

};


/**
* Vertex snapshot specialisation for skinned vertices.
*/
class SkinnedVertexSnapshot : public VertexSnapshot
{
public:
	virtual ~SkinnedVertexSnapshot() {}

	void init( VerticesPtr pVertices, const NodePtrVector& nodes, bool useSoftwareSkinner, bool bumpMapped )
	{
		BW_GUARD;
		pVertices_ = pVertices;
		worldTransforms_.resize( nodes.size() );
		std::avector<Matrix>::iterator mit = worldTransforms_.begin();
		NodePtrVector::const_iterator it = nodes.begin();
		while (it != nodes.end())
		{
			*mit = (*it)->worldTransform();
			++it;
			++mit;
		}
		useSoftwareSkinner_ = useSoftwareSkinner;
		bumpMapped_ = bumpMapped;
	}

	void init( VerticesPtr pVertices, const std::avector<Matrix>& transforms, bool useSoftwareSkinner, bool bumpMapped )
	{
		BW_GUARD;
		pVertices_ = pVertices;
		worldTransforms_.resize( transforms.size() );
		worldTransforms_.assign( transforms.begin(), transforms.end() );		
		useSoftwareSkinner_ = useSoftwareSkinner;
		bumpMapped_ = bumpMapped;
	}

	virtual bool getVertexDepths( uint32 startVertex, uint32 nVertices, float* pOutDepths )
	{
		BW_GUARD;
		if (bumpMapped_)
		{
			getDepths<VertexXYZNUVTBPC>( transformedVertsTB_, startVertex, nVertices, pOutDepths);
		}
		else
		{
			getDepths<VertexXYZNUV>( transformedVerts_, startVertex, nVertices, pOutDepths);
		}

		return true;
	}

	virtual uint32 setVertices( uint32 startVertex, uint32 nVertices, bool staticLighting )
	{
		BW_GUARD;
		uint32 vertexBase = startVertex;
		if (!useSoftwareSkinner_)
		{
			pVertices_->setVertices( false, staticLighting );
		}
		else if (pVertices_->softwareSkinner())
		{
			pVertices_->bindStreams( false, true, bumpMapped_ );

			BaseSoftwareSkinnerPtr pSkinner = pVertices_->softwareSkinner();
			if (bumpMapped_)
			{
				//DynamicVertexBuffer
				Moo::DynamicVertexBufferBase2<VertexXYZNUVTBPC>& vb = Moo::DynamicVertexBufferBase2<VertexXYZNUVTBPC>::instance();
				VertexXYZNUVTBPC* pVerts = vb.lock2( nVertices );
				if (pVerts)
				{
					if (transformedVertsTB_.size()) //already transformed by the sorting
						memcpy( pVerts, &transformedVertsTB_.front(), sizeof(transformedVertsTB_.front()) * transformedVertsTB_.size() );
					else
						pSkinner->transformVertices( pVerts, startVertex, nVertices, &worldTransforms_.front() );
					vb.unlock();				
					vertexBase = vb.lockIndex();
					vb.set( 0 );
				}
			}
			else
			{
				//DynamicVertexBuffer
				Moo::DynamicVertexBufferBase2<VertexXYZNUV>& vb = Moo::DynamicVertexBufferBase2<VertexXYZNUV>::instance();
				VertexXYZNUV* pVerts = vb.lock2( nVertices );
				if (pVerts)
				{
					if (transformedVerts_.size()) //already transformed by the sorting
						memcpy( pVerts, &transformedVerts_.front(), sizeof(transformedVerts_.front()) * transformedVerts_.size() );
					else
						pSkinner->transformVertices( pVerts, startVertex, nVertices, &worldTransforms_.front() );
					vb.unlock();				
					vertexBase = vb.lockIndex();
					vb.set( 0 );
				}
			}

		}
		transformedVerts_.clear();
		transformedVertsTB_.clear();
		return vertexBase;
	}

	virtual void fini()
	{
		pVertices_ = NULL;
	}

protected:
	VerticesPtr	pVertices_;
	std::avector<Matrix> worldTransforms_;
	bool	useSoftwareSkinner_;
	bool	bumpMapped_;

private:
	template <class T> void getDepths( VectorNoDestructor<T>& verts, uint32 startVertex, uint32 nVertices, float* pOutDepths )
	{
		BW_GUARD;
		Vector3 eye (Moo::rc().invView().row(3).x, Moo::rc().invView().row(3).y, Moo::rc().invView().row(3).z);
		float* pDepth = pOutDepths;
		BaseSoftwareSkinnerPtr pSkinner = pVertices_->softwareSkinner();
		verts.resize( nVertices );
		pSkinner->transformVertices( &verts.front(), startVertex, nVertices, &worldTransforms_.front() );
		for (uint i=0; i<verts.size(); i++)
		{
			*(pDepth++) = (verts[i].pos_ - eye).lengthSquared();
		}
	}

	// These are used for the sorting
	VectorNoDestructor<VertexXYZNUVTBPC>	transformedVertsTB_;
	VectorNoDestructor<VertexXYZNUV>		transformedVerts_;	
};


VertexSnapshotPtr Vertices::getSnapshot( const NodePtrVector& nodes, bool skinned, bool bumpMapped )
{
	BW_GUARD;
	VertexSnapshotPtr pSnapshot;

	if (pSoftwareSkinner_)
	{
		SmartPointer<SkinnedVertexSnapshot> pSS = skinnedSnapshotCache_.get<SkinnedVertexSnapshot>();
		pSS->init( this, nodes, !skinned, bumpMapped );
		pSnapshot = pSS;
	}
	else
	{
		SmartPointer<RigidVertexSnapshot> pSS = rigidSnapshotCache_.get<RigidVertexSnapshot>();
		pSS->init( this, nodes.front()->worldTransform() );
		pSnapshot = pSS;
	}

	return pSnapshot;
}


VertexSnapshotPtr Vertices::getSnapshot( const std::avector<Matrix>& transforms, bool skinned, bool bumpMapped )
{
	BW_GUARD;
	VertexSnapshotPtr pSnapshot;

	if (pSoftwareSkinner_)
	{
		SmartPointer<SkinnedVertexSnapshot> pSS = skinnedSnapshotCache_.get<SkinnedVertexSnapshot>();		
		pSS->init( this, transforms, !skinned, bumpMapped );
		pSnapshot = pSS;
	}
	else
	{
		SmartPointer<RigidVertexSnapshot> pSS = rigidSnapshotCache_.get<RigidVertexSnapshot>();		
		pSS->init( this, transforms[0] );
		pSnapshot = pSS;
	}

	return pSnapshot;
}


HRESULT Vertices::load( )
{
	BW_GUARD;
	HRESULT res = E_FAIL;
	release();

	// Is there a valid device pointer?
	if (Moo::rc().device() == (void*)NULL)
	{
		return res;
	}

	// find our data
	BinaryPtr vertices;
	BinaryPtr uvStream;
	BinaryPtr colourStream;

	uint noff = resourceID_.find( ".primitives/" );
	if (noff < resourceID_.size())
	{
		// everything is normal
		noff += 11;
		DataSectionPtr pPrimFile =
			PrimitiveFile::get( resourceID_.substr( 0, noff ) );
		if (pPrimFile)
		{
			vertices = pPrimFile->readBinary(
				resourceID_.substr( noff+1 ) );

			// build the stream name
			std::string stream = resourceID_.substr( noff+1 );
			uint dot = stream.find_last_of('.');
			if (dot == std::string::npos)
			{
				stream = "";
			}
			else
			{
				stream = stream.substr(0, dot+1);
			}
			
			// try the uv2 stream.
			uvStream = pPrimFile->readBinary( stream + "uv2" );
			
			// try the colour stream.
			colourStream = pPrimFile->readBinary( stream + "colour" );
		}
	}
	else
	{
		// find out where the data should really be stored
		std::string fileName, partName;
		splitOldPrimitiveName( resourceID_, fileName, partName );

		// read it in from this file
		vertices = fetchOldPrimitivePart( fileName, partName );
	}

	// Open the binary resource for these vertices
	//BinaryPtr vertices = BWResource::instance().rootSection()->readBinary( resourceID_ );
	if (vertices)
	{
		DWORD usageFlag = rc().mixedVertexProcessing() ? D3DUSAGE_SOFTWAREPROCESSING : 0;

		// Get the vertex header
		const VertexHeader* vh = reinterpret_cast< const VertexHeader* >( vertices->data() );
		nVertices_ = vh->nVertices_;

		_strlwr( const_cast< char* >( vh->vertexFormat_ ) );

		format_ = vh->vertexFormat_;
		// create the right type of vertex
		if (std::string( vh->vertexFormat_ ) == "xyznuv")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			const Moo::VertexXYZNUV* pSrcVertices = reinterpret_cast< const Moo::VertexXYZNUV* >( vh + 1 );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED
			copyVertexNormals2( vertexNormals2_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( 
					nVertices_ * sizeof( Moo::VertexXYZNUV ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				// Try to lock the vertexbuffer.
				Moo::SimpleVertexLock vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUV), 0);
				if (vl)
					memcpy( vl, pSrcVertices, sizeof( Moo::VertexXYZNUV ) * nVertices_ );// Fill the vertexbuffer.
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUV );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznduv")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			const Moo::VertexXYZNDUV* pSrcVertices = reinterpret_cast< const Moo::VertexXYZNDUV* >( vh + 1 );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED
			copyVertexNormals2( vertexNormals2_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( 
					nVertices_ * sizeof( Moo::VertexXYZNDUV ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				// Try to lock the vertexbuffer.
				SimpleVertexLock vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNDUV), 0 );
				if (vl)
					memcpy( vl, pSrcVertices, sizeof( Moo::VertexXYZNDUV ) * nVertices_ );
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNDUV );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuvtb")
		{
			Moo::VertexBuffer pVertexBuffer;

				// Get the vertices.
			const Moo::VertexXYZNUVTB* pSrcVertices = reinterpret_cast< const Moo::VertexXYZNUVTB* >( vh + 1 );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED
			copyTangentSpace( vertexNormals_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUVTBPC ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				// Try to lock the vertexbuffer.
				VertexLock<Moo::VertexXYZNUVTBPC> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUVTBPC), 0 );
				if (vl)
				{
					// Fill the vertexbuffer.
					for (uint32 i = 0; i < nVertices_; i++)
					{
						vl[i] = pSrcVertices[i];
					}
				}
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUVTBPC );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuv2tb")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			const Moo::VertexXYZNUV2TB* pSrcVertices = reinterpret_cast< const Moo::VertexXYZNUV2TB* >( vh + 1 );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED
			copyTangentSpace( vertexNormals_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUV2TBPC ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				// Try to lock the vertexbuffer.
				VertexLock<Moo::VertexXYZNUV2TBPC> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUV2TBPC), 0 );
				if (vl)
				{
					// Fill the vertexbuffer.
					for (uint32 i = 0; i < nVertices_; i++)
					{
						vl[i] = pSrcVertices[i];
					}
				}
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUV2TBPC );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuviiiww")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			ScopedVertexArray<Moo::VertexXYZNUVIIIWW> sva;
			Moo::VertexXYZNUVIIIWW* pSrcVertices = sva.init(
				reinterpret_cast< const Moo::VertexXYZNUVIIIWW* >( vh + 1 ), nVertices_ );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED				
			copyVertexNormals( vertexNormals3_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUVIIIWWPC ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{
				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				if ( !verifyIndices3<VertexXYZNUVIIIWW>( pSrcVertices, nVertices_ ) )
				{
					ERROR_MSG( "Moo::Vertices::load: Vertices in %s contain invalid bone indices\n", resourceID_.c_str() );
				}
				SoftwareSkinner< SoftSkinVertex >* pSkinner = new SoftwareSkinner< SoftSkinVertex >;
				pSkinner->init( pSrcVertices, nVertices_ );
				pSoftwareSkinner_ = pSkinner;

				// Try to lock the vertexbuffer.
				VertexLock<Moo::VertexXYZNUVIIIWWPC> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUVIIIWWPC), 0 );
				if (vl)
				{
					// Fill the vertexbuffer.
					for (uint32 i = 0; i < nVertices_; i++)
					{
						vl[i] = pSrcVertices[i];
					}
				}
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUVIIIWWPC );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuviiiwwtb")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			ScopedVertexArray<Moo::VertexXYZNUVIIIWWTB> sva;
			Moo::VertexXYZNUVIIIWWTB* pSrcVertices = sva.init(
				reinterpret_cast< const Moo::VertexXYZNUVIIIWWTB* >( vh + 1 ), nVertices_ );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );
#ifdef EDITOR_ENABLED				
			copyTangentSpace( vertexNormals_, pSrcVertices, nVertices_ );
#endif //EDITOR_ENABLED

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUVIIIWWTBPC ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				if ( !verifyIndices3<VertexXYZNUVIIIWWTB>( pSrcVertices, nVertices_ ) )
				{
					ERROR_MSG( "Moo::Vertices::load: Vertices in %s contain invalid bone indices\n", resourceID_.c_str() );
				}
				SoftwareSkinner< SoftSkinBumpVertex >* pSkinner = new SoftwareSkinner< SoftSkinBumpVertex >;
				pSkinner->init( pSrcVertices, nVertices_ );
				pSoftwareSkinner_ = pSkinner;

				VertexLock<Moo::VertexXYZNUVIIIWWTBPC> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUVIIIWWTBPC), 0 );
				// Try to lock the vertexbuffer.
				if (vl)
				{
					// Fill the vertexbuffer.
					for (uint32 i = 0; i < nVertices_; i++)
					{
						vl[i] = pSrcVertices[i];
					}
				}
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUVIIIWWTBPC );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuvitb")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			ScopedVertexArray<Moo::VertexXYZNUVITB> sva;
			Moo::VertexXYZNUVITB* pSrcVertices = sva.init(
				reinterpret_cast< const Moo::VertexXYZNUVITB* >( vh + 1 ), nVertices_ );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUVITBPC ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{
				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				if ( !verifyIndices1<VertexXYZNUVITB>( pSrcVertices, nVertices_ ) )
				{
					ERROR_MSG( "Moo::Vertices::load: Vertices in %s contain invalid bone indices\n", resourceID_.c_str() );
				}
				SoftwareSkinner< RigidSkinBumpVertex >* pSkinner = new SoftwareSkinner< RigidSkinBumpVertex >;
				pSkinner->init( pSrcVertices, nVertices_ );
				pSoftwareSkinner_ = pSkinner;

				VertexLock<Moo::VertexXYZNUVITBPC> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUVITBPC), 0 );
				// Try to lock the vertexbuffer.
				if (vl)
				{
					// Fill the vertexbuffer.
					for (uint32 i = 0; i < nVertices_; i++)
					{

						vl[i] = pSrcVertices[i];
					}
				}
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUVITBPC );
			}
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuvi")
		{
			Moo::VertexBuffer pVertexBuffer;

			// Get the vertices.
			ScopedVertexArray<Moo::VertexXYZNUVI> sva;
			Moo::VertexXYZNUVI* pSrcVertices = sva.init(
				reinterpret_cast< const Moo::VertexXYZNUVI* >( vh + 1 ), nVertices_ );

			copyVertexPositions( vertexPositions_, pSrcVertices, nVertices_ );

			// Create the vertex buffer
			if (SUCCEEDED( res = pVertexBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNUVI ), usageFlag, 0, 
					D3DPOOL_MANAGED ) ))
			{

				// Set up the smartpointer to the vertex buffer
				vertexBuffer_ = pVertexBuffer;

				// PCWJ - removed index verification check for Mesh Particle System type vertices.  Nobody should
				// be using this vertex format any more except for mesh particles.				
				//if ( !verifyIndices1<VertexXYZNUVI>( pSrcVertices, nVertices_ ) )
				//{
				//	ERROR_MSG( "Moo::Vertices::load: Vertices in %s contain invalid bone indices\n", resourceID_.c_str() );
				//}
				SoftwareSkinner< RigidSkinVertex >* pSkinner = new SoftwareSkinner< RigidSkinVertex >;
				pSkinner->init( pSrcVertices, nVertices_ );
				pSoftwareSkinner_ = pSkinner;

				VertexLock<Moo::VertexXYZNUVI> vl( vertexBuffer_, 0, nVertices_*sizeof(Moo::VertexXYZNUVI), 0 );
				// Try to lock the vertexbuffer.
				if (vl)
					memcpy( vl, pSrcVertices, sizeof( Moo::VertexXYZNUVI ) * nVertices_ );//// Fill the vertexbuffer.
				else
					res = E_FAIL;
				vertexStride_ = sizeof( Moo::VertexXYZNUVI );
			}
		}
		else
		{
			ERROR_MSG( "Failed to recognise vertex format: %s\n", vh->vertexFormat_ );
		}

		pDecl_ = VertexDeclaration::get( vh->vertexFormat_ );
		pStaticDecl_ = VertexDeclaration::get( std::string(vh->vertexFormat_) + std::string("_d") );

		// It's worth considering the usage of the declarations here.
		// Note that if the vertices are software skinned, they will use a 
		// different declaration when drawn. Is it the case that for each 
		// Vertices object? they will only use one declaration ever? If so, then
		// it could be detected that the software skinning will be used so that
		// declaration can be places in pDecl_ .... 

		// for now there are two extra declarations... pSoftwareDecl_ and pSoftwareDeclTB_
		// we would want to get rid of these ultimately.
		// perhaps we can just detect the change of materials and then re-create the
		// required declarations??? The only thing that will cause issues is if the 
		// same Vertices object is used multiple times with different declarations.

		//try the secondary uv streams.
		if (colourStream)
		{
			if (streams_ == NULL)
				streams_ = new StreamContainer();

			// attach a new stream.
			SmartPointer<VertexStreamHolder<ColourStream>> streamPtr;
			streamPtr = new VertexStreamHolder<ColourStream>();
			streamPtr ->load( colourStream->cdata(), nVertices_ );
			streams_->streamData_.push_back( streamPtr );

			// get the added stream declaration.
			Moo::VertexDeclaration* pColourDecl = VertexDeclaration::get( "colour" );
			Moo::VertexDeclaration* pColourDecl2 = VertexDeclaration::get( "colour2" );
			if (pColourDecl)
			{
				if (pDecl_)
				{
					// combine the stream declaration with the actual to 
					// create a new one.
					pDecl_ = VertexDeclaration::combine( pDecl_, pColourDecl );
				}
				
				if (pStaticDecl_)
				{
					// combine the stream declaration with the actual to 
					// create a new one.
					pStaticDecl_ = VertexDeclaration::combine( pStaticDecl_, pColourDecl2 );
				}
	
				// and do the same with the software decls...
				streams_->updateDeclarations( pColourDecl ); 				
			}
		}

		if (uvStream)
		{
			if (streams_ == NULL)
				streams_ = new StreamContainer();

			SmartPointer<VertexStreamHolder<UV2Stream>> streamPtr;
			streamPtr = new VertexStreamHolder<UV2Stream>();
			streamPtr ->load( uvStream->cdata(), nVertices_ );
			streams_->streamData_.push_back( streamPtr );

			Moo::VertexDeclaration* pUV2Decl = VertexDeclaration::get( "uv2" );
			if (pUV2Decl)
			{
				if (pDecl_)
				{
					pDecl_ = VertexDeclaration::combine( pDecl_, pUV2Decl );
				}
				
				if (pStaticDecl_)
				{
					pStaticDecl_ = VertexDeclaration::combine( pStaticDecl_, pUV2Decl );
				}

				//software decls...
				streams_->updateDeclarations( pUV2Decl );
			}
		}
	}
	else
	{
		ERROR_MSG( "Failed to read binary resource: %s\n", resourceID_.c_str() );
	}

	// Add the buffer to the preload list so that it can get uploaded
	// to video memory
	vertexBuffer_.addToPreloadList();

	if (streams_)
	{
		streams_->preload();
	}
	return res;
}


HRESULT Vertices::release( )
{
	BW_GUARD;
	vertexBuffer_.release();
	if (streams_)
	{
		streams_->release();
	}
	nVertices_ = 0;
	return S_OK;
}

/**
  * Bind the vertex data and choose a vertex declaration to use.
  */
HRESULT Vertices::bindStreams( bool staticLighting, bool softwareSkinned /*= false*/, bool bumpMapped /*= false*/ )
{
	if (streams_)
	{
		streams_->set();
	}

	HRESULT hr = E_FAIL;
	if (softwareSkinned)
	{
		if (bumpMapped)
		{
			if (streams_)
			{
				hr = Moo::rc().setVertexDeclaration( streams_->pSoftwareDeclTB_->declaration() );
			}
			else
			{
				static VertexDeclaration* pDecl = VertexDeclaration::get( "xyznuvtb" );
				hr = Moo::rc().setVertexDeclaration( pDecl->declaration() );
			}
		}
		else
		{
			if (streams_)
			{
				hr = Moo::rc().setVertexDeclaration( streams_->pSoftwareDecl_->declaration() );
			}
			else
			{
				static VertexDeclaration* pDecl = VertexDeclaration::get( "xyznuv" );
				hr = Moo::rc().setVertexDeclaration( pDecl->declaration() );
			}
		}
	}
	else
	{
		DX::VertexDeclaration * vd = pDecl_->declaration();

		if (staticLighting && pStaticDecl_)
		{
			vd = pStaticDecl_->declaration();
		}
		hr = Moo::rc().setVertexDeclaration( vd );
	}
	return hr;
}

/**
* This method transforms vertices and prepares them for drawing.
*/
HRESULT Vertices::setTransformedVertices( bool tb, const NodePtrVector& nodes )
{
	BW_GUARD;
	if (pSoftwareSkinner_)
	{
		bindStreams( false, true, tb );

		if (tb)
		{
			if (!pSkinnerVertexBuffer_.valid() || vbBumped_ == false)
			{
				//DynamicVertexBuffer
				Moo::DynamicVertexBufferBase2<VertexXYZNUVTBPC>& vb = Moo::DynamicVertexBufferBase2<VertexXYZNUVTBPC>::instance();
				VertexXYZNUVTBPC* pVerts = vb.lock( nVertices_ );
				pSoftwareSkinner_->transformVertices( pVerts, 0, nVertices_, nodes );
				vb.unlock();				
				pSkinnerVertexBuffer_ = vb.vertexBuffer();
				vbBumped_ = true;
			}
			return pSkinnerVertexBuffer_.set( 0, 0, sizeof( VertexXYZNUVTBPC ) );
		}
		else
		{
			if (!pSkinnerVertexBuffer_.valid() || vbBumped_ == true)
			{
				//DynamicVertexBuffer
				Moo::DynamicVertexBufferBase2<VertexXYZNUV>& vb = Moo::DynamicVertexBufferBase2<VertexXYZNUV>::instance();
				VertexXYZNUV* pVerts = vb.lock( nVertices_ );
				pSoftwareSkinner_->transformVertices( pVerts, 0, nVertices_, nodes );
				vb.unlock();				
				pSkinnerVertexBuffer_ = vb.vertexBuffer();
				vbBumped_ = false;
			}
			return pSkinnerVertexBuffer_.set( 0, 0, sizeof( VertexXYZNUV ) );
		}
	}
	else
	{
		return setVertices( false );
	}
	return S_OK;
}

/**
 * This method prepares vertices for drawing.
 */
HRESULT Vertices::setVertices( bool software, bool staticLighting )
{
	BW_GUARD_PROFILER( Vertices_setVertices );

	// Does our vertexbuffer exist.
	HRESULT hr = E_FAIL;

	// Does our vertexbuffer exist?
	if (!vertexBuffer_.valid())
	{
		// If not load it
		hr = load();

		if (FAILED( hr ))
			return hr;
	}

	hr = bindStreams( staticLighting );
	
	// Set up the stream source(s).
	if ( SUCCEEDED( hr ) )
	{	
		hr = vertexBuffer_.set( 0, 0, vertexStride_ );
	}

	return hr;
}


/**
 *	This method opens up the primitives file and vertices sub-file and
 *	returns them in the output parameters.
 *	@param	pPrimFile	[out] returned smartpointer to primitives DataSection.
 *	@param	vertices	[out] returned smartpointer to vertices BinSection.
 *	@param	partName	[out] name of vertices section in primitives file.
 *	@return	bool		if the primitives file and vertices file were opened.
 */
bool Vertices::openSourceFiles(
	DataSectionPtr& pPrimFile,
	BinaryPtr& vertices,
	std::string& partName )
{
	BW_GUARD;
	// find our data		
	uint noff = resourceID_.find( ".primitives/" );
	if (noff < resourceID_.size())
	{		
		noff += 11;
		pPrimFile =	PrimitiveFile::get( resourceID_.substr( 0, noff ) );		
		partName = resourceID_.substr( noff+1 );
	}
	else
	{
		// find out where the data should really be stored
		std::string fileName;		
		splitOldPrimitiveName( resourceID_, fileName, partName );		
		std::string id = fileName + ".primitives";
		pPrimFile = PrimitiveFile::get( id );		
	}

	if (pPrimFile)
	{
		vertices = pPrimFile->readBinary( partName );
	}
	else
	{
		ERROR_MSG( "Could not open primitive file to find vertices: %s\n", resourceID_.c_str());
		return false;
	}

	if (!vertices)
	{
		ERROR_MSG( "Could not open vertices in file: %s\n", resourceID_.c_str());
		return false;
	}

	return true;
}


/**
 *	This method exists because hardskinned vertices have been deprecated in
 *	1.8, and removed in 1.9.  This method resaves hard-skinned vertex information
 *	in skinned format.
 *	@return	bool	Success or Failure.
 */
bool Vertices::resaveHardskinnedVertices()
{
	BW_GUARD;
	DataSectionPtr pPrimFile;
	BinaryPtr vertices;
	std::string partName;

	if (this->openSourceFiles(pPrimFile,vertices,partName))
	{
		// Get the vertex header
		const VertexHeader* vh = reinterpret_cast< const VertexHeader* >( vertices->data() );
		nVertices_ = vh->nVertices_;
		_strlwr( const_cast< char* >( vh->vertexFormat_ ) );
		format_ = vh->vertexFormat_;

		// create the right type of vertex
		int nVerts = vh->nVertices_;
		if (std::string( vh->vertexFormat_ ) == "xyznuvitb")
		{				
			//Go from XYZNUVITB to XYZNUVIIIWWTBPC			
			const VertexXYZNUVITB * pSrcVerts = reinterpret_cast< const VertexXYZNUVITB* >( vh + 1 );
			//int dstSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVIIIWWTBPC) * nVerts;
			int dstSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVIIIWWTB) * nVerts;
			int srcSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVITB) * nVerts;
			//extraSize accounts for any information that is in our data section that
			//we don't know about (for instance morph targets)
			int extraSize = vertices->len() - srcSize;
			char * output = new char[dstSize + extraSize];

			VertexHeader* vh2 = (VertexHeader*)output;
			ZeroMemory( vh2, sizeof( vh2 ) );
			vh2->nVertices_ = nVerts;
			//bw_snprintf( vh2->vertexFormat, sizeof( vh2->vertexFormat ), "xyznuviiiwwtbpc" );
			bw_snprintf( vh2->vertexFormat_, sizeof( vh2->vertexFormat_ ), "xyznuviiiwwtb" );

			//VertexXYZNUVIIIWWTBPC * outVerts = reinterpret_cast< VertexXYZNUVIIIWWTBPC * >(vh2 + 1);
			VertexXYZNUVIIIWWTB * outVerts = reinterpret_cast< VertexXYZNUVIIIWWTB * >(vh2 + 1);
			for (int i=0; i<nVerts; i++)
			{
				outVerts[i] = pSrcVerts[i];				
			}

			//copy trailing information from source data file
			memcpy( output + dstSize, vertices->cdata() + srcSize, extraSize );

			INFO_MSG( "Converted file %s\n", resourceID_.c_str() );			
			BinaryPtr pVerts = 
				new BinaryBlock(output, dstSize + extraSize, "BinaryBlock/vertices");
			pPrimFile->writeBinary( partName, pVerts );
			pPrimFile->save();
			delete[] output;
			return true;
		}
		else if (std::string( vh->vertexFormat_ ) == "xyznuvi")
		{
			//Go from XYZNUVI to XYZNUVIIIWWPC			
			const VertexXYZNUVI * pSrcVerts = reinterpret_cast< const VertexXYZNUVI* >( vh + 1 );
			//int dstSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVIIIWWPC) * nVerts;
			int dstSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVIIIWW) * nVerts;
			int srcSize = sizeof(VertexHeader) + sizeof(VertexXYZNUVI) * nVerts;
			int extraSize = vertices->len() - srcSize;
			char * output = new char[dstSize + extraSize];

			VertexHeader* vh2 = (VertexHeader*)output;
			ZeroMemory( vh2, sizeof( vh2 ) );
			vh2->nVertices_ = nVerts;
			//bw_snprintf( vh2->vertexFormat, sizeof( vh2->vertexFormat ), "xyznuviiiwwpc" );
			bw_snprintf( vh2->vertexFormat_, sizeof( vh2->vertexFormat_ ), "xyznuviiiww" );

			//VertexXYZNUVIIIWWPC * outVerts = reinterpret_cast< VertexXYZNUVIIIWWPC * >(vh2 + 1);
			VertexXYZNUVIIIWW * outVerts = reinterpret_cast< VertexXYZNUVIIIWW * >(vh2 + 1);
			for (int i=0; i<nVerts; i++)
			{
				outVerts[i] = pSrcVerts[i];				
			}

			//copy trailing information from source data file
			memcpy( output + dstSize, vertices->cdata() + srcSize, extraSize );

			INFO_MSG( "Converted file %s\n", resourceID_.c_str() );
			BinaryPtr pVerts = 
				new BinaryBlock(output, dstSize + extraSize, "BinaryBlock/vertices");
			pPrimFile->writeBinary( partName, pVerts );
			pPrimFile->save();
			delete[] output;
			return true;
		}
		else
		{
			ERROR_MSG( "Cannot change from vertex format: %s (not yet implemented)\n", vh->vertexFormat_ );
			return false;
		}
	}
	else
	{		
		//openSourceFiles outputs an errormsg, so no need to do it here.
		return false;
	}

	return true;
}


std::ostream& operator<<(std::ostream& o, const Vertices& t)
{
	o << "Vertices\n";
	return o;
}

}
