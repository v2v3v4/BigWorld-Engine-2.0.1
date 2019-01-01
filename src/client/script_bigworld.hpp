/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/*~ module BigWorld
 *
 *	The BigWorld Module is a Python module that provides the world related
 *	interfaces to the client. It is used to find out application specific
 *	information, such as unique, special-case Entities, connecting Entities and
 *	other game objects by their respective IDs. It is used to provide
 *	information on game world state from the server.
 *
 */
#ifndef SCRIPT_BIGWORLD_HPP
#define SCRIPT_BIGWORLD_HPP

#include <Python.h>

#include "resmgr/datasection.hpp"

class BaseCamera;
class ProjectionAccess;
class XConsole;
class InputEvent;

/**
 *	This namespace contains functions exclusive to the scripting of the client.
 */
namespace BigWorldClientScript
{
	bool init( DataSectionPtr engineConfig );
	void fini();
	void tick();

	bool sinkKeyboardEvent( const InputEvent& event );
	
	void clearSpaces();
	
	void getPythonConsoles( XConsole *& pOutConsole, XConsole *& pErrConsole );
	void setPythonConsoles( XConsole *  pOutConsole, XConsole *  pErrConsole );

	// TODO:PM This is probably not the best place for this.
	bool addAlert( const char * alertType, const char * msg );
}

#endif // SCRIPT_BIGWORLD_HPP
