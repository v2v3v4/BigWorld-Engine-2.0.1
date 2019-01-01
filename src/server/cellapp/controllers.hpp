/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLLERS_HPP
#define CONTROLLERS_HPP

#include "controller.hpp"

#include <map>

class BinaryIStream;
class BinaryOStream;
class Entity;

#include "Python.h"

class Controllers
{
public:
	Controllers();
	~Controllers();

	void readGhostsFromStream( BinaryIStream & data, Entity * pEntity );
	void readRealsFromStream( BinaryIStream & data, Entity * pEntity );

	void writeGhostsToStream( BinaryOStream & data );
	void writeRealsToStream( BinaryOStream & data );

	void createGhost( BinaryIStream & data, Entity * pEntity );
	void deleteGhost( BinaryIStream & data, Entity * pEntity );
	void updateGhost( BinaryIStream & data );

	ControllerID addController( ControllerPtr pController, int userArg,
			Entity * pEntity );
	bool delController( ControllerID id, Entity * pEntity,
			bool warnOnFailure = true );
	void modController( ControllerPtr pController, Entity * pEntity );

	void startReals();
	void stopReals( bool isFinalStop );

	PyObject * py_cancel( PyObject * args, Entity * pEntity );

private:
	ControllerID nextControllerID();

	typedef std::map< ControllerID, ControllerPtr > Container;
	Container container_;

	ControllerID lastAllocatedID_;
};

#endif // CONTROLLERS_HPP
