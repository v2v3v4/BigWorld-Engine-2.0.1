/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"

#include "mailbox.hpp"

#include "autotrace.hpp"
#include "blocking_response_handler.hpp"
#include "retrying_remote_method.hpp"
#include "web_integration.hpp"

#include "baseapp/baseapp_int_interface.hpp"

#include "cstdmf/base64.h"

#include "entitydef/entity_description.hpp"
#include "entitydef/entity_description_map.hpp"

#include "network/bundle.hpp"

#include "server/backup_hash_chain.hpp"
#include "server/base_backup_switch_mailbox_visitor.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

// -----------------------------------------------------------------------------
// Section: WebEntityMailBox
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( WebEntityMailBox )

PY_BEGIN_METHODS( WebEntityMailBox )
	PY_METHOD( serialise )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( WebEntityMailBox )
	PY_ATTRIBUTE( id )
	PY_ATTRIBUTE( keepAliveSeconds )
PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( WebEntityMailBox )

// default keep alive interval for new mailboxes
uint32 WebEntityMailBox::s_defaultKeepAliveSeconds = 0;

/**
 *	Register with the mailbox factory.
 */
void WebEntityMailBox::initMailboxFactory()
{
	PyEntityMailBox::registerMailBoxComponentFactory(
		EntityMailBoxRef::BASE,
		WebEntityMailBox::createFromRef,
		&WebEntityMailBox::s_type_
	);

	PyEntityMailBox::registerMailBoxRefEquivalent(
		WebEntityMailBox::Check, WebEntityMailBox::staticRef );
}


/**
 *	Creates a new base entity mailbox. The reference must be to a remote base
 *	entity.
 *
 *	@param	ref entity mailbox reference to a remote base entity
 *	@return a new web entity mailbox
 */
PyObject * WebEntityMailBox::createFromRef( const EntityMailBoxRef & ref )
{
	return new WebEntityMailBox( ref );
}

/**
 *	Constructor
 *
 *	@param	ref	the entity mailbox reference to the remote entity
 */
WebEntityMailBox::WebEntityMailBox( const EntityMailBoxRef& ref ):
		PyEntityMailBox( &WebEntityMailBox::s_type_ ),
		ref_( ref ),
		pBundle_( NULL ),
		keepAliveSeconds_( s_defaultKeepAliveSeconds )
{
	// Remap the mailbox if need be.
	const BackupHashChain & hashChain = 
		WebIntegration::instance().baseAppBackupHashChain();
	Mercury::Address newAddr = hashChain.addressFor( ref_.addr, ref_.id );
	this->address( newAddr );

	this->sendKeepAlive();
}


/**
 *	Sends the keep-alive message with the configured keep-alive interval
 *	to the base.
 */
void WebEntityMailBox::sendKeepAlive()
{
	if (!keepAliveSeconds_)
	{
		return;
	}

	// Send keep alive with the configured keep alive interval period

	Mercury::Bundle b;

	b.startMessage( BaseAppIntInterface::setClient );
	b << ( EntityID ) ref_.id;

	BaseAppIntInterface::startKeepAliveArgs & startKeepAliveArgs =
		BaseAppIntInterface::startKeepAliveArgs::start( b );
	startKeepAliveArgs.interval = keepAliveSeconds_;

	Mercury::NetworkInterface & networkInterface = 
		WebIntegration::instance().interface();

	networkInterface.send( ref_.addr, b );
	networkInterface.processUntilChannelsEmpty();
}


/**
 *	Destructor.
 */
WebEntityMailBox::~WebEntityMailBox()
{
	if (pBundle_)
	{
		delete pBundle_;
	}
}


/**
 *	Overridden from PyEntityMailBox.
 *
 *	@param	methodName
 */
