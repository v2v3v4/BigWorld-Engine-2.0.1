/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_TRANSFER_QUAD_HPP
#define PY_TRANSFER_QUAD_HPP

#include "filter_quad.hpp"
#include "filter_quad_factory.hpp"
#include "moo/com_object_wrap.hpp"
#include "moo/vertex_declaration.hpp"
#include "moo/vertex_formats.hpp"
#include "romp/custom_mesh.hpp"

namespace PostProcessing
{
	/**
	 *	This class implements a simple FilterQuad for use
	 *	by PyPhase objects that uses 1 sample and no offsets.
	 */
	class PyTransferQuad : public FilterQuad
	{
		Py_Header( PyTransferQuad, FilterQuad )
		DECLARE_FILTER_QUAD( PyTransferQuad )
	public:
		PyTransferQuad( PyTypePlus *pType = &s_type_ );
		~PyTransferQuad();

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
		void addVert( Moo::VertexXYZNUV2& v );
		void buildMesh();

		CustomMesh< Moo::FourTapVertex > verts4tap_;
		Moo::VertexDeclaration* pDecl4tap_;
	};
};

#ifdef CODE_INLINE
#include "py_transfer_quad.ipp"
#endif

#endif //#ifndef PY_TRANSFER_QUAD_HPP