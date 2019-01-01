/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_HPP
#define EFFECT_HPP

#include "phase.hpp"


#ifdef EDITOR_ENABLED
class GeneralEditor;
#endif // EDITOR_ENABLED


namespace PostProcessing
{
	/**
	 *	This class allows the post-processing
	 *	manager to work with a list of effects; it also ensures
	 *	that all effects are python objects.
	 */
	class Effect : public PyObjectPlus
	{
		Py_Header( Effect, PyObjectPlus )
	public:
		Effect( PyTypePlus *pType = &s_type_ );
		~Effect();

		virtual void tick( float dTime );
		virtual void draw( class Debug* );

		virtual bool load( DataSectionPtr );
		virtual bool save( DataSectionPtr );

		typedef std::vector<PhasePtr> Phases;
		const Phases& phases() const { return phases_; }

#ifdef EDITOR_ENABLED
		virtual void edEdit( GeneralEditor * editor );
#endif // EDITOR_ENABLED

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );
		PY_FACTORY_DECLARE()
		PY_RW_ATTRIBUTE_DECLARE( phasesHolder_, phases )
		PY_RW_ATTRIBUTE_DECLARE( name_, name )
		PY_RW_ATTRIBUTE_DECLARE( bypass_, bypass )

	private:
		Phases phases_;
		PySTLSequenceHolder<Phases>	phasesHolder_;
		std::string name_;
		Vector4ProviderPtr bypass_;
	};

	typedef SmartPointer<Effect> EffectPtr;
};

PY_SCRIPT_CONVERTERS_DECLARE( PostProcessing::Effect )

#ifdef CODE_INLINE
#include "effect.ipp"
#endif

#endif //#ifndef EFFECT_HPP
