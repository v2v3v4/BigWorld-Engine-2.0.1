/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MODULE_HPP
#define MODULE_HPP

#include "input/input.hpp"
#include "resmgr/datasection.hpp"

/**
 *	Module is the interface for all application modules.
 */
class Module : public InputHandler, public ReferenceCount
{
public:
	Module() {};

	virtual ~Module();

	// This is called when the module manager creates a module.
	virtual bool init( DataSectionPtr pSection );

	// These are called when a module enters or leaves the stack.
	virtual void onStart();
	virtual int  onStop();

	// These are called when a module stays, or stayed on the stack.
	virtual void onPause();
	virtual void onResume( int exitCode );

	/**
	 *	This pure virtual method is called each frame when this module is the
	 *	active one.
	 *
	 *	@dTime	The time taken for the last frame.
	 */
	virtual void updateFrame( float dTime ) = 0;
	virtual void render( float dTime ) = 0;
};


/**
 *	This class extends module with a simple framework for rendering. It handles
 *	the beginScene and endScene calls and also displays the console manager.
 */
class FrameworkModule : public Module
{
protected:
	// Override from Module
	virtual void updateFrame( float dTime );

	// Extended methods
	virtual bool updateState( float dTime );
	virtual void render( float dTime );

private:
	uint32	backgroundColour_;
};

#ifdef CODE_INLINE
#include "module.ipp"
#endif

#endif // MODULE_HPP
