/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "module.hpp"
#include "options.hpp"

#include "cstdmf/dogwatch.hpp"
#include "moo/render_context.hpp"
#include "romp/console_manager.hpp"
#include "romp/geometrics.hpp"

#ifndef CODE_INLINE
#include "module.ipp"
#endif

static DogWatch	g_watchDrawConsole( "Draw Console" );

// -----------------------------------------------------------------------------
// Section: Module
// -----------------------------------------------------------------------------

/**
 *	Destructor.
 */
Module::~Module()
{
}


/**
 *	This method should be overridden by derived classes. It is called when this
 *	module initially is created by the module manager.
 */
bool Module::init( DataSectionPtr pSection )
{
	return true;
}


/**
 *	This method should be overridden by derived classes. It is called when this
 *	module initially becomes the active module.
 *
 *	@see onStop
 */
void Module::onStart()
{
}


/**
 *	This method should be overridden by derived classes. It is called when this
 *	module is removed from the module stack.
 *
 *	@see onStart
 *
 *	@param	exitCode	The exit status for this module.  This exit code
 *						is passed on to the module about to be resume()d
 */
int Module::onStop()
{
	return 0;
}


/**
 *	This method should be overridden by derived classes. It is called when this
 *	module temporarily stops being the active module when another module is
 *	placed on the stack.
 *
 *	@see onResume
 */
void Module::onPause()
{
}


/**
 *	This method should be overridden by derived classes. It is called when this
 *	module becomes the active module again after being paused.
 *
 *	@param exitCode		The exitCode of the module that has just left the stack.
 *
 *	@see onPause
 */
void Module::onResume( int exitCode )
{
}


// -----------------------------------------------------------------------------
// Section: FrameworkModule
// -----------------------------------------------------------------------------

/**
 *	This method overrides the module method to add some extra framework.
 */
void FrameworkModule::updateFrame( float dTime )
{
	bool shouldRender = this->updateState( dTime );
}


/**
 *	This method is meant to be overridden by derived classes. It is called when
 *	the module should update its state.
 *
 *	@param dTime	The time in seconds taken by the last frame.
 *
 *	@return	The method should return true if rendering should occur after this
 *		call and false otherwise.
 */
bool FrameworkModule::updateState( float dTime )
{
	return true;
}


/**
 *	This method is meant to be overridden by derived classes. It is called when
 *	the module should render itself. When this method is called, beginScene has
 *	already been called on the render device. endScene is called after this
 *	method has been called.
 *
 *	@param dTime	The time in seconds taken by the last frame.
 */
void FrameworkModule::render( float dTime )
{
}

// module.cpp
