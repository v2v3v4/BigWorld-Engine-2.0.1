/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "python_server.hpp"

#include "cstdmf/config.hpp"


#if ENABLE_PYTHON_TELNET_SERVICE


#include <node.h>
#include <grammar.h>
#include <parsetok.h>
#include <errcode.h>

extern grammar _PyParser_Grammar;

DECLARE_DEBUG_COMPONENT(0)

#define TELNET_ECHO			1
#define TELNET_LINEMODE		34
#define TELNET_SE			240
#define TELNET_SB			250
#define TELNET_WILL			251
#define TELNET_DO			252
#define TELNET_WONT			253
#define TELNET_DONT			254
#define TELNET_IAC			255

#define ERASE_EOL			"\033[K"

#define KEY_CTRL_C			3
#define KEY_CTRL_D			4
#define KEY_BACKSPACE		8
#define KEY_DEL				127
#define KEY_ESC				27

#define MAX_HISTORY_LINES	50

/**
 *	This constructor intitialises the PythonConnection given an existing
 *	socket.
 */
PythonConnection::PythonConnection( PythonServer * owner,
			Mercury::EventDispatcher & dispatcher, int fd,
			std::string welcomeString ) :
		dispatcher_( dispatcher ),
		owner_( owner ),
		telnetSubnegotiation_( false ),
		historyPos_( -1 ),
		charPos_( 0 ),
		active_( false ),
		multiline_()
{
	socket_.setFileDescriptor( fd );
	dispatcher_.registerFileDescriptor( socket_, this );

	unsigned char options[] =
	{
		TELNET_IAC, TELNET_WILL, TELNET_ECHO,
		TELNET_IAC, TELNET_WONT, TELNET_LINEMODE, 0
	};

	this->write( (char*)options );
	this->write( welcomeString.c_str() );
	this->write( "\r\n" );
	this->writePrompt();
}

/**
 *	This is the destructor.
 *	It deregisters the socket with Mercury. Note that the Endpoint
 *	destructor will close the socket.
 */
PythonConnection::~PythonConnection()
{
	dispatcher_.deregisterFileDescriptor( socket_ );
}

/**
 *	This method is called by Mercury when the socket is ready for reading.
 *	It processes user input from the socket, and sends it to Python.
 *
 *	@param fd	Socket that is ready for reading.
 */
int PythonConnection::handleInputNotification(int fd)
{
	MF_ASSERT(fd == (int)socket_);

	char buf[256];
	int i, bytesRead;

	bytesRead = recv(fd, buf, sizeof(buf), 0);

	if (bytesRead == -1)
	{
		ERROR_MSG( "PythonConnection %d: Read error\n", fd );
		owner_->deleteConnection(this);
		return 1;
	}

	if (bytesRead == 0)
	{
		owner_->deleteConnection(this);
		return 1;
	}

	for(i = 0; i < bytesRead; i++)
	{
		readBuffer_.push_back(buf[i]);
	}

	while (!readBuffer_.empty())
	{
		int c = (unsigned char)readBuffer_[0];

		// Handle (and ignore) telnet protocol commands.

		if (c == TELNET_IAC)
		{
			if (!this->handleTelnetCommand())
				return 1;
			continue;
		}

		if (c == KEY_ESC)
		{
			if (!this->handleVTCommand())
				return 1;
			continue;
		}

		// If we're in telnet subnegotiation mode, ignore normal chars.

		if (telnetSubnegotiation_)
		{
			readBuffer_.pop_front();
			continue;
		}

		switch (c)
		{
			case '\r':
				this->handleLine();
				break;

			case KEY_BACKSPACE:
			case KEY_DEL:
				this->handleDel();
				break;

			case KEY_CTRL_C:
			case KEY_CTRL_D:
				owner_->deleteConnection(this);
				return 1;

			case '\0':
			case '\n':
				// Ignore these
				readBuffer_.pop_front();
				break;

			default:
				this->handleChar();
				break;
		}
	}

	return 1;
}

/**
 * 	This method handles telnet protocol commands. Well actually it handles
 * 	a subset of telnet protocol commands, enough to get Linux and Windows
 *	telnet working in character mode.
 */
