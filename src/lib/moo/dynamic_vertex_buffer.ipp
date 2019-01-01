/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

namespace Moo
{

INLINE
void DynamicVertexBufferBase::release( )
{
	lockBase_ = 0;
	vertexBuffer_.release();
}

INLINE

void DynamicVertexBufferBase::resetLock( )
{
	reset_ = true;	
}

INLINE
HRESULT DynamicVertexBufferBase::unlock( )
{
	BW_GUARD_PROFILER( DynamicVertexBufferBase_unlock );

//	MF_ASSERT( locked_ == true );
	locked_ = false;
	return vertexBuffer_.unlock( );
}

INLINE
Moo::VertexBuffer DynamicVertexBufferBase::vertexBuffer( )
{
//	MF_ASSERT( vertexBuffer_.pComObject() != NULL );
	return vertexBuffer_;
}

INLINE
HRESULT DynamicVertexBufferBase::set( UINT streamNumber, UINT offsetInBytes, UINT stride )
{
	return vertexBuffer_.set( streamNumber, offsetInBytes, stride );
}

}
/*dynamic_vertex_buffer.ipp*/
