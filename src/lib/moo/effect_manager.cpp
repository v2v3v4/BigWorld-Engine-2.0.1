/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "effect_manager.hpp"

#include "resmgr/auto_config.hpp"
#include "effect_state_manager.hpp"
#include "managed_effect.hpp"

// Singleton instance implementation
BW_SINGLETON_STORAGE( Moo::EffectManager );

namespace Moo
{

namespace 
{ 
	// Named constants
	AutoConfigString s_shaderIncludePath( "system/shaderIncludePaths" );
	const int c_MaxPixelShaderVersion = 3;
}

// -----------------------------------------------------------------------------
// Section: EffectManager
// -----------------------------------------------------------------------------


// Constructor
EffectManager::EffectManager():
	effects_(),
	includePaths_(),
	effectsLock_(),
	macros_(),
	fxoInfix_(""),
	listeners_()
{
	BW_GUARD;
	addIncludePaths( s_shaderIncludePath );

	// Initialise the macro settings for the effect files
	// These settings are used to select different graphics
	// options when compiling effects
	EffectMacroSetting::setupSettings();
	EffectMacroSetting::updateManagerInfix();

	// Set up the max pixelshader version graphics option
	int maxPSVerSupported = (Moo::rc().psVersion() & 0xff00) >> 8;
	psVerCapSettings_ = 
		Moo::makeCallbackGraphicsSetting(
		"SHADER_VERSION_CAP", "Shader Version Cap", *this, 
		&EffectManager::setPSVerCapOption	, 
		c_MaxPixelShaderVersion-maxPSVerSupported,
		false, false);

	for (int i=0; i<=c_MaxPixelShaderVersion; ++i)
	{
		const int shaderVersion = (c_MaxPixelShaderVersion-i);
		std::stringstream verStream;
		verStream << "SHADER_MODEL_" << shaderVersion;
		std::stringstream descStream;
		if ( shaderVersion == 0 )
		{
			descStream << "Fixed Function";
		}
		else
		{
			descStream << "Shader Model " << shaderVersion;
		}

		bool supported = shaderVersion <= maxPSVerSupported;
		supported = supported && !(shaderVersion == 0 && maxPSVerSupported == 1);
		psVerCapSettings_->addOption(verStream.str(), descStream.str(), supported);
	}
	Moo::GraphicsSetting::add(psVerCapSettings_);
}

// Destructor
EffectManager::~EffectManager()
{
	BW_GUARD;
	// Remove references to all effects waiting for initialisation.
	effectInitQueue_.clear();
 
	// Count remaining references, should be exactly 0 or there are reference
	// leaks
	Effects::iterator	it	= effects_.begin();
	uint32				i	= 0;
	bool				msg	= false;

	while (it != effects_.end())
	{
		if ( !msg )
		{
			WARNING_MSG( "EffectManager::~EffectManager - not empty on destruct.\n"
				"This could indicate a possible reference leak or destruction order"
				" issue.\n" );
			msg = true;
		}

		WARNING_MSG( "%d - Effect %s not unloaded in time. \n", 
					++i, it->first.c_str() );

		// next managed effect
		it++;
	}

	EffectMacroSetting::finiSettings();
}

/**
*	This method adds the includepaths from pathString to the include search path.
*	@param pathString the paths to add, paths can be separated by ;
*/
void EffectManager::addIncludePaths( const std::string& pathString )
{
	BW_GUARD;
	std::string rest = pathString;
	while (rest.length())
	{
		std::string::size_type index = rest.find_first_not_of( ";" );
		if (index != std::string::npos)
		{
			rest = rest.substr( index );
		}

		index = rest.find_first_of( ";" );
		std::string token;
		if (index != std::string::npos)
		{
			token = rest.substr( 0, index );
			rest = rest.substr( index + 1 );
		}
		else
		{
			token = rest;
			rest = "";
		}
		index = token.find_first_not_of(" ");
		std::string::size_type end = token.find_last_not_of(" ");
		if (index != std::string::npos && end != std::string::npos)
		{
			token = BWResource::formatPath( token.substr( index, end - index + 1 ) );
			DEBUG_MSG( "EffectManager::addIncludePaths - adding include path %s\n", token.c_str() );
			includePaths_.push_back( token );
		}
	}
}

/*
*	Remove the effect from the effect manager.
*/
void EffectManager::deleteEffect( ManagedEffect* pEffect )
{
	BW_GUARD;
	SimpleMutexHolder smh( effectsLock_ );

	this->delListener( pEffect );

	Effects::iterator it = effects_.begin();
	Effects::iterator end = effects_.end();
	while (it != end)
	{
		if (it->second == pEffect)
		{
			effects_.erase( it );
			return;
		}
		else
			it++;
	}
}

/**
*	Sets the current pixel shader version cap. This will force all 
*	MaterialEffects that use GetNextValidTechnique to limit the valid 
*	techniques to those within the current pixel shader version cap. 
*	Effect files registered as graphics settings will also be capped,
*	but they can be individually reset later. Implicitly called when
*	the user changes the PS Version Cap's active option.
*
*	@param activeOption		the new active option index.
*/
// TODO: this should really be in EffectMaterial.
void EffectManager::setPSVerCapOption(int activeOption)
{
	BW_GUARD;
	Effects::const_iterator effIt = this->effects_.begin();
	Effects::const_iterator effEnd = this->effects_.end();

	while (effIt != effEnd)
	{
		EffectTechniqueSetting * setting = effIt->second->graphicsSettingEntry();

		if ( setting == NULL && 
			effIt->second->registerGraphicsSettings( effIt->first ) )
		{
			// on creation success.
			setting = effIt->second->graphicsSettingEntry();
		}

		// If we did create a setting, use it 
		if ( setting )
		{
			setting->setPSCapOption(this->PSVersionCap());
		}
		effIt++;
	}

	// update all listeners (usually EffectMaterial instances)
	this->listenersLock_.grab();
	ListenerVector::const_iterator listIt  = this->listeners_.begin();
	ListenerVector::const_iterator listEnd = this->listeners_.end();
	while (listIt != listEnd)
	{
		(*listIt)->onSelectPSVersionCap(c_MaxPixelShaderVersion-activeOption);
		++listIt;
	}
	this->listenersLock_.give();
}

/**
*	Get the effect from the manager, create it if it's not there.
*
*	@param resourceID the name of the effect to load
*	@return smart pointer to the loaded effect
*/
ManagedEffectPtr EffectManager::get( const std::string& resourceID )
{
	BW_GUARD;

	// See if we have a copy of the effect already
	ManagedEffect* pEffect = this->find( resourceID );
	if (!pEffect)
	{
		// Prevent simultaneous shader compilation in multiple threads.
		//
		// EffectIncludes is not thread safe despite the fact we allow 
		// compilation in different threads - for example when pre-loading (main
		// thread) or streaming (loading thread).
		//
		// Compiling in both threads simultaneously probably means the calling 
		// code is doing something wrong. In this case we stall the thread and 
		// wait for previous compilation to finish.
		compileMutex_.grab();

		// Create the effect if it could not be found.
		pEffect = new ManagedEffect;
		if (pEffect->load( resourceID ))
		{
			this->add( pEffect, resourceID );

			// We still need to init the techniques, this can only be done in 
			// the main thread, so we will wait until the effect gets used.
 			bool done = false;
			if ( g_renderThread )
 			{
 				done = pEffect->finishInit();
 			}
 			
			if (!done)
			{
				SimpleMutexHolder smh( effectInitQueueMutex_ );
				effectInitQueue_.push_back( pEffect );
			}
		}
		else
		{
			ERROR_MSG( "EffectManager::get - unable to load %s\n", resourceID.c_str() );
			delete pEffect;
			pEffect = NULL;
		}

		// unblock compilation in other threads.
		compileMutex_.give();
	}
	return pEffect;
}

/**
*	As the name suggests, this only compiles a shader and discards
*	the results.
*
*	@param resourceID the name of the effect to compile.
*/
void EffectManager::compileOnly( const std::string& resourceID )
{
	BW_GUARD;

	// See if we have a copy of the effect already
	ManagedEffectPtr pEffect = this->find( resourceID );
	if (!pEffect)
	{
		// Create the effect if it could not be found.
		pEffect = new ManagedEffect;
		pEffect->compile( resourceID );

		pEffect = NULL;		
	}	
}

/**
*	The method checks if the passed effect needs to be recompiled before
*	it can be used.
*
*	NOTE:  The effect if not compiled by this method, only a check is
*	is performed to see if it would need recompilation.
*
*	@param resourceID the name of the effect to check.
*/
bool EffectManager::needRecompile( const std::string& resourceID )
{
	BW_GUARD;

	SimpleMutexHolder smh( compileMutex_ );

	// See if we have a copy of the effect already
	std::string fxoInfix   = this->fxoInfix();
	std::string fileName   = BWResource::removeExtension(resourceID);
	std::string objectName = !fxoInfix.empty() 
		? fileName + "." + fxoInfix + ".fxo"
		: fileName + ".fxo";

	this->pEffectIncludes()->resetDependencies();

	std::string effectPath = BWResource::getFilePath( resourceID );
	this->pEffectIncludes()->currentPath( effectPath );

	// Check if the file or any of its dependents have been modified.
	return this->checkModified( objectName, resourceID );
}


/**
*	Find an effect in the effect manager
*	@param resourceID the name of the effect
*	@return pointer to the effect
*/
ManagedEffect* EffectManager::find( const std::string& resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( effectsLock_ );
	ManagedEffect* pEffect = NULL;
	Effects::iterator it = effects_.find( resourceID );
	if (it != effects_.end())
		pEffect = it->second;

	return pEffect;
}

/**
*	Add an effect to the effect manager
*	@param pEffect the effect
*	@param resourceID the name of the effect
*/
void EffectManager::add( ManagedEffect* pEffect, const std::string& resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( effectsLock_ );
	effects_.insert( std::make_pair( resourceID, pEffect ) );
}

void EffectManager::createUnmanagedObjects()
{
	BW_GUARD;
	SimpleMutexHolder smh( effectsLock_ );

	Effects::iterator it = effects_.begin();
	Effects::iterator end = effects_.end();
	while (it != end)
	{
		if (it->second)
		{
			it->second->pEffect()->OnResetDevice();
		}
		it++;
	}
}

void EffectManager::deleteUnmanagedObjects()
{
	BW_GUARD;
	SimpleMutexHolder smh( effectsLock_ );

	Effects::iterator it = effects_.begin();
	Effects::iterator end = effects_.end();
	while (it != end)
	{
		if (it->second)
		{
			it->second->pEffect()->OnLostDevice();
		}
		it++;
	}
}

/**
*	Sets effect file compile time macro definition.
*
*	@param	macro	macro name.
*	@param	value	macro value.
*/
void EffectManager::setMacroDefinition(
									   const std::string & macro, 
									   const std::string & value)
{
	this->macros_[macro] = value;
}

/**
*	Returns effect file compile time macro definitions.
*/
const EffectManager::StringStringMap & EffectManager::macros()
{
	return this->macros_;
}

/**
*	Returns fxo file infix.
*/
const std::string & EffectManager::fxoInfix() const
{
	return this->fxoInfix_;
}

/**
*	Sets fxo file infix. This infix is used when compiling the fx files to 
*	differentiate different compilation settings (most commonly different
*	macro definition). It's currently used by the EffectMacroSetting.
*/
void EffectManager::fxoInfix(const std::string & infix)
{
	this->fxoInfix_ = infix;
}

/**
*	Retrieves the current pixel shader version cap (major version number).
*/
int EffectManager::PSVersionCap() const
{
	return (c_MaxPixelShaderVersion - this->psVerCapSettings_->activeOption());
}

/**
*	Set the current pixel shader version cap (major version number).
*/
void EffectManager::PSVersionCap( int psVersion )
{
	this->psVerCapSettings_->selectOption( c_MaxPixelShaderVersion - psVersion );
}

/**
*	This method returns the singleton's state manager.
*
*	@return		Pointer to the StateManager.
*/
StateManager * EffectManager::pStateManager()
{
	if (!pStateManager_)
	{
		pStateManager_ = new StateManager();
	}
	return pStateManager_.get();
}


/**
*	This method returns the singleton's effect includes class.
*
*	@return		Pointer to the EffectInclude.
*/
EffectIncludes * EffectManager::pEffectIncludes()
{
	if (!pEffectIncludes_)
	{
		pEffectIncludes_ = new EffectIncludes();
	}
	return pEffectIncludes_.get();
}


/**
*	Registers an EffectManager listener instance.
*/
void EffectManager::addListener(IListener * listener)
{
	BW_GUARD;
	this->listenersLock_.grab();
	ListenerVector::iterator it = std::find(
		this->listeners_.begin(), 
		this->listeners_.end(), 
		listener);
	MF_ASSERT_DEV( it == this->listeners_.end());

	if( it == this->listeners_.end() )
		this->listeners_.push_back(listener);
	this->listenersLock_.give();
}

/**
*	Unregisters an EffectManager listener instance.
*/
void EffectManager::delListener(IListener * listener)
{
	BW_GUARD;
	this->listenersLock_.grab();
	ListenerVector::iterator listIt = std::find(
		this->listeners_.begin(), this->listeners_.end(), listener);

	if( listIt != this->listeners_.end() )
		this->listeners_.erase(listIt);

	this->listenersLock_.give();
}

bool EffectManager::hashResource( const std::string& name, MD5::Digest& result )
{
	BW_GUARD;

	StrDigestMap::iterator it = hashCache_.find( name );
	if (it != hashCache_.end())
	{
		result = it->second;
		return true;
	}


	DataSectionPtr ds = BWResource::openSection( name );
	if (!ds)
	{
		return false;
	}

	BinaryPtr data = ds->asBinary();
	if (!data)
	{
		return false;
	}

	//double start = (double)timestamp();
	MD5 md5;
	md5.append( data->data(), (int)data->len() );
	md5.getDigest( result );

	hashCache_[ name ] = result;
	return true;
}

std::string EffectManager::resolveInclude( const std::string& name )
{
	BW_GUARD;
	if (BWResource::fileExists( name ))
	{
		return name;
	}

	std::string pathname = this->pEffectIncludes()->currentPath() + name;
	bool found = BWResource::fileExists( pathname );

	// Not found in the current path, go through the rest.
	if (!found)
	{
		const EffectManager::IncludePaths& paths = this->includePaths();
		EffectManager::IncludePaths::const_iterator it  = paths.begin();
		EffectManager::IncludePaths::const_iterator end = paths.end();
		while ( it != end )
		{
			pathname = *(it++) + name;
			if (BWResource::fileExists( pathname ))
			{
				found = true;
				break;
			}
		}
	}

	return found ? pathname : "";
}

/**
*	Runs finishInit on all un-initialised effect materials.
*	This function should only be called from the render thread.
*/
bool EffectManager::finishEffectInits()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( g_renderThread == true )
	{
		return false;
	}

