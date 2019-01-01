/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "type_traits.hpp"

// -----------------------------------------------------------------------------
// Section: class MySqlTypeTraits
// -----------------------------------------------------------------------------

const std::string MySqlTypeTraits<std::string>::TINYBLOB( "TINYBLOB" );
const std::string MySqlTypeTraits<std::string>::BLOB( "BLOB" );
const std::string MySqlTypeTraits<std::string>::MEDIUMBLOB( "MEDIUMBLOB" );
const std::string MySqlTypeTraits<std::string>::LONGBLOB( "LONGBLOB" );

// type_traits.cpp
