/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_PHASE_EDITOR_PROXIES
#define PY_PHASE_EDITOR_PROXIES


#ifdef EDITOR_ENABLED

#include "gizmo/pch.hpp"
#include "gizmo/general_properties.hpp"
#include "resmgr/string_provider.hpp"
#include "../../tools/common/material_proxies.hpp"
#include "../../tools/common/material_properties.hpp"


class BaseMaterialProxy;
typedef SmartPointer< BaseMaterialProxy > BaseMaterialProxyPtr;


namespace PostProcessing
{

class PyPhase;
typedef SmartPointer< PyPhase > PyPhasePtr;


class PyPhaseEditor;
typedef SmartPointer< PyPhaseEditor > PyPhaseEditorPtr;


/**
 *	This class implements a texture filename editor property proxy
 */
class PyPhaseTextureProxy : public UndoableDataProxy< StringProxy >
{
public:
	PyPhaseTextureProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnTex getFn, MaterialPropertiesUser::SetFnTex setFn,
		const std::string& descName, PyPhasePtr pPhase, Moo::EffectMaterialPtr pEffectMaterial );
	
	virtual std::string EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( std::string v );

	virtual bool EDCALL setPermanent( std::string v );

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnTex getFn_;
	MaterialPropertiesUser::SetFnTex setFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
	Moo::EffectMaterialPtr pMaterial_;
};


/**
 *	This class implements a bool editor property proxy
 */
class PyPhaseBoolProxy : public UndoableDataProxy< BoolProxy >
{
public:
	PyPhaseBoolProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnBool getFn, MaterialPropertiesUser::SetFnBool setFn,
		const std::string& descName, PyPhasePtr pPhase );
	
	virtual bool EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( bool v );

	virtual bool EDCALL setPermanent( bool v );

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnBool getFn_;
	MaterialPropertiesUser::SetFnBool setFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
};


/**
 *	This class implements an integer editor property proxy
 */
class PyPhaseIntProxy : public UndoableDataProxy< IntProxy >
{
public:
	PyPhaseIntProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnInt getFn, MaterialPropertiesUser::SetFnInt setFn,
		MaterialPropertiesUser::RangeFnInt rangeFn, const std::string& descName, PyPhasePtr pPhase );
	
	virtual int32 EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( int32 v );

	virtual bool EDCALL setPermanent( int32 v );

	bool getRange( int32& min, int32& max ) const;

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnInt getFn_;
	MaterialPropertiesUser::SetFnInt setFn_;
	MaterialPropertiesUser::RangeFnInt rangeFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
};


/**
 *	This class implements an enum editor property proxy
 */
class PyPhaseEnumProxy : public UndoableDataProxy< IntProxy >
{
public:
	PyPhaseEnumProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnEnum getFn, MaterialPropertiesUser::SetFnEnum setFn,
		const std::string& descName, PyPhasePtr pPhase );
	
	virtual int32 EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( int32 v );

	virtual bool EDCALL setPermanent( int32 v );

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnEnum getFn_;
	MaterialPropertiesUser::SetFnEnum setFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
};


/**
 *	This class implements a floating-point editor property proxy
 */
class PyPhaseFloatProxy : public UndoableDataProxy< FloatProxy >
{
public:
	PyPhaseFloatProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnFloat getFn, MaterialPropertiesUser::SetFnFloat setFn,
		MaterialPropertiesUser::RangeFnFloat rangeFn, const std::string& descName, PyPhasePtr pPhase );
	
	virtual float EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( float v );

	virtual bool EDCALL setPermanent( float v );

	bool getRange( float& min, float& max, int& digits ) const;

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnFloat getFn_;
	MaterialPropertiesUser::SetFnFloat setFn_;
	MaterialPropertiesUser::RangeFnFloat rangeFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
};


/**
 *	This class implements a Vector4 editor property proxy
 */
class PyPhaseVector4Proxy : public UndoableDataProxy< Vector4Proxy >
{
public:
	PyPhaseVector4Proxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnVec4 getFn, MaterialPropertiesUser::SetFnVec4 setFn,
		const std::string& descName, PyPhasePtr pPhase );
	
	virtual Vector4 EDCALL get() const;
	
	virtual std::string EDCALL opName() { return LocaliseUTF8(L"POST_PROCESSING/EDIT_PHASE", descName_.c_str() ); }

	virtual void EDCALL setTransient( Vector4 v );

	virtual bool EDCALL setPermanent( Vector4 v );

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnVec4 getFn_;
	MaterialPropertiesUser::SetFnVec4 setFn_;
	std::string descName_;
	PyPhasePtr pPhase_;
};


/**
 *	This helper class is a proxy that uses simple a getter and a setter to
 *	edit a property, but does not insert undo operations.
 */
class PyPhaseFilterQuadTypeProxy : public IntProxy
{
public:
	PyPhaseFilterQuadTypeProxy( PyPhaseEditorPtr pItem );

	virtual IntProxy::Data EDCALL get() const;

	virtual void EDCALL set( IntProxy::Data v, bool transient, bool addBarrier = true );

private:
	PyPhaseEditorPtr pItem_;
};


} // namespace PostProcessing


#endif // EDITOR_ENABLED


#endif // PY_PHASE_EDITOR_PROXIES
