/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_LOG_HPP
#define USER_LOG_HPP

#include "logging_component.hpp"
#include "user_components.hpp"
#include "user_segment.hpp"
#include "unary_integer_file.hpp"

/**
 * A UserLog manages a single user's section of a log.  It is mainly
 * responsible for managing the monolithic files in the user's directory.
 * The segmented files are managed by UserSegments.
 */
class UserLog  : public SafeReferenceCount
{
public:
	UserLog( uint16 uid, const std::string &username );
	virtual ~UserLog();

	virtual bool init( const std::string rootPath );

	bool isGood() const;

	uint16 getUID() const;
	std::string getUsername() const;

	bool hasActiveSegments() const;
	std::string activeSegmentSuffix() const;

protected:
	// The unix UID associated to the user being referenced by this UserLog.
	uint16 uid_;

	// The name of the user being reference by this UserLog.
	std::string username_;

	// Full path to the UserLog being referenced.
	std::string path_;

	// Is the UserLog initialised and ready to use.
	bool isGood_;

	// List of UserSegments currently available for the owning user.
	UserSegments userSegments_;

	// List of UserComponents this user has logged.
	UserComponents userComponents_;

	// The file containing the UID associated to this user.
	UnaryIntegerFile uidFile_;
};

#endif // USER_LOG_HPP
