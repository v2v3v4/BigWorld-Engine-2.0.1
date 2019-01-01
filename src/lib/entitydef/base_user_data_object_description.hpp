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
 *	This file provides the implementation of the base class from which
 *  entities defined with a def derived.
 *
 * 	@ingroup udo
 */

#ifndef BASE_USER_DATA_OBJECT_DESCRIPTION_HPP
#define BASE_USER_DATA_OBJECT_DESCRIPTION_HPP

#include "Python.h"	// Included in data_description.hpp and
					// method_description.hpp but should come before system
					// includes

#include <string>
#include <vector>
#include <float.h>

#include "data_description.hpp"
#include "method_description.hpp"
#include "network/basictypes.hpp"
#include "resmgr/datasection.hpp"

class MD5;

class AddToStreamVisitor;
/**
 *	This class is used to describe a type of User Data Object. It describes all properties
 *	a chunk item. It is normally created on startup when the user data objects.xml file is parsed.
 *
 * 	@ingroup udo
 */
class BaseUserDataObjectDescription
{
public:
	BaseUserDataObjectDescription();
	virtual ~BaseUserDataObjectDescription();

	virtual bool	parse( const std::string & name,
				DataSectionPtr pSection = NULL, bool isFinal = true );
	
	void addToDictionary( DataSectionPtr pSection, PyObject* pObject ) const;

	const std::string&		name() const;


	unsigned int			propertyCount() const;
	DataDescription*		property( unsigned int n ) const;
	virtual DataDescription*	findProperty( const std::string& name ) const;

protected:
	
	virtual	bool			parseProperties( DataSectionPtr pProperties ) = 0;
	virtual bool			parseInterface( DataSectionPtr pSection,
											const char * interfaceName );
	virtual bool			parseImplements( DataSectionPtr pInterfaces );

	virtual const std::string getDefsDir() const = 0;
	virtual const std::string getClientDir() const = 0;
	virtual const std::string getCellDir() const = 0;
	virtual const std::string getBaseDir() const = 0;

	std::string			name_;

	typedef std::vector< DataDescription >		Properties;
	Properties 			properties_;

	typedef std::map< std::string, unsigned int > PropertyMap;
	PropertyMap			propertyMap_;

	#ifdef EDITOR_ENABLED
	std::string			editorModel_;
#endif

};

#ifdef CODE_INLINE
#include "base_user_data_object_description.ipp"
#endif

#endif // BASE_USER_DATA_OBJECT_DESCRIPTION_HPP
