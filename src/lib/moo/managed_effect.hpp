/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MANAGED_EFFECT_HPP
#define MANAGED_EFFECT_HPP

#include "moo_dx.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/singleton.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/guard.hpp"
#include "math/vector4.hpp"
#include "graphics_settings.hpp"
#include "effect_constant_value.hpp"
#include "com_object_wrap.hpp"
#include "base_texture.hpp"
#include "graphics_settings.hpp"
#include "effect_manager.hpp"


class BinaryBlock;
typedef SmartPointer<BinaryBlock> BinaryPtr;

namespace Moo
{

// Forward declarations
class EffectTechniqueSetting;
class EffectMacroSetting;
class StateManager;
class EffectIncludes;


/**
 * This class is the base class for the effect properties.
 */
class EffectProperty : public SafeReferenceCount
{
public:
	virtual EffectProperty* clone() const = 0;
	virtual bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty ) = 0;
	virtual bool be( const bool & b ) { return false; }
	virtual bool be( const float & f ) { return false; }
	virtual bool be( const int & i ) { return false; }
	virtual bool be( const Matrix & m ) { return false; }
	virtual bool be( const Vector4 & v ) { return false; }
	virtual bool be( const BaseTexturePtr pTex ) { return false; }
	virtual bool be( const std::string & s ) { return false; }
	virtual bool getBool( bool & b ) const { return false; };
	virtual bool getInt( int & i ) const { return false; };
	virtual bool getFloat( float & f ) const { return false; };
	virtual bool getVector( Vector4 & v ) const { return false; };
	virtual bool getMatrix( Matrix & m ) const { return false; };
	virtual bool getResourceID( std::string & s ) const { return false; };
	virtual void asVector4( Vector4 & v ) const { getVector( v ); } // never fails
	virtual void setParent( const EffectProperty* pParent ) {};
	virtual void save( DataSectionPtr pDS ) = 0;
private:
};

typedef SmartPointer<EffectProperty> EffectPropertyPtr;
typedef ConstSmartPointer<EffectProperty> ConstEffectPropertyPtr;
typedef std::map< std::string, EffectPropertyPtr> EffectPropertyMappings;

/**
*	A structure representing a D3D effect technique.
*/
struct TechniqueInfo
{
	TechniqueInfo();

	D3DXHANDLE			handle_;
	bool				supported_;
	
	std::string			name_;
	std::string			settingLabel_;
	std::string			settingDesc_;
	VisualChannelPtr	channel_;

	int					psVersion_;
	bool				skinned_;
	bool				bumpMapped_;
	bool				dualUV_;
};

/**
*	Used internally to register graphics settings created by annotating effect
*	files and their techniques. Effect files sometimes provide alternative 
*	techniques to render the same material. Normally, the techniques to be 
*	used is selected based on hardware capabilities. By annotating the effect
*	files with specific variables, you can allow the user to select between
*	the supported techniques via the graphics settings facility. 
*
*	To register an effect file as a graphics setting, you need (1) to add
*	a top level parameter named "graphicsSetting", with a string annotation 
*	named "label". This string will be used as the setting label property:
*
*	int graphicsSetting
*	<
*		string label = "PARALLAX_MAPPING";
*	>;
*	
*	And (2) have, for each selectable technique, a string annotation named
*	"label". This string will be used as the option entry label property:
*
*	technique lighting_shader1
*	< 
*		string label = "SHADER_MODEL_1";
*	>
*	{ (...) }
*
*	technique software_fallback
*	< 
*		string label = "SHADER_MODEL_0";
*	>
*	{ (...) }
*/
class EffectTechniqueSetting : public GraphicsSetting
{
public:
	typedef GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;

	/**
	*	Interface for objects desiring callbacks on technique selection.
	*/
	class IListener
	{
	public:
		virtual void onSelectTechnique(
			ManagedEffect * effect, D3DXHANDLE hTec) = 0;
	};

	EffectTechniqueSetting(ManagedEffect * owner,
		const std::string & label ,
		const std::string & desc);

	void addTechnique(const TechniqueInfo &info);	
	virtual void onOptionSelected(int optionIndex);
	void setPSCapOption(int psVersionCap);

	D3DXHANDLE activeTechniqueHandle() const;

	int addListener(IListener * listener);
	void delListener(int listenerId);

private:	
	typedef std::pair<D3DXHANDLE, int>        D3DXHandlesPSVerPair;
	typedef std::vector<D3DXHandlesPSVerPair> D3DXHandlesPSVerVector;
	typedef std::map<int, IListener *>        ListenersMap;

	ManagedEffect        * owner_;
	D3DXHandlesPSVerVector techniques_;
	ListenersMap           listeners_;
	SimpleMutex	           listenersLock_;

	static int listenersId;
};

/**
 *	This class manages an individual d3d effect and its standard properties.
 */
