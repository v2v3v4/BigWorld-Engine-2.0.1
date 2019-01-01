/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __MAILBOX_HPP__
#define __MAILBOX_HPP__

#include "Python.h"

#include "entitydef/mailbox_base.hpp"

#include <set>

class BackupHashChain;
class BlockingResponseHandler;

namespace Mercury
{
	class Bundle;
	class ReplyMessageHandler;
}

/**
 *	Mailbox to a remote base entity. Method calls on this object block until the
 *	return values are received, if any.
 */
class WebEntityMailBox : public PyEntityMailBox
{
	// Python object declarations
	Py_Header( WebEntityMailBox, PyEntityMailBox )


public:
	WebEntityMailBox( const EntityMailBoxRef & ref );
	virtual ~WebEntityMailBox();

	// virtuals from PyEntityMailBox
	virtual EntityID id() const 
		{ return ref_.id; }

	virtual const Mercury::Address address() const;
	virtual void address( const Mercury::Address & addr );

	virtual const MethodDescription* findMethod( const char* ) const;
	virtual BinaryOStream* getStream( const MethodDescription &, 
		Mercury::ReplyMessageHandler * pHandler = NULL );
	virtual void sendStream();
	virtual PyObject * pyRepr();
	
	// virtuals from PyObjectPlus
	virtual PyObject * pyGetAttribute( const char * attr );
	virtual int pySetAttribute( const char * attr, PyObject * value );

	// public methods
	static void initMailboxFactory();

	static PyObject * createFromRef( const EntityMailBoxRef & ref );

	EntityMailBoxRef ref() const;

	static EntityMailBoxRef staticRef( PyObject * pThis )
	{ return static_cast< const WebEntityMailBox * >( pThis )->ref(); }

	static uint32 defaultKeepAliveSeconds()
	{	return s_defaultKeepAliveSeconds; }

	static void defaultKeepAliveSeconds( uint32 newValue )
	{	s_defaultKeepAliveSeconds = newValue; }

	uint32 keepAliveSeconds() { return keepAliveSeconds_; }
	void keepAliveSeconds( uint32 value );

	// Python attributes
	PY_RO_ATTRIBUTE_DECLARE( ref_.id, id );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( uint32, keepAliveSeconds,
		keepAliveSeconds );

	PyObject * serialise();
	PY_AUTO_METHOD_DECLARE( RETOWN, serialise, END )

	static PyObject * deserialise( const std::string & serialised );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETOWN, deserialise,
		ARG( std::string, END )  )

	static void remapMailBoxes( const BackupHashChain & hashChain );
	
private:
	void sendKeepAlive();


private:
	EntityMailBoxRef 			ref_;
	Mercury::Bundle * 			pBundle_;
	uint32 						keepAliveSeconds_;

	static uint32				s_defaultKeepAliveSeconds;
};

PY_SCRIPT_CONVERTERS_DECLARE( WebEntityMailBox )

#endif // __MAILBOX_HPP__
