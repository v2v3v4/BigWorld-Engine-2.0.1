/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_PCH_HPP
#define MOO_PCH_HPP

#ifdef _WIN32
#ifndef MF_SERVER
#include "cstdmf/pch.hpp"
#include "resmgr/datasection.hpp"
#include "resmgr/bwresource.hpp"

#include "moo_dx.hpp"
#include "moo_math.hpp"
#include "moo/forward_declarations.hpp"
#include "vertex_formats.hpp"
#include "vertex_buffer.hpp"
#include "index_buffer.hpp"
#include "dynamic_index_buffer.hpp"
#include "dynamic_vertex_buffer.hpp"

#endif // MF_SERVER
#endif // _WIN32

#endif // MOO_PCH_HPP
