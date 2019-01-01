/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_DATABASE_CREATION_HPP
#define MYSQL_DATABASE_CREATION_HPP

class IDatabase;

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}


IDatabase * createMySqlDatabase(
		Mercury::NetworkInterface & interface,
		Mercury::EventDispatcher & dispatcher );

#endif // MYSQL_DATABASE_CREATION_HPP