const MethodDescription* WebEntityMailBox::findMethod(
	const char* methodName ) const
{
	const EntityDescriptionMap & entities =
		WebIntegration::instance().entityDescriptions();

	const EntityDescription & entity =
		entities.entityDescription( ref_.type() );
	const EntityMethodDescriptions & baseMethods = entity.base();
	MethodDescription* pMethod = baseMethods.find( std::string( methodName ) );

	return pMethod;
}


/**
 *	Overridden from PyEntityMailBox. Returns an initialised stream particular
 *	to a remote method.
 *
 *	@param	methodDesc the method description
 */
BinaryOStream* WebEntityMailBox::getStream( 
		const MethodDescription & methodDesc, 
		Mercury::ReplyMessageHandler * pHandler /*=NULL*/ )
{
	if (pBundle_)
	{
		delete pBundle_;
	}
	pBundle_ = new Mercury::Bundle();

	Mercury::Bundle & b = *pBundle_;

	b.startMessage( BaseAppIntInterface::setClient );
	b << ( EntityID ) ref_.id;

	// Send keep alive with the configured keep alive interval period
	if (keepAliveSeconds_)
	{
		BaseAppIntInterface::startKeepAliveArgs & startKeepAliveArgs =
			BaseAppIntInterface::startKeepAliveArgs::start( b );
		startKeepAliveArgs.interval = keepAliveSeconds_;
	}

	if (pHandler)
	{
		b.startRequest( BaseAppIntInterface::callBaseMethod, 
			pHandler );
	}
	else
	{
		b.startMessage( BaseAppIntInterface::callBaseMethod );
	}

	b << uint16( methodDesc.internalIndex() );

	return pBundle_;
}


/**
 *	Overridden from PyEntityMailBox.
 */
void WebEntityMailBox::sendStream()
{
	if (!pBundle_)
	{
		ERROR_MSG( "WebEntityMailBox::sendStream: No stream to send!\n" );
		return;
	}

	Mercury::NetworkInterface & interface = 
		WebIntegration::instance().interface();

	interface.send( ref_.addr, *pBundle_ );

	delete pBundle_;
	pBundle_ = NULL;
}


/**
 *	Overridden from PyEntityMailBox.
 */
PyObject * WebEntityMailBox::pyRepr()
{
	const EntityDescriptionMap & entityMap =
		WebIntegration::instance().entityDescriptions();
	const EntityDescription& entityDesc =
		entityMap.entityDescription( ref_.type() );

	const char * location = "???";
	switch (ref_.component())
	{
		case EntityMailBoxRef::CELL:
			location = "Cell";
		break;
		case EntityMailBoxRef::BASE:
			location = "Base";
		break;
		case EntityMailBoxRef::CLIENT:
			location = "Client";
		break;
		case EntityMailBoxRef::BASE_VIA_CELL:
			location = "BaseViaCell";
		break;
		case EntityMailBoxRef::CLIENT_VIA_CELL:
			location = "ClientViaCell";
		break;
		case EntityMailBoxRef::CELL_VIA_BASE:
			location = "CellViaBase";
		break;
		case EntityMailBoxRef::CLIENT_VIA_BASE:
			location = "ClientViaBase";
		break;

		default: break;
	}

	return PyString_FromFormat( "%s mailbox id: %d type: %s[%u] addr: %s",
		location, ref_.id,
		entityDesc.name().c_str(), ref_.type(), ref_.addr.c_str() );
}


/**
 *	Returns the entity mailbox reference for this mailbox.
 */
EntityMailBoxRef WebEntityMailBox::ref() const
{
	return ref_;
}

/**
 *	PyObjectPlus's pyGetAttribute method.
 */
PyObject * WebEntityMailBox::pyGetAttribute( const char * attr )
{
	// Note: This is basically the same as the implementation present in
	// PyObjectPlus It uses the PY_SETATTR_STD macro which iterates through
	// s_attributes_, a class static instance which is used with
	// PY_RW_ATTRIBUTE_* and PY_RO_ATTRIBUTE_* macros.
	//
	// Calling PyObjectPlus's pyGetAttribute is not enough to get the
	// attributes we want, as it will be using PyObjectPlus's s_attributes_
	// instead of WebEntityMailBox's.

	const MethodDescription * pDescription = this->findMethod( attr );
	if (pDescription != NULL)
	{
		return new RetryingRemoteMethod( this, pDescription );
	}

	PY_GETATTR_STD();

	return PyEntityMailBox::pyGetAttribute( attr );
}

