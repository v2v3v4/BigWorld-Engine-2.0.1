/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_RESOURCE_REFS_HPP
#define PY_RESOURCE_REFS_HPP

#pragma warning( disable:4786 )

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

class ResourceRef;
typedef std::vector<ResourceRef> ResourceRefs;

/*~ class BigWorld.PyResourceRefs
 *	@components{ client, tools }
 *
 *	This class is a resource reference holder.  It cannot be constructed
 *	explicitly, instead you must call BigWorld.loadResourceListBG - the
 *	ResourceRefs object is passed into the callback function.
 *	This class holds references to those resources it is initialised with,
 *	so as long as you hold a reference to the PyResourceRefs object, you will
 *	also be holding onto references to all of its resources.
 *
 *	You can retrieve the resource list via the resourceIDs attribute.
 *	You should check if any resources in the resourceIDs list is in the
 *	failedIDs list before constructing objects from them.
 */
/**
 *	This class is a resource reference holder.  It cannot be constructed
 *	explicitly, instead you must call BigWorld.loadResourceListBG; the
 *	ResourceRefs object is passed into the callback function.
 *	This class holds references to those resources it is initialised with,
 *	so as long as you hold a reference to the ResoureRefs object, you will
 *	also be holding onto references to all of its resources.
 *
 *	You can retrieve the resource list via the resourceIDs attribute.
 *	You should check if any resources in the resourceIDs list is in the
 *	failedIDs list before constructing objects from them.
 */
class PyResourceRefs : public PyObjectPlus
{
	Py_Header( PyResourceRefs, PyObjectPlus )

public:	
	PyResourceRefs(
		PyObject * pResourceIDs,
		PyObject * pCallbackFn,
		int priority,
		PyTypePlus * pType = &s_type_ );

	PyResourceRefs( const ResourceRefs& rr, PyTypePlus * pType = &s_type_ );	

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );	

	PY_METHOD_DECLARE( py_pop );
	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )

	PyObject * 			subscript( PyObject * entityID );
	int					length();
	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static int			s_length( PyObject * self );

	void				addLoadedResourceRef( ResourceRef& r );
	void				addFailedResourceID( const std::string& resourceID );	
	
	PY_FACTORY_DECLARE()	

protected:	
	ResourceRefs		resourceRefs_;	
	std::vector<std::string> pFailedIDs_;

private:
	PyObject *			pyResource( const std::string& resourceID );
	ResourceRef*		refByName( const std::string& name );
	void				erase( const std::string& name );
};

PY_SCRIPT_CONVERTERS_DECLARE( PyResourceRefs )

#endif // PY_RESOURCE_REFS_HPP
