/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// This file contains mainLoopAction, which is called every tick. It is used to
// add simple functionality to client2.

#include "../client2/entity.hpp"

#include "connection/server_connection.hpp"

void callMethod( ServerConnection & serverConnection,
					  Entity * pPlayer,
					  const char * methodName,
					  bool isOnCell )
{
	if (pPlayer == NULL)
	{
		std::cout << "Player is NULL" << std::endl;
		return;
	}

	// find the type of entity that this is
	const EntityType& avatarType = pPlayer->type();
	// grab the description of this type
	const EntityDescription& avatarDescription = 
		avatarType.description();
	// get the methods on the server (cell) that this entity supports
	const EntityMethodDescriptions & methods = 
		isOnCell ? avatarDescription.cell() : avatarDescription.base();

	// find an interesting function
	MethodDescription * pFunctionNoParams = methods.find( methodName );

	if (pFunctionNoParams != NULL)
	{
		// fetch the corresponding message id
		const int msgID = pFunctionNoParams->exposedIndex();
		// work out our python arguments... we choose none
		PyObject * args = PyTuple_New(0);
		// check if arguments are valid
		if ( pFunctionNoParams->areValidArgs( false, args, false ) )
		{
			// begin a message call
			// use startEntityMessage if we want to send to another entity 
			// (instead of Avatar)
			BinaryOStream& stream = 
				isOnCell ?
					serverConnection.startAvatarMessage( msgID ) :
					serverConnection.startProxyMessage( msgID );
			// stream on the parameters (none)
			bool ok = pFunctionNoParams->addToStream( false, args, stream );

			if (!ok)
				std::cout << "ok: " << ok << std::endl;
		}
		else
		{
			std::cout << "invalid args" << std::endl;
		}
		// cleanup arguments
		Py_DECREF(args);
	}
	else
	{
		std::cout << avatarDescription.name()
			<< " has no " <<
			(isOnCell ? "cell" : "base") << " method " << methodName << std::endl;
	}
}


void mainLoopAction( ServerConnection & serverConnection,
					  Entity * pPlayer )
{
	static int count = 0;

	if (++count >= 10)
	{
		count = 0;
		callMethod( serverConnection, pPlayer, "dummyFunctionNoParams", true );
	}
}

void shutdownAction( ServerConnection & serverConnection,
					Entity * pPlayer )
{
	std::cout << "Sending logOff" << std::endl;
	callMethod( serverConnection, pPlayer, "logOff", false );
	serverConnection.send();
}

// main_loop_action.cpp
