/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ASSETPROCESSOR_HPP
#define ASSETPROCESSOR_HPP

#if defined( ASSETPROCESSOR_EXPORTS )
#define ASSETPROCESSOR_API __declspec(dllexport)
#elif defined( ASSETPROCESSOR_IMPORTS )
#define ASSETPROCESSOR_API __declspec(dllimport)
#else
#pragma message( "ASSETPROCESSOR_IMPORTS or ASSETPROCESSOR_EXPORTS should be defined" )
#endif

/**
 *	This dll exposes assetprocessor to python, via a simport import command.
 */

extern "C"
{

	extern ASSETPROCESSOR_API void init_AssetProcessor();

}

#endif