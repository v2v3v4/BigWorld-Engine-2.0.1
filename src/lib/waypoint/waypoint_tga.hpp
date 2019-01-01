/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _WAYPOINT_TGA_HEADER
#define _WAYPOINT_TGA_HEADER

#ifdef _WIN32
#define PACKED
#pragma pack(1)
#else
#define PACKED __attribute__((packed))
#endif

struct WaypointTGAHeader
{
	unsigned char   extraLength 		PACKED;
	unsigned char   colourMapType		PACKED;
	unsigned char   imageType			PACKED;
	unsigned short  colourMapStart		PACKED;
	unsigned short  colourMapLength		PACKED;
	unsigned char   colourMapDepth		PACKED;
	unsigned short  x					PACKED;
	unsigned short  y					PACKED;
	unsigned short  width				PACKED;
	unsigned short  height				PACKED;
	unsigned char   bpp					PACKED;
	unsigned char   imageDescriptor		PACKED;

	// These fields stored in the 'extra data' part of the header.

	Vector3			gridMin				PACKED;
	float			gridResolution		PACKED;
};

#endif
