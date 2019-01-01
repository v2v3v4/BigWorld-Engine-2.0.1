/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PLUGIN_LIBRARY_HPP
#define PLUGIN_LIBRARY_HPP


/**
 *	This namespace contains functions related to plugin libraries.
 */
namespace PluginLibrary
{
	void loadAllFromDirRelativeToApp(
		bool prefixWithAppName, const char * partialDir );
};


#endif // PLUGIN_LIBRARY_HPP
