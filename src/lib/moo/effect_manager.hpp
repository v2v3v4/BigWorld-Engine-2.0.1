/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __EFFECT_MANAGER_HPP__
#define __EFFECT_MANAGER_HPP__

#include <deque>

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/singleton.hpp"
#include "math/vector4.hpp"

#include "moo_dx.hpp"
#include "forward_declarations.hpp"
#include "device_callback.hpp"
#include "effect_state_manager.hpp"
#include "effect_includes.hpp"

namespace Moo
{
/**
*	This class loads and manages managed effects.
*/
class EffectManager : public DeviceCallback, public Singleton< EffectManager >
{
public:

	/**
	*	Interface for objects desiring callbacks at selection of PS version cap.
	*/
	class IListener
	{
	public:
		virtual void onSelectPSVersionCap(int psVerCap) = 0;
	};

	typedef std::map< std::string, std::string >	StringStringMap;	
	typedef std::vector< std::string >				IncludePaths;

	EffectManager();
	~EffectManager();

	ManagedEffectPtr	get( const std::string& resourceID );
// TODO make private
	void				deleteEffect( ManagedEffect* pEffect );

	void compileOnly( const std::string& resourceID );
	bool needRecompile( const std::string& resourceID );
	bool hashResource( const std::string& name, MD5::Digest& result );
	std::string resolveInclude( const std::string& name );
	bool finishEffectInits();

	void createUnmanagedObjects();
	void deleteUnmanagedObjects();

	const IncludePaths& includePaths() const { return includePaths_; }
	IncludePaths& includePaths() { return includePaths_; }

	void addIncludePaths( const std::string& pathString );

	bool checkModified( const std::string& objectName, 
						const std::string& resName, 
						MD5::Digest* resDigest=NULL );

	bool registerGraphicsSettings();

	void setMacroDefinition(
		const std::string & key, 
		const std::string & value);

	const StringStringMap & macros();

	const std::string & fxoInfix() const;
	void fxoInfix(const std::string & infix);

	int PSVersionCap() const;
	void PSVersionCap( int psVersion );

	StateManager * pStateManager();

	EffectIncludes * pEffectIncludes();

	void addListener(IListener * listener);
	void delListener(IListener * listener);

private:

	typedef GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;
	typedef std::map< std::string, ManagedEffect* > Effects;
	typedef std::list< ManagedEffectPtr >			EffectList;
	typedef std::vector<IListener *>				ListenerVector;

	ManagedEffect*		find( const std::string& resourceID );
	void				add( ManagedEffect* pEffect, 
								const std::string& resourceID );

	void			setPSVerCapOption(int);

	// All loaded effects
	// These are *not reference counted* in here, so that they get unloaded 
	// manually via the ManagedEffect destructor when all *external* references
	// are gone, e.g. the last referencing EffectMaterial was destroyed.
	// If we kept a reference here, ManagedEffects would never unload.
	Effects							effects_;
	
	// Effects awaiting initialisation *are reference counted*, so they don't get
	// deleted while awaiting initialisation.
	EffectList						effectInitQueue_;
	SimpleMutex						effectInitQueueMutex_;

	typedef std::map<std::string, MD5::Digest> StrDigestMap;
	StrDigestMap hashCache_;

	SmartPointer< StateManager >	pStateManager_;
	SmartPointer< EffectIncludes >	pEffectIncludes_;
	IncludePaths					includePaths_;
	SimpleMutex						effectsLock_;
	StringStringMap					macros_;	
	std::string						fxoInfix_;

	ListenerVector					listeners_;
	SimpleMutex						listenersLock_;
	GraphicsSettingPtr				psVerCapSettings_;
	SimpleMutex						compileMutex_;
};

}
#endif 
