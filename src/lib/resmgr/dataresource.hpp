/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_RESOURCE_HPP
#define DATA_RESOURCE_HPP

// Standard Library Headers.
#include <stdio.h>
#include <string>

// Standard MF Library Headers.
#include "cstdmf/smartpointer.hpp"

// Application Specific Headers.
#include "datasection.hpp"


/**
 *	An abstract class that defines the common interface for the opening and
 *	parsing of a resource file. It can be treated as a factory that reads in
 *	the file and produces handles to accessing the various section of the
 *	file parsed.
 */
class DataHandle : public ReferenceCount
{
public:
	/// @name Smart Pointers for DataHandle.
	//@{
	/// DataHandlePtr can be treated as a normal pointer.
	typedef SmartPointer<DataHandle> DataHandlePtr;

	/// ConstDataHandlePtr can be treated as a const pointer.
	typedef ConstSmartPointer<DataHandle> ConstDataHandlePtr;
	//@}


	///	@name DataHandle Error States.
	//@{
	enum Error
	{
		DHE_NoError, DHE_LoadFailed, DHE_SaveFailed,
		XML_InitialisationFailed,
		XML_SAXParseWarning, XML_SAXParseError, XML_SAXParseFatalError,
		XML_IllegalPath, XML_ChangedURL,
		XML_UnknownError
	};
	//@}


	///	@name Constructors/Destructor for DataHandle.
	//@{
	/// This constructor simply initialises the error state variables and
	///	stores the file-name for refresh.
	DataHandle( const std::string &fileName = "" );
	virtual ~DataHandle() {};
	//@}


	///	@name Interface Methods to the DataResource.
	//@{
	/// This method returns a pointer to a DataSection representing the top of
	///	the parse tree for the resource.
	virtual DataSectionPtr getRootSection( void ) = 0;

	/// This method will reload the resource from the path specified.
	virtual DataHandle::Error reload( void ) = 0;

	/// This method will save the resource to the path specified.
	virtual DataHandle::Error save( const std::string &fileName ) = 0;

	/// This method returns the file-name.
	const std::string &fileName( void );
	//@}


	///	@name Methods associated with error handling and debugging.
	//@{
	/// This method is used to quietly obtain some user-meaningful text
	///	describing the error that has occurred.
	const std::string &errorMessage( void );

	/// This method is used to determine if the last operation resulted in
	/// success or a failure.
	DataHandle::Error errorState( void );
	//@}

protected:
	///	@name Debug Data.
	//@{
	DataHandle::Error lastError_;
	std::string lastErrorMessage_;
	//@}

	/// File name.
	std::string fileName_;
};


/// Identifies the type of the DataResource.
enum DataResourceType
{
	RESOURCE_TYPE_XML
};


/**
 *	A class that acts an abstract file resource. It uses the proxy-pattern
 *	with a smart pointer to DataHandle being polymorphic depending on the
 *	file type used.
 */
class DataResource : public ReferenceCount
{
public:
	/// @name Smart Pointers for DataResource.
	//@{
	/// DataResourcePtr can be treated as a normal pointer.
	typedef SmartPointer<DataResource> DataResourcePtr;

	/// ConstDataResourcePtr can be treated as a const pointer.
	typedef ConstSmartPointer<DataResource> ConstDataResourcePtr;
	//@}


	///	@name Constructor(s) for DataResource.
	//@{
	DataResource( void );
	DataResource( const std::string &fileName );
	DataResource( const std::string &fileName,
			DataResourceType type, bool createIfInvalid = false );
	//@}


	///	@name Interface Methods to the DataResource.
	//@{
	/// This method returns a pointer to a DataSection representing the top of
	///	the parse tree for the resource.
	DataSectionPtr getRootSection( void );

	///	This method will load the resource from the path specified.
	DataHandle::Error load( const std::string &fileName,
                            DataResourceType type = RESOURCE_TYPE_XML );

	/// This method will reload the resource from the path specified.
	DataHandle::Error reload( void );

	/// This method will save the resource to the path specified.
	DataHandle::Error save( const std::string &fileName = "" );

	/// This method returns the file-name.
	const std::string &fileName( void );

	///	This method returns true if the DataResource has been loaded.
	bool loaded( void );
	//@}


	///	@name Methods associated with error handling and debugging.
	//@{
	/// This method is used to quietly obtain some user-meaningful text
	///	describing the error that has occurred.
	const std::string &errorMessage( void );

	/// This method is used to determine if the last operation resulted in
	/// success or a failure.
	DataHandle::Error errorState( void );
	//@}


private:
	///	Pointer to the polymorphic DataHandle.
	DataHandle::DataHandlePtr pResource_;

    void ensureFileExists( const std::string& fileName );
};

#endif

/* DataResource.hpp */
