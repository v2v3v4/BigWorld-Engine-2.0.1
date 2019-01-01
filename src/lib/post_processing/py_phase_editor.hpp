/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_PHASE_EDITOR
#define PY_PHASE_EDITOR


#ifdef EDITOR_ENABLED

#include "../../tools/common/material_properties.hpp"


namespace PostProcessing
{

	
class PyPhase;
typedef SmartPointer< PyPhase > PyPhasePtr;


class PyPhaseEditor : public MaterialPropertiesUser, public ReferenceCount
{
public:
	PyPhaseEditor( PyPhase * phase );

	virtual void proxySetCallback();

	virtual std::string textureFeed( const std::string& descName ) const;

	virtual std::string defaultTextureDir() const;

	virtual std::string exposedToScriptName( const std::string& descName ) const;

	virtual StringProxyPtr textureProxy( BaseMaterialProxyPtr proxy,
		GetFnTex getFn, SetFnTex setFn, const std::string& uiName, const std::string& descName ) const;

	virtual BoolProxyPtr boolProxy( BaseMaterialProxyPtr proxy,
		GetFnBool getFn, SetFnBool setFn, const std::string& uiName, const std::string& descName ) const;
	
	virtual IntProxyPtr enumProxy( BaseMaterialProxyPtr proxy,
		GetFnEnum getFn, SetFnEnum setFn, const std::string& uiName, const std::string& descName ) const;

	virtual IntProxyPtr intProxy( BaseMaterialProxyPtr proxy,
		GetFnInt getFn, SetFnInt setFn, RangeFnInt rangeFn, const std::string& uiName, const std::string& descName ) const;

	virtual FloatProxyPtr floatProxy( BaseMaterialProxyPtr proxy,
		GetFnFloat getFn, SetFnFloat setFn, RangeFnFloat rangeFn, const std::string& uiName, const std::string& descName ) const;

	virtual Vector4ProxyPtr vector4Proxy( BaseMaterialProxyPtr proxy,
		GetFnVec4 getFn, SetFnVec4 setFn, const std::string& uiName, const std::string& descName ) const;

	std::string getOutputRenderTarget() const;

	bool setOutputRenderTarget( const std::string & rt );

	bool getClearRenderTarget() const;

	bool setClearRenderTarget( const bool & clear );

	std::string getMaterialFx() const;

	bool setMaterialFx( const std::string & materialFx );
	
	int getFilterQuadType() const;

	bool setFilterQuadType( const int & filterQuadType, bool addBarrier );

private:
	PyPhase * pPhase_;
};


} // namespace PostProcessing


#endif // EDITOR_ENABLED


#endif // PY_PHASE_EDITOR
