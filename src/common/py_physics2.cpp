/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

// This file contains code that uses both the pyscript and physics2 libraries
// and is common to many processes. This is not big enough to be its own
// library.

// BW Tech Headers
#include "physics2/material_kinds.hpp"

#include "pyscript/py_data_section.hpp"
#include "pyscript/script.hpp"


int PyPhysics2_token = 0;

/*~ function BigWorld.getMaterialKinds
 *	@components{ client, cell }
 *
 *	This method returns a list of id and section tuples of the
 *	material kind list, in the order in which they appear in the XML file
 *
 *	@return		The list of (id, DataSection) tuples of material kinds
 */
/**
 *	This method returns a list of id and section tuples of material kinds.
 */

namespace // anonymous
{

PyObject* getMaterialKinds()
{
	const MaterialKinds::Map& kinds = MaterialKinds::instance().materialKinds();

	int size = kinds.size();
	PyObject* pList = PyList_New( size );

	MaterialKinds::Map::const_iterator iter = kinds.begin();
	int i = 0;

	while (iter != kinds.end())
	{
		PyObject * pTuple = PyTuple_New( 2 );

		PyTuple_SetItem( pTuple, 0, PyInt_FromLong( iter->first ) );
		PyTuple_SetItem( pTuple, 1, new PyDataSection( iter->second ) );

		PyList_SetItem( pList, i, pTuple );

		i++;
		++iter;
	}

	return pList;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, getMaterialKinds, END, BigWorld )

} // namespace (anonymous)

// py_physics2.cpp
