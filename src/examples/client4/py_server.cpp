/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

//#include "pch.hpp"

#ifdef _WIN32
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#endif

#include "py_server.hpp"

#include "entity.hpp"
#include "entity_type.hpp"

#include "connection/server_connection.hpp"

#include "entitydef/method_description.hpp"

#include "pyscript/script.hpp"

extern EntityID g_playerID;
extern ServerConnection * g_serverConnection;

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

// -----------------------------------------------------------------------------
// Section: ServerCaller
// -----------------------------------------------------------------------------

/**
 *	This class implements a simple helper Python type. Objects of this type are
 *	used to represent methods that the client can call on the server.
 */
class ServerCaller : public PyObjectPlus
{
	Py_Header( ServerCaller, PyObjectPlus );

public:
	/**
	 *	This constructor is used to initialise a ServerCaller.
	 *
	 *	@param entity		The entity the method is being called on.
	 *	@param pDescription	The description associated with the method to call.
	 */
	ServerCaller( Entity & entity, const MethodDescription * pDescription,
			bool isProxyCaller,
			PyTypePlus * pType = &ServerCaller::s_type_ ) :
		PyObjectPlus( pType ),
		entity_( entity ),
		pDescription_( pDescription ),
		isProxyCaller_( isProxyCaller )
	{
		Py_INCREF( &entity_ );
	}

	/**
	 *	Destructor.
	 */
	~ServerCaller()
	{
		Py_DECREF( &entity_ );
	}

	PY_METHOD_DECLARE( pyCall );


private:
	Entity & entity_;
	const MethodDescription * pDescription_;
	bool isProxyCaller_;
};


/**
 *	This method is called with this object is "called" in script. This is what
 *	makes this object a functor. It is responsible for putting the call on the
 *	bundle that will be sent to the server.
 *
 *	@param args	A tuple containing the arguments of the call.
 *
 *	@return		Py_None on success and NULL on failure. If an error occurs, the
 *				Python error string is set.
 */
PyObject * ServerCaller::pyCall( PyObject * args )
{
	if (g_serverConnection != NULL && entity_.id() < (1L<<30) && g_serverConnection->online())
	{
		if (!pDescription_->areValidArgs( false, args, true ))
		{
			return NULL;	// exception already set by areValidArgs
		}

		BinaryOStream	* pStream;
		const int msgID = pDescription_->exposedIndex();

		if (isProxyCaller_)
		{
			pStream = &g_serverConnection->startProxyMessage( msgID );
		}
		else
		{
			if (g_playerID == entity_.id())
			{
				pStream = &g_serverConnection->startAvatarMessage( msgID );
			}
			else
			{
				pStream = &g_serverConnection->startEntityMessage( msgID, entity_.id() );
			}
		}

		bool ok = pDescription_->addToStream( false, args, *pStream );
		if (!ok)
		{
			PyErr_Format( PyExc_SystemError,
				"Unexpected error in call to %s",
				pDescription_->name().c_str() );
			return NULL;
		}
	}
	// we don't actually throw an error if there's no server... or else
	// we'd have to have a whole collection of 'offline' scripts as well.

	Py_Return;
}

PY_TYPEOBJECT_WITH_CALL( ServerCaller )

PY_BEGIN_METHODS( ServerCaller )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( ServerCaller )
PY_END_ATTRIBUTES()




// -----------------------------------------------------------------------------
// Section: PyServer
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyServer )

PY_BEGIN_METHODS( PyServer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyServer )
PY_END_ATTRIBUTES()

///	Constructor
PyServer::PyServer( Entity * pEntity,
			const EntityMethodDescriptions & methods,
			bool isProxyCaller,
			PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pEntity_( pEntity ),
	methods_( methods ),
	isProxyCaller_( isProxyCaller )
{
}

///  Destructor
PyServer::~PyServer()
{
}

/**
 *	This method returns the attributes associated with this object. This is
 *	actually a dummy object. It returns a method object that is used to make a
 *	call to the server.
 */
PyObject * PyServer::pyGetAttribute( const char * attr )
{
	const MethodDescription * pDescription = methods_.find( attr );
	if (pDescription != NULL)
	{
		if (!pDescription->isExposed())
		{
			PyErr_Format( PyExc_TypeError, "%s.%s is not an exposed method",
				pEntity_->type().name().c_str(), pDescription->name().c_str() );
			return NULL;
		}
		if (pEntity_ == NULL)
		{
			PyErr_Format( PyExc_TypeError, "%s.%s is no longer callable since "
				"its entity no longer exists on this client.",
				pEntity_->type().name().c_str(), pDescription->name().c_str() );
			return NULL;
		}

		return new ServerCaller( *pEntity_, pDescription, isProxyCaller_ );
	}

	PY_GETATTR_STD();

	return this->PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Return additional members
 */
PyObject * PyServer::pyAdditionalMembers( PyObject * pBaseSeq )
{
	uint sz = PySequence_Size( pBaseSeq );
	int extra = 0;
	for (uint i = 0; i < methods_.size(); i++)
	{
		if (methods_.internalMethod( i )->isExposed())
			extra++;
	}

	PyObject * pTuple = PyTuple_New( sz + extra );
	for (uint i = 0; i < sz; i++)
	{
		PyTuple_SetItem( pTuple, i, PySequence_GetItem( pBaseSeq, i ) );
	}

	int index = sz;
	for (uint i = 0; i < methods_.size(); i++)
	{
		if (methods_.internalMethod( i )->isExposed())
		{
			PyTuple_SetItem( pTuple, index++,
				Script::getData( methods_.internalMethod( i )->name() ) );
		}
	}

	MF_ASSERT( index == PyTuple_GET_SIZE( pTuple ) );

	return pTuple;
}

/**
 *	Our entity no longer wants us <sob>!
 */
void PyServer::disown()
{
	pEntity_ = NULL;
}


// py_server.cpp
