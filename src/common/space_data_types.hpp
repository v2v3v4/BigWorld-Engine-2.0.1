/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_DATA_TYPES_HPP
#define SPACE_DATA_TYPES_HPP

#include "cstdmf/stdmf.hpp"


const uint16 SPACE_DATA_TOD_KEY = 0;
/**
 *	This structure is used to send time-of-day data via the SpaceData mechanism.
 */
struct SpaceData_ToDData
{
	float	initialTimeOfDay;
	float	gameSecondsPerSecond;
};


/// This constant is the index for SpaceData concerning what geometry data is
/// mapped into the space. It influences both the client and the server.
const uint16 SPACE_DATA_MAPPING_KEY_CLIENT_SERVER = 1;

/// This constant is the index for SpaceData concerning what geometry data is
/// mapped into the space. It only influences the client. The server will not
/// load this geometry.
const uint16 SPACE_DATA_MAPPING_KEY_CLIENT_ONLY = 2;

/**
 *	This structure is used to send information about what geometry data is
 *	mapped into a space and where it is mapped.
 */
struct SpaceData_MappingData
{
	float	matrix[4][4];	// like this since unaligned
	//char	path[];
};


const uint16 SPACE_DATA_FIRST_USER_KEY = 256;
const uint16 SPACE_DATA_FINAL_USER_KEY = 32511;

const uint16 SPACE_DATA_FIRST_CELL_ONLY_KEY = 16384;


#endif // SPACE_DATA_TYPES_HPP
