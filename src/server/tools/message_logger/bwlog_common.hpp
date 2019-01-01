/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWLOG_COMMON_HPP
#define BWLOG_COMMON_HPP

#include "format_strings.hpp"
#include "hostnames.hpp"
#include "unary_integer_file.hpp"
#include "log_component_names.hpp"

#include <string>

class BWLogCommon
{
public:
	const char *getHostByAddr( uint32 addr );
	const char *getComponentByID( int typeID ) const;

protected:
	bool initRootLogPath( const char *rootPath );
	bool initCommonFiles( const char *mode );
	bool initUserLogs( const char *mode );

	virtual bool onUserLogInit( uint16 uid, const std::string &username ) = 0;

	std::string rootLogPath_;

	UnaryIntegerFile version_;

	LogComponentNames componentNames_;

	Hostnames hostnames_;

	FormatStrings formatStrings_;
};

#endif // BWLOG_COMMON_HPP
