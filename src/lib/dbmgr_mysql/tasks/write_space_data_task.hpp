/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WRITE_SPACE_DATA_TASK_HPP
#define WRITE_SPACE_DATA_TASK_HPP

#include "background_task.hpp"

#include "cstdmf/memory_stream.hpp"
#include "network/basictypes.hpp"

class BinaryIStream;
class MySql;

/**
 *	This class encapsulates the MySqlDatabase::writeSpaceData() operation
 *	so that it can be executed in a separate thread.
 */
class WriteSpaceDataTask : public MySqlBackgroundTask
{
public:
	WriteSpaceDataTask( BinaryIStream & spaceData );


	// MySqlBackgroundTask overrides
	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

protected:
	virtual void onRetry();

private:
	uint32 writeSpaceDataStreamToDB( MySql & connection,
		BinaryIStream & stream );

	MemoryOStream	data_;
	uint32			numSpaces_;
};

#endif // WRITE_SPACE_DATA_TASK_HPP
