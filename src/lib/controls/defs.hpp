/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_DEFS_HPP
#define CONTROLS_DEFS_HPP

//
// Common defines for the control library.
//

// DLL defines:
#ifdef CONTROLS_IMPORT
#define CONTROLS_DLL __declspec(dllimport)
#endif
#ifdef CONTROLS_EXPORT
#define CONTROLS_DLL __declspec(dllexport)
#endif
#ifndef CONTROLS_DLL
#define CONTROLS_DLL
#endif

#endif // CONTROLS_DEFS_HPP
