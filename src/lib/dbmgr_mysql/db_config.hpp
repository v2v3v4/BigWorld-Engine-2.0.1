/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATABASE_CONFIG_HPP
#define DATABASE_CONFIG_HPP

#include "connection_info.hpp"

#include "cstdmf/stdmf.hpp"

namespace DBConfig
{

const ConnectionInfo & connectionInfo();

}	// namespace DBConfig

#endif // DATABASE_CONFIG_HPP
