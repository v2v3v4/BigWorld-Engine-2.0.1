/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ROMP_PCH_HPP
#define ROMP_PCH_HPP

#ifdef _WIN32
#ifndef MF_SERVER

#include "cstdmf/pch.hpp"
#include "resmgr/datasection.hpp"

#include "math/vector2.hpp"
#include "math/matrix.hpp"
#include "math/quat.hpp"

#include "moo/vertex_formats.hpp"
#include "moo/vertex_buffer.hpp"
#include "moo/index_buffer.hpp"
#include "moo/dynamic_index_buffer.hpp"
#include "moo/dynamic_vertex_buffer.hpp"
#include "moo/effect_material.hpp"

#endif // ndef MF_SERVER
#endif // _WIN32

#endif // ROMP_PCH_HPP
