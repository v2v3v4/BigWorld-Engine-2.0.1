/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITYDEF_PCH_HPP
#define ENTITYDEF_PCH_HPP

#ifdef _WIN32

#include "cstdmf/pch.hpp"

#ifndef MF_SERVER

#include "data_description.hpp"
#include "entity_description.hpp"
#include "entity_description_map.hpp"
#include "method_description.hpp"
#include "volatile_info.hpp"
#include "py_volatile_info.hpp"

#endif // ndef MF_SERVER
#endif // _WIN32

#endif // ENTITYDEF_PCH_HPP