bool PythonConnection::handleTelnetCommand()
{
	// TODO: Need to check that there is a second byte on readBuffer_.
	unsigned int cmd = (unsigned char)readBuffer_[1];
	unsigned int bytesNeeded = 2;
	char str[256];

	switch (cmd)
	{
		case TELNET_WILL:
		case TELNET_WONT:
		case TELNET_DO:
		case TELNET_DONT:
			bytesNeeded = 3;
			break;

		case TELNET_SE:
			telnetSubnegotiation_ = false;
			break;

		case TELNET_SB:
			telnetSubnegotiation_ = true;
			break;

		case TELNET_IAC:
			// A literal 0xff. We don't care!
			break;

		default:
			sprintf(str, "Telnet command %d unsupported.\r\n", cmd);
			this->write(str);
			break;
	}

	if (readBuffer_.size() < bytesNeeded)
		return false;

	while (bytesNeeded)
	{
		bytesNeeded--;
		readBuffer_.pop_front();
	}

	return true;
}

bool PythonConnection::handleVTCommand()
{
	// Need 3 chars before we are ready.
	if (readBuffer_.size() < 3)
		return false;

	// Eat the ESC.
	readBuffer_.pop_front();

	if (readBuffer_.front() != '[' && readBuffer_.front() != 'O')
		return true;

	// Eat the [
	readBuffer_.pop_front();

	switch (readBuffer_.front())
	{
		case 'A':
			this->handleUp();
			break;

		case 'B':
			this->handleDown();
			break;

		case 'C':
			this->handleRight();
			break;

		case 'D':
			this->handleLeft();
			break;

		default:
			return true;
	}

	readBuffer_.pop_front();
	return true;
}


/**
 * 	This method handles a single character. It appends or inserts it
 * 	into the buffer at the current position.
 */
void PythonConnection::handleChar()
{
	// @todo: Optimise redraw
	currentLine_.insert(charPos_, 1, (char)readBuffer_.front());
	int len = currentLine_.length() - charPos_;
	this->write(currentLine_.substr(charPos_, len).c_str());

	for(int i = 0; i < len - 1; i++)
		this->write("\b");

	charPos_++;
	readBuffer_.pop_front();
}

#if 0
/**
 * 	This method returns true if the command would fail because of an EOF
 * 	error. Could use this to implement multiline commands.. but later.
 */
static bool CheckEOF(char *str)
{
	node *n;
	perrdetail err;
	n = PyParser_ParseString(str, &_PyParser_Grammar, Py_single_input, &err);

	if (n == NULL && err.error == E_EOF )
	{
		printf("EOF\n");
		return true;
	}

	printf("OK\n");
	PyNode_Free(n);
	return false;
}
#endif

/**
 * 	This is a variant on PyRun_SimpleString. It does basically the
 *	same thing, but uses Py_single_input, so the Python compiler
 * 	will mark the code as being interactive, and print the result
 *	if it is not None.
 *
 *	@param command		Line of Python to execute.
 */
static int MyRun_SimpleString(char *command)
{
	// Ignore lines that only contain comments
	{
		char * pCurr = command;
		while (*pCurr != '\0' && *pCurr != ' ' && *pCurr != '\t')
		{
			if (*pCurr == '#')
				return 0;
			++pCurr;
		}
	}

	PyObject *m, *d, *v;
	m = PyImport_AddModule("__main__");
	if (m == NULL)
		return -1;
	d = PyModule_GetDict(m);

	v = PyRun_String(command, Py_single_input, d, d);

	if (v == NULL) {
		PyErr_Print();
		return -1;
	}
	Py_DECREF(v);
	if (Py_FlushLine())
		PyErr_Clear();
	return 0;
}

/**
 * 	This method handles an end of line. It executes the current command,
 *	and adds it to the history buffer.
 */
void PythonConnection::handleLine()
{
	readBuffer_.pop_front();
	this->write("\r\n");

	if (currentLine_.empty())
	{
		currentLine_ = multiline_;
		multiline_ = "";
	}
	else
	{
		historyBuffer_.push_back(currentLine_);

		if (historyBuffer_.size() > MAX_HISTORY_LINES)
		{
			historyBuffer_.pop_front();
		}

		if (!multiline_.empty())
		{
			multiline_ += "\n" + currentLine_;
			currentLine_ = "";
		}
	}

	if (!currentLine_.empty())
	{
		currentLine_ += "\n";

		if (currentLine_[ currentLine_.length() - 2 ] == ':')
		{
			multiline_ += currentLine_;
		}
		else
		{
			active_ = true;
			MyRun_SimpleString((char *)currentLine_.c_str());
			// PyObject * pRes = PyRun_String( (char *)currentLine_.c_str(), true );
			active_ = false;
		}
	}

	currentLine_ = "";
	historyPos_ = -1;
	charPos_ = 0;

	this->writePrompt();
}


/**
 *	This method handles a del character.
 */