class ManagedEffect : 
	public	SafeReferenceCount,
	public	EffectTechniqueSetting::IListener,
	public	EffectManager::IListener
{
public:
	// Types
	typedef std::pair<D3DXHANDLE, EffectConstantValuePtr*>	MappedConstant;
	typedef std::vector<MappedConstant>						MappedConstants;
	typedef std::vector<RecordedEffectConstantPtr>		RecordedEffectConstants;

	// Functions
	ManagedEffect();
	~ManagedEffect();

	bool load( const std::string& resourceName );	
	bool registerGraphicsSettings(const std::string & effectResource);

	ID3DXEffect* pEffect() { return pEffect_.pComObject(); }
	const std::string& resourceID() { return resourceID_; }

	EffectPropertyMappings& defaultProperties() { return defaultProperties_; };

	void setAutoConstants();
	void recordAutoConstants( RecordedEffectConstants& recordedList );

	BinaryPtr compile( const std::string& resName, D3DXMACRO * preProcessDefinition=NULL, 
						bool force=false, std::string* result=NULL );
	bool cache(BinaryPtr bin, D3DXMACRO * preProcessDefinition=NULL );

	bool validateAllTechniques();
	bool validateShaderVersion( ID3DXEffect *	d3dxeffect, 
								D3DXHANDLE		hTechnique) const;
	bool getFirstValidTechnique(D3DXHANDLE &	hT);

	EffectTechniqueSetting * graphicsSettingEntry();

	int			maxPSVersion(D3DXHANDLE hTechnique) const;
	static int	maxPSVersion(ID3DXEffect * d3dxeffect, D3DXHANDLE hTechnique);
	
	INLINE D3DXHANDLE	getCurrentTechnique() const;
	bool				setCurrentTechnique( D3DXHANDLE hTec, bool setExplicit );
	const std::string	currentTechniqueName();

	VisualChannelPtr ManagedEffect::getChannel( D3DXHANDLE techniqueOverride = NULL );

	bool finishInit();

	bool readyToUse() const { return initComplete_ && validated_; }
	
	bool skinned( D3DXHANDLE techniqueOverride = NULL );
	bool bumpMapped( D3DXHANDLE techniqueOverride = NULL );
	bool dualUV( D3DXHANDLE techniqueOverride = NULL );

	typedef std::vector< TechniqueInfo >	TechniqueInfoCache;
	
	const TechniqueInfoCache& techniques() { return techniques_; }

private:
	typedef SmartPointer<EffectTechniqueSetting>	EffectTechniqueSettingPtr;

	// Functions
	// Override default constructors so we don't accidentally copy this class.
	ManagedEffect( const ManagedEffect& );
	ManagedEffect& operator=( const ManagedEffect& );

	D3DXMACRO * generatePreprocessors();

	INLINE TechniqueInfo*	findTechniqueInfo( D3DXHANDLE handle );

	// overrides for  EffectTechniqueSetting::listener
	virtual void onSelectTechnique(ManagedEffect * effect, D3DXHANDLE hTec);
	virtual void onSelectPSVersionCap(int psVerCap);

	// Member Data
	ComObjectWrap<ID3DXEffect>	pEffect_;
	D3DXHANDLE					hCurrentTechnique_;
	bool						techniqueExplicitlySet_;
	bool						initComplete_;

	int							settingsListenerId_;

	EffectPropertyMappings		defaultProperties_;
	MappedConstants				mappedConstants_;
	std::string					resourceID_;
	TechniqueInfoCache			techniques_;

	std::string					settingName_;
	std::string					settingDesc_;
	EffectTechniqueSettingPtr   settingEntry_;
	bool						settingsAdded_;

	// This effect has all techniques validated.
	bool						validated_;

	// The index of the last valid technique 
	uint32						firstValidTechniqueIndex_;
};


/**
 * @internal
 *	Used internally to register settings that work by redefining effect files 
 *	compile time macros. This settings require the client application to be 
 *	restarted form them to come into effect.
 *
 *	Note: EffectMacroSetting instances need to be static to make sure it gets 
 *	instantiated before the EffectManager is initialised). For an example on
 *	how to use EffectMacroSetting, see romp/sky_light_map.cpp.
 */
class EffectMacroSetting : public GraphicsSetting
{
	friend class EffectManager;

public:
	typedef SmartPointer<EffectMacroSetting> EffectMacroSettingPtr;
	typedef std::vector<EffectMacroSettingPtr> MacroSettingVector;
	typedef std::vector<std::string> StringVector;
	typedef void(*SetupFunc)(EffectMacroSetting &);

	EffectMacroSetting(
		const std::string & label, 
		const std::string & desc, 
		const std::string & Macro, 
		SetupFunc setupFunc);

	void addOption(
		const std::string & label, 
		const std::string & desc, 
		             bool   isSupported, 
		const std::string & value);

	virtual void onOptionSelected(int optionIndex);

	size_t numMacroOptions() const { return values_.size(); }
	std::string macroName() const { return macro_; }
	void macroValues(StringVector& ret) const { ret = values_; }

	static void setupSettings();
	static void getMacroSettings(MacroSettingVector& settings);
	static void updateManagerInfix();
	static void finiSettings();

private:	
	static void add(EffectMacroSettingPtr setting);

	std::string macro_;
	SetupFunc   setupFunc_;

	StringVector values_;
};


/**
 *	An interface for objects that require setup of properties
 *	including creation and validity checking. Generally used for
 *	GUI property fields.
 */
class EffectPropertyFunctor : public ReferenceCount
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection ) = 0;
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect ) = 0;
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc ) = 0;
private:
};
typedef SmartPointer<EffectPropertyFunctor> EffectPropertyFunctorPtr;

typedef std::map< std::string, EffectPropertyFunctorPtr > PropertyFunctors;
extern PropertyFunctors g_effectPropertyProcessors;

}

#ifdef CODE_INLINE
#include "managed_effect.ipp"
#endif

#endif // MANAGED_EFFECT_HPP
