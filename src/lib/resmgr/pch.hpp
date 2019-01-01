/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESMGR_PCH_HPP
#define RESMGR_PCH_HPP

#ifdef _WIN32

#include "cstdmf/pch.hpp"

#ifndef MF_SERVER

#include "bin_section.hpp"
#include "binary_block.hpp"
#include "bwresource.hpp"
#include "dataresource.hpp"
#include "datasection.hpp"
#include "dir_section.hpp"
#include "access_monitor.hpp"
#include "sanitise_helper.hpp"

#endif // MF_SERVER

#endif // _WIN32

#endif // RESMGR_PCH_HPP
