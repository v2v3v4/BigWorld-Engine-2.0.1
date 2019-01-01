/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "custom_mesh.hpp"
#include "moo/render_context.hpp"
#include "moo/dynamic_vertex_buffer.hpp"

#ifndef CODE_INLINE
#include "custom_mesh.ipp"
#endif

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
template <class T>
CustomMesh<T>::CustomMesh( D3DPRIMITIVETYPE pType )
:primitiveType_( pType )
{
}


/**
 *	Destructor.
 */
template <class T>
CustomMesh<T>::~CustomMesh()
{
}


/**
 *	This method draws the custom mesh.
 */
template <class T>
HRESULT CustomMesh<T>::draw()
{
	int numPrimitives = this->nPrimitives( this->size() );
	if (numPrimitives)
	{
		Moo::rc().setPixelShader( NULL );
		Moo::rc().setVertexShader( NULL );
		Moo::rc().setFVF( T::fvf() );

		uint32 lockIndex = 0;
		//DynamicVertexBuffer
		Moo::DynamicVertexBufferBase2<T>& vb =
			Moo::DynamicVertexBufferBase2<T>::instance();
		if ( vb.lockAndLoad( &front(), this->size(), lockIndex ) &&
			 SUCCEEDED(vb.set( 0 )) )
		{
			HRESULT res = Moo::rc().drawPrimitive( primitiveType_, lockIndex, numPrimitives );
			//Reset stream
			vb.unset( 0 );		
			return res;
		}
	}
	return S_OK;
}


/**
 *	This method draws the custom mesh as an effect,
 *	meaning no vertex shader is set up.
 */
template <class T>
HRESULT CustomMesh<T>::drawEffect(Moo::VertexDeclaration* decl)
{
	int numPrimitives = this->nPrimitives( this->size() );
	if (numPrimitives)
	{
		if (decl)
		{
			Moo::rc().setVertexDeclaration(decl->declaration());
		}
		else
		{
			Moo::rc().setFVF( T::fvf() );
		}
		uint32 lockIndex = 0;
		//DynamicVertexBuffer
		Moo::DynamicVertexBufferBase2<T>& vb = 
			Moo::DynamicVertexBufferBase2<T>::instance();
		if ( vb.lockAndLoad( &front(), this->size(), lockIndex ) &&
			 SUCCEEDED(vb.set( 0 )) )
		{
			HRESULT res = Moo::rc().drawPrimitive( primitiveType_, lockIndex, numPrimitives );
			//Reset stream
			vb.unset( 0 );
			return res;
		}
	}
	return S_OK;
}


/**
 *	This method draws a range of vertices from the custom mesh.
 */
template <class T>
HRESULT CustomMesh<T>::drawRange( uint32 from, uint32 to )
{
	int numPrimitives = this->nPrimitives( to-from );
	if (numPrimitives)
	{
		Moo::rc().setFVF( T::fvf() );
		Moo::rc().setVertexShader( NULL );

		//DynamicVertexBuffer
		Moo::DynamicVertexBufferBase2<T>& vb = Moo::DynamicVertexBufferBase2<T>::instance();
		uint32 lockIndex = 0;
		if ( vb.lockAndLoad( &front()+from, numPrimitives, lockIndex ) &&
			 SUCCEEDED(vb.set( 0 )) )
		{
			HRESULT res = Moo::rc().drawPrimitive( primitiveType_, lockIndex, numPrimitives );
			vb.unset( 0 );
			return res;
		}
	}
	return S_OK;
}


/**
 *	This method converts nVerts into nPrims given our prim type.
 */
template <class T>
uint32 CustomMesh<T>::nPrimitives( uint32 meshSize )
{
	switch ( primitiveType_ )
	{
	case D3DPT_POINTLIST:
		return meshSize;
		break;
	case D3DPT_LINELIST:
		return meshSize / 2;
		break;
	case D3DPT_LINESTRIP:
		return meshSize - 1;
		break;
	case D3DPT_TRIANGLESTRIP:
		return meshSize - 2;
		break;
	case D3DPT_TRIANGLELIST:
		return meshSize / 3;
		break;
	case D3DPT_TRIANGLEFAN:
		return meshSize - 2;
		break;
	}

	return 0;
}


// mesh.cpp
