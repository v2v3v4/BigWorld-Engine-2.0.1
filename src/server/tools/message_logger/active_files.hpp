/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACTIVE_FILES_HPP
#define ACTIVE_FILES_HPP

#include "text_file_handler.hpp"
#include "user_log_writer.hpp"

/**
 * This class represents a file that tracks the entries and args files that
 * are currently being written to.  mltar.py respects this list in --move mode
 * and doesn't touch the files in it.
 */
class ActiveFiles : public TextFileHandler
{
public:
	bool init( const std::string &logPath, UserLogs *pUserLogs );

	virtual bool read();
	virtual bool handleLine( const char *line );

	bool update();

	bool deleteFile();

private:
	// The absolute path to the log directory the active_files file is in.
	std::string logPath_;

	// A list of UserLogs that should be used when updating the active_files
	UserLogs *pUserLogs_;
};

#endif // ACTIVE_FILES_HPP
