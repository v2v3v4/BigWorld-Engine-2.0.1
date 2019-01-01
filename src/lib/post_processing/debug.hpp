/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PP_DEBUG_HPP
#define PP_DEBUG_HPP

#include "cstdmf/init_singleton.hpp"
#include "romp/py_render_target.hpp"

namespace PostProcessing
{
	class Debug : public PyObjectPlus
	{
		Py_Header( Debug, PyObjectPlus )
	public:
		Debug( PyTypePlus *pType = &s_type_ );
		virtual ~Debug() {};
		bool enabled() const { return enabled_; }
		void enabled( bool e ){ enabled_ = e; }
		virtual void beginChain( uint32 nEffects );
		virtual void beginEffect( uint32 nPhases );
		void drawDisabledPhase();
		void copyPhaseOutput( Moo::RenderTargetPtr pRT );
		virtual RECT nextPhase();
		PyRenderTargetPtr pRT() const;

		virtual Vector4 phaseUV( uint32 effect, uint32 phase, uint32 nEffects, uint32 nPhases );
		PY_AUTO_METHOD_DECLARE( RETDATA, phaseUV, ARG( uint32, ARG( uint32, ARG( uint32, ARG( uint32, END ) ) ) ) );

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		PY_RW_ATTRIBUTE_DECLARE( pRT_, renderTarget );

		PY_FACTORY_DECLARE()
	private:
		int32  nEffects_;
		int32  nPhases_;
		int32  effect_;
		int32  phase_;

		PyRenderTargetPtr pRT_;
		bool enabled_;
		typedef Debug This;
	};

	typedef SmartPointer<Debug> DebugPtr;
};

#ifdef CODE_INLINE
#include "debug.ipp"
#endif

#endif //#ifndef PP_DEBUG_HPP
