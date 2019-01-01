/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CUSTOM_HPP
#define CUSTOM_HPP

#include "network/basictypes.hpp"
#include "resmgr/datasection.hpp"

#include <string>

// this file contains hook functions you can use to customise the
// database system

// if an entity doesn't exist when logging on, an implementation of
// IDatabase should call this function and add the result into the
// database; if this function returns NULL, then the IDatabase
// implementation should fail gracefully
// (function in IDatabase concerned is readOrCreateEntity)
DataSectionPtr createNewEntity( EntityTypeID, const std::string & name );

#endif // CUSTOM_HPP