void PythonConnection::handleDel()
{
	if (charPos_ > 0)
	{
		// @todo: Optimise redraw
		currentLine_.erase(charPos_ - 1, 1);
		this->write("\b" ERASE_EOL);
		charPos_--;
		int len = currentLine_.length() - charPos_;
		this->write(currentLine_.substr(charPos_, len).c_str());

		for(int i = 0; i < len; i++)
			this->write("\b");
	}

	readBuffer_.pop_front();
}


/**
 * 	This method handles a key up event.
 */
void PythonConnection::handleUp()
{
	if (historyPos_ < (int)historyBuffer_.size() - 1)
	{
		historyPos_++;
		currentLine_ = historyBuffer_[historyBuffer_.size() -
			historyPos_ - 1];

		// @todo: Optimise redraw
		this->write("\r" ERASE_EOL);
		this->writePrompt();
		this->write(currentLine_.c_str());
		charPos_ = currentLine_.length();
	}
}


/**
 * 	This method handles a key down event.
 */
void PythonConnection::handleDown()
{
	if (historyPos_ >= 0 )
	{
		historyPos_--;

		if (historyPos_ == -1)
			currentLine_ = "";
		else
			currentLine_ = historyBuffer_[historyBuffer_.size() -
				historyPos_ - 1];

		// @todo: Optimise redraw
		this->write("\r" ERASE_EOL);
		this->writePrompt();
		this->write(currentLine_.c_str());
		charPos_ = currentLine_.length();
	}
}


/**
 * 	This method handles a key left event.
 */
void PythonConnection::handleLeft()
{
	if (charPos_ > 0)
	{
		charPos_--;
		this->write("\033[D");
	}
}


/**
 * 	This method handles a key left event.
 */
void PythonConnection::handleRight()
{
	if (charPos_ < currentLine_.length())
	{
		charPos_++;
		this->write("\033[C");
	}
}


/**
 *	This method sends output to the socket.
 *	We don't care too much about buffer overflows or write errors.
 *	If the connection drops, we'll hear about it when we next read.
 */
void PythonConnection::write(const char* str)
{
	int len = strlen(str);
	send(socket_, str, len, 0);
}

/**
 * 	This method prints a prompt to the socket.
 */
void PythonConnection::writePrompt()
{
	return this->write( multiline_.empty() ? ">>> " : "... " );
}

/**
 *	This is the constructor. It does not do any initialisation work, just
 *	puts the object into an initial sane state. Call startup to start
 *	the server.
 *
 *	@see startup
 */
PythonServer::PythonServer( std::string welcomeString ) :
	PyOutputWriter( "", false, &PythonServer::s_type_ ),
	pDispatcher_( NULL ),
	prevStderr_( NULL ),
	prevStdout_( NULL ),
	welcomeString_( welcomeString )
{
}

/**
 *	This is the destructor. It calls shutdown to ensure that the server
 *	has shutdown.
 *
 *	@see shutdown
 */
PythonServer::~PythonServer()
{
	this->shutdown();
}

/*~ class BigWorld.PythonServer
 *	This class provides access to the Python interpreter via a TCP connection.
 *	It starts listening on a given port, and creates a new PythonConnection
 *	for every connection it receives.
 */
PY_TYPEOBJECT( PythonServer )


/**
 *	This method starts up the Python server, and begins listening on the
 *	given port. It redirects Python stdout and stderr, so that they can be
 *	sent to all Python connections as well as stdout.
 *
 *	@param dispatcher The event dispatcher with which to register file descriptors.
 *	@param ip		The ip on which to listen.
 *	@param port		The port on which to listen.
 *	@param ifspec	If specified, the name of the interface to open the port on.
 */
bool PythonServer::startup( Mercury::EventDispatcher & dispatcher,
		uint32_t ip, uint16_t port, const char * ifspec )
{
	pDispatcher_ = &dispatcher;
	pSysModule_ = PyImport_ImportModule("sys");

	if (!pSysModule_)
	{
		ERROR_MSG( "PythonServer: Failed to import sys module\n" );
		return false;
	}

	prevStderr_ = PyObject_GetAttrString( pSysModule_, "stderr" );
	prevStdout_ = PyObject_GetAttrString( pSysModule_, "stdout" );

	PyObject_SetAttrString( pSysModule_, "stderr", (PyObject *)this );
	PyObject_SetAttrString( pSysModule_, "stdout", (PyObject *)this );

	listener_.socket(SOCK_STREAM);
	listener_.setnonblocking(true);

#ifndef _WIN32  // WIN32PORT
	// int val = 1;
	// setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));
#endif

	u_int32_t ifaddr = ip;
	if (ifspec)
	{	// try binding somewhere else if ifspec != 0
		// note that we never bind everywhere now ... oh well...
		//  prolly good for security that
		char ifname[IFNAMSIZ];
		listener_.findIndicatedInterface( ifspec, ifname );
		listener_.getInterfaceAddress( ifname, ip );
		// if anyone fails then ifaddr simple does not get set
	}
	if (listener_.bind( htons( port ), ifaddr ) == -1)
	{
		if (listener_.bind(0) == -1)
		{
			WARNING_MSG("PythonServer: Failed to bind to port %d\n", port);
			this->shutdown();
			return false;
		}
	}

	listener_.getlocaladdress(&port, NULL);
	port = ntohs(port);

	listen( listener_, 1 );
	pDispatcher_->registerFileDescriptor( listener_, this );

	INFO_MSG( "Python server is running on port %d\n", port );

	return true;
}

