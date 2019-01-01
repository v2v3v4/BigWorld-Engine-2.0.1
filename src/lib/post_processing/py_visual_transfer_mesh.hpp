/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_VISUAL_TRANSFER_MESH_HPP
#define PY_VISUAL_TRANSFER_MESH_HPP

#include "filter_quad.hpp"
#include "filter_quad_factory.hpp"
#include "moo/visual.hpp"

namespace PostProcessing
{
	/**
	 *	This class implements a FilterQuad that just
	 *	draws a visual file.
	 */
	class PyVisualTransferMesh : public FilterQuad
	{
		Py_Header( PyVisualTransferMesh, FilterQuad )
		DECLARE_FILTER_QUAD( PyVisualTransferMesh )
	public:
		PyVisualTransferMesh( PyTypePlus *pType = &s_type_ );
		~PyVisualTransferMesh();

		void preDraw( Moo::EffectMaterialPtr pMat )	{};
		void draw();
		bool save( DataSectionPtr );
		bool load( DataSectionPtr );

#ifdef EDITOR_ENABLED
		void edEdit( GeneralEditor * editor, FilterChangeCallback pCallback );
		std::string edGetResourceID() const;
		bool edSetResourceID( const std::string & resID );
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		const std::string& resourceID() const	{ return resourceID_; }
		void resourceID( const std::string& );

		PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, resourceID, resourceID )

		PY_FACTORY_DECLARE()
	private:
		Moo::VisualPtr		visual_;
		std::string			resourceID_;
		
#ifdef EDITOR_ENABLED
		FilterChangeCallback pCallback_;
#endif // EDITOR_ENABLED
	};
};

#ifdef CODE_INLINE
#include "py_visual_transfer_mesh.ipp"
#endif

#endif //#ifndef PY_VISUAL_TRANSFER_MESH_HPP