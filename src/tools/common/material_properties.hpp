/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATERIAL_PROPERTIES_HPP
#define MATERIAL_PROPERTIES_HPP

#define EDCALL __stdcall
#define WORLDEDITORDLL_API
#include "gizmo/general_properties.hpp"
#include "moo/managed_effect.hpp"
#include "moo/texture_manager.hpp"
#include "cstdmf/bw_functor.hpp"


class BaseMaterialProxy;
typedef SmartPointer< BaseMaterialProxy > BaseMaterialProxyPtr;


/**
 *	Abstract interface to be implemented by the app wanting to edit material
 *	properties.
 */	
class MaterialPropertiesUser
{
public:
	// textureProxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< std::string > > GetFnTex;
	typedef SmartPointer< BWBaseFunctor1< std::string > > SetFnTex;

	// boolProxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< bool > > GetFnBool;
	typedef SmartPointer< BWBaseFunctor1< bool > > SetFnBool;

	// enumProxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< uint32 > > GetFnEnum;
	typedef SmartPointer< BWBaseFunctor1< uint32 > > SetFnEnum;

	// intProxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< int > > GetFnInt;
	typedef SmartPointer< BWBaseFunctor1< int > > SetFnInt;
	typedef SmartPointer< BWBaseFunctor2R< int&, int&, bool > > RangeFnInt; // min, max

	// floatProxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< float > > GetFnFloat;
	typedef SmartPointer< BWBaseFunctor1< float > > SetFnFloat;
	typedef SmartPointer< BWBaseFunctor3R< float&, float&, int&, bool > > RangeFnFloat; // min, max, digits

	// vector4Proxy typedefs
	typedef SmartPointer< BWBaseFunctor0R< Vector4 > > GetFnVec4;
	typedef SmartPointer< BWBaseFunctor1< Vector4 > > SetFnVec4;

	// Methods
	virtual void proxySetCallback() = 0;

	virtual std::string textureFeed( const std::string& descName ) const = 0;

	virtual std::string defaultTextureDir() const = 0;

	virtual std::string exposedToScriptName( const std::string& descName ) const = 0;	

	virtual StringProxyPtr textureProxy( BaseMaterialProxyPtr proxy,
		GetFnTex getFn, SetFnTex setFn, const std::string& uiName, const std::string& descName ) const = 0;

	virtual BoolProxyPtr boolProxy( BaseMaterialProxyPtr proxy,
		GetFnBool getFn, SetFnBool setFn, const std::string& uiName, const std::string& descName ) const = 0;
	
	virtual IntProxyPtr enumProxy( BaseMaterialProxyPtr proxy,
		GetFnEnum getFn, SetFnEnum setFn, const std::string& uiName, const std::string& descName ) const = 0;

	virtual IntProxyPtr intProxy( BaseMaterialProxyPtr proxy,
		GetFnInt getFn, SetFnInt setFn, RangeFnInt rangeFn, const std::string& uiName, const std::string& descName ) const = 0;

	virtual FloatProxyPtr floatProxy( BaseMaterialProxyPtr proxy,
		GetFnFloat getFn, SetFnFloat setFn, RangeFnFloat rangeFn, const std::string& uiName, const std::string& descName ) const = 0;

	virtual Vector4ProxyPtr vector4Proxy( BaseMaterialProxyPtr proxy,
		GetFnVec4 getFn, SetFnVec4 setFn, const std::string& uiName, const std::string& descName ) const = 0;
};


/**
 *	This class creates all the properties and proxies needed to be able to edit
 *	the material's properties.
 */
class MaterialProperties
{
public:

	/**
	 *	By calling runtimeInitMaterialProperties, the section processor
	 *	map for Moo::EffectProperties is overridden, and replaced with
	 *	EditorEffectPropreties.  This allows both viewing of tweakables
	 *	in editors, and the saving of properties via the above interface.
	 *
	 *	If runtimeInitMaterialProperties is not called, then all materials
	 *	will use Moo::EffectProperties instead, and the dynamic_cast calls
	 *	in the editor code will fail.
	 */
	static bool runtimeInitMaterialProperties();


	static void beginProperties();

	static bool addMaterialProperties( Moo::EffectMaterialPtr material, GeneralEditor * editor,
				bool canExposeToScript, bool canUseTextureFeeds, MaterialPropertiesUser * callback );

	static void endProperties();

};


#endif