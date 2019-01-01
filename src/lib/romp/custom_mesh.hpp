/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CUSTOM_MESH_HPP
#define CUSTOM_MESH_HPP

#include "moo/moo_dx.hpp"
#include "moo/vertex_declaration.hpp"
#include "cstdmf/vectornodest.hpp"

/**
 *	This templatised class gives the ability to
 *	create a custom mesh.
 *
 *	Template vertex format type must have an
 *	int fvf() method.
 */
template <class T>
class CustomMesh : public VectorNoDestructor< T >
{
public:
	explicit CustomMesh( D3DPRIMITIVETYPE primitiveType = D3DPT_TRIANGLELIST );
	~CustomMesh();

	D3DPRIMITIVETYPE	primitiveType() const	{ return primitiveType_; }

	int			vertexFormat() const { return T::fvf(); }
	int			nVerts() const	{ return size(); }

	HRESULT		draw();
	HRESULT		drawEffect( Moo::VertexDeclaration* decl = NULL );
	HRESULT		drawRange( uint32 from, uint32 to );

private:
	uint32		nPrimitives( uint32 nVertices );
	CustomMesh( const CustomMesh& );
	CustomMesh& operator=( const CustomMesh& );

	D3DPRIMITIVETYPE primitiveType_;
};

#ifdef CODE_INLINE
#include "custom_mesh.ipp"
#endif

//custom_mesh.cpp is inlined here for its template function definitions.
//we could have inlined it in this header file, but that would
//look ugly.  And the implementation is still in a .cpp file.
#include "custom_mesh.cpp"

#endif // CUSTOM_MESH_HPP
