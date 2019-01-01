/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _RESOURCE_LOADER_HPP_
#define _RESOURCE_LOADER_HPP_

#include <set>

// Interface to control the visibility of the splash dialog
class ISplashVisibilityControl
{
public:
	// Pure virtual method to set splash visibility
	virtual void setSplashVisible( bool setVisibility ) = 0;
};


// This class can be used to preload resources
class ResourceLoader
{
public:
	typedef std::list< std::string > EffectList;
	// Accessors to the singleton instance
	static ResourceLoader& instance();
	static ResourceLoader* instancePtr();
	static void fini();

	// Precompiles effect files
	void precompileEffects( std::vector<ISplashVisibilityControl*>& SVCs );

private:
	// Private singleton constructor
	ResourceLoader() {}

	// Returns true if the folder should be excluded from the search
	bool excludedFolder( const std::string& folderName ) const;

	// Returns the list of effect file folders from the resources.xml files
	bool getEffectFileFolders( std::vector<DataSectionPtr>& shaderFolders );

	// Finds all files with "extension" in "folderName".
	void findInFolder(
			EffectList& result, const std::string& folderName,
			const std::string& extension, const bool searchSubfolders );

	// Caches a particular effect
	bool compileEffects( EffectList& effects, std::vector<ISplashVisibilityControl*>& SVCs );

	// Helper message to pump all paint messages through the message
	// dispatcher
	void pumpPaintMsgs();

	// Singleton instance
	static ResourceLoader* instance_;
};


#endif  // _RESOURCE_LOADER_HPP_
