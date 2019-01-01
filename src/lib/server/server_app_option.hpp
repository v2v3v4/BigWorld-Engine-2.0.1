/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_APP_OPTION_HPP
#define SERVER_APP_OPTION_HPP

#include "bwconfig.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/intrusive_object.hpp"
#include "cstdmf/watcher.hpp"


/**
 *	This class is the base class for objects used to initialise a
 *	ServerAppOption. These are created instead of adding the functionality to
 *	ServerAppOption so that all options can sit closer in memory.
 */
class ServerAppOptionIniter :
	public IntrusiveObject< ServerAppOptionIniter >
{
public:
	ServerAppOptionIniter( const char * configPath,
			const char * watcherPath,
			Watcher::Mode watcherMode );

	virtual void init() = 0;
	virtual void print() = 0;

	static void initAll();
	static void printAll();
	static void deleteAll();

protected:
	const char * configPath_;
	const char * watcherPath_;
	Watcher::Mode watcherMode_;

	static Container * s_pContainer_;
};


/**
 *	This class is a ServerAppOptionIniter that is type specific.
 */
template <class TYPE>
class ServerAppOptionIniterT : public ServerAppOptionIniter
{
public:
	ServerAppOptionIniterT( TYPE & value,
			const char * configPath,
			const char * watcherPath,
			Watcher::Mode watcherMode ) :
		ServerAppOptionIniter( configPath, watcherPath, watcherMode ),
		value_( value ),
		defaultValue_( value )
	{
	}

	virtual void init()
	{
		if (configPath_[0])
		{
			if (!BWConfig::update( configPath_, value_ ))
			{
				WARNING_MSG( "ServerAppConfig::init: "
						"%s not read from bw.xml chain. Using default (%s)\n",
					configPath_, watcherValueToString( value_ ).c_str() );
			}
		}

		if (watcherPath_[0])
		{
			MF_WATCH( watcherPath_, value_, watcherMode_ );
		}
	}

	virtual void print()
	{
		const char * name = configPath_;

		if ((name  == NULL) || (name[0] == '\0'))
		{
			return;
		}

		if (value_ == defaultValue_)
		{
			INFO_MSG( "  %-34s = %s\n",
					name, watcherValueToString( value_ ).c_str() );
		}
		else
		{
			INFO_MSG( "  %-34s = %s (default: %s)\n",
					name,
					watcherValueToString( value_ ).c_str(),
					watcherValueToString( defaultValue_ ).c_str() );
		}
	}

private:
	TYPE & value_;
	TYPE defaultValue_;
};


/**
 *	This class is used to add a configuration option to an application. It
 *	handles reading the value from bw.xml, adding a watcher entry and printing
 *	out the value of the setting on startup.
 */
template <class TYPE>
class ServerAppOption
{
public:
	ServerAppOption( TYPE value,
			const char * configPath,
			const char * watcherPath,
			Watcher::Mode watcherMode = Watcher::WT_READ_WRITE ) :
		value_( value )
	{
		new ServerAppOptionIniterT< TYPE >( value_,
				configPath, watcherPath, watcherMode );
	}

	TYPE operator()() const	{ return value_; }
	void set( TYPE value ) { value_ = value; }
	TYPE & getRef()	{ return value_; }

private:
	TYPE value_;
};

#endif // SERVER_APP_OPTION_HPP
