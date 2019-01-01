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


#include <Python.h>
#include <deque>
#include <string>

typedef unsigned short uint16_t;

#include "network/interfaces.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "network/endpoint.hpp"
#include "pyscript/script.hpp"

namespace Mercury
{
class EventDispatcher;
}

class PythonServer;

/**
 *	This class implements a subset of the telnet protocol.
 */
class TelnetConnection : public Mercury::InputNotificationHandler
{
public:
	TelnetConnection( Mercury::EventDispatcher & dispatcher, int fd );
	virtual ~TelnetConnection();

	int 						handleInputNotification(int fd);
	void 						write(const char* str);
	bool						active() const	{ return active_; }

protected:
	/**
	 *	Handle a character.
	 *	Return false if you need more input.
	 *	Otherwise remove the input you use and return true.
	 */
	virtual bool				handleChar()
		{ readBuffer_.pop_front(); return false; }
	virtual bool				handleVTCommand()
		{ return this->handleChar(); }
	virtual void				connectionBad() = 0;

	std::deque<unsigned char>	readBuffer_;
	bool						active_;

private:
	Mercury::EventDispatcher *	pDispatcher_;
	Endpoint					socket_;
	bool						handleTelnetCommand();

	bool						telnetSubnegotiation_;
};

/**
 *	This class represents a single TCP connection to the Python interpreter.
 */
class PythonConnection : public TelnetConnection
{
public:
	PythonConnection( PythonServer* owner,
		Mercury::EventDispatcher & dispatcher, int fd);
	~PythonConnection();

	void						writePrompt();

private:
	virtual bool				handleVTCommand();
	virtual bool				handleChar();
	virtual void				connectionBad();

	void						handleLine();
	void						handleDel();
	void						handlePrintableChar();
	void						handleUp();
	void						handleDown();
	void						handleLeft();
	void						handleRight();

	std::deque<std::string>		historyBuffer_;
	std::string					currentLine_;
	std::string					currentCommand_;
	PythonServer*				owner_;
	int							historyPos_;
	unsigned int				charPos_;
};


class KeyboardConnection;

/**
 *	This class provides access to the Python interpreter via a TCP connection.
 *	It starts listening on a given port, and creates a new PythonConnection
 *	for every connection it receives.
 */
class PythonServer :
	public PyObjectPlus,
	public Mercury::InputNotificationHandler
{
	Py_Header( PythonServer, PyObjectPlus );

public:
	PythonServer();
	virtual ~PythonServer();

	bool			startup( Mercury::EventDispatcher & dispatcher,
						uint16_t port);
	void			shutdown();
	PyObject*		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject* value );
	void			deleteConnection(TelnetConnection* pConnection);
	uint16_t		port() const;

	void			pollInput();

private:
	int	handleInputNotification(int fd);
	PY_METHOD_DECLARE(py_write);

	PY_RW_ATTRIBUTE_DECLARE(softspace_, softspace);

	std::vector<PythonConnection*> connections_;

	Endpoint		listener_;
	Mercury::EventDispatcher *	pDispatcher_;
	PyObject*		prevStderr_;
	PyObject*		prevStdout_;
	PyObject*		pSysModule_;
	int				softspace_;

	Endpoint		kbListener_;
	std::vector<KeyboardConnection*> kbConnections_;
};


#endif // ENABLE_PYTHON_TELNET_SERVICE


#endif // _PYTHON_SERVER_HEADER
