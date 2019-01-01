/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MIGRATE_MAILBOX_VISITOR_HPP
#define MIGRATE_MAILBOX_VISITOR_HPP

#include "entitydef/mailbox_base.hpp"

class MigrateMailBoxVisitor : public PyEntityMailBoxVisitor
{
public:
	MigrateMailBoxVisitor() {}
	virtual ~MigrateMailBoxVisitor() {}

	virtual void onMailBox( PyEntityMailBox * pMailBox )
	{
		pMailBox->migrate();
	}
};

#endif // MIGRATE_MAILBOX_VISITOR_HPP
