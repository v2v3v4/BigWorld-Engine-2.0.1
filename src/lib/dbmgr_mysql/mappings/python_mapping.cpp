/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "python_mapping.hpp"

#include "blob_mapping.hpp"

#include "cstdmf/binary_stream.hpp"
#include "entitydef/data_types.hpp"
#include "pyscript/pickler.hpp"
#include "resmgr/datasection.hpp"


/**
 * Constructor.
 */
PythonMapping::PythonMapping( const Namer & namer, const std::string & propName,
		bool isNameIndex, int length, DataSectionPtr pDefaultValue ) :
	StringLikeMapping( namer, propName, isNameIndex, length )
{
	if (pDefaultValue)
	{
		defaultValue_ = pDefaultValue->as< std::string >();
	}

	if (defaultValue_.empty())
	{
		defaultValue_ = PythonMapping::getPickler().pickle( Py_None );
	}
	else if (PythonDataType_IsExpression( defaultValue_ ))
	{
		PythonMapping::pickleExpression( defaultValue_, defaultValue_ );

		if (defaultValue_.empty())
		{
			CRITICAL_MSG( "PythonMapping::PythonMapping: "
					"the default value evaluated expression for %s "
					"could not be pickled",
				propName.c_str() );
		}
	}
	else
	{
		BlobMapping::decodeSection( defaultValue_, pDefaultValue );
	}

	if (defaultValue_.length() > charLength_)
	{
		ERROR_MSG( "PythonMapping::PythonMapping: "
					"Default value for property '%s' is too big to fit inside "
					"column. Defaulting to None\n",
				propName.c_str() );

		defaultValue_ = PythonMapping::getPickler().pickle( Py_None );

		if (defaultValue_.length() > charLength_)
		{
			CRITICAL_MSG( "PythonMapping::PythonMapping: "
						"Even None cannot fit inside column. Please "
						"increase DatabaseSize of property '%s'\n",
					propName.c_str() );
		}
	}
}


/**
 *	This method evaluates expr as a Python expression, pickles the
 *	resulting object and stores it in output.
 *
 *	@param output The string where the pickled data will be placed.
 *	@param expr   The expression to pickle.
 */
void PythonMapping::pickleExpression( std::string & output,
		const std::string & expr )
{
	PyObjectPtr pResult( Script::runString( expr.c_str(), false ),
			PyObjectPtr::STEAL_REFERENCE );

	PyObject * 	pToBePickled = pResult ? pResult.getObject() : Py_None;

	output = PythonMapping::getPickler().pickle( pToBePickled );
}


/**
 *	This static method returns the Pickler object to be used for pickling.
 *
 *	@returns The static Pickler object.
 */
Pickler & PythonMapping::getPickler()
{
	static Pickler pickler;

	return pickler;
}

// python_mapping.cpp
