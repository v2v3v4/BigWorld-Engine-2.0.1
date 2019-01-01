/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLIENT_ENTITY_MAILBOX_HPP
#define CLIENT_ENTITY_MAILBOX_HPP

#include "entitydef/mailbox_base.hpp"

class EntityDescription;
class MethodDescription;
class Proxy;

/**
 *	This class is a mailbox that delivers to the client.
 */
class ClientEntityMailBox: public PyEntityMailBox
{
public:
	ClientEntityMailBox( Proxy & proxy ) :
		PyEntityMailBox(),
		proxy_( proxy ) {}

	virtual EntityID id() const;

	virtual void address( const Mercury::Address & address ) {}
	virtual const Mercury::Address address() const;

	virtual PyObject * pyGetAttribute( const char * attr );
	virtual BinaryOStream * getStream( const MethodDescription & methodDesc, 
		Mercury::ReplyMessageHandler * pHandler = NULL );
	virtual void sendStream();
	virtual const MethodDescription * findMethod( const char * attr ) const;

	BinaryOStream * getStreamForEntityID( int methodID, EntityID entityID );

	const EntityDescription& getEntityDescription() const;

	EntityMailBoxRef ref() const;
	static EntityMailBoxRef static_ref( PyObject * pThis )
		{ return ((const ClientEntityMailBox*)pThis)->ref(); }


private:
	Proxy & proxy_;
};

#endif // CLIENT_ENTITY_MAILBOX_HPP
