/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPTED_MODULE_HPP
#define SCRIPTED_MODULE_HPP

typedef struct _object PyObject;

#include "resmgr/datasection.hpp"

#include "module.hpp"

/**
 *	This class implements an application module that
 *	delegates all events to an associated script object.
 *
 *	This makes it easy to create unique application modules
 *	entirely in python.
 *
 *	The ScriptedModule is the reason there are the pyUpdate
 *	and pyRender utility methods defined in the WorldEditor script
 *	module.  ( this is an example of how to use scripted module )
 */
class ScriptedModule : public FrameworkModule
{
public:
	ScriptedModule();
	~ScriptedModule();

	bool init( DataSectionPtr pSection );

	virtual void onStart();
	virtual int  onStop();

	virtual void onPause();
	virtual void onResume( int exitCode );

	virtual bool updateState( float dTime );
	virtual void render( float dTime );

	virtual bool handleKeyEvent( const KeyEvent & /*event*/ );
	virtual bool handleMouseEvent( const MouseEvent & /*event*/ );

private:
	ScriptedModule( const ScriptedModule& );
	ScriptedModule& operator=( const ScriptedModule& );

	PyObject * pScriptObject_;
};



#ifdef CODE_INLINE
#include "scripted_module.ipp"
#endif

#endif // SCRIPTED_MODULE_HPP
