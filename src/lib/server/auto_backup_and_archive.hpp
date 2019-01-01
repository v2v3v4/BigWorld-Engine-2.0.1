/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef AUTO_BACKUP_AND_ARCHIVE_HPP
#define AUTO_BACKUP_AND_ARCHIVE_HPP

#include "pyscript/script.hpp"

namespace AutoBackupAndArchive
{

/**
 *	Enum used for setting auto backup or auto archive settings.
 */
enum Policy
{
	NO = 0,
	YES = 1,
	NEXT_ONLY = 2,
};



void addNextOnlyConstant( PyObject * pModule );


} // end namespace AutoBackupAndArchive

namespace Script
{

int setData( PyObject * pObj,
		AutoBackupAndArchive::Policy & value, 
		const char * varName = "" );

} // end namespace Script

#endif // AUTO_BACKUP_AND_ARCHIVE_HPP
