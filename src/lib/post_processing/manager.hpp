/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "cstdmf/init_singleton.hpp"
#include "effect.hpp"
#include "debug.hpp"

namespace PostProcessing
{
	class Manager : public InitSingleton< Manager >
	{
	public:
		void tick( float dTime );
		void draw();
		void debug( DebugPtr d )	{ debug_ = d; }
		DebugPtr debug() const		{ return debug_; }
		typedef std::vector<EffectPtr> Effects;
		const Effects& effects() const { return effects_; }
		PY_MODULE_STATIC_METHOD_DECLARE( py_chain )
		PY_MODULE_STATIC_METHOD_DECLARE( py_debug )
		PY_MODULE_STATIC_METHOD_DECLARE( py_profile )
		PY_MODULE_STATIC_METHOD_DECLARE( py_save )
		PY_MODULE_STATIC_METHOD_DECLARE( py_load )
	private:
		PyObject* getChain() const;
		PyObject* setChain( PyObject * args );
		Effects effects_;
		DebugPtr debug_;
		typedef Manager This;
	};
};

#ifdef CODE_INLINE
#include "manager.ipp"
#endif

#endif //#ifndef MANAGER_HPP