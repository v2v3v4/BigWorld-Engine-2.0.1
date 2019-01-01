/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_SCRIPT_HPP
#define WORLD_EDITOR_SCRIPT_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include <Python.h>


/**
 *	This namespace contains functions relating to scripting WorldEditor.
 */
namespace WorldEditorScript
{
	bool init( DataSectionPtr pDataSection );
	void fini();
	std::string spaceName();
}


#ifdef CODE_INLINE
#include "world_editor_script.ipp"
#endif


#endif // WORLD_EDITOR_SCRIPT_HPP
