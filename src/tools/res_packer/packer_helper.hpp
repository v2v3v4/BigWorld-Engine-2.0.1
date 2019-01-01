/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __PACKER_HELPER_HPP__
#define __PACKER_HELPER_HPP__

#include <stdio.h>

#include <string>

class PackerHelper
{
public:
	// retrieve the command line parameters passed to res_packer
	static int argc() { return s_argc_; }
	static char** argv() { return s_argv_; }

	static void paths( const std::string& inPath, const std::string& outPath );

	static const std::string& inPath();

	static const std::string& outPath();

	// Inits BWResource and AutoConfig strings to the paths of the "file" parameters
	static bool initResources();

	// Inits Moo, creating a D3D device, etc.
	static bool initMoo();

	// Helper method to copy a file (platform-independent)
	static bool copyFile( const std::string& src, const std::string& dst, bool silent = false );

	// Helper method to ask if a file exists (platform-independent)
	static bool fileExists( const std::string& file );

	// Helper method to check if a file1 is newer than a file2 (platform-independent)
	static bool isFileNewer( const std::string& file1, const std::string& file2 );

	// Helper class to delete files on destruction. Ideal for temp files
	class FileDeleter
	{
	public:
		FileDeleter( const std::string& file ) : file_( file ) {};
		~FileDeleter() { remove( file_.c_str() ); }
	private:
		std::string file_;
	};

private:
	static int s_argc_;
	static char** s_argv_;
	static std::string s_basePath_;
	static std::string s_inPath;
	static std::string s_outPath;

#ifndef MF_SERVER
	// AutoConfig and method to get the cursor textures, so we can set them to the
	// appropriate format. It has to be done this way to avoid the overkill of
	// having to modify the MouseCursor class and including ashes, pyscript,
	// python and probably some other libs, all for this simple task.
	// Be aware that if the class MouseCursor in src/lib/ashes changes, this
	// AutoConfig and method need to be checked and changed if necesary to reflect
	// any changes in the tag names and/or the required texture format.
	static void initTextureFormats();
#endif

public:
	// Thes methods are not meant to be used by the packers!
	static void setCmdLine( int argc, char** argv )
		{ s_argc_ = argc; s_argv_ = argv; }
	static void setBasePath( const std::string& basePath )
		{ s_basePath_ = basePath; }
};


#endif // __PACKER_HELPER_HPP__
