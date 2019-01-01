/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_VIEWER_CONNECTION_HPP
#define CELL_VIEWER_CONNECTION_HPP

#include "network/endpoint.hpp"
#include "network/interfaces.hpp"

namespace Mercury
{
class EventDispatcher;
}

namespace CellViewerExport
{
	// protocol uses printable ascii letters for easy debugging.

	enum RequestTypes
	{
		GET_ENTITY_TYPE_NAMES = 'n',
		GET_ENTITIES = 'b',
		GET_GRID = 'e'
	};

	enum EntitySelect
	{
		REAL = '0',
		GHOST = '1'
	};

};

class Cell;
class CellApp;


/**
 *	This class is responsible for communicating with the Cell Viewer
 *	application.
 */
class CellViewerConnection : public Mercury::InputNotificationHandler
{
public:
	CellViewerConnection( Mercury::EventDispatcher & dispatcher,
			int fd, const CellApp & cellApp );
	virtual ~CellViewerConnection();
	void sendReply( const void * buf, uint32 len );
	void sendReply_brief( const void * buf, uint32 len );
	template <typename A> void receiveData( A & buf );

private:
	virtual int handleInputNotification( int fd );

	Mercury::EventDispatcher * pDispatcher_;
	Endpoint socket_;
	const CellApp & cellApp_;
};

#endif // CELL_VIEWER_CONNECTION_HPP
