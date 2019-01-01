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
#include "py_point_sprite_transfer_mesh.hpp"
#include "moo/render_context.hpp"

#pragma warning (disable:4355)	//this used in member initialisation list

#ifndef CODE_INLINE
#include "py_point_sprite_transfer_mesh.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "PostProcessing", 0 )


namespace PostProcessing
{

// Python statics
PY_TYPEOBJECT( PyPointSpriteTransferMesh )

PY_BEGIN_METHODS( PyPointSpriteTransferMesh )	
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyPointSpriteTransferMesh )
PY_END_ATTRIBUTES()

/*~ class PostProcessing.PyPointSpriteTransferMesh
 *	@components{ client, tools }
 *	A PyPointSpriteTransferMesh can be used by a PyPhase object, and provides the geometry
 *	for the phase.  In this case, the geometry is one point sprite for every pixel on the
 *	screen.
 *	The PointSpriteTransferMesh performs n-taps on the source material's
 *	textures, in groups of 4.  If you specify more than 4 taps, then the filter
 *	quad will draw using n/4 passes.  For these operations, you should make
 *	sure the material you use is additive, so the results can accumulate.
 */
/*~ function PostProcessing.PointSpriteTransferMesh
 *	@components{ client, tools }
 *	Factory function to create and return a PostProcessing PyPointSpriteTransferMesh object.
 *	@return A new PostProcessing PyPointSpriteTransferMesh object.
 */
PY_FACTORY_NAMED( PyPointSpriteTransferMesh, "PointSpriteTransferMesh", _PostProcessing )
IMPLEMENT_FILTER_QUAD( PyPointSpriteTransferMesh, PyPointSpriteTransferMesh )


PyPointSpriteTransferMesh::PyPointSpriteTransferMesh( PyTypePlus *pType ):
	FilterQuad( pType )
{
}


PyPointSpriteTransferMesh::~PyPointSpriteTransferMesh()
{
}


/**
 *	This method builds the mesh used to render with.
 */
void PyPointSpriteTransferMesh::buildMesh()
{	
	pDecl_ = Moo::VertexDeclaration::get( "xyzuv" );

	uint32 width = (uint32)Moo::rc().screenWidth();
	uint32 height = (uint32)Moo::rc().screenHeight();
	uint32 nBytes = width * height * sizeof( Moo::VertexXYZUV );

	pVertexBuffer_ = new Moo::VertexBufferWrapper<Moo::VertexXYZUV>();
	nPixels_ = width * height;
	pVertexBuffer_->reset( nPixels_ );

	float w = 1.f;
	float h = 1.f;
	Vector3 fixup( -0.5f / Moo::rc().halfScreenWidth(), 0.5f / Moo::rc().halfScreenHeight(), 0.f );
	Moo::VertexBufferWrapper<Moo::VertexXYZUV>& vertexBuffer = *pVertexBuffer_.getObject();
	
	size_t idx = 0;
	if (pVertexBuffer_->lock())
	{
		for (uint32 x=0; x<width; x++)
		{
			for (uint32 y=0; y<height; y++)
			{
				float u = (float)x / (float)width;
				float v = (float)y / (float)height;
				float uBias = (u-0.5f)*2.f;
				float vBias = (v-0.5f)*2.f;

				Moo::VertexXYZUV vert;
	
				vert.pos_.set(uBias,vBias,0.1f);
				vert.pos_ += fixup;
				vert.uv_.set(u,1.f-v);

				vertexBuffer[idx++] = vert;
			}
		}
		pVertexBuffer_->unlock();
	}
}


/**
 *	This method draws the filter quad using the current device state.
 */
void PyPointSpriteTransferMesh::draw()
{
	uint32 nPixels = (uint32)Moo::rc().screenWidth() * (uint32)Moo::rc().screenHeight();
	if (nPixels_ != nPixels)
	{
		this->buildMesh();
	}
	
	HRESULT hr;
	Moo::rc().setVertexDeclaration(pDecl_->declaration());
	pVertexBuffer_->activate(0);
	hr = Moo::rc().drawPrimitive( D3DPT_POINTLIST, 0, nPixels_ );
	if (FAILED(hr))
	{
		ERROR_MSG( "PyPointSpriteTransferMesh::draw - error drawing %lx\n", hr );
	}
	pVertexBuffer_->deactivate(0);
}


bool PyPointSpriteTransferMesh::save( DataSectionPtr pDS )
{	
	DataSectionPtr pSect = pDS->newSection( "PyPointSpriteTransferMesh" );
	return true;
}


bool PyPointSpriteTransferMesh::load( DataSectionPtr pSect )
{
	return true;
}


PyObject * PyPointSpriteTransferMesh::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();
	return FilterQuad::pyGetAttribute(attr);
}


int PyPointSpriteTransferMesh::pySetAttribute( const char *attr, PyObject *value )
{
	PY_SETATTR_STD();
	return FilterQuad::pySetAttribute(attr,value);
}


PyObject* PyPointSpriteTransferMesh::pyNew(PyObject* args)
{
	return new PyPointSpriteTransferMesh;
}

}
