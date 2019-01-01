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
#include "resource_loader.hpp"

#include "moo/effect_manager.hpp"
#include "moo/managed_effect.hpp"
#include "resmgr/file_system.hpp"
#include "resmgr/multi_file_system.hpp"
#include "shader_loading_dialog.hpp"

/** Initialise the singleton instance. */
ResourceLoader* ResourceLoader::instance_ = 0;

/**
 *	Singleton reference accessor.
 *
 *	@returns A reference to the singleton instance.
 */
ResourceLoader& ResourceLoader::instance()
{
	BW_GUARD;

    return *instancePtr();
}


/**
 *	Singleton pointer accessor.
 *
 *	@returns A pointer to the singleton instance.
 */
ResourceLoader* ResourceLoader::instancePtr()
{
	BW_GUARD;

	if (!instance_)
		instance_ = new ResourceLoader();

	return instance_;
}


/*static*/ void ResourceLoader::fini()
{
	BW_GUARD;

	delete instance_;
	instance_ = NULL;
}


/**
 *	Precompiles effect files to reduce the number of stalls when loading spaces.
 *
 *	@param	svc	A pointer to the splash dialog control.  This is used to hide the
 *				splash screen while compiling the effect files.
 */
void ResourceLoader::precompileEffects( std::vector<ISplashVisibilityControl*>& SVCs )
{
	BW_GUARD;

	// Get the list of folders containing effect files
	std::vector<DataSectionPtr> shaderFolders;
	if (!getEffectFileFolders( shaderFolders ))
		return;

	EffectList effectNames;
	for ( uint i = 0; i < shaderFolders.size(); i++ )
	{
		this->findInFolder( effectNames, shaderFolders[i]->asString(), "fx", true );
	}

	this->compileEffects( effectNames, SVCs );
}


/**
 *	Adds the list of effect file folders to shaderFolders.  This method gets the list
 *	of folders from the resources.xml files.
 *
 *	@param	shaderFolders	A vector of data sections, each an effect folder string.
 *	@returns	Success or failure.
 */
bool ResourceLoader::getEffectFileFolders( std::vector<DataSectionPtr>& shaderFolders )
{
	BW_GUARD;

	// Check that there is at least one resources.xml file
	DataSectionPtr pResourcesXML = BWResource::openSection( "resources.xml" );
	if ( !pResourcesXML )
	{
		// Rerurn failure
		return false;
	}

	// Open all resources.xml files and collate them
	MultiFileSystemPtr mfs = BWResource::instance().fileSystem();
	std::vector<BinaryPtr> sections;
	mfs->collateFiles( "resources.xml", sections );

	// Extract the list of effect file folders
	for (std::vector<BinaryPtr>::iterator it = sections.begin();
		 it != sections.end();
		 ++it )
	{
		DataSectionPtr pSec = DataSection::createAppropriateSection( "root", *it );
		pSec->openSections( "shaderPaths/path", shaderFolders );
	}

	// Return success
	return true;
}


/**
 *	Returns true if the folder should be excluded from the search.
 *
 *	@param	folderName	The folder name being checked.
 *	@return	True if the folder should be ignored, false otherwise.
 */
bool ResourceLoader::excludedFolder( const std::string& folderName ) const
{
	BW_GUARD;

	if (folderName == "CVS" || folderName == ".svn" || folderName == ".bwthumbs")
		return true;
	else
		return false;
}


/**
 *	Finds all files with a particular extension in a folder or folder tree.
 *
 *	@param	result				A vector of the files found.
 *	@param	folderName			The folder we're searching in.
 *	@param	extension			The file extension being searched for.
 *	@param	searchSubfolders	Should we search all subfolders of folderName?
 */
void ResourceLoader::findInFolder(
		EffectList& result, const std::string& folderName,
		const std::string& extension, const bool searchSubfolders )
{
	BW_GUARD;

	MultiFileSystemPtr mfs = BWResource::instance().fileSystem();
	IFileSystem::Directory pFolder = mfs->readDirectory( folderName );

	if ( pFolder.size() )
	{
		IFileSystem::Directory::iterator it = pFolder.begin();
		IFileSystem::Directory::iterator end = pFolder.end();
		while ( it != end )
		{
			// Don't want to look in CVS folders
			if ( excludedFolder( *it ) )
			{
				++it;
				continue;
			}

			// If this is a subfolder and the search sub folder flag is
			// true, perform a recursive search through the folder
			std::string filePath = folderName + (*it);
			IFileSystem::Directory pSubFolder = mfs->readDirectory( filePath );
			if ( searchSubfolders && pSubFolder.size() )
			{
				this->findInFolder(
						result, filePath + "/", extension, true );
			}
			// Otherwise check if this file has the correct extension
			else if ( BWResource::getExtension( (*it) ) == extension )
			{
				result.push_back( filePath );
			}

			++it;
		}
	}
}


/**
 *	Compiles the passed effect files.  This method will switch off the passed splash screen while compiling
 *	the effect files so that both dialogs don't fight for the users attention.
 *
 *	@param	effects	A vector of effect files to compile.
 *	@param	svc		The splash screen visibility control.
 *	@returns	Success or failure.
 */
bool ResourceLoader::compileEffects( EffectList& effects, std::vector<ISplashVisibilityControl*>& SVCs )
{
	BW_GUARD;

	// Check how many need to be compiled
	EffectList::iterator it;
	for ( it = effects.begin(); it != effects.end(); )
	{
		if ( !Moo::EffectManager::instance().needRecompile( *it ) )
		{
			it = effects.erase( it );
			continue;
		}

		++it;
	}

	CShaderLoadingDialog* load = 0;
	if ( effects.size() )
	{
		std::vector<ISplashVisibilityControl*>::iterator svcIt;
		for (svcIt = SVCs.begin(); svcIt != SVCs.end(); ++svcIt)
			(*svcIt)->setSplashVisible( false );		// Call to pure virtual method

		pumpPaintMsgs();
		load = new CShaderLoadingDialog();
		load->setRange( effects.size() );
	}

	for (	EffectList::iterator it = effects.begin();
			it != effects.end(); ++it)
	{
		pumpPaintMsgs();
		Moo::EffectManager::instance().compileOnly( *it );
		if (load) load->step();
	}

	INFO_MSG( "ResourceLoader::cacheEffects - CACHING COMPLETE\n" );

	if (load)
	{
		delete load;

		std::vector<ISplashVisibilityControl*>::iterator svcIt;
		for (svcIt = SVCs.begin(); svcIt != SVCs.end(); ++svcIt)
			(*svcIt)->setSplashVisible( true );		// Call to pure virtual method

		pumpPaintMsgs();
	}


	return true;
}

/**
 *	Helper method used to pump all paint messages through the message queue.
 *	This must be done when switching the splash dialog on and off since there
 *	can be a considerable delay between the message being sent and the
 *	message getting processed.
 */
void ResourceLoader::pumpPaintMsgs()
{
	BW_GUARD;

	MSG msg;
	while( ::PeekMessage( &msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE ) )
	{
		::DispatchMessage( &msg );
	}
}
