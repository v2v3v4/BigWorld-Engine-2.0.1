/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_POINT_SPRITE_TRANSFER_MESH_HPP
#define PY_POINT_SPRITE_TRANSFER_MESH_HPP

#include "filter_quad.hpp"
#include "filter_quad_factory.hpp"
#include "moo/com_object_wrap.hpp"
#include "moo/vertex_declaration.hpp"
#include "moo/vertex_buffer_wrapper.hpp"
#include "moo/vertex_formats.hpp"

namespace PostProcessing
{
	//{u offset, v offset, weight}
	typedef Vector3 FilterSample;

	/**
	 *	This class implements a FilterQuad that renders
	 *	every pixel on the screen as point sprites
	 */
	class PyPointSpriteTransferMesh : public FilterQuad
	{
		Py_Header( PyPointSpriteTransferMesh, FilterQuad )
		DECLARE_FILTER_QUAD( PyPointSpriteTransferMesh )
	public:
		PyPointSpriteTransferMesh( PyTypePlus *pType = &s_type_ );
		~PyPointSpriteTransferMesh();

		void preDraw( Moo::EffectMaterialPtr pMat )	{};
		void draw();
		bool save( DataSectionPtr );
		bool load( DataSectionPtr );

#ifdef EDITOR_ENABLED
		void edEdit( GeneralEditor * editor, FilterChangeCallback pCallback ) { /*nothing to do*/ }
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		PY_FACTORY_DECLARE()
	private:
		void buildMesh();
		uint32 nPixels_;
		Moo::VertexDeclaration* pDecl_;
		typedef SmartPointer<Moo::VertexBufferWrapper<Moo::VertexXYZUV>> VertexBufferWrapperPtr;
		VertexBufferWrapperPtr pVertexBuffer_;
	};
};

#ifdef CODE_INLINE
#include "py_point_sprite_transfer_mesh.ipp"
#endif

#endif //#ifndef PY_POINT_SPRITE_TRANSFER_MESH_HPP