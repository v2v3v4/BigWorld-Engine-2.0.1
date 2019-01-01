/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FILENAME_CHECKER_HPP
#define FILENAME_CHECKER_HPP


#ifdef  _WIN32


#include "cstdmf/stringmap.hpp"
#include "cstdmf/concurrency.hpp"


class FilenameCaseChecker
{
public:
	FilenameCaseChecker();

	bool check(std::string const &path, bool warnIfNotCorrect = true);

	std::string filenameOnDisk(std::string const &path);

protected:
	typedef StringHashMap<std::string>	FilenameCache;

	FilenameCache::iterator cachePath(std::string const &path, bool *wasInCache);

	bool compare(std::string const &path1, std::string const &path2) const;

private:
	FilenameCaseChecker(FilenameCaseChecker const &);				// not allowed
	FilenameCaseChecker &operator=(FilenameCaseChecker const &);	// not allowed

private:
	FilenameCache	cache_;
	SimpleMutex		mutex_;
};


#endif // _WIN32


#endif // FILENAME_CHECKER_HPP
