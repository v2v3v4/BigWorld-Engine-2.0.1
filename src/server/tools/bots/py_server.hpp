/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_SERVER_HPP
#define PY_SERVER_HPP

#include <string>

#include "entitydef/entity_description.hpp"
#include "pyscript/pyobject_plus.hpp"

class Entity;

/*~ class BigWorld.PyServer
 *  A PyServer is the device through which an entity is able to call methods
 *  on it's instance on a server. BigWorld creates two PyServer objects
 *  as attributes for each client entity, "cell" and "base", which can be
 *  used to send method calls to the entity's cell and base instances,
 *  respectively. These cannot be created in script.
 *
 *  PyServer objects have no default attributes or methods, however BigWorld
 *  populates them with ServerCaller objects which represent the methods that
 *  can be called. These are built using information provided in the entity
 *  def files. A ServerCaller object can then be called as if it were the
 *  method that it represents, which causes it to add a request to call the
 *  appropriate function on the next bundle that is sent to the server.
 *
 *  This is not supported for licensees who have not licensed the BigWorld
 *  server technology.
 */
/**
 *	This class presents the interface of the server part of an
 *	entity to the scripts that run on the client.
 */
class PyServer : public PyObjectPlus
{
	Py_Header( PyServer, PyObjectPlus )

public:
	PyServer( Entity * pEntity,
		const EntityMethodDescriptions & methods,
		bool isProxyCaller,
		PyTypePlus * pType = &PyServer::s_type_ );
	~PyServer();

	PyObject * pyGetAttribute( const char * attr );

	PyObject * pyAdditionalMembers( PyObject * pBaseSeq );

	void disown();

private:
	Entity * pEntity_;
	const EntityMethodDescriptions & methods_;
	bool isProxyCaller_;
};

#endif // PY_SERVER_HPP
