/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CSTDMF_PCH_HPP
#define CSTDMF_PCH_HPP

#ifdef _WIN32

// identifier was truncated to '255' characters in the browser information
#pragma warning(disable: 4786)

// class name too long
#pragma warning(disable: 4503)

// no matching delete operator for placement new operators, which can't have
// one in any case. applies to the Aligned class
#pragma warning(disable: 4291)

#include "aligned.hpp"
#include "binary_stream.hpp"
#include "binaryfile.hpp"
#include "config.hpp"
#include "bw_util.hpp"
#include "debug.hpp"
#include "dogwatch.hpp"
#include "dprintf.hpp"
#include "guard.hpp"
#include "intrusive_object.hpp"
#include "md5.hpp"
#include "memory_counter.hpp"
#include "memory_stream.hpp"
#include "profile.hpp"
#include "resource_counters.hpp"
#include "smartpointer.hpp"
#include "stdmf.hpp"
#include "stringmap.hpp"
#include "timestamp.hpp"
#include "vectornodest.hpp"
#include "watcher.hpp"
#include "watcher_path_request.hpp"

#endif // _WIN32

#endif // CSTDMF_PCH_HPP