/**
 *	PyObjectPlus's pySetAttribute method.
 */
int WebEntityMailBox::pySetAttribute( const char * attr, PyObject * value )
{
	// See note in pyGetAttribute above.
	PY_SETATTR_STD();

	return PyEntityMailBox::pySetAttribute( attr, value );
}

/**
 *	Set the keep-alive interval.
 *
 *	@param value interval in seconds.
 */
void WebEntityMailBox::keepAliveSeconds( uint32 value )
{
	keepAliveSeconds_ = value;
	this->sendKeepAlive();
}


/**
 *	This methods serialises a mailbox's data to a string, so that it can be
 *	recreated with WebEntityMailBox::deserialise(). It actually uses base64
 *	so that it can be safely passed as a null-terminated C-string.
 *
 *	@see WebEntityMailBox::deserialise()
 *	@return	a new reference to a Python string object containing a
 *	serialised mailbox string
 */
PyObject * WebEntityMailBox::serialise()
{
	PyObjectPtr pickleModule( PyImport_ImportModule( "cPickle" ),
		PyObjectPtr::STEAL_REFERENCE );
	if (!pickleModule)
	{
		return NULL;
	}

	MemoryOStream serialised;

	serialised << keepAliveSeconds_ << ref_;

	const char * data = reinterpret_cast<char *>( serialised.data() );
	return PyString_FromString(
		Base64::encode( data, serialised.size() ).c_str() );
}


/**
 *	This method deserialises a serialised mailbox string and recreates a
 *	mailbox object.
 *
 *	@see WebEntityMailBox::serialise()
 *	@return a new reference to the Python mailbox as represented in the
 *	serialised mailbox string , or NULL or the given string was an
 *	invalid serialised mailbox string.
 */
PyObject * WebEntityMailBox::deserialise( const std::string & serialisedB64 )
{
	std::string serialisedData;
	if (-1 == Base64::decode( serialisedB64, serialisedData ) ||
			serialisedData.size() <
				sizeof( uint32 ) + sizeof( EntityMailBoxRef ))
	{
		PyErr_SetString( PyExc_ValueError,
			"invalid mailbox serialised string" );
		return NULL;
	}

	// first 4 bytes is keep alive seconds value
	uint32 keepAliveSeconds = 0;

	// the rest is the mailbox reference
	EntityMailBoxRef ref;

	MemoryIStream serialised( serialisedData.data(), serialisedData.size() );

	serialised >> keepAliveSeconds >> ref;

	// Returns a new reference
	WebEntityMailBox * mailbox = new WebEntityMailBox( ref );

	mailbox->keepAliveSeconds( keepAliveSeconds );

	return mailbox;
}

PY_MODULE_STATIC_METHOD( WebEntityMailBox, deserialise, BigWorld )


/**
 *	Remap the mailboxes according to the new hash chain.
 */
/*static*/
void WebEntityMailBox::remapMailBoxes( const BackupHashChain & hashChain )
{
	BaseBackupSwitchMailBoxVisitor visitor( hashChain );
	PyEntityMailBox::visit( visitor );
}


/**
 *	Return the address of the mailbox.
 */
const Mercury::Address WebEntityMailBox::address() const
{
	return ref_.addr;
}


/**
 *	Reset the address of the mailbox.
 */
void WebEntityMailBox::address( const Mercury::Address & newAddr )
{
	// Can't just assign the address because we want to preserve the salt, as
	// it contains information about the mailbox type and entity type.
	ref_.addr.ip = newAddr.ip;
	ref_.addr.port = newAddr.port;
}


// mailbox.cpp
