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

#include "remote_stepper.hpp"
#include "endpoint.hpp"

#ifdef _WIN32
#ifndef MF_SERVER
extern const char * compileTimeString;
#else // MF_SERVER
const char * compileTimeString = __TIMESTAMP__;
#endif // MF_SERVER
#endif

DECLARE_DEBUG_COMPONENT2( "App", -1 )

/**
 *	Constructor
 */
RemoteStepper::RemoteStepper() :
	lep_( NULL ),
	cep_( NULL )
{
	lep_ = new Endpoint();
	lep_->socket( SOCK_STREAM );
	lep_->bind( htons( 11111 ) );
	lep_->listen();
	lep_->setnonblocking( true );

	u_int32_t	localip = 0;
	lep_->getInterfaceAddress( "eth0", localip );
	dprintf( "Waiting for remote stepper connection on %d.%d.%d.%d:11111\n",
		((localip)&255), ((localip>>8)&255), ((localip>>16)&255), ((localip>>24)&255) );

#if 0
	// wait up to .1s for a connection here
	int nloops = 0;
	while (!this->tryaccept())
	{
		Sleep( 50 );

		if (++nloops > 100/50)
		{
			dprintf( "Given up waiting for a connection.\n" );
			break;
		}
	}
#endif
}

/**
 *	Destructor
 */
RemoteStepper::~RemoteStepper()
{
	if (cep_ != NULL)
	{
		cep_->close();
		delete cep_;
	}

	lep_->close();
	delete lep_;
}


/**
 *	Try to accept a connection
 */
bool RemoteStepper::tryaccept()
{
	cep_ = lep_->accept();
	if (cep_ == NULL) return false;

	cep_->setnonblocking( false );

	dprintf( "Got remote stepper connection\n" );

	std::string sendout;
#ifdef _WIN32
	sendout = "Welcome to the BigWorld Client.\r\nCompile time was";
	sendout += compileTimeString;
#endif // _WIN32
	sendout += "\r\nStarting game...\r\n";
	cep_->send( sendout.c_str(), sendout.length() );
	return true;
}


enum TelnetEscape
{
	TELNET_WILL			= 251,
	TELNET_DO			= 252,
	TELNET_WONT			= 253,
	TELNET_DONT			= 254,
	TELNET_IAC			= 255
};



/**
 *	Step method
 */
void RemoteStepper::stepInt( const std::string & desc, bool wait )
{
	TRACE_MSG( "Remote stepper at step %s\n", desc.c_str() );

	if (cep_ == NULL)
	{
		if (!tryaccept()) return;
	}

	std::string sendout = "Now at step: " + desc + "\r\n";
	cep_->send( sendout.c_str(), sendout.length() );

	if (!wait) return;

	sendout = "Press enter to continue\r\n";
	cep_->send( sendout.c_str(), sendout.length() );

	char	recvBuf[128];
	int		recvAmt;
	bool	gotEnter = false;
	while (!gotEnter &&
		(recvAmt = cep_->recv( recvBuf, sizeof(recvBuf) )) > 0)
	{
		for (int i = 0; i < recvAmt; i++)
		{
			if (recvBuf[i] == '\n')
			{
				gotEnter = true;
				sendout = "Continuing...\r\n";
				cep_->send( sendout.c_str(), sendout.length() );
//				Sleep( 50 );
			}
		}
	}
	if (recvAmt < 0)
	{
		dprintf( "Remote stepper got error on connection.\n" );
		cep_->close();
		delete cep_;
		cep_ = NULL;
	}
}


static RemoteStepper * pUfa = NULL;

/**
 *	Static start method
 */
void RemoteStepper::start()
{
	if (pUfa == NULL) pUfa = new RemoteStepper();
}

/**
 *	Static step method
 */
void RemoteStepper::step( const std::string & desc, bool wait )
{
	if (pUfa != NULL) pUfa->stepInt( desc, wait );
}

// remote_stepper.cpp