/**
 * 	This method shuts down the Python server.
 * 	It closes the listener port, disconnects all connections,
 * 	and restores Python stderr and stdout.
 */
void PythonServer::shutdown()
{
	std::vector<PythonConnection *>::iterator it;

	// Disconnect all connections, and clear our connection list.

	for(it = connections_.begin(); it != connections_.end(); it++)
	{
		delete *it;
	}

	connections_.clear();

	// Shutdown the listener socket if it is open.

	if (listener_.good())
	{
		MF_ASSERT( pDispatcher_ != NULL );
		pDispatcher_->deregisterFileDescriptor( (int)listener_ );
		listener_.close();
	}

	// If stderr and stdout were redirected, restore them.

	if (prevStderr_)
	{
		PyObject_SetAttrString(pSysModule_, "stderr", prevStderr_);
		Py_DECREF( prevStderr_ );
		prevStderr_ = NULL;
	}

	if (prevStdout_)
	{
		PyObject_SetAttrString(pSysModule_, "stdout", prevStdout_);
		Py_DECREF( prevStdout_ );
		prevStdout_ = NULL;
	}

	pDispatcher_ = NULL;
	pSysModule_ = NULL;
}


/**
 * 	This method is called by Python whenever there is new data for
 * 	stdout or stderror. We redirect it to all the connections, and
 * 	then print it out as normal. CRs are subsituted for CR/LF pairs
 * 	to facilitate printing on Windows.
 *
 * 	@param args		A Python tuple containing a single string argument
 */
void PythonServer::printMessage( const std::string & msg )
{
	PyObject * pResult =
		PyObject_CallMethod( prevStdout_, "write", "s#", msg.c_str(), msg.length() );
	if (pResult)
	{
		Py_DECREF( pResult );
	}
	else
	{
		PyErr_Clear();
	}

	std::string cookedMsg;
	cookedMsg.reserve( msg.size() );
	std::string::const_iterator iter;
	for (iter = msg.begin(); iter != msg.end(); iter++)
	{
		if (*iter == '\n')
			cookedMsg += "\r\n";
		else
			cookedMsg += *iter;
	}

	std::vector<PythonConnection *>::iterator it;

	for(it = connections_.begin(); it != connections_.end(); it++)
	{
		if ((*it)->active())
		{
			(*it)->write( cookedMsg.c_str() );
		}
	}
}

/**
 * 	This method deletes a connection from the python server.
 *
 *	@param pConnection	The connection to be deleted.
 */
void PythonServer::deleteConnection(PythonConnection* pConnection)
{
	std::vector<PythonConnection *>::iterator it;

	for(it = connections_.begin(); it != connections_.end(); it++)
	{
		if (*it == pConnection)
		{
			delete *it;
			connections_.erase(it);
			return;
		}
	}

	WARNING_MSG("PythonServer::deleteConnection: %p not found",
			pConnection);
}

/**
 *	This method is called by Mercury when our file descriptor is
 *	ready for reading.
 */
int PythonServer::handleInputNotification( int fd )
{
	(void)fd;
	MF_ASSERT(fd == (int)listener_);

	sockaddr_in addr;
	socklen_t size = sizeof(addr);

	int socket = accept(listener_, (sockaddr *)&addr, &size);


	if (socket == -1)
	{
		TRACE_MSG("PythonServer: Failed to accept connection: %d\n", errno);
		return 1;
	}

	TRACE_MSG("PythonServer: Accepted new connection from %s\n",
			inet_ntoa(addr.sin_addr));
	connections_.push_back(
			new PythonConnection( this, *pDispatcher_, socket, welcomeString_ ) );

	return 1;
}

/**
 * 	This method returns the port on which our file descriptor is listening.
 */
uint16_t PythonServer::port() const
{
	uint16_t port = 0;
	listener_.getlocaladdress( &port, NULL );
	port = ntohs( port );
	return port;
}


#endif // ENABLE_PYTHON_TELNET_SERVICE
