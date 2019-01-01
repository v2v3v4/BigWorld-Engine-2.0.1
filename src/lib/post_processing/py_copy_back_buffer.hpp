/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_COPY_BACK_BUFFER_HPP
#define PY_COPY_BACK_BUFFER_HPP

#include "phase.hpp"
#include "phase_factory.hpp"
#include "filter_quad.hpp"
#include "romp/py_material.hpp"
#include "romp/py_render_target.hpp"

namespace PostProcessing
{
	/**
	 *	This class derives from PostProcessing::Phase, and
	 *	provides a way to get a copy of the back buffer in
	 *	a render target.  It does this by doing a StretchRect
	 *	from the back buffer, which allows the card to resolve
	 *	anti-aliasing at that time.
	 */
	class PyCopyBackBuffer : public Phase
	{
		Py_Header( PyCopyBackBuffer, Phase )
		DECLARE_PHASE( PyCopyBackBuffer )
	public:
		PyCopyBackBuffer( PyTypePlus *pType = &s_type_ );
		~PyCopyBackBuffer();

		void tick( float dTime );
		void draw( class Debug*, RECT* = NULL );

		bool load( DataSectionPtr );
		bool save( DataSectionPtr );

		static uint32 drawCookie();
		static void incrementDrawCookie();

#ifdef EDITOR_ENABLED
		void edChangeCallback( PhaseChangeCallback pCallback );
		void edEdit( GeneralEditor * editor );

		std::string getOutputRenderTarget() const;
		bool setOutputRenderTarget( const std::string & rt );
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		PY_RW_ATTRIBUTE_DECLARE( pRenderTarget_, renderTarget )
		PY_RW_ATTRIBUTE_DECLARE( name_, name )

		PY_FACTORY_DECLARE()
	private:
		bool			isDirty() const;
		PyRenderTargetPtr pRenderTarget_;
		std::string		name_;
		static uint32	s_drawCookie_;
		uint32			drawCookie_;

#ifdef EDITOR_ENABLED
		PhaseChangeCallback pCallback_;
#endif // EDITOR_ENABLED

		bool renderTargetFromString( const std::string & resourceID );
	};	
};


#ifdef CODE_INLINE
#include "py_copy_back_buffer.ipp"
#endif

#endif //#ifndef PY_COPY_BACK_BUFFER_HPP
