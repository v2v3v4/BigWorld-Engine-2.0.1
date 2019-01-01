/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MORPH_VERTICES_HPP
#define MORPH_VERTICES_HPP

#include "vertices.hpp"
#include "primitive_file_structs.hpp"
#include "cstdmf/stringmap.hpp"

namespace Moo
{

typedef SmartPointer< class MorphTargetBase > MorphTargetPtr;


/**
 *	This class represents the base for the individual morph targets.
 */
class MorphTargetBase : public ReferenceCount
{
public:
	/**
	 *	The morphing data for a vertex.
	 */
	struct MorphVertex
	{
		Vector3 delta_;
		int		index_;
	};

	typedef std::vector< MorphVertex > MVVector;

	MorphTargetBase( const MorphVertex* vertices, int nVertices,
		const std::string & identifier, int index );
	virtual ~MorphTargetBase();

	virtual void apply( void* vertices, float amount ) = 0;

	const std::string&	identifier( ) const { return identifier_; };
	int					index( ) const { return index_; };

	std::vector<MorphVertex>& morphVertices() { return morphVertices_; };
	const std::vector<MorphVertex>& morphVertices() const { return morphVertices_; };

	void applyPosOnly( Vector3* pos, float amount )
	{
		BW_GUARD;
		MVVector::iterator it = morphVertices_.begin();
		MVVector::iterator end = morphVertices_.end();
		while (it != end)
		{	
			const MorphVertex& mv = *it++;
			pos[mv.index_] += mv.delta_ * amount;
		}
	}

protected:
	std::string		identifier_;
	int				index_;

	MVVector morphVertices_;
};

/**
 *	The implementation of the application of the morph target information stored
 *	in the base class to a list of vertices.
 */
template <class VertexType> 
class MorphTarget : public MorphTargetBase
{
public:
	MorphTarget( const MorphVertex* vertices, int nVertices,
			const std::string & identifier, int index )
		: MorphTargetBase( vertices, nVertices, identifier, index )
	{
	}
	void apply( void* vertices, float amount )
	{
		BW_GUARD;
		VertexType* verts = reinterpret_cast<VertexType*>( vertices );
		MVVector::iterator it = morphVertices_.begin();
		MVVector::iterator end = morphVertices_.end();
		while (it != end)
		{	
			const MorphVertex& mv = *it++;
			verts[mv.index_].pos_ += mv.delta_ * amount;
		}
	}
protected:
};

/**
 *	This pure virtual class defines a simple utility
 *	interface for a	reference counted list of vertices.
 */

class VertexListBase : public ReferenceCount
{
public:
	VertexListBase();
	virtual ~VertexListBase();

	virtual void * vertices() = 0;
	virtual uint32 nVertices() = 0;
protected:
};

class VertexListBase;
typedef SmartPointer<VertexListBase> VertexListPtr;

/**
 *	This class controls a group of vertices that can morph between keyframes
 */
class MorphVertices : public Vertices
{
public:
	MorphVertices(const std::string& resourceID, int numNodes);
	~MorphVertices();

	HRESULT		load( );
	HRESULT		release( );
	HRESULT		setVertices( bool software, bool staticLighting = false );
	HRESULT		setTransformedVertices( bool tb, const NodePtrVector& nodes );

	static void	addMorphValue( const std::string& channelName, float value, float blend )
	{
		morphValues_[channelName].addValue( value, blend, globalCookie_ );
	}

	static void incrementCookie()
	{
		globalCookie_++;
	}

	static void globalBlend( float blend )
	{
		globalBlend_ = blend;
	}

	static float morphValue( const std::string& channelName )
	{
		return morphValues_[channelName].value(globalCookie_);
	}

	VertexSnapshotPtr	getSnapshot( const NodePtrVector& nodes, bool skinned, bool bumpMapped );

	typedef std::vector< MorphTargetPtr > MorphTargetVector;
	const MorphTargetVector& morphTargets() { return morphTargets_; }

	const VertexPositions& vertexPositions() const;

private:

	template<class VertexType>
	HRESULT setVerticesType( bool software )
	{
		BW_GUARD;
		static std::vector< VertexType > verts;
		verts.assign( (VertexType*) vertexList_->vertices(), (VertexType*) vertexList_->vertices() + nVertices_ );
		MorphTargetVector::iterator it = morphTargets_.begin();
		MorphTargetVector::iterator end = morphTargets_.end();
		while (it != end)
		{
			float val = morphValues_[(*it)->identifier()].value( globalCookie_);
			if (val > 0.001f)
				(*it)->apply( &verts.front(), val );
			++it;
		}

		if (!software)
		{
			bool streamOffset = 
				(Moo::rc().deviceInfo(Moo::rc().deviceIndex()).caps_.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET) != 0;
			DynamicVertexBuffer<VertexType>& vb = DynamicVertexBuffer<VertexType>::instance();
			VertexType* vs = NULL;
			if(streamOffset)
				vs = vb.lock2( nVertices_ );
			else
				vs = vb.lock( nVertices_ );

			if (vs)
			{
				memcpy( vs, &verts.front(), sizeof(VertexType) * nVertices_ );
				vb.unlock();
				return vb.vertexBuffer().set( 0, vertexStride_ * vb.lockIndex(), vertexStride_ );
			}
			return E_FAIL;
		}
		else
		{
			VertexType* vs = DynamicSoftwareVertexBuffer<VertexType>::instance().lock( nVertices_ );
			memcpy( vs, &verts.front(), sizeof(VertexType) * nVertices_ );
			DynamicSoftwareVertexBuffer<VertexType>::instance().unlock();
			return DynamicSoftwareVertexBuffer<VertexType>::instance().vertexBuffer().set( 0, 0, vertexStride_ );
		}
	}

	MorphTargetVector morphTargets_;
	VertexListPtr vertexList_;
	VertexListPtr vertexListCache_;
	VertexSnapshotCache skinnedSnapshotCache_;
	VertexSnapshotCache rigidSnapshotCache_;

	struct MorphValue
	{
	public:
		MorphValue()
		: cookie_( 0 ),
		  value_( 0 ),
		  totalBlend_( 0 )
		{
		}
		void addValue( float value, float blend, uint32 cookie )
		{
			if (cookie_ != cookie)
			{
				cookie_ = cookie;
				value_ = 0;
				totalBlend_ = 0;
			}
			value_ += value * blend;
			totalBlend_ += blend;
		}
		float value( uint32 cookie )
		{
			if (cookie == cookie_)
			{
				return value_;
			}
			return (value_ = 0);
		}
	private:
		uint32 cookie_;
		float value_;
		float totalBlend_;
	};

	static uint32 globalCookie_;
	static float  globalBlend_;
	static StringHashMap<MorphVertices::MorphValue> morphValues_;

	MorphVertices( const MorphVertices& );
	MorphVertices& operator=( const MorphVertices& );
};

} 
#endif // MORPH_VERTICES_HPP
