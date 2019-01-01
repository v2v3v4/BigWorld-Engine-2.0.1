/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHASE_HPP
#define PHASE_HPP


#ifdef EDITOR_ENABLED

#include "moo/moo_dx.hpp"

class GeneralEditor;

#endif // EDITOR_ENABLED


namespace PostProcessing
{
	/**
	 *	This class is an abstract base class for Post-Processing Phases.
	 *	It ensures derivated classes are PyObjects.
	 */
	class Phase : public PyObjectPlus
	{
		Py_Header( Phase, PyObjectPlus )
	public:
		Phase( PyTypePlus *pType = &s_type_ );
		~Phase();

		virtual void tick( float dTime ) = 0;
		virtual void draw( class Debug*, RECT* = NULL ) = 0;

		virtual bool load( DataSectionPtr ) = 0;
		virtual bool save( DataSectionPtr ) = 0;

#ifdef EDITOR_ENABLED
		typedef void (*PhaseChangeCallback)( bool needsReload );
		virtual void edChangeCallback( PhaseChangeCallback pCallback ) = 0;
		virtual void edEdit( GeneralEditor * editor ) = 0;
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

	private:
	};

	typedef SmartPointer<Phase> PhasePtr;
};

PY_SCRIPT_CONVERTERS_DECLARE( PostProcessing::Phase )

#ifdef CODE_INLINE
#include "phase.ipp"
#endif

#endif //#ifndef PHASE_HPP