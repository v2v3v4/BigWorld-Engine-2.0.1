/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include "moo_math.hpp"
#include "moo_dx.hpp"
#include "com_object_wrap.hpp"
#include "cstdmf/smartpointer.hpp"
#include "material.hpp"
#include "index_buffer.hpp"
#include "primitive_file_structs.hpp"

namespace Moo
{

typedef SmartPointer< class Primitive > PrimitivePtr;
typedef SmartPointer< class Vertices > VerticesPtr;

/**
 * This class represents a collection of grouped indices. The file format for
 * this is a .primitive file.
 */
class Primitive : public SafeReferenceCount
{
public:

	virtual HRESULT		setPrimitives();
	virtual HRESULT		drawPrimitiveGroup( uint32 groupIndex );
	virtual HRESULT		release( );
	virtual HRESULT		load( );

	uint32				nPrimGroups() const;
	const PrimitiveGroup&	primitiveGroup( uint32 i ) const;
	uint32				maxVertices() const;

	const std::string&	resourceID() const;
	void				resourceID( const std::string& resourceID );

	D3DPRIMITIVETYPE	primType() const;

	const IndicesHolder& indices() const { return indices_; }

	Primitive( const std::string& resourceID );
	virtual ~Primitive();
	
	const Vector3& origin( uint32 i ) const;
	void calcGroupOrigins( const VerticesPtr verts );
protected:

	typedef std::vector< PrimitiveGroup > PrimGroupVector;


	PrimGroupVector		primGroups_;

	std::vector<Vector3>	groupOrigins_;

	uint32				nIndices_;
	uint32				maxVertices_;
	std::string			resourceID_;
	D3DPRIMITIVETYPE	primType_;
	IndicesHolder		indices_;
	IndexBuffer			indexBuffer_;
};

}

#ifdef CODE_INLINE
#include "primitive.ipp"
#endif

#endif
