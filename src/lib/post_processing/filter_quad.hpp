/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FILTER_QUAD_HPP
#define FILTER_QUAD_HPP

#include "resmgr/datasection.hpp"
#include "moo/effect_material.hpp"

#ifdef EDITOR_ENABLED

class GeneralEditor;

#endif // EDITOR_ENABLED


namespace PostProcessing
{
	/**
	 *	This class abstract class implements the interface for
	 *	filter quads, used by PyPhases.
	 */
	class FilterQuad : public PyObjectPlus
	{
		Py_Header( FilterQuad, PyObjectPlus )

	public:
		FilterQuad( PyTypePlus *pType = &s_type_ );
		~FilterQuad();

		virtual void preDraw( Moo::EffectMaterialPtr pMat ) = 0;
		virtual void draw() = 0;
		virtual bool save( DataSectionPtr ) = 0;
		virtual bool load( DataSectionPtr ) = 0;

#ifdef EDITOR_ENABLED
		typedef void (*FilterChangeCallback)( bool needsReload );
		virtual void edEdit( GeneralEditor * editor, FilterChangeCallback pCallback ) = 0;
		virtual const char * creatorName() const = 0;
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

	private:		
	};

	typedef SmartPointer<FilterQuad> FilterQuadPtr;
};

PY_SCRIPT_CONVERTERS_DECLARE( PostProcessing::FilterQuad )

#ifdef CODE_INLINE
#include "filter_quad.ipp"
#endif

#endif //#ifndef FILTER_QUAD_HPP