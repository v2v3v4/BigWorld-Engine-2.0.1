/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POST_PROCESSING_PCH_HPP
#define POST_PROCESSING_PCH_HPP

#ifdef _WIN32
#ifndef MF_SERVER

#include "cstdmf/pch.hpp"
#include "resmgr/datasection.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "math/matrix.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/stl_to_py.hpp"
#include "pyscript/script_math.hpp"

#endif // ndef MF_SERVER
#endif // _WIN32

#endif // POST_PROCESSING_PCH_HPP
