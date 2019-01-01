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

#include "gui_shader.hpp"

#ifndef CODE_INLINE
#include "gui_shader.ipp"
#endif

#include "cstdmf/debug.hpp"
DECLARE_DEBUG_COMPONENT2( "2DComponents", 0 )


PY_TYPEOBJECT( GUIShader )

PY_BEGIN_METHODS( GUIShader )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( GUIShader )
PY_END_ATTRIBUTES()


template <> GUIShaderFactory::ObjectMap * GUIShaderFactory::pMap_;



/**
 *	Constructor
 */
GUIShader::GUIShader( PyTypePlus * pType )
:PyObjectPlus( pType )
{
	BW_GUARD;	
}


/**
 *	Destructor
 */
GUIShader::~GUIShader()
{
	BW_GUARD;	
}


/**
 *	This method processes a GUI component, applying this shader to its vertices
 *
 *	@param component	The component to process
 *	@param dTime		The delta time for the current frame of the application
 *
 *	@return false		To stop processing child components.
 */
bool
GUIShader::processComponent( SimpleGUIComponent& component, float dTime )
{
	BW_GUARD;
	return false;
}

// gui_shader.cpp
