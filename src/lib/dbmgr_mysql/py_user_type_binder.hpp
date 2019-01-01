/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_PY_USER_TYPE_BINDER_HPP
#define MYSQL_PY_USER_TYPE_BINDER_HPP

#include "namer.hpp"

#include "cstdmf/smartpointer.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <stack>

class CompositePropertyMapping;
class DataSection;
class PropertyMapping;

typedef SmartPointer< DataSection > DataSectionPtr;
typedef SmartPointer< PropertyMapping > PropertyMappingPtr;
typedef SmartPointer< CompositePropertyMapping > CompositePropertyMappingPtr;


/**
 *	This class allows scripts to specify how a DataSection should be bound to
 *	sql tables it then allows createUserTypeMapping to pull out a
 *	PropertyMappingPtr to apply this.
 */
class PyUserTypeBinder : public PyObjectPlus
{
	Py_Header( PyUserTypeBinder, PyObjectPlus )

public:
	PyUserTypeBinder( const Namer & namer, const std::string & propName,
		DataSectionPtr pDefaultValue, PyTypePlus * pType = &s_type_ );

	void beginTable( const std::string & propName );
	bool bind( const std::string & propName, const std::string & typeName,
			int databaseLength );
	bool endTable();


	// this method lets createUserTypeMapping figure out its return value
	PropertyMappingPtr getResult();

	PyObject * pyGetAttribute( const char * attr );

private:
	PY_AUTO_METHOD_DECLARE( RETVOID, beginTable, ARG( std::string, END ) );
	PY_AUTO_METHOD_DECLARE( RETOK, endTable, END );
	PY_AUTO_METHOD_DECLARE( RETOK, bind,
			ARG( std::string, ARG( std::string, OPTARG( int, 255, END ) ) ) );

	struct Context
	{
		CompositePropertyMappingPtr	pCompositeProp;
		Namer						namer;
		DataSectionPtr				pDefaultValue;

		Context( CompositePropertyMappingPtr pProp,
				const Namer & inNamer,
				const std::string & propName, bool isTable,
				DataSectionPtr pDefault ) :
			pCompositeProp(pProp),
			namer( inNamer, propName, isTable ),
			pDefaultValue( pDefault )
		{}

		Context( CompositePropertyMappingPtr pProp,
				const Namer & inNamer,
				DataSectionPtr pDefault ) :
			pCompositeProp( pProp ),
			namer( inNamer ),
			pDefaultValue( pDefault )
		{}
	};

	std::stack< Context > tables_;

	const Context & curContext() const;
};


typedef SmartPointer< PyUserTypeBinder > PyUserTypeBinderPtr;

#endif // MYSQL_PY_USER_TYPE_BINDER_HPP
