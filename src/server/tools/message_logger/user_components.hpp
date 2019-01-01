/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_COMPONENTS_HPP
#define USER_COMPONENTS_HPP

#include "binary_file_handler.hpp"

#include "log_component_names.hpp"

#include "network/basictypes.hpp"

#include <string>
#include <map>


class LoggingComponent;
class LoggerComponentMessage;

class UserComponentVisitor
{
public:
	virtual bool onComponent( const LoggingComponent &component ) = 0;
};


/**
 * The registry of processes within each UserLog.  Contains static info
 * about each process such as name, pid, etc.
 */
class UserComponents : public BinaryFileHandler
{
public:
	UserComponents();
	virtual ~UserComponents();

	bool init( const char *root, const char *mode );

	virtual bool read();
	virtual void flush();
	bool write( LoggingComponent *logComponent );

	// Candidate for cleanup. Only used by the reader
	const LoggingComponent *getComponentByID( int id );

	// Candidate for cleanup. Only used by the writer.
	LoggingComponent *getComponentByAddr( const Mercury::Address &addr );

	// Candidate for cleanup. Only used by writer.
	LoggingComponent *getComponentFromMessage(
								const LoggerComponentMessage &msg,
								const Mercury::Address &addr,
								LogComponentNames &logComponentNames );

	bool erase( const Mercury::Address &addr );

	const char *getFilename() const;

	int getID();

	bool visitAllWith( UserComponentVisitor &visitor ) const;

private:
	std::string filename_;

	typedef std::map< Mercury::Address, LoggingComponent* > AddrMap;
	AddrMap addrMap_;

	typedef std::map< int, LoggingComponent* > IDMap;
	IDMap idMap_;

	int idTicker_;
};

#endif // USER_COMPONENTS_HPP
