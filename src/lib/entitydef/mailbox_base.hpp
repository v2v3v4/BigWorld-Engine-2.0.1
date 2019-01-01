/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MAILBOX_BASE_HPP
#define MAILBOX_BASE_HPP

#include "network/basictypes.hpp"
#include "cstdmf/binary_stream.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <list>

class EntityType;
class MethodDescription;
class PyEntityMailBox;

namespace Mercury
{
class ReplyMessageHandler;
}


/**
 *	Interface for visiting mailboxes in PyEntityMailBox::visit()
 */
class PyEntityMailBoxVisitor
{
public:

	/**
	 *	Constructor. 
	 */
	PyEntityMailBoxVisitor() {}


	/** 
	 *	Destructor. 
	 */
	virtual ~PyEntityMailBoxVisitor() {}


	/**
	 *	Virtual method for visiting a single mail box.
	 */
	virtual void onMailBox( PyEntityMailBox * pMailBox ) = 0;
};


/**
 *	This class is used to represent a destination of an entity that messages
 *	can be sent to.
 *
 *	Its virtual methods are implemented differently on each component.
 */
class PyEntityMailBox: public PyObjectPlus
{
	Py_Header( PyEntityMailBox, PyObjectPlus )

public:
	PyEntityMailBox( PyTypePlus * pType = &PyEntityMailBox::s_type_ );
	virtual ~PyEntityMailBox();

	virtual PyObject * pyGetAttribute( const char * attr );

	PyObject * pyRepr();

	virtual const MethodDescription * findMethod( const char * attr ) const = 0;

	/**
	 *	Get a stream for the remote method to add arguments to. 
	 *
	 *	@param methodDesc	The method description.
	 *	@param pHandler		If the method requires a request, this is the
	 *						reply handler to use.
	 */
	virtual BinaryOStream * getStream( const MethodDescription & methodDesc, 
			Mercury::ReplyMessageHandler * pHandler = NULL ) = 0;
	virtual void sendStream() = 0;
	static PyObject * constructFromRef( const EntityMailBoxRef & ref );
	static bool reducibleToRef( PyObject * pObject );
	static EntityMailBoxRef reduceToRef( PyObject * pObject );
	
	virtual EntityID id() const = 0;
	virtual void address( const Mercury::Address & addr ) = 0;
	virtual const Mercury::Address address() const = 0;

	virtual void migrate() {}

	typedef PyObject * (*FactoryFn)( const EntityMailBoxRef & ref );
	static void registerMailBoxComponentFactory(
		EntityMailBoxRef::Component c, FactoryFn fn,
		PyTypeObject * pType );

	typedef bool (*CheckFn)( PyObject * pObject );
	typedef EntityMailBoxRef (*ExtractFn)( PyObject * pObject );
	static void registerMailBoxRefEquivalent( CheckFn cf, ExtractFn ef );

	PY_PICKLING_METHOD_DECLARE( MailBox )

	static void visit( PyEntityMailBoxVisitor & visitor );

private:
	typedef std::list< PyEntityMailBox * > Population;
	static Population s_population_;

	Population::iterator populationIter_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyEntityMailBox )

namespace Script
{
	int setData( PyObject * pObj, EntityMailBoxRef & mbr,
		const char * varName = "" );
	PyObject * getData( const EntityMailBoxRef & mbr );
};

#endif // MAILBOX_BASE_HPP
