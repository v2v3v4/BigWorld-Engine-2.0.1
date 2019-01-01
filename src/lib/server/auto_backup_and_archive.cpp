/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "auto_backup_and_archive.hpp"


namespace Script
{

int setData( PyObject * pObj,
		AutoBackupAndArchive::Policy & value, 
		const char * varName )
{
	int intVal = int(value);

	int result = Script::setData( pObj, intVal, varName );

	if ((result == 0) &&
			(0 <= intVal) && (intVal <= 2))
	{
		value = AutoBackupAndArchive::Policy( intVal );
		return 0;
	}
	else
	{
		PyErr_Format( PyExc_ValueError,
				"%s must be set to an integer between 0 and 2", varName );
		return -1;
	}
}

} // namespace Script

void AutoBackupAndArchive::addNextOnlyConstant( PyObject * pModule )
{

	PyObjectPtr pNextOnly( Script::getData( AutoBackupAndArchive::NEXT_ONLY ),
		PyObjectPtr::STEAL_REFERENCE );
	PyObject_SetAttrString( pModule, "NEXT_ONLY", pNextOnly.get() );
}

// auto_backup_and_archive.cpp
