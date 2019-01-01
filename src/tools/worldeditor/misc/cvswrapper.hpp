/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CVSWRAPPER_HPP
#define CVSWRAPPER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include <set>


struct CVSLog
{
	virtual ~CVSLog(){};
	virtual void add( const std::wstring& msg ) = 0;
};

class CVSWrapper
{
	std::string output_;
	static std::string cvsPath_;
	std::string workingPath_;
	static unsigned int batchLimit_;
	static bool enabled_;
	static bool directoryCommit_;
	static std::string dirToIgnore_;

	CVSLog* log_;

	enum Status
	{
		UNKNOWN = 0,
		ADDED,
		REMOVED,
		UPTODATE,
		LOCALMODIFIED,
		NEEDCHECKOUT
	};

	static bool isFile( const std::string& pathName );
	static bool isDirectory( const std::string& pathName );
	static bool exists( const std::string& pathName );
	static bool exec( std::string cmd, std::string workingDir, int& exitCode, std::string& output, CVSLog* log );
public:
	enum InitResult
	{
		SUCCESS = 0,
		DISABLED,
		FAILURE
	};
	static InitResult init();

	CVSWrapper( const std::string& workingPath, CVSLog* log = NULL );

	static bool ignoreDir( const std::string & dir ) { return dir == dirToIgnore_; }

	void refreshFolder( const std::string& relativePathName );

	bool editFiles( std::vector<std::string> filesToEdit );
	bool revertFiles( std::vector<std::string> filesToRevert );
	bool updateFolder( const std::string& relativePathName );
	bool commitFiles( const std::set<std::string>& filesToCommit,
		const std::set<std::string>& foldersToCommit,
		const std::string& commitMsg );

	bool isInCVS( const std::string& relativePathName );
	static bool enabled()	{	return enabled_;	}

	void removeFile( const std::string& relativePathName );
	std::set<std::string> addFolder( std::string relativePathName, const std::string& commitMsg, bool checkParent = true );
	bool addFile( std::string relativePathName, bool isBinary, bool recursive );

	const std::string& output() const;
};


#endif // CVSWRAPPER_HPP
