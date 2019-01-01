/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WRITE_TO_DB_REPLY_HPP
#define WRITE_TO_DB_REPLY_HPP

#include "cstdmf/smartpointer.hpp"

/**
 *	This class handles a reply from the DBMgr.
 */
class WriteToDBReplyHandler
{
public:
	virtual ~WriteToDBReplyHandler() {};
	virtual void onWriteToDBComplete( bool succeeded ) = 0;
};


/**
 *	This class calls the reply handler once the entity has been written
 *	to disk, and optionally, backed up to it's backup baseapp.
 */
class WriteToDBReplyStruct : public ReferenceCount
{
public:
	WriteToDBReplyStruct( WriteToDBReplyHandler * pHandler );
	bool expectsReply()	const { return pHandler_ != NULL; }
	void onWriteToDBComplete( bool succeeded );
	void onBackupComplete();

private:
	bool succeeded_;
	bool backedUp_;
	bool writtenToDB_;
	WriteToDBReplyHandler * pHandler_;
};

typedef SmartPointer<WriteToDBReplyStruct> WriteToDBReplyStructPtr;

#endif // WRITE_TO_DB_REPLY_HPP
