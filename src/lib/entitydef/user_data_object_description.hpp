/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 *	This file provides the implementation of the UserDataObjectDescription class.
 */
#ifndef USER_DATA_OBJECT_DESCRIPTION_HPP
#define USER_DATA_OBJECT_DESCRIPTION_HPP
#include "Python.h"	// Included in data_description.hpp and
					// method_description.hpp but should come before system
#include <vector>
#include <float.h>

#include "data_description.hpp"
#include "method_description.hpp"
#include "resmgr/datasection.hpp"
#include "base_user_data_object_description.hpp"

class MD5;

class AddToStreamVisitor;
/**
 *	This class is used to describe a type of User Data Object. It describes all properties of
 *	the data object. It is normally created on startup when the user_data_objects.xml file is parsed.
 */
class UserDataObjectDescription: public BaseUserDataObjectDescription
{
public:
	UserDataObjectDescription():
		domain_(NONE)
	{}

	/* A user data object only lives on one of the base, cell or client.
	   Accordingly any DATA_OBJECT links on entities should have the appropriate flag set
	   so that it can be read in the appropriate domain. */

	enum UserDataObjectDomain{
		NONE = 0x0,
		BASE = 0x1,
		CELL = 0x2,
		CLIENT =0x4,
	};
	const UserDataObjectDomain domain() const{ return domain_;	}

protected:
	bool parseProperties( DataSectionPtr pProperties );
	bool parseInterface( DataSectionPtr pSection, const char * interfaceName );

	const std::string	getDefsDir() const;
	const std::string	getClientDir() const;
	const std::string	getCellDir() const;
	const std::string	getBaseDir() const;

	/* This will be specified in every UserDataObject def file
	 * And interpreted by the chunkloading sequence 
	 * for each domain to determine if it should load a given user data object
	 * TODO: DEFAULT TO 0, and print error 
	 */
	UserDataObjectDomain domain_;
};

#endif // UserDataObject_DESCRIPTION_HPP
