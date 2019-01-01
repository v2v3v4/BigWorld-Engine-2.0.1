/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_CONFIG_LITE_HPP
#define SPEEDTREE_CONFIG_LITE_HPP

/**
 *	To enable support for speedtree in BigWorld, define SPEEDTREE_SUPPORT as 1. 
 *	To disable the feature, define it as 0. When speedtree suport is disabled, 
 *	objects and classes from src/lib/speedtree can still be used, but they will
 *	have empty implementation. To avoid interrupting the execution of its client
 *	code, it will not generate errors, trigger asserts or throw exceptions, but
 *	it will print warnings to the debug output.
 *
 *	To build BigWorld with support for SpeedTree, you will need SpeedTreeRT SDK 
 *	4.2 or later installed and a current license. For detailed instructions on how 
 *	to setup SpeedTreeRT SDK, please refer to client_and_tools_build_instructions.html 
 *	in (bigworld/doc).
 */

// For now, disable speedtree 
// on linux (server) by default
#if defined(WIN32) && !defined(INDIE) && defined(_DLL)
#define SPEEDTREE_SUPPORT 0
#else
#define SPEEDTREE_SUPPORT 0
#endif

// Sset SPT_ENABLE_NORMAL_MAPS to 0 
// disable normal maps. Don't forget 
// to do the same in speedtree.fxh
#define SPT_ENABLE_NORMAL_MAPS 1

// set ENABLE_BB_OPTIMISER to 0 to disable
// the billboard optimiser from the build
#define ENABLE_BB_OPTIMISER 1

// Set ENABLE_MATRIX_OPT to 0 to disable
// the matrix array optimisaton. Don't
// forget to do the same in speedtree.fxh
#define ENABLE_MATRIX_OPT 1

// Define the current version of the .ctree files.
#define SPT_CTREE_VERSION		103

#define OPT_BUFFERS

#endif // SPEEDTREE_CONFIG_LITE_HPP
