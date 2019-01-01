/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCE_TABLE_HPP
#define RESOURCE_TABLE_HPP

#include "pyobject_plus.hpp"
#include "script.hpp"

#include <set>


class ResourceTable;
typedef SmartPointer<ResourceTable> ResourceTablePtr;
class DataSection;
typedef SmartPointer<DataSection> DataSectionPtr;

/**
 *	This class is a table for scripts to use as a resource.
 */
class ResourceTable : public PyObjectPlus
{
	Py_Header( ResourceTable, PyObjectPlus )

public:
	ResourceTable( DataSectionPtr pSect, ResourceTablePtr pParent,
		PyTypePlus * pType = &s_type_ );
	~ResourceTable();

	static PyObject * New( const std::string & resourceID,
		PyObjectPtr updateFn );
	PY_AUTO_FACTORY_DECLARE( ResourceTable,
		ARG( std::string, OPTARG( PyObjectPtr, NULL, END ) ) )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	bool link( PyObjectPtr updateFn, bool callNow = true );
	PY_AUTO_METHOD_DECLARE( RETOK, link, NZARG( PyObjectPtr, END ) )
	void unlink( PyObjectPtr updateFn );
	PY_AUTO_METHOD_DECLARE( RETVOID, unlink, NZARG( PyObjectPtr, END ) )

	PY_RO_ATTRIBUTE_DECLARE( pValue_, value )

	PY_SIZE_INQUIRY_METHOD( pyMap_length )
	PY_BINARY_FUNC_METHOD( pyMap_subscript )

	PyObject * at( int index );
	PY_AUTO_METHOD_DECLARE( RETOWN, at, ARG( int, END ) )
	PyObject * sub( const std::string & key );
	PY_AUTO_METHOD_DECLARE( RETOWN, sub, ARG( std::string, END ) )

	std::string keyOfIndex( int index );
	PY_AUTO_METHOD_DECLARE( RETDATA, keyOfIndex, ARG( int, END ) )
	int indexOfKey( const std::string & key );
	PY_AUTO_METHOD_DECLARE( RETDATA, indexOfKey, ARG( std::string, END ) )

	PY_RO_ATTRIBUTE_DECLARE( index_, index );
	std::string key();
	PY_RO_ATTRIBUTE_DECLARE( this->key(), key );


private:
	ResourceTable( const ResourceTable& );
	ResourceTable& operator=( const ResourceTable& );

	DataSectionPtr		pSect_;
	ResourceTablePtr	pParent_;
	uint				index_;

	PyObjectPtr			pValue_;

	std::vector<ResourceTablePtr>	keyedEntries_;

	typedef std::set<PyObjectPtr> LinkSet;
	LinkSet				links_;

	static PyMappingMethods s_map_methods_;

	friend struct RTSCompare;
};

PY_SCRIPT_CONVERTERS_DECLARE( ResourceTable )


#endif // RESOURCE_TABLE_HPP
