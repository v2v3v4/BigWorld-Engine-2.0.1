/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "logging_component.hpp"

#include "user_log.hpp"
#include "user_components.hpp"


LoggingComponent::LoggingComponent( UserComponents *userComponents ) :
	userComponentsFilename_( userComponents->filename() )
{ }


LoggingComponent::LoggingComponent( UserComponents *userComponents,
		const Mercury::Address &addr, const LoggerComponentMessage &msg,
		int ttypeid ) :
	msg_( msg ),
	appid_( 0 ),
	typeid_( ttypeid ),
	addr_( addr ),
	id_( userComponents->getID() ),
	fileOffset_( -1 ),
	userComponentsFilename_( userComponents->filename() )
{ }


void LoggingComponent::updateFirstEntry( const std::string & suffix,
											const int numEntries )
{
	firstEntry_ = LogEntryAddress( suffix, numEntries );
}


/**
 *  This method is not const as you'd probably expect because it needs to set
 *  this object's fileOffset_ field prior to writing.
 */
void LoggingComponent::write( FileStream &os )
{
	fileOffset_ = os.tell();
	os << addr_ << id_ << appid_ << typeid_;
	msg_.write( os );
	firstEntry_.write( os );
	os.commit();
}


void LoggingComponent::read( FileStream &is )
{
	fileOffset_ = is.tell();
	is >> addr_ >> id_ >> appid_ >> typeid_;
	msg_.read( is );
	firstEntry_.read( is );
}


bool LoggingComponent::written() const
{
	return fileOffset_ != -1;
}


bool LoggingComponent::setAppInstanceID( int id )
{
	appid_ = id;

	// If this component has already been written to disk (which is almost
	// certain) then we need to overwrite the appid field in the components file
	if (this->written())
	{
		FileStream file( userComponentsFilename_.c_str(), "r+" );

		// Seek to the exact offset of the app id
		file.seek( fileOffset_ );
		file >> addr_ >> id_;

		// Overwrite old ID
		file << appid_;
		file.commit();
	}

	return true;
}


int LoggingComponent::getAppTypeID() const
{
	return id_;
}


const Mercury::Address & LoggingComponent::getAddress() const
{
	return addr_;
}


std::string LoggingComponent::getString() const
{
	char buf[ 256 ];
	bw_snprintf( buf, sizeof( buf ), "%s%02d (id:%d) %s",
		msg_.componentName_.c_str(), appid_, id_, addr_.c_str() );
	return std::string( buf );
}

// logging_component.cpp
