/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __TERRAIN_PCH_HPP__
#define __TERRAIN_PCH_HPP__

#ifdef _WIN32

// Standard headers
#include <vector>
#include <string>
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"


// Math Headers
#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "math/matrix.hpp"

// Moo Headers
#ifndef MF_SERVER
#	include "moo/com_object_wrap.hpp"
#	include "moo/effect_material.hpp"
#	include "moo/managed_texture.hpp"
#	include "moo/material.hpp"
#	include "moo/moo_dx.hpp"
#	include "moo/render_context.hpp"
#endif // MF_SERVER
#endif // _WIN32

#endif // __TERRAIN_PCH_HPP__