	SimpleMutexHolder		mutex( EffectManager::effectInitQueueMutex_ );
	EffectList::iterator	eit = effectInitQueue_.begin();
	bool					success= true;

	while( eit != effectInitQueue_.end()  )
	{
		if ( (*eit)->finishInit() )
		{
			eit = effectInitQueue_.erase( eit );
		}
		else
		{
			//will try again next frame, this is ok.
			success = false;
			++eit;
		}
	}

	return success;
}

/**
*	Runs through all loaded effect files, registering 
*	their graphics settings when relevant. 
*
*	@return	False if application was quit during processing.
*/
bool EffectManager::registerGraphicsSettings()
{
	BW_GUARD;
	bool result = true;
	Effects::iterator it = effects_.begin();
	Effects::iterator end = effects_.end();
	while (it != end)
	{
		if (it->second)
		{
			if (!it->second->registerGraphicsSettings(it->first))
			{
				result = false;
				break;
			}
		}
		it++;
	}
	return result;
}

/**
*	This method checks if the specified resource file is modified.
*	@param objectName object file built from the resource file.
*	@param resName resource file.
*	@param outResDigest optional parameter, saves the MD5 hash digest of resName.
*	@return true if the resource file or its includes has been updated and needs a recompile.
*/
bool EffectManager::checkModified( const std::string& objectName, const std::string& resName, MD5::Digest* outResDigest )
{
	BW_GUARD;
	//Check the .fxo for recompilation
	//1) check to see if the .fxo even exists
	//2) check if the stored checksum differs from the .fx
	//3) check if the stored checksum for any included files differ

	MD5::Digest resDigest;
	if (!hashResource( resName, resDigest ))
	{
		// If the .fx doesn't exist, see if the fxo exists.
		if( BWResource::instance().rootSection()->openSection( objectName ) )
		{
			return false; // If fxo exists but fx doesn't, just use the fxo.
		}
		else
		{
			DEBUG_MSG( "EffectManager::checkModified: both .fx and .fxo are missing (%s)\n", resName.c_str() );
			return false;
		}
	}

	if ( outResDigest )
	{
		*outResDigest = resDigest;
	}

	DataSectionPtr pSection = BWResource::instance().rootSection()->openSection( objectName );
	if ( !pSection )
	{
		return true; // fxo doesn't exist, build it.
	}


	//
	// See if the checksum stored in the .fxo differs to the .fx file's actual checksum
	DataSectionPtr hashDataSection = pSection->openSection( "hash" );
	if (! hashDataSection )
	{
		return true; // must be an old fxo, rebuild it.
	}

	BinaryPtr hashBin = hashDataSection->asBinary();
	IF_NOT_MF_ASSERT_DEV( hashBin != NULL )
	{
		return true;
	}

	std::string quotedDigest;

	const char* pDigestValues = reinterpret_cast<const char*>( hashBin->data() );
	quotedDigest.assign( pDigestValues, pDigestValues + hashBin->len() );

	MD5::Digest storedResDigest;
	if (! storedResDigest.unquote(quotedDigest) )
	{
		return true; // bad data, rebuild it.
	}

	if ( storedResDigest != resDigest )
	{
		return true; // there's a hash digest mismatch, rebuild it.
	}


	//
	// Go through and see if any of the dependancies have changed.
	DataSectionPtr effectDataSection = pSection->openSection( "effect" );
	if ( !effectDataSection )
	{
		return true; // force recompile for old file
	}

	std::vector< std::pair<std::string, MD5::Digest> >::iterator itDep;
	std::vector< std::pair<std::string, MD5::Digest> > dependencies;

	// Go through and collect the dependancy list
	for ( DataSectionIterator it = pSection->begin(); it != pSection->end(); it++ )
	{
		DataSectionPtr pDS = *it;
		if(pDS->sectionName() == "depends")
		{
			// Grab the dependancy name
			DataSectionPtr nameDataSection = pDS->openSection( "name" );
			if ( !nameDataSection )
			{
				return true; // must be an old fxo, rebuild it.
			}

			std::string depName;
			BinaryPtr pStringData = nameDataSection->asBinary();
			const char* pValues = reinterpret_cast<const char*>( pStringData->data() );
			depName.assign( pValues, pValues + pStringData->len() );

			// Grab the stored dependancy hash
			DataSectionPtr hashDataSection = pDS->openSection( "hash" );
			if (! hashDataSection )
			{
				return true; // must be an old fxo, rebuild it.
			}

			BinaryPtr hashBin = hashDataSection->asBinary();
			IF_NOT_MF_ASSERT_DEV( hashBin != NULL )
			{
				return true;
			}

			std::string quotedDigest;
			const char* pDigestValues = reinterpret_cast<const char*>( hashBin->data() );
			quotedDigest.assign( pDigestValues, pDigestValues + hashBin->len() );

			MD5::Digest storedDepDigest;
			if (! storedDepDigest.unquote(quotedDigest) )
			{
				return true; // bad data, rebuild it.
			}

			dependencies.push_back( std::make_pair(depName, storedDepDigest) );
		}
	}

	// Go through the dependancy list and see if any mismatch
	for ( itDep = dependencies.begin(); itDep != dependencies.end(); itDep++ )
	{
		std::string name = (*itDep).first;		
		MD5::Digest storedDepDigest = (*itDep).second;

		std::string pathname = resolveInclude( name );

		MD5::Digest depDigest;
		if ( !hashResource(pathname, depDigest) )
		{
			return true; // If the dependancy is missing then rebuild.
		}

		if ( depDigest != storedDepDigest )
		{
			return true; // If the dependancy has changed then rebuild.
		}
	}

	// If we got this far, then we don't have to rebuild.
	return false;
}

}
