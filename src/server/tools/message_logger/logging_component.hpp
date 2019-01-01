/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGGING_COMPONENT_HPP
#define LOGGING_COMPONENT_HPP

#include "user_components.hpp"
#include "log_entry_address.hpp"

#include "network/logger_message_forwarder.hpp"

/**
 * This class represents a persistent process somewhere that is sending to
 * a log and is responsible for reading and writing users "components" files.
 */
class LoggingComponent
{
public:
	// Candidate for cleanup. only required by the 'reader'
	LoggingComponent( UserComponents *userComponents );

	// Candidate for cleanup. only required by 'writer'
	LoggingComponent( UserComponents *userComponents,
		const Mercury::Address &addr,
		const LoggerComponentMessage &msg, int ttypeid );

	void write( FileStream &out );
	void read( FileStream &in );
	bool written() const;

	bool setAppInstanceID( int id );
	int getAppTypeID() const;

	const Mercury::Address & getAddress() const;

	std::string getString() const;

	void updateFirstEntry( const std::string & suffix,
									const int numEntries );

	// Candidate for cleanup. Make these private. Currently used by PyQuery
	LoggerComponentMessage msg_;
	int32 appid_;	// ID known as amongst server components, eg. cellapp01
	int32 typeid_;	// Process-type ID assigned to cellapp, baseapp etc
	LogEntryAddress firstEntry_;

private:

	Mercury::Address addr_;

	int32 id_;		// Unique ID per Component object

	int fileOffset_;

	std::string userComponentsFilename_;
};

#endif // LOGGING_COMPONENT_HPP
