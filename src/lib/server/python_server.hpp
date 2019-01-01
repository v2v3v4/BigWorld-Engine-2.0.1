/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _PYTHON_SERVER_HEADER
#define _PYTHON_SERVER_HEADER

#include "cstdmf/config.hpp"

#if ENABLE_PYTHON_TELNET_SERVICE


#include "Python.h"

#include <deque>
#include <string>

#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/py_output_writer.hpp"
#include "pyscript/script.hpp"

class PythonServer;

#ifndef _WIN32  // WIN32PORT
#else //ifndef _WIN32  // WIN32PORT
typedef unsigned short uint16_t;
#endif //ndef _WIN32  // WIN32PORT

/**
 *	This class represents a single TCP connection to the Python interpreter.
 */
class PythonConnection : public Mercury::InputNotificationHandler
{
public:
	PythonConnection( PythonServer* owner,
			Mercury::EventDispatcher & dispatcher,
			int fd,
			std::string welcomeString );
	virtual ~PythonConnection();

	void 						write( const char* str );
	void						writePrompt();

	bool						active() const	{ return active_; }

private:
	int 						handleInputNotification( int fd );
	bool						handleTelnetCommand();
	bool						handleVTCommand();
	void						handleLine();
	void						handleDel();
	void						handleChar();
	void						handleUp();
	void						handleDown();
	void						handleLeft();
	void						handleRight();

	Mercury::EventDispatcher &	dispatcher_;
	std::deque<unsigned char>	readBuffer_;
	std::deque<std::string>		historyBuffer_;
	std::string					currentLine_;
	std::string					currentCommand_;
	Endpoint					socket_;
	PythonServer*				owner_;
	bool						telnetSubnegotiation_;
	int							historyPos_;
	unsigned int				charPos_;
	bool						active_;
	std::string					multiline_;
};

/**
 *	This class provides access to the Python interpreter via a TCP connection.
 *	It starts listening on a given port, and creates a new PythonConnection
 *	for every connection it receives.
 */
class PythonServer :
	public PyOutputWriter,
	public Mercury::InputNotificationHandler
{
	Py_Header( PythonServer, PyObjectPlus );

public:
	PythonServer( std::string welcomeString = "Welcome to PythonServer." );
	virtual ~PythonServer();

	bool			startup( Mercury::EventDispatcher & dispatcher,
						uint32_t ip, uint16_t port, const char * ifspec = 0 );
	void			shutdown();
	void			deleteConnection( PythonConnection* pConnection );
	uint16_t		port() const;

protected:
	void			printMessage( const std::string & msg );
private:
	int	handleInputNotification(int fd);
	std::vector<PythonConnection*> connections_;

	Endpoint		listener_;
	Mercury::EventDispatcher *	pDispatcher_;
	PyObject *		prevStderr_;
	PyObject *		prevStdout_;
	PyObject *		pSysModule_;
	std::string		welcomeString_;
};


#endif // ENABLE_PYTHON_TELNET_SERVICE


#endif // _PYTHON_SERVER_HEADER
