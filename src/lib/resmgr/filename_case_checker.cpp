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
#include "resmgr/filename_case_checker.hpp"
#include "cstdmf/string_utils.hpp"

#ifdef  _WIN32


#include <shlwapi.h>
#include <shellapi.h>
#include <shtypes.h>
#include <shlobj.h>


/**
 *	This is the FilenameCaseChecker constructor.
 */
FilenameCaseChecker::FilenameCaseChecker()
{
}


/**
 *	This checks that the file on disk matches the case of path.
 *
 *	@param path			The file to check.
 *	@param warnIfNotCorrect	Print a warning if the case is not correct the 
 *						first time that the file is checked.
 *	@returns			True if the case of path matches the file on disk, 
 *						false if they differ.  If the file does not exist
 *						then true is returned (it is assumed that this is a
 *						new file).
 */
bool FilenameCaseChecker::check
(
	std::string		const &path,
	bool			warnIfNotCorrect /*= true*/
)
{
	SimpleMutexHolder holder(mutex_);

	bool wasInCache = false;
	FilenameCache::iterator it = cachePath(path, &wasInCache);

	const std::string &cpath = it->second;
	bool caseSame = compare(cpath, path);

	if (!caseSame && !wasInCache && warnIfNotCorrect)
	{
		WARNING_MSG
		(
			"Case in filename %s does not match case on disk %s\n",
			path.c_str(),
			cpath.c_str()
		); 
	}
	
	return caseSame;
}


/**
 *	This gets the name of a file as it is on disk.
 *
 *	@param path		The filename.
 *	@returns		The name of the file as it is on disk.  If the file does
 *					not exist then path is returned.
 */
std::string FilenameCaseChecker::filenameOnDisk(std::string const &path)
{
	SimpleMutexHolder holder(mutex_);

	FilenameCache::iterator it = cachePath(path, NULL);
	return it->second;
}


/**
 *	This makes sure that path and the real filename on disk are cached.
 *
 *	@param path		The path to cache.
 *	@param wasInCache	If this is not NULL then it's set to true if the path
 *					was already in the cache and false if the path was not in
 *					the cache.
 *	@returns		An iterator to the cached value.
 */
FilenameCaseChecker::FilenameCache::iterator 
FilenameCaseChecker::cachePath
(
	std::string		const &path, 
	bool			*pWasInCache
)
{
	FilenameCache::iterator it = cache_.find(path);	
	bool wasInCache = it != cache_.end();
	if (!wasInCache)
	{
		// The name has not yet been cached.

		// Convert path to wide-string with the slashes in the windows style:
		std::string spath = path;
		std::replace(spath.begin(), spath.end(), '/', '\\');
		std::wstring wpath(spath.begin(), spath.end());

		// Create a PIDL from the filename:
		HRESULT hr = S_OK;
		ITEMIDLIST *pidl = NULL;
		DWORD flags = 0;
		hr = ::SHILCreateFromPath(wpath.c_str(), &pidl, &flags);

		std::string upath; // This gets set to the name on disk.

		if (SUCCEEDED(hr))
		{
			// The path is a real path, convert the PIDL to a filename:
			wchar_t wbuffer[MAX_PATH];
			::SHGetPathFromIDList( pidl, wbuffer );
			::ILFree( pidl );
			bw_wtoutf8( wbuffer, upath );
		}
		else
		{
			// The path does not exist.  Just use the original path as the 
			// path on disk.
			upath = path;
		}

		// Add a new cache entry:
		cache_.insert(std::make_pair<>(path, upath));
		it = cache_.find(path);
	}

	if (pWasInCache != NULL)
		*pWasInCache = wasInCache;

	return it;
}


/**
 *	This function compares two paths to see if they are the same.  It
 *	ignores slashes and the first drive letter.
 *
 *	@param path1	The first path.
 *	@param path2	The second path.
 *	@returns		True if the paths are the same, excluding the drive
 *					letter and any slashes.
 */
bool FilenameCaseChecker::compare
(
	std::string		const &path1, 
	std::string		const &path2
) const
{
	// Handle trivial cases:
	if (path1.empty() || path2.empty())
		return path1.empty() && path2.empty();

	std::string p1	= path1;
	std::string p2	= path2;

	// Make any drive letters lowercase:
	if (p1.length() >= 2 && p1[1] == ':') 
		p1[0] = tolower(p1[0]);
	if (p2.length() >= 2 && p2[1] == ':') 
		p2[0] = tolower(p2[0]);

	// Make back slashes forward slashes:
	std::replace(p1.begin(), p1.end(), '/', '\\');
	std::replace(p2.begin(), p2.end(), '/', '\\');

	// Remove any trailing slashes:
	if (p1[p1.length() - 1] == '\\')
	{
		p1 = p1.substr(0, p1.length() - 1);
	}
	if (p2[p2.length() - 1] == '\\')
	{
		p2 = p2.substr(0, p2.length() - 1);
	}

	return p1 == p2;
}


#endif // _WIN32
