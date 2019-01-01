/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UMBRA_CONFIG_HPP
#define UMBRA_CONFIG_HPP

/*
 * Umbra configuration, to turn Umbra off, change UMBRA_ENABLE to 0
 * The umbra library to link to is defined in chunk_umbra.cpp
 */
#if defined(WIN32) && !defined(MF_SERVER) && !defined(INDIE)
#define UMBRA_ENABLE 0
#else
#define UMBRA_ENABLE 0
#endif

#endif

/* umbra_config.hpp */
