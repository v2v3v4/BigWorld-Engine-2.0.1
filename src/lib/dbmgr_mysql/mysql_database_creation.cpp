/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "mysql_database_creation.hpp"

#include "mysql_database.hpp"

IDatabase * createMySqlDatabase(
		Mercury::NetworkInterface & interface,
		Mercury::EventDispatcher & dispatcher )
{
	try
	{
		MySqlDatabase * pDatabase = new MySqlDatabase( interface, dispatcher );

		return pDatabase;
	}
	catch (std::exception & e)
	{
		return NULL;
	}
}

// mysql_database_creation.cpp
